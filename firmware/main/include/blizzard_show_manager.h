/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Show Manager -> blizzard_show_manager.h
 *
 */

#ifndef BLIZZARD_SHOW_MANAGER_H
#define BLIZZARD_SHOW_MANAGER_H

#include<inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BPF_ERROR_NAME "ERROR"
#define BPF_ERROR_CODE 0xFFFFFFFF
#define BPF_EMPTY_NAME "EMPTY"
#define BPF_EMPTY_CODE 0x00000000

#define BPF_HEAD_MAGIC {'B', 'l', 'i', 'z', 'z', 'a', 'r', 'd'} //8
#define BPF_FRAME_MAGIC {'F', 'r', 'a', 'm'} //4
#define BPF_TAIL_STOP_CODE {'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 't', 'h', 'e', ' ', 'e', 'n', 'd', '.'} //16

#define SHOW_BYTE_SIZE 0xCC0000

#define BPF_FULL_FRAME 0xFFFF

#define BPF_TYPE_HEAD 1
#define BPF_TYPE_FRAME 2
#define BPF_TYPE_TAIL 3

#define BPF_NAME_SIZE 64
#define BPF_MAX_DIFFS (512/sizeof(BlizzardPlaybackFileFrameDiff))
#define BPF_MAX_FRAME_SIZE ((sizeof(BlizzardPlaybackFileFrameDiff) * BPF_MAX_DIFFS) + sizeof(BlizzardPlaybackFileFrame))

#define BPF_MAGIC_OFFSET 0
#define BPF_TOTAL_FRAMES_OFFSET (sizeof(uint8_t) * 8)
#define BPF_TOTAL_TIME_OFFSET (BPF_TOTAL_FRAMES_OFFSET + sizeof(uint32_t))
#define BPF_FIRST_DELAY_OFFSET (BPF_TOTAL_TIME_OFFSET + sizeof(uint32_t))
#define BPF_NAME_OFFSET (BPF_FIRST_DELAY_OFFSET + sizeof(uint32_t))
#define BPF_STARTING_DMX_OFFSET (BPF_NAME_OFFSET + sizeof(uint8_t) * BPF_NAME_SIZE)

typedef struct BlizzardPlaybackFileHeader {
  uint8_t _magic[8]; 
  uint32_t _total_frames; //8
  uint32_t _total_time; //ms
  uint32_t _delay;
  uint8_t _name[BPF_NAME_SIZE];
  uint8_t _starting_dmx[512];
}__attribute__((packed)) BlizzardPlaybackFileHeader;

typedef struct BlizzardPlaybackFileFrame {
  uint8_t _magic[4];
  uint32_t _delay; //14
  uint16_t _diff_count; //0xFFFF -> full frame is next
}__attribute__((packed)) BlizzardPlaybackFileFrame;

typedef struct BlizzardPlaybackFileFrameFull {
  uint8_t _dmx[512];
}__attribute__((packed)) BlizzardPlaybackFileFrameFull;

typedef struct BlizzardPlaybackFileFrameDiff {
  uint16_t _address;
  uint16_t _value;
}__attribute__((packed)) BlizzardPlaybackFileFrameDiff;

typedef struct BlizzardPlaybackFileTail {
  uint8_t _magic[16];
}__attribute__((packed)) BlizzardPlaybackFileTail;

uint8_t init_show_manager(void);
void tick_show(void);

void change_show(char *showname);
void loadShow(void);

void play_show(void);
void pause_show(void);
void stop_show(void);
void toggle_show(void);
void seek_show(uint32_t frame);

uint8_t get_show_state(void);
uint8_t get_show_internal_state(void);
uint8_t get_show_ok(void);

uint8_t get_show_on_start(void);
void change_show_on_start(uint8_t show_on_start);

uint8_t get_show_on_loop(void);
void change_show_on_loop(uint8_t show_on_loop);

void get_show_name(char* name, uint8_t cap);

uint32_t get_current_frame(void);
uint32_t get_total_frames(void);

uint32_t get_current_timestamp(void);
uint32_t get_total_time(void);

uint8_t *get_head_magic();
uint8_t *get_frame_magic();
uint8_t *get_tail_magic();
uint8_t get_head_magic_size();
uint8_t get_frame_magic_size();
uint8_t get_tail_magic_size();
char *get_show_name_pointer();

/* Under the Hood */
uint8_t check_show(void);
uint8_t setShowOK(void);
uint8_t setShowError(void);
uint8_t setShowEmpty(void);

uint8_t nextFrame(void);
uint8_t prevFrame(void);
void seekFrame(uint32_t frame);

uint8_t readHeadIntoBuffer(void);
uint8_t readFrameIntoBuffer(uint32_t pointer);
uint8_t checkBytes(uint8_t* buffer, uint8_t* check, uint8_t length);
uint8_t readShow(uint32_t offset, uint8_t* buffer, uint32_t length);

//Debug
void dumpShow(uint32_t start, uint8_t reps);
uint8_t testRW();

#ifdef __cplusplus
}
#endif

#endif

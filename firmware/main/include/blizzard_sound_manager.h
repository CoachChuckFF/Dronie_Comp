#ifndef BLIZZARD_SOUND_MANAGER_H
#define BLIZZARD_SOUND_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CLIP_CHIRP 0
#define CLIP_DOT 1
#define CLIP_OHHI 2


uint8_t init_sound_manager(void);

void play_sound(uint8_t clip);
void stop_sound(void);

uint8_t get_playing(void);

int scaleSound(uint8_t* d_buff, uint8_t* s_buff, uint32_t len);
void playSound(void *arg);

#ifdef __cplusplus
}
#endif

#endif


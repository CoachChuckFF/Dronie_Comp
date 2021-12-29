/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Recorder Manager -> blizzard_recorder_manager.h
 *
 */

#ifndef BLIZZARD_RECORDER_MANAGER_H
#define BLIZZARD_RECORDER_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>

#define RECORDER_COUNTDOWN_TICK 3000
#define RECORDER_CHECK_FRAME_TICK 22
#define RECORDED_FILE_NAME {'R','e','c','o','r','d','e','d', ' ', 'S', 'h', 'o', 'w', 0, 0, 0}

uint8_t init_recorder_manager(void);

void start_recording(void);
void stop_recording(void);
void tick_recorder(void);
uint8_t get_recorder_state(void);

void writeFirstDMX(void);
bool checkFrame(void);
bool hasEnoughSpace(void);
void setBuffer(void);
void writeFrame(void);
void finishHeader(void);
void writeEnd(void);

#ifdef __cplusplus
}
#endif

#endif
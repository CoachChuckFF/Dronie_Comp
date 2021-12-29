/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Global Defines-> blizzard_timings.h
 *
 */

#ifndef BLIZZARD_TIMINGS_DEFINES_H
#define BLIZZARD_TIMINGS_DEFINES_H

#include "blizzard_keys.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BATTERY_TICK_DELAY 1000
#define BATTERY_SLEEP_TICK_DELAY 100

#define CONNECTION_TICK_TIMOUT 5000

#define UI_USER_INPUT_FEEDBACK_TIMEOUT 34 
#define UI_USER_PLAY_STOP_FEEDBACK_TIMEOUT 377
#define UI_USER_TICK_TIMEOUT (1000*15)
#define UI_BATTERY_TIMEOUT (1000*30*1)
#define UI_SECRET_TIMEOUT (1000*60*8)

#ifdef __cplusplus
}
#endif

#endif
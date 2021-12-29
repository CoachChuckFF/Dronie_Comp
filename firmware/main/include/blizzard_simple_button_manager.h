/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * UI Manager -> blizzard_button_manager.h
 *
 */

#ifndef BLIZZARD_BUTTON_MANAGER_H
#define BLIZZARD_BUTTON_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/gpio.h"

#define BUTTON_HOLD_TIMEOUT 1000
#define BUTTON_TAP_TIMEOUT 200
#define BUTTON_DOUBLE_TAP_TIMEOUT 200

#define BUTTON_STABLE_COUNT 30
#define BUTTON_COOLDOWN 50

#define BUTTON_LEVEL_GPIO ADC1_CHANNEL_0
#define BUTTON_LEVEL_ATTEN ADC_ATTEN_DB_11

#define BUTTON_NOT_PRESSED_LEVEL (1638)
#define BUTTON_PRESSED_HIGH_LEVEL (3000)
#define BUTTON_PRESSED_LOW_LEVEL (340)


uint8_t init_button_manager(void);

uint8_t tick_button(void);

uint8_t getPressed(void);
uint8_t getStablePressed(void);

#ifdef __cplusplus
}
#endif

#endif

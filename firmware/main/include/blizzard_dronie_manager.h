/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * UI Manager -> blizzard_button_manager.h
 *
 */

#ifndef BLIZZARD_DRONIE_MANAGER_H
#define BLIZZARD_DRONIE_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "blizzard_colors.h"

#define DRONIE_MOVE_PIN 22
#define DRONIE_MOVE_PIN_BIT (1ULL<<DRONIE_MOVE_PIN)

#define DRONIE_LED_COUNT 5

#define DRONIE_EDIT_TIMEOUT 8
#define DRONIE_ACTION_COOLDOWN_LOW 3000
#define DRONIE_ACTION_COOLDOWN_HI 21000

uint8_t init_dronie_manager(void);

void tick_dronie(void);
uint8_t get_dronie_timeout(void);

uint8_t get_dronie_mode(void);
void set_dronie_mode(uint8_t mode);

void touch_dronie_motor(void);
void tick_dronie_motor(void);
 
void get_dronie_led(uint8_t index, color_t *ret);
void set_dronie_led(uint8_t index, color_t *set);

uint8_t get_dronie_motor_state(void);
void set_dronie_motor_state(uint8_t state);

void play_dronie_dot(void);
void play_dronie_dash(void);

#ifdef __cplusplus
}
#endif

#endif

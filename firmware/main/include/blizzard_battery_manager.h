/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Battery Manager -> blizzard_battery_manager.h
 *
 */

#ifndef BLIZZARD_BATTERY_MANAGER_H
#define BLIZZARD_BATTERY_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

extern uint32_t ulp_detect_high;
extern uint32_t ulp_detect_low;
extern uint32_t ulp_wakeup_count;
extern uint32_t ulp_held_count;
extern uint32_t ulp_entry;

#define BATTERY_LEVEL_GPIO ADC1_CHANNEL_3
#define BATTERY_LEVEL_ATTEN ADC_ATTEN_DB_11

#define BATTERY_AVERAGE_RESOLUTION 3
#define ULP_WAKEUP_COUNT 30

uint8_t init_battery_manager(void);

void tick_battery(void);

bool battery_is_charging(void);
bool battery_has_valid_input(void);

uint8_t get_battery_state(void);
uint8_t get_battery_level(void);

//goes to low power mode
void go_the_fuck_to_sleep(void);

#ifdef __cplusplus
}
#endif

#endif
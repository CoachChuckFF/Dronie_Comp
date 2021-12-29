/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * UI Manager -> blizzard_ui_manager.h
 *
 */

#ifndef BLIZZARD_SIMPLE_UI_MANAGER_H
#define BLIZZARD_SIMPLE_UI_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>
#include "blizzard_colors.h"

uint8_t init_ui_manager(void);

void tick_data(void);
void tick_ui(void);

uint8_t get_current_ui_state(void);
void set_listening_ui_state(void);
void set_idle_ui_state(void);
void set_prerecording_ui_state(void);
void set_recording_ui_state(void);
void set_ui_crit(bool crit);
bool get_ui_crit(void);
void set_ui_normal(void);
void set_ui_locate(bool locate);
bool get_ui_locate(void);
void set_ui_mute(bool mute);
bool get_ui_mute(void);

void handleInputFeedback(color_t color, uint16_t spin);
void handleAction(uint8_t action);
void handleBatteryLogic(void);
void handleViewLogic(void);
void handleIdleLogic(void);

#ifdef __cplusplus
}
#endif

#endif
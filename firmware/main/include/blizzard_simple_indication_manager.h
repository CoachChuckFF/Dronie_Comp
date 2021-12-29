/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * UI Manager -> blizzard_indication_manager.h
 *
 */

#ifndef BLIZZARD_INDICATION_MANAGER_H
#define BLIZZARD_INDICATION_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "blizzard_global_defines.h"

#define CLUE_TICK 144
#define CLUE_1 "... --- -.-. ...?.--. ..- --.. --.. .-.. .?-....-?-.- . . .--.?-.. --- ..- -... .-.. .?- .- .--. .--. .. -. --.?????"
#define CLUE_2 "----- -..- ....- ..--- -.... -.-. -.... .---- -.... ...-- -.... -... -.... ..-. --... ..... --... ....- ..--- ----- --... ....- -.... ---.. -.... ..... -.... . ..--- ----- --... ...-- -.... ..... --... ....- ..--- ----- ....- -.... -.... ----. -.... ..--- -.... ..-. -.... . -.... .---- -.... ...-- -.... ...-- -.... ----. ..--- ----- ....- ....- ....- -.. ..... ---.. ..--- ----- ....- .---- -.... ....- -.... ....- --... ..--- -.... ..... --... ...-- --... ...-- -.... ..... --... ...-- ..--- ----- --... ....- -.... ..-. ..--- ----- ....- -.... --... ..... -.... -.-. -.... -.-.?????"
#define CLUE_3 "-.-- --- ..-?.-- .. -. -.-.--?..-. ..- .-.. .-.. .--.-. -.... ----.?????"

#define BATTERY_RESET_PERIOD 8

uint8_t init_indication_manager(void);
void tick_indicator(uint8_t state);
uint8_t power_indicator(bool on);

color_t* get_current_color(void);
void change_indicator(uint8_t red, uint8_t green, uint8_t blue);
void change_indicator_bright(uint8_t red, uint8_t green, uint8_t blue, double brightness);
void change_indicator_color(color_t color);
void change_indicator_color_bright(color_t color, double brightness);


void handleAnimation(uint8_t animation, color_t color);

void tickBreath(uint32_t tick, color_t color);
void tickBreathIndex(uint32_t tick, color_t color, uint8_t index);
void tickFlash(uint32_t tick, color_t color);
void tickSwap(uint32_t tick, color_t one, color_t two);
void tickDits(uint32_t tick, uint8_t level, color_t color); //pulse x level - black
void tickBatteryCrit(uint32_t tick);
void tickMorseCode(char * code, uint16_t length, color_t color);
void tickRainbow(uint32_t tick);

color_t get_dmx_color(uint16_t address);
void danger_battery_animation(void);
void sleep_animation(void);
void morgenstimmung_animation(void);

#ifdef __cplusplus
}
#endif

#endif
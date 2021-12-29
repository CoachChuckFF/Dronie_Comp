/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, November 2018
 *
 * Protocol Manager -> blizzard_ws8212_driver.h
 *
 * The ws2812 manager is able to control X amount of WS2812 compatible leds.
 * It uses a modified spi protocol only useing the MOSI pin. The frequency is set 
 * 4x the frequency a WS2812 led requires.
 */

#ifndef BLIZZARD_WS2812_MANAGER_H
#define BLIZZARD_WS2812_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include "blizzard_colors.h"


/*----------------------- Pin Information ------------------------------------*/

#define WS_MISO_PIN (-1)
#define WS_MOSI_PIN 23
#define WS_CLK_PIN  (-1)
#define WS_CS_PIN   (-1)

/*----------------------- Type Information -----------------------------------*/

typedef struct led_t{
    uint8_t green[4];
    uint8_t red[4];
    uint8_t blue[4];
}__attribute__((packed)) led_t;

/*----------------------- Reset Defines --------------------------------*/

#define RESET_LED_LENGTH 5

/*----------------------- Functions ------------------------------------------*/

uint8_t init_ws2812_manager(uint16_t length);
uint8_t redrum_ws2812_manager(void);

void set_all_leds(uint8_t red, uint8_t green, uint8_t blue);
void set_all_leds_bright(uint8_t red, uint8_t green, uint8_t blue, double brightness);
void set_all_leds_color(color_t color);
void set_all_leds_color_bright(color_t color, double brightness);

void set_led(uint16_t index, uint8_t red, uint8_t green, uint8_t blue);
void set_led_bright(uint16_t index, uint8_t red, uint8_t green, uint8_t blue, double brightness);
void set_led_color(uint16_t index, color_t color);
void set_led_color_bright(uint16_t index, color_t color, double brightness);

//Helpers
void kickoff(void);
void ledValueTransform(uint8_t value, uint8_t *dest);

#ifdef __cplusplus
}
#endif

#endif

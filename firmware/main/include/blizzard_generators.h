/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Blizzard Reserves -> blizzard_generators.h
 *
 */

#ifndef BLIZZARD_GENERATORS_H
#define BLIZZARD_GENERATORS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

uint8_t generate_random_u8(uint8_t* generated, uint8_t min, uint8_t max); //inclusive
uint8_t generate_random_string(char* generated, uint8_t char_count);
uint8_t generate_garbage(uint8_t* generated, uint8_t length);

#ifdef __cplusplus
}
#endif

#endif



/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Blizzard Reserves -> blizzard_reserves_defines.h
 *
 */

#ifndef BLIZZARD_HELPERS_H
#define BLIZZARD_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define SET_FP_NO_CHANGE 0
#define SET_FP_ADD_COUNT 1


uint32_t blz_generate_u32(uint32_t min, uint32_t max);

uint8_t generate_random_u16(uint16_t* generated, uint16_t min, uint16_t max);
uint8_t blz_generate_random_u8(uint8_t* generated, uint8_t min, uint8_t max); //inclusive
uint8_t blz_generate_random_string(char* generated, uint8_t char_count);
uint8_t blz_generate_garbage(uint8_t* generated, uint8_t length);

uint8_t blz_min_u8(uint8_t a, uint8_t b);
uint16_t blz_min_u16(uint16_t a, uint16_t b);
uint32_t blz_min_u32(uint32_t a, uint32_t b);

uint8_t blz_max_u8(uint8_t a, uint8_t b);
uint16_t blz_max_u16(uint16_t a, uint16_t b);
uint32_t blz_max_u32(uint32_t a, uint32_t b);

uint8_t blz_strlen(char * string, uint8_t max);

uint8_t blz_is_char_safe(char c);
void blz_str_to_safe(char * string, uint8_t len);

uint8_t blz_set_filepath(char * buffer, char * fp, uint8_t count);
uint8_t blz_set_string(char * buffer, const char * string);

uint8_t hex_dumper(uint8_t* hex, uint32_t size); 
uint16_t to_right_endian_16(uint16_t hex); 
uint32_t to_right_endian_32(uint32_t hex); 

#ifdef __cplusplus
}
#endif

#endif



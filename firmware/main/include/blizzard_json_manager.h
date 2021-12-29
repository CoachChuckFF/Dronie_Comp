/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 * 
 * The JSON Manager parses JSON from Artnet 
 * 
 * JSON Manager -> blizzard_json_manager.h
 *
 */

#ifndef BLIZZARD_JSON_MANAGER_H
#define BLIZZARD_JSON_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include "cJSON.h"

#define JSON_ROOT_INDEX (MAX_JSON_ITEMS - 1)
#define JSON_ERROR_INDEX (MAX_JSON_ITEMS - 2)
#define JSON_TEMP_INDEX (MAX_JSON_ITEMS - 3)

#define JSON_ODYSSEY_KEY "Odyssey"
#define JSON_ODYSSEY_VALUE 0x03

uint8_t init_json_manager(void);
uint8_t parse_json(char* json, char* ret_json);
uint8_t setJSONError(uint8_t enable, cJSON **json, const char* message, uint8_t reason, uint8_t action);

#ifdef __cplusplus
}
#endif

#endif
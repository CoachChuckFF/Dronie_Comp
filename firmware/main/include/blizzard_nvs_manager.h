/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * NVS_Manager -> blizzard_nvs_manager.h
 *
 */

#ifndef BLIZZARD_NVS_MANAGER_H
#define BLIZZARD_NVS_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <inttypes.h>
#include "esp_err.h"


#define NVS_NAMESPACE "Blizzard"

uint8_t init_nvs_manager(void);
void reset_esp32(void);
void dd_esp32(void);

//generic getters and setters
uint8_t get_nvs_config(const char* key, uint8_t data_type, void* data);
uint8_t set_nvs_config(const char* key, uint8_t data_type, void* data);

//helper functions
uint8_t readU8(const char* key, uint8_t* value);
uint8_t readU16(const char* key, uint16_t* value);
uint8_t readU32(const char* key, uint32_t* value);
uint8_t readString(const char* key, char* value);
uint8_t readBlob(const char* key, uint8_t* value, uint8_t len);


uint8_t writeU8(const char* key, uint8_t value);
uint8_t writeU16(const char* key, uint16_t value);
uint8_t writeU32(const char* key, uint32_t value);
uint8_t writeString(const char* key, char* value);
uint8_t writeBlob(const char* key, uint8_t* value, uint8_t len);

uint8_t populateNVS(uint8_t should_qc);

uint8_t checkString(char* string);
const char* errorToString(esp_err_t retVal);

#ifdef __cplusplus
}
#endif

#endif

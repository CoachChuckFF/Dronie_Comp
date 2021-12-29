/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * OTA Manager -> blizzard_ota_manager.h
 *
 */

#ifndef BLIZZARD_OTA_MANAGER_H
#define BLIZZARD_OTA_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>

#define OTA_TIMEOUT (13*1000)
#define BYTE_TO_PACKET_COUNT ((MAX_DMX_SLOTS - 1) * 2)

uint8_t init_ota_manager(void);
uint8_t get_ota_state(void);
void tick_ota(void);

uint8_t read_show_data(uint32_t offset, uint16_t length, uint8_t* buffer);
uint8_t read_dmx_data(uint8_t* buffer);

uint8_t start_firmware_ota(void);
uint8_t start_show_ota(void);
int write_ota(uint32_t block, uint32_t bin_length, uint8_t *buffer, uint32_t packet_length);
uint8_t end_firmware_ota(void); //last bytes are padded as 0xFF - writes faster to EEPROM
uint8_t end_show_ota(void); //last bytes are padded as 0xFF - writes faster to EEPROM

#ifdef __cplusplus
}
#endif

#endif
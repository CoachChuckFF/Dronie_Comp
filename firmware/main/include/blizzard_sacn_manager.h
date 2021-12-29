/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Protocol Manager -> blizzard_sacn_manager.h
 *
 */

#ifndef BLIZZARD_SACN_MANAGER_H
#define BLIZZARD_SACN_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#define SACN_BASE_IP 0xEFFF0000

#define SACN_PORT 0x15C0 // or 5568

#define SACN_ID { 0x41, 0x53, 0x43, 0x2D, 0x45, 0x31, 0x2E, 0x31, 0x37, 0x00, 0x00, 0x00 }

typedef volatile struct SACNDataPacket{
    /* Root Layer */
    uint16_t _preamble_size;
    uint16_t _postamble_size;
    uint8_t  _acn_id[12];
    uint8_t _flags_length_h;
    uint8_t _flags_length_l;
    uint32_t _root_vector;
    uint8_t  _cid[16];

    /* Frame Layer */
    uint8_t _frame_flength_h;
    uint8_t _frame_flength_l;
    uint32_t _frame_vector;
    uint8_t  _source_name[64];
    uint8_t  _priority;
    uint16_t _sync_address;
    uint8_t  _sequence;
    uint8_t  _options;
    uint8_t _universe_h;
    uint8_t _universe_l;

    /* DMP Layer */
    uint8_t _dmp_flength_h;
    uint8_t _dmp_flength_l;
    uint8_t  _dmp_vector;
    uint8_t  _type;
    uint16_t _first_address;
    uint16_t _address_increment;
    uint8_t _dmx_slots_h;
    uint8_t _dmx_slots_l;
    uint8_t  _dmx_data[513];
}__attribute__((packed)) SACNDataPacket;

/*External*/
uint8_t init_sacn_manager(void);
uint8_t start_sacn(void);
void stop_sacn(void);
void tick_sacn(void);

void createMulticastSocket();
void addSocketToMulticast();

uint8_t change_sacn_universe(uint16_t universe);
uint16_t get_sacn_universe(void);

/*Internal*/
void receivesACN();
uint32_t universeToIP();
uint8_t parsesACNDataPacket();

#ifdef __cplusplus
}
#endif

#endif

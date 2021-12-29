/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Protocol Manager -> blizzard_protocol_manager.h
 *
 */

#ifndef BLIZZARD_PROTOCOL_MANAGER_H
#define BLIZZARD_PROTOCOL_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <inttypes.h>


uint8_t init_protocol_manager(void);

uint8_t change_active_protocol(uint8_t protocol);

void start_listening(void);
void stop_listening(void);

uint8_t change_active_protocol(uint8_t protocol);

uint8_t get_active_protocol(void);

#ifdef __cplusplus
}
#endif

#endif

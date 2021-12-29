/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Global Defines-> blizzard_actions.h
 *
 */

#ifndef BLIZZARD_ACTIONS_H
#define BLIZZARD_ACTIONS_H

#ifdef __cplusplus
extern "C" {
#endif
 
#define ACTION_SET_GENERAL_CONFIG           1
#define ACTION_GET_GENERAL_CONFIG           2

#define ACTION_SET_CONNECTION               10
#define ACTION_RECONNECT_WIFI               11
#define ACTION_RECONNECT_AP                 12
#define ACTION_RECONNECT_ETHERNET           13
#define ACTION_SET_DHCP                     14
#define ACTION_SET_IP                       15
#define ACTION_GET_ACTIVE_IP                16
#define ACTION_SET_DISABLE_WIFI_ON_ETHERNET 17
#define ACTION_GET_NETWORK_CONNECTIONS      18
#define ACTION_GET_NETWORK_INFO             19
#define ACTION_GET_MAC                      20

#define ACTION_SET_ACTIVE_PROTOCOL          30

#define ACTION_REBOOT                       50
#define ACTION_FACTORY_RESET                51

#define ACTION_SNAPSHOT_RECORD              80
#define ACTION_START_RECORD                 81
#define ACTION_STOP_RECORD                  82
#define ACTION_PLAY_RECORD                  83
#define ACTION_PAUSE_RECORD                 84
#define ACTION_STEP_RECORD                  85
#define ACTION_SET_FRAME_RECORD             86
#define ACTION_DELETE_RECORD                87
#define ACTION_IMPORT_RECORD                88
#define ACTION_EXPORT_RECORD                89

#define ACTION_START_PLAYBACK               0xC0
#define ACTION_PAUSE_PLAYBACK               0xC1
#define ACTION_STOP_PLAYBACK                0xC2
#define ACTION_SEEK                         0xC7
#define ACTION_GET_PLAYBACK_NAME            0xC3
#define ACTION_GET_CURRENT_FRAME            0xC5
#define ACTION_GET_TOTAL_FRAMES             0xC6
#define ACTION_GET_CURRENT_TIMESTAMP        0xC8
#define ACTION_GET_TOTAL_TIME               0xC9
#define ACTION_SET_SHOW_ON_LOOP             0xCA
#define ACTION_SET_SHOW_ON_START            0xCB
#define ACTION_CHECK_SHOW                   0xCC

#define ACTION_START_LISTENING              0xD0
#define ACTION_STOP_LISTENING               0xD1


#ifdef __cplusplus
}
#endif

#endif
/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Global Defines-> blizzard_global_defines.h
 *
 */

#ifndef BLIZZARD_GLOBAL_DEFINES_H
#define BLIZZARD_GLOBAL_DEFINES_H

#include <inttypes.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "blizzard_errors.h"
#include "blizzard_actions.h"
#include "blizzard_keys.h"
#include "blizzard_states.h"
#include "blizzard_colors.h"
#include "blizzard_pins.h"
#include "blizzard_timings.h"
#include "soc_it_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FIRMWARE_VERSION FIRMWARE_VERSION_HIGH
#define FIRMWARE_BUILD FIRMWARE_VERSION_LOW

//types - do not fuck with them
#define ENABLE          1
#define DISABLE         0

#define SEND            1
#define RECEIVE         0

#define HIGH            1
#define LOW             0

#define TRUE            1
#define FALSE           0

#define DATA_U8         0
#define DATA_U16        1
#define DATA_U32        2
#define DATA_STRING     3

#define MEDIUM_WIFI     1 
#define MEDIUM_ETHERNET 2 
#define MEDIUM_ACTIVE   3
#define MEDIUM_AP       4 
#define MEDIUM_NONE     0 
#define MEDIUM_DEVICE_CONNECTED 5

#define CONNECTION_INTERNAL 3
#define CONNECTION_EXTERNAL 5

#define CONNECTION_TYPE_AP CONNECTION_INTERNAL
#define CONNECTION_TYPE_STA CONNECTION_EXTERNAL

#define IP_IP           1 
#define IP_NETMASK      2 
#define IP_GATEWAY      3 

#define PROTOCOL_DMX    1
#define PROTOCOL_ANYFI  2
#define PROTOCOL_ARTNET 3 
#define PROTOCOL_SACN   4
#define PROTOCOL_READ   5
#define PROTOCOL_REVERT 6
#define PROTOCOL_NONE   0

//artnet merge modes
#define MERGE_NONE      0   
#define MERGE_HTP       1
#define MERGE_LTP       2

//ticks
#define TICK_MERGE      10000
#define TICK_SYNC        3000

//network settings
#define NETWORK_AP_IP IPADDR4_INIT(0xC0A80403)

//connections
#define CONNECTION_ETH  BIT(0)
#define CONNECTION_WIFI BIT(1)
#define CONNECTION_AP   BIT(2)

//sd card
#define SD_MOUNT_POINT "/sd"
#define SHOW_DIRECTORY_NAME "/shows"
#define SHOW_DIRECTORY SD_MOUNT_POINT "/shows"
#define SHOW_FILEPATH_PREAMBLE SHOW_DIRECTORY "/" 
#define SHOW_FILETYPE ".bpf"
#define SHOW_COUNTER "_000"

//maxes
#define MAX_FILENAME_LENGTH 32
#define MAX_STRING_LENGTH 64
#define MAX_SSID_LENGTH 32
#define MAX_PASS_LENGTH 64
#define MAX_LONG_NAME MAX_STRING_LENGTH
#define MAX_SHORT_NAME 18
#define MAX_IP_LENGTH 17
#define MAX_KEY_LENGTH 15
#define MAX_DMX_SLOTS 513
#define MAX_SACN_UNIVERSE 63999
#define MAX_ARTNET_NET 0x07
#define MAX_ARTNET_SUBNET 0x0F
#define MAX_ARTNET_UNIVERSE 0x0F
#define MAX_SOC_INFO 512
#define MAX_JSON_ITEMS 9
#define MAX_MAC_SIZE 6
#define MAX_EZ_CONNECT_SSID_SIZE 33
#define MAX_EZ_CONNECT_PASS_SIZE 65

/*----------------------- RDM Information ------------------------------------*/
/** The maximum size for an RDM packet, including the two checksum bytes. */
#define RDM_MAX_BYTES 257

/** The minimum size for an RDM packet. */
#define RDM_MIN_BYTES 26

/** The maximum length of the Parameter Data in an RDM packet. */
#define RDM_MAX_PDL 231

/** The length of the RDM message preceding the Parameter Data. */
#define RDM_HEADER_SIZE 24

/** The length of the RDM message preceding the Parameter Data. */
#define RDM_MAX_UID_COUNT 32 //its really 36, but 32 without a repeater

//show defined
#define SHOW_OK 0
#define SHOW_EMPTY 1
#define SHOW_ERROR 2

#ifdef __cplusplus
}
#endif

#endif


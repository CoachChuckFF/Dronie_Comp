/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Global Defines-> blizzard_defaults.h
 *
 */

#ifndef BLIZZARD_SOC_H
#define BLIZZARD_SOC_H

#include "blizzard_keys.h"
#include "blizzard_global_defines.h"

#ifdef __cplusplus
extern "C" {
#endif


    #define SOC_IT_ID 0x34
    #define FIRMWARE_VERSION_HIGH 4 //So we can upgrade
        #define FIRMWARE_VERSION_LOW 2

    #define QC_SSID "blizznet"
    #define QC_PASS "destroyer"
    #define QC_NAME "Needs Factory Reset (QC MODE)"

    #define DEFAULT_KEYS {\
        QC_KEY,\
        RESET_KEY,\
        LOG_KEY,\
        LOG_OFFSET_KEY,\
        DEVICE_NAME_KEY,\
        FIXTURE_TYPE_KEY,\
        FIRMWARE_VERSION_KEY,\
        FIRMWARE_SUB_VERSION_KEY,\
        FIRMWARE_AUTHOR_KEY,\
        SSID_KEY,\
        PASS_KEY,\
        CONNECTION_TYPE_KEY,\
        WIFI_DHCP_ENABLE_KEY,\
        WIFI_IP_KEY,\
        WIFI_NETMASK_KEY,\
        WIFI_GATEWAY_KEY,\
        AP_PASS_KEY,\
        ACTIVE_PROTOCOL_KEY,\
        ARTNET_MERGE_KEY,\
        ARTNET_NET_KEY,\
        ARTNET_SUBNET_KEY,\
        ARTNET_UNIVERSE_KEY,\
        SACN_UNIVERSE_KEY,\
        SHOW_ON_START_KEY,\
        SHOW_ON_LOOP_KEY,\
        SHOW_STATUS_KEY,\
        LED0_KEY,\
        LED1_KEY,\
        LED2_KEY,\
        LED3_KEY,\
        LED4_KEY\
    }

    #define DEFAULT_TYPES {\
        /*QC Flag*/DATA_U8,\
        /*Reset Flag*/DATA_U8,\
        /*Log*/DATA_STRING,\
        /*Log Offset*/DATA_U16,\
        /*Device Name*/DATA_STRING,\
        /*Fixture Type*/DATA_U16,\
        /*Version*/DATA_U8,\
        /*Sub Version*/DATA_U8,\
        /*Author Key*/DATA_STRING,\
        /*SSID*/DATA_STRING,\
        /*PASS*/DATA_STRING,\
        /*Connection Type*/DATA_U8,\
        /*WiFi DHCP Enable*/DATA_U8,\
        /*WiFi Static IP*/DATA_U32,\
        /*WiFi Static Netmask*/DATA_U32,\
        /*WiFi Static Gateway*/DATA_U32,\
        /*WiFi AP Password*/DATA_STRING,\
        /*Active Protocol*/DATA_U8,\
        /*Artnet Merge*/DATA_U8,\
        /*Artnet Net*/DATA_U8,\
        /*Artnet Subnet*/DATA_U8,\
        /*Artnet Universe*/DATA_U8,\
        /*SACN Universe*/DATA_U16,\
        /*Show On Start*/DATA_U8,\
        /*Show On Loop*/DATA_U8,\
        /*Show Status*/DATA_U8,\
        /*LED0*/DATA_U32,\
        /*LED1*/DATA_U32,\
        /*LED2*/DATA_U32,\
        /*LED3*/DATA_U32,\
        /*LED4*/DATA_U32\
    }

    #define DEFAULT_STRINGS {\
        /*LOG*/"[REDACTED]",\
        /*Name*/"SN-F09F90A6F09FA4962333",\
        /*Author*/"Talon Corp",\
        /*SSID (32)*/"",\
        /*Password (64)*/"",\
        /*AP Pass*/"taloncorp"\
    }

    #define DEFAULT_U32 {\
        /*IP*/0x0101A8C0,\
        /*Net*/0x00FFFFFF,\
        /*Gateway*/0x00000000,\
        /*LED0*/0xFFFFFFFF,\
        /*LED1*/0xFFFFFFFF,\
        /*LED2*/0xFFFFFFFF,\
        /*LED3*/0xFFFFFFFF,\
        /*LED4*/0xFFFFFFFF\
    }

    #define DEFAULT_U16 {\
        /*Log Offset*/0,\
        /*Device ID*/SOC_IT_ID,\
        /*SACN Universe*/1\
    }

    #define DEFAULT_U8 {\
        /*QC Key*/ENABLE,\
        /*Reset Key*/ENABLE,\
        /*Version*/FIRMWARE_VERSION_HIGH,\
        /*Sub Version*/FIRMWARE_VERSION_LOW,\
        /*Connection Type*/CONNECTION_TYPE_STA,\
        /*DHCP*/ENABLE,\
        /*Active Protocol*/PROTOCOL_ARTNET,\
        /*Artnet Merge*/MERGE_HTP,\
        /*Artnet Net*/0,\
        /*Artnet Subnet*/0,\
        /*Artnet Universe*/0,\
        /*Show on Start*/ENABLE,\
        /*Show on Loop*/ENABLE,\
        /*Show Status*/SHOW_EMPTY\
    }

#ifdef __cplusplus
}
#endif

#endif
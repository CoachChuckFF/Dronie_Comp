/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Global Defines-> blizzard_keys.h
 *
 */

#ifndef BLIZZARD_KEYS_H
#define BLIZZARD_KEYS_H

#ifdef __cplusplus
extern "C" {
#endif

//Function Keys
#define QC_KEY "QC" //bool
#define RESET_KEY "RESET" //bool
#define LOG_KEY "LOG" //String
#define LOG_OFFSET_KEY "LOG_OFFSET" //U16

//Device Info
#define DEVICE_NAME_KEY "DEVICE_NAME" //String - Doubles as AP SSID
#define DEVICE_SHORT_NAME_KEY "SHAWTY" //String
#define FIXTURE_TYPE_KEY "FIXTURE_TYPE" //String - String of Fixture Type
#define FIRMWARE_VERSION_KEY "FIRM_VER" //uint8_t
#define FIRMWARE_SUB_VERSION_KEY "FIRM_SUB" //uint8_t
#define FIRMWARE_AUTHOR_KEY "FIRM_AUTH" //String 

//Network Stuff
#define ETHERNET_DHCP_ENABLE_KEY "ETH_DHCP" //bool
#define ETHERNET_IP_KEY "ETH_IP" //String
#define ETHERNET_NETMASK_KEY "ETH_NETMASK" //String
#define ETHERNET_GATEWAY_KEY "ETH_GATEWAY" //String

#define CONNECTION_TYPE_KEY "CONNECTION_TYPE" //U8

#define SSID_KEY "SSID" //String
#define PASS_KEY "PASS" //String
#define WIFI_DHCP_ENABLE_KEY "WIFI_DHCP" //bool
#define WIFI_IP_KEY "WIFI_IP" //String
#define WIFI_NETMASK_KEY "WIFI_NETMASK" //String
#define WIFI_GATEWAY_KEY "WIFI_GATEWAY" //String

#define AP_PASS_KEY "AP_PASS" //String

//Protocol Info
#define ACTIVE_PROTOCOL_KEY "ACTIVE_PROTOCOL" //uint8_t 

#define ARTNET_MERGE_KEY "ARTNET_MERGE" //uint8_t
#define ARTNET_NET_KEY "ARTNET_NET" //uint8_t -> 0x00 - 0x07
#define ARTNET_SUBNET_KEY "ARTNET_SUB" //uint8_t -> 0x00 - 0x0F
#define ARTNET_UNIVERSE_KEY "ARTNET_UNI" //uint8_t 0x00 - 0x0F

#define SACN_UNIVERSE_KEY "SACN_UNIVERSE" //uint16_t -> 0x0000 -> 0xFFFF

//Show Info
#define SHOW_ON_START_KEY "SHOW_START" //uint8_t
#define SHOW_ON_LOOP_KEY "SHOW_LOOP" //uint8_t
#define SHOW_STATUS_KEY "SHOW_STATS" //uint8_t


#define OWNER_KEY "LOG" //String
#define SN_KEY "DEVICE_NAME" //String
#define MAN_KEY "FIRM_AUTH" //String
#define LED0_KEY "LED0" //U32
#define LED1_KEY "LED1" //U32
#define LED2_KEY "LED2" //U32
#define LED3_KEY "LED3" //U32
#define LED4_KEY "LED4" //U32


#ifdef __cplusplus
}
#endif

#endif
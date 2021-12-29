/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Network Manager -> blizzard_network_manager.h
 *
 */

#ifndef BLIZZARD_NETWORK_MANAGER_H
#define BLIZZARD_NETWORK_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "lwip/sockets.h"
#include "esp_event.h"

/*----------------------------- Ethernet Defines ---------------------------------*/

//#include "eth_phy/phy_lan8720.h"

#define PIN_PHY_POWER CONFIG_PHY_POWER_PIN
#define PIN_SMI_MDC   CONFIG_PHY_SMI_MDC_PIN
#define PIN_SMI_MDIO  CONFIG_PHY_SMI_MDIO_PIN

/*----------------------------- WiFi Defines ---------------------------------*/

#define RETRY_ATTEMPTS 3
#define RETRY_DELAY (100)
#define CONNECTION_TIMEOUT (23000)
#define STOP_WIFI_DELAY (1000)

#define UNSPECIFIED 1
#define AUTH_EXPIRE 2
#define AUTH_LEAVE 3
#define ASSOC_EXPIRE 4
#define ASSOC_TOOMANY 5
#define NOT_AUTHED 6
#define NOT_ASSOCED 7
#define ASSOC_LEAVE 8
#define ASSOC_NOT_AUTHED 9
#define DISASSOC_PWRCAP_BAD 10
#define DISASSOC_SUPCHAN_BAD 11
#define IE_INVALID 12 
#define MIC_FAILURE 13
#define FOUR_WAY_HANDSHAKE_TIMEOUT 14
#define GROUP_KEY_UPDATE_TIMEOUT 15 
#define IE_IN_4WAY_DIFFERS 16
#define GROUP_CIPHER_INVALID 17
#define PAIRWISE_CIPHER_INVALID 18
#define AKMP_INVALID 19
#define UNSUPP_RSN_IE_VERSION 20
#define INVALID_RSN_IE_CAP 21 
#define EIGHT02_1X_AUTH_FAILED 22
#define CIPHER_SUITE_REJECTED 23
#define BEACON_TIMEOUT 24
#define NO_AP_FOUND 201
#define AUTH_FAIL 202
#define ASSOC_FAIL 203
#define HANDSHAKE_TIMEOUT 204

#define MAX_AP_CONNECTIONS 1
#define BEACON_INTERVAL 1000

/*----------------------------- Functions ------------------------------------*/

uint8_t init_network_manager(void);
void tick_network(void);

void initWifi(void);
uint8_t startEZConnect(void);
uint8_t startWifi(void);
uint8_t startWifiAP(void);
uint8_t stopWifi(void);
uint8_t reconnect_wifi(void);
uint8_t reconnect_ap(void);
uint8_t startEthernet(void);
uint8_t stopEthernet(void);
uint8_t reconnect_ethernet(void);

uint8_t change_connection(uint8_t connection);
uint8_t switchConnection(uint8_t connection);
uint8_t get_connection(void);

uint8_t change_dhcp_enable(bool enable, uint8_t medium);
uint8_t configureDHCP(bool enable, uint8_t medium);
bool get_dhcp_enable(uint8_t medium);

uint8_t change_ip(uint8_t medium, uint8_t ip_type, ip_addr_t* address);
uint8_t configureIp(uint8_t medium, uint8_t ip_type, ip_addr_t* address);
uint8_t get_active_ip(uint8_t medium, uint8_t ip_type, ip_addr_t* address);
uint8_t get_nvs_ip(uint8_t medium, uint8_t ip_type, ip_addr_t* address);
uint8_t getNVSIP(const char* key, ip_addr_t* address);

uint8_t get_mac(uint8_t* mac);

/*----------------------------- Network Connections Functions -----------------------------*/

uint8_t get_network_connections(void);
void setNetworkConnection(uint8_t connection);
void clearNetworkConnection(uint8_t connection);
uint8_t check_network_connection(uint8_t connection);
uint8_t get_ez_connect_running(void);
uint8_t get_ez_found_channel(void);
uint8_t get_device_connected(void);
uint8_t get_wifi_error(void);

/*----------------------------- Active Medium -----------------------------------------*/
void setActiveMedium(void);
uint8_t get_active_medium(void);

/*----------------------------- Handler Functions ----------------------------*/

void blizzardConnectWifi(void);
void blizzardEventHandler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);
uint8_t ezConnectDetectAtFull(uint8_t* ssid, uint8_t* pass, uint8_t bypass);

/*----------------------------- Debug Functions ------------------------------*/

void dump_network_information(void);
const char * medium_to_string(uint8_t medium);

/*----------------------------- Backlog ------------------------------*/
uint8_t set_network_defaults(void);

#ifdef __cplusplus
}
#endif

#endif


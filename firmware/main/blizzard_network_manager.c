/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Connection Manager -> blizzard_network_manager.c
 *
 * The network manager makes sure that the BB is always in a state of network
 * be that Wi-Fi, AP or a combination. It also managers statically and
 * DHCP assigned IP addresses.
 * 
 */

#include "blizzard_network_manager.h"


#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include "esp_err.h"

//#include "rom/ets_sys.h"
#include "esp32/rom/ets_sys.h"
//#include "rom/gpio.h"
#include "esp32/rom/gpio.h"

#include "esp_wifi.h"
#include "esp_attr.h"

#include "esp_event.h"
#include "esp_wifi_types.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

//Header Includes
#include "lwip/sockets.h"
#include "esp_event.h"
#include "esp_smartconfig.h"


#include "blizzard_nvs_manager.h"
#include "blizzard_artnet_manager.h"


static const char *Tag = "CONNECTION";

/*---General---*/
uint8_t Network_Connections = 0;
uint8_t Active_Connection = CONNECTION_TYPE_AP;
uint8_t Active_Medium = MEDIUM_NONE;

/*---WiFi---*/
bool Wifi_DHCP_Enable = true;
bool Has_SSID = false;

//flags
bool Wifi_Init = false;
bool Bad_Wifi = false;
bool Bad_SSID = false;
bool Bad_Password = false;
bool EZ_Connect_Running = false;
bool EZ_Found_Channel = false;
bool EZ_Connect_Got_SSID = false;
bool EZ_Connect_Needs_Reset = false;
bool Switching_Connection = false;

//event group
static EventGroupHandle_t Wifi_Event_Group;
const int Wifi_Connected_Bit = BIT0;
const int Wifi_Disconnected_Bit = BIT1;

/*---AP---*/
//flags
bool Device_Connected = false;
//event group
const int AP_Enabled_Bit = BIT2;
const int AP_Disabled_Bit = BIT3;

/*---IP Info---*/
esp_netif_t *ESP_NETIF_IF_STA;
esp_netif_t *ESP_NETIF_IF_AP;


/*
 * Function: init_network_manager(void)
 * ----------------------------
 *   starts the tcpip adapter, will try to configure the
 *   last known medium and then try to connect
 *
 *   returns: CONNECTION_WIFI_EVENT_CREATE_ERROR CONNECTION_GET_MUTEX_ERROR
 *            startConnection Errors SUCCESS
 */
uint8_t init_network_manager(void)
{
    uint8_t wifiDHCPEnable;
    uint8_t retVal;

    ESP_LOGI(Tag, "Connections are in the locker room...");

    esp_netif_init();

    Wifi_Event_Group = xEventGroupCreate();
    if(Wifi_Event_Group == NULL){
        ESP_LOGE(Tag, "Not enough FreeRTOS heap space - wifi - init network");
        return CONNECTION_WIFI_EVENT_CREATE_ERROR;
    }

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //Netif setters
    ESP_NETIF_IF_STA = esp_netif_create_default_wifi_sta();
    ESP_NETIF_IF_AP = esp_netif_create_default_wifi_ap();

    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &blizzardEventHandler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &blizzardEventHandler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &blizzardEventHandler, NULL) );

    retVal = get_nvs_config(WIFI_DHCP_ENABLE_KEY, DATA_U8, &wifiDHCPEnable);
    if(retVal != SUCCESS){
        ESP_LOGE(Tag, "Error getting wifi dhcp - init network: %d", retVal);
        return retVal;
    }

    if(configureDHCP((wifiDHCPEnable) ? true : false, MEDIUM_WIFI) != SUCCESS){
        ESP_LOGE(Tag, "Error setting wifi dhcp - init network: %d", retVal);
        return retVal;
    }

    xEventGroupSetBits(Wifi_Event_Group, Wifi_Disconnected_Bit);
    xEventGroupClearBits(Wifi_Event_Group, Wifi_Connected_Bit);

    retVal = get_nvs_config(CONNECTION_TYPE_KEY, DATA_U8, &Active_Connection);
    if(retVal != SUCCESS){
      ESP_LOGE(Tag, "Error getting active connection - init network: %d", retVal);
      return retVal;
    }

    switch(Active_Connection){
      case CONNECTION_TYPE_AP:
        startWifiAP();
      break;
      case CONNECTION_TYPE_STA:
        startWifi();
      break;
      default:
        ESP_LOGE(Tag, "There is no way in hell that this message should ever be read...");
        return 69;
    }

    return retVal;
}

/*
 * Function: tick_network()
 * ----------------------------
 *   Checks if there is a network if not it will increment its counter
 *   if its counter reaches "timeout" - ESP32 will be forecd into AP Mode
 *
 *   returns: SUCCESS
 */
void tick_network(){
    static uint32_t to = CONNECTION_TIMEOUT;

    if(!(check_network_connection(0xFF))){
      if(!to--){
        switch(Active_Connection){
          case CONNECTION_TYPE_AP: //AP
            reconnect_ap();
          break;
          case CONNECTION_TYPE_STA: //WiFi
            if(!EZ_Connect_Running || EZ_Connect_Needs_Reset){
              EZ_Connect_Needs_Reset = false;
              reconnect_wifi();
            }
          break;
          default:
            ESP_LOGE(Tag, "There is no way in hell that this message should ever be read...");
        }
        to = CONNECTION_TIMEOUT;
      }
    }
}

/*--------------------------------- Start Stop WiFi/AP -------------------------------------*/

void initWifi(){
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM));
  Wifi_Init = true;
}
/*
 * Function: startWifi()
 * ----------------------------
 *   attempts to start the wifi with preset configurations
 *   if override is set - it will try to connect regardless of dwoe
 *
 *   returns: SUCCESS, NETWORK_ERROR
 */
uint8_t startWifi(){
  wifi_config_t wifiConfig;
  uint8_t retVal; 

  if(!Wifi_Init){
    initWifi();
  }

  Bad_Wifi = false;
  Bad_Password = false;
  Bad_SSID = false;

  memset(&wifiConfig, 0, sizeof(wifiConfig));

  retVal = get_nvs_config(SSID_KEY, DATA_STRING, wifiConfig.sta.ssid);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error getting ssid - start wifi: %d", retVal);
    return retVal;
  }

  Has_SSID = wifiConfig.sta.ssid[0] != '\0'; //if first char is not null then there is an SSID
  wifiConfig.sta.ssid[MAX_SSID_LENGTH-1] = '\0'; //Null terminate for safety

  retVal = get_nvs_config(PASS_KEY, DATA_STRING, wifiConfig.sta.password);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error getting pass - start wifi: %d", retVal);
    return retVal;
  }

  wifiConfig.sta.password[MAX_PASS_LENGTH-1] = '\0'; //Null terminate for safety

  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiConfig) );

  ESP_LOGI(Tag, "STA: %s, Pass: %s", wifiConfig.sta.ssid, wifiConfig.sta.password);

  ESP_ERROR_CHECK( esp_wifi_start() );

  return SUCCESS;
}

/*
 * Function: startEZConnect()
 * ----------------------------
 *   starts sniffing for the EZ connect
 *
 *   returns: SUCCESS, NETWORK_ERROR
 */
uint8_t startEZConnect()
{

  if(EZ_Connect_Running) return SUCCESS;

  if(!Wifi_Init){
    initWifi();
  }

  ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
  smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );

  EZ_Connect_Running = true;
  EZ_Connect_Got_SSID = false;
  EZ_Found_Channel = false;

  return SUCCESS;
}

/*
 * Function: startWifiAP()
 * ----------------------------
 *   attempts to start the wifi AP with preset configurations
 *
 *   returns: SUCCESS, NETWORK_ERROR
 */
uint8_t startWifiAP(){
    wifi_config_t wifiConfig;
    uint8_t i, passLength = 0, retVal = SUCCESS;

    if(!Wifi_Init){
      initWifi();
    }

    memset(&wifiConfig, 0, sizeof(wifiConfig));

    retVal = get_nvs_config(DEVICE_NAME_KEY, DATA_STRING, wifiConfig.ap.ssid);
    if(retVal != SUCCESS){
        ESP_LOGE(Tag, "Error getting ssid - start ap: %d", retVal);
    }

    wifiConfig.ap.ssid[MAX_SSID_LENGTH-1] = '\0'; //Null terminate for safety

    retVal = get_nvs_config(AP_PASS_KEY, DATA_STRING, wifiConfig.ap.password);
    if(retVal != SUCCESS){
        ESP_LOGE(Tag, "Error getting pass - start ap: %d", retVal);
    }

    wifiConfig.ap.password[MAX_PASS_LENGTH-1] = '\0'; //Null terminate for safety

    //only want one person connecting at a time
    wifiConfig.ap.max_connection = MAX_AP_CONNECTIONS;

    //beacon every interval
    wifiConfig.ap.beacon_interval = BEACON_INTERVAL;

    for(i = 0; i < MAX_PASS_LENGTH; i++){
      if(wifiConfig.ap.password[i]){
        passLength++;
      }
    }

    if(passLength >= 8){
      wifiConfig.ap.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
      wifiConfig.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, &wifiConfig) );

    ESP_LOGI(Tag, "AP: %s, Pass: %s", wifiConfig.ap.ssid, wifiConfig.ap.password);

    ESP_ERROR_CHECK( esp_wifi_start() );

    return retVal;
}

/*
 * Function: stopWifi(void)
 * ----------------------------
 *   attempts to stop the wifi
 *
 *   returns: CONNECTION_STOP_WIFI_ERROR SUCCESS
 */
uint8_t stopWifi(){

    //Stop EZ connect until in STA
    if(EZ_Connect_Running){
      if(!EZ_Connect_Got_SSID || Switching_Connection){
        Switching_Connection = false;
        EZ_Connect_Running = false;
        EZ_Connect_Got_SSID = false;
        EZ_Found_Channel = false;
        esp_smartconfig_stop();
      }
    }

    //We kinda just ignore these errors
    ESP_LOGI(Tag, "Disconnect wifi %d", esp_wifi_disconnect());

    ESP_LOGI(Tag, "Stop wifi %d", esp_wifi_stop());

    //Taskwait until disconnected
    // xEventGroupWaitBits(Wifi_Event_Group, Wifi_Disconnected_Bit,
    // false, true, CONNECTION_TIMEOUT);
    ESP_LOGI(Tag, "Done Disconnecting");

    return SUCCESS;
}

uint8_t reconnect_wifi(){
  uint8_t retVal;


  retVal = stopWifi();
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error stopping wifi - reconnect wifi: %d", retVal);
    return 69;
  }

  retVal = startWifi();
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error starting wifi - reconnect wifi: %d", retVal);
    return 69;
  }

  return retVal;
}

uint8_t reconnect_ap(){
  uint8_t retVal;

  retVal = stopWifi();
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error stopping ap - reconnect ap: %d", retVal);
    return 69;
  }

  retVal = startWifiAP();
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error starting ap - reconnect ap: %d", retVal);
    return 69;
  }

  return retVal;
}

/*--------------------------------- Connection -------------------------------------*/

/*
 * Function: change_connection(uint8_t connection)
 * ----------------------------
 *   switches connection type to either internal (AP) or external Ethernet/WiFi
 *
 *   returns: SUCCESS
 */
uint8_t change_connection(uint8_t connection){
  uint8_t retVal;

  if(Active_Connection == connection){
    ESP_LOGI(Tag, "%d connection is already the active connection", connection);
    return SUCCESS;
  }

  retVal = switchConnection(connection);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error changing connection - change connection: %d", retVal);
    return retVal;
  }
  
  retVal = set_nvs_config(CONNECTION_TYPE_KEY, DATA_U8, &connection);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting nvs connection - change connection: %d", retVal);
    return retVal;
  }

  return SUCCESS;
}

/*
 * Function: switchConnection(uint8_t connection)
 * ----------------------------
 *   switches connection type to either internal (AP) or external Ethernet/WiFi
 *
 *   returns: SUCCESS
 */
uint8_t switchConnection(uint8_t connection){
  uint8_t retVal = SUCCESS;

  switch(connection){
    case CONNECTION_TYPE_AP: //AP
      Switching_Connection = true;
      retVal = reconnect_ap();
    break;
    case CONNECTION_TYPE_STA: //WiFi
      Switching_Connection = true;
      retVal = reconnect_wifi();
    break;
    default:
      ESP_LOGE(Tag, "There is no way in hell that this message should ever be read...");
      return 69;
  }

  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error switching connection to %s - switch connection", (connection == CONNECTION_TYPE_AP) ? "Internal" : "External");
    return 69;
  }

  Active_Connection = connection;
  return SUCCESS;
}

/*
 * Function: get_connection(void)
 * ----------------------------
 *   returns active connection type - either internal (AP) or external Ethernet/WiFi
 *
 *   returns: SUCCESS
 */
uint8_t get_connection(){
  return Active_Connection;
}

/*--------------------------------- DHCP -------------------------------------*/

/*
 * Function: change_dhcp_enable(uint8_t enable, uint8_t medium)
 * ----------------------------
 *   starts or stops the dhcp server depending on configurations
 *   if dhcp is disabled, the user stored ip/net/gate will become
 *   the new network network
 *
 *   returns: SUCCESS
 */
uint8_t change_dhcp_enable(bool enable, uint8_t medium){
  uint8_t retVal;

  if(medium == MEDIUM_ACTIVE){
    medium = Active_Medium;
  }

  switch(medium){
    case MEDIUM_WIFI:
      if(enable && Wifi_DHCP_Enable){
        ESP_LOGI(Tag, "Wifi's DHCP is already enabled");
        return SUCCESS;
      } else if(!enable && !Wifi_DHCP_Enable){
        ESP_LOGI(Tag, "Wifi's DHCP is already disabled");
        return SUCCESS;
      }
    break;
    default:
      ESP_LOGI(Tag, "Can't change DHCP on %d medium", medium);
      return SUCCESS;
  }

  retVal = configureDHCP(enable, medium);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error configuring dhcp - change dhcp: %d", retVal);
    return retVal; 
  }

  switch(medium){
    case MEDIUM_WIFI:
      retVal = set_nvs_config(WIFI_DHCP_ENABLE_KEY, DATA_U8, &enable);
      break;
  }

  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting dhcp for %d medium in nvs - change dhcp: %d", medium, retVal);
    return retVal; 
  }

  return SUCCESS; 

}

/*
 * Function: configureDHCP(bool enable, uint8_t medium)
 * ----------------------------
 *   attempts to configure the dhcp driver - enable or disable for a given medium
 *
 *   returns: SUCCESS, CONNECTION_DHCP_MEDIUM_ERROR
 */
uint8_t configureDHCP(bool enable, uint8_t medium){
  esp_netif_ip_info_t ipInfo;
  esp_err_t retVal = SUCCESS;

  if(medium == MEDIUM_ACTIVE){
    medium = Active_Medium;
  }


  switch(medium){
    case MEDIUM_WIFI:
      if(!enable){

        retVal = esp_netif_get_ip_info(ESP_NETIF_IF_STA, &ipInfo);
        if(retVal != SUCCESS){
          ESP_LOGE(Tag, "Error getting preset data - configure dhcp: %d", retVal);
          return retVal;
        }

        retVal = get_nvs_ip(MEDIUM_WIFI, IP_IP, (ip_addr_t *)&(ipInfo.ip));
        if(retVal != SUCCESS){
          ESP_LOGE(Tag, "Error getting wifi ip - configure dhcp: %d", retVal);
          return retVal;
        }
        retVal = get_nvs_ip(MEDIUM_WIFI, IP_NETMASK, (ip_addr_t *)&(ipInfo.netmask));
        if(retVal != SUCCESS){
          ESP_LOGE(Tag, "Error getting wifi netmask - configure dhcp: %d", retVal);
          return retVal;
        }
        // retVal = get_nvs_ip(MEDIUM_WIFI, IP_GATEWAY, (ip_addr_t *)&(ipInfo.gw));
        // if(retVal != SUCCESS){
        //   ESP_LOGE(Tag, "Error getting wifi gateway - configure dhcp: %d", retVal);
        //   return retVal;
        // }

        retVal = esp_netif_dhcpc_stop(ESP_NETIF_IF_STA);
        if(retVal != ESP_OK){
          ESP_LOGE(Tag, "Error stopping wifi dhcp - configure dhcp: %s", errorToString(retVal));
          return retVal;
        }
        retVal = esp_netif_set_ip_info(ESP_NETIF_IF_STA, &ipInfo);
        if(retVal != ESP_OK){
          ESP_LOGE(Tag, "Error setting wifi ip info - configure dhcp: %s", errorToString(retVal));
          return retVal;
        }
      } else {
        retVal = esp_netif_dhcpc_start(ESP_NETIF_IF_STA);
        if(retVal != ESP_OK){
          ESP_LOGE(Tag, "Error starting wifi dhcp configure dhcp: %s", errorToString(retVal));
          return retVal;
        }
      }
      Wifi_DHCP_Enable = enable;
      break;
    default:
      ESP_LOGE(Tag, "Not a valid medium for enable dhcp");
      return CONNECTION_DHCP_MEDIUM_ERROR;
  }
  return SUCCESS;
}


/*
 * Function:get_dhcp_enable(uint8_t medium)
 * ----------------------------
 *   returns the status of dhcp for a given medium
 * 
 *   returns: true/false or CONNECTION_DHCP_MEDIUM_ERROR
 */
bool get_dhcp_enable(uint8_t medium){

  if(medium == MEDIUM_ACTIVE){
    medium = Active_Medium;
  }

  switch(medium){
    case MEDIUM_WIFI:
      return Wifi_DHCP_Enable;
  }

  return true;
}

/*------------------------------- IP ---------------------------------------*/

/*
 * Function: change_ip(uint8_t medium, uint8_t ip_type, ip_addr_t* address)
 * ----------------------------
 *   attemps to configure the ip with the given parameters, if successful,
 *   it sets the corresponding nvs feild
 * 
 *   returns: CONNECTION_SET_IP_ERROR, configureIp errors, SUCCESS
 */
uint8_t change_ip(uint8_t medium, uint8_t ip_type, ip_addr_t* address){
  uint8_t retVal;
  uint32_t tempIp;

  if(medium == MEDIUM_ACTIVE){
    medium = Active_Medium;
  }

  if(address == NULL){
    ESP_LOGE(Tag, "Null address - change ip");
    return 69; 
  }

  if(!(medium == MEDIUM_WIFI)){
    ESP_LOGE(Tag, "Invalid medium - change ip");
    return 69;
  }

  if(!(ip_type == IP_IP || ip_type == IP_NETMASK || ip_type == IP_GATEWAY)){
    ESP_LOGE(Tag, "Invalid ip type - change ip");
    return 69;
  }

  retVal = configureIp(medium, ip_type, address);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error configuring ip - change ip: %d", retVal);
    return retVal; 
  }

  tempIp = ip_addr_get_ip4_u32(address);

  switch(medium){
    case MEDIUM_WIFI:
      switch(ip_type){
        case IP_IP:
            retVal = set_nvs_config(WIFI_IP_KEY, DATA_U32, &tempIp);
          break;
        case IP_NETMASK:
            retVal = set_nvs_config(WIFI_NETMASK_KEY, DATA_U32, &tempIp);
          break;
        case IP_GATEWAY:
            retVal = set_nvs_config(WIFI_GATEWAY_KEY, DATA_U32, &tempIp);
          break;
      }
      break;
  }

  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting %d ip for %d medium in nvs  - change ip: %d", ip_type, medium, retVal);
    return retVal; 
  }

  return SUCCESS; 
}

/*
 * Function: configureIp(uint8_t medium, uint8_t ip_type, ip_addr_t* address)
 * ----------------------------
 *   attempts to configure an individual ip address to the ESP dhcp driver
 *
 *   returns: SUCCESS, CONNECTION_CONFIGURE_IP_ERROR
 */
uint8_t configureIp(uint8_t medium, uint8_t ip_type, ip_addr_t* address){
  esp_netif_ip_info_t ip;
  // esp_interface_t interface;
  // esp_netif_config_t cfg = ESP_NETIF_DEFAULT_WIFI_STA();
  // esp_netif_t *ESP_NETIF_IF_STA = esp_netif_new(&cfg);
  esp_err_t retVal;

  switch(medium){
    case MEDIUM_WIFI:
      if(Wifi_DHCP_Enable){
        return SUCCESS;
      }
      // interface = ESP_IF_WIFI_STA;
      break;
  }

  //To get other settings just right
  memset(&ip, 0, sizeof(esp_netif_ip_info_t));
  retVal = esp_netif_get_ip_info(ESP_NETIF_IF_STA, &ip);
  if(retVal != ESP_OK) {
    ESP_LOGE(Tag, "Error getting ip info - configure ip: %d", retVal);
    return retVal;
  }

  switch(ip_type){
    case IP_IP:
      ip.ip.addr = ip_addr_get_ip4_u32(address);
      break;
    case IP_NETMASK:
      ip.netmask.addr = ip_addr_get_ip4_u32(address);
      break;
    case IP_GATEWAY:
      ip.gw.addr = ip_addr_get_ip4_u32(address);
      break;
  }
  
  retVal = esp_netif_set_ip_info(ESP_NETIF_IF_STA, &ip);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting ip info - configure ip: %d", retVal);
    return retVal;
  }

  return SUCCESS;
}

/*
 * Function: uint8_t get_active_ip(uint8_t medium, uint8_t ip_type, ip_addr_t* address)
 * ----------------------------
 *  returns the current ip into the address parameter from the ESP network interface
 *  given the medium and the ip type requested. 
 *
 *  returns: CONNECTION_GET_IP_ERROR, SUCCESS
 */
uint8_t get_active_ip(uint8_t medium, uint8_t ip_type, ip_addr_t* address){
  esp_netif_ip_info_t ip;
  // esp_interface_t interface;
  esp_err_t retVal = 0;

  if(address == NULL){
    ESP_LOGE(Tag, "Null address - get ip");
    return CONNECTION_GET_IP_ERROR;
  }

  if(medium == MEDIUM_ACTIVE){
    medium = Active_Medium;
  }

  switch(medium){
    case MEDIUM_WIFI:
      memset(&ip, 0, sizeof(esp_netif_ip_info_t));
      retVal = esp_netif_get_ip_info(ESP_NETIF_IF_STA, &ip);
      break;
    case MEDIUM_AP:
      memset(&ip, 0, sizeof(esp_netif_ip_info_t));
      retVal = esp_netif_get_ip_info(ESP_NETIF_IF_AP, &ip);
      break;
    default:
      ESP_LOGE(Tag, "Invalid medium trying to get ip: medium %d", medium);
      return CONNECTION_GET_IP_ERROR;
  }  


  if(retVal != ESP_OK) {
    ESP_LOGE(Tag, "Error getting ip: %d", retVal);
    return retVal;
  }

  switch(ip_type){
    case IP_IP:
      ip_addr_set_ip4_u32(address, ip.ip.addr);
      break;
    case IP_NETMASK:
      ip_addr_set_ip4_u32(address, ip.netmask.addr);
      break;
    case IP_GATEWAY:
      ip_addr_set_ip4_u32(address, ip.gw.addr);
      break;
    default:
      ESP_LOGE(Tag, "Invalid ip type %d trying to configure ip: medium %d",ip_type, medium);
      return CONNECTION_GET_IP_ERROR;
  }

  return SUCCESS;
}

/*
 * Function: get_nvs_ip(uint8_t medium, uint8_t ip_type, ip_addr_t* address)
 * ----------------------------
 *  returns the ip address from the nvs as a ip_addr_t
 *
 *  returns: CONNECTION_GET_NVS_IP_ERROR, SUCCESS
 */
uint8_t get_nvs_ip(uint8_t medium, uint8_t ip_type, ip_addr_t* address){
  uint8_t retVal;

  if(address == NULL){
    ESP_LOGE(Tag, "Null address - get ip from medium");
    return CONNECTION_GET_NVS_IP_ERROR;
  }  
  
  if(medium == MEDIUM_ACTIVE){
    medium = Active_Medium;
  }

  switch(medium){
    case MEDIUM_WIFI:
      switch(ip_type){
        case IP_IP:
          retVal = getNVSIP(WIFI_IP_KEY, address);
          break;
        case IP_NETMASK:
          retVal = getNVSIP(WIFI_NETMASK_KEY, address);
          break;
        case IP_GATEWAY:
          retVal = getNVSIP(WIFI_GATEWAY_KEY, address);
          break;
        default:
          ESP_LOGE(Tag, "Invalid %d ip type - wifi - get nvs ip", ip_type);
          return CONNECTION_GET_NVS_IP_ERROR;
      }
      break;
    case MEDIUM_AP:
    case MEDIUM_NONE:
      ESP_LOGI(Tag, "AP or no Medium ip's are not configurable");
      return SUCCESS;
    default:
      ESP_LOGE(Tag, "Invalid medium type - get ip from medium");
      return CONNECTION_GET_NVS_IP_ERROR;
  }

  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error getting ip from medium: %d", retVal);
    return retVal;
  }

  return SUCCESS;

}

/*
 * Function: getNVSIP(const char* key, ip_addr_t* address)
 * ----------------------------
 *  returns the ip address from the nvs as a ip_addr_t
 *
 *  returns: CONNECTION_GET_NVS_IP_ERROR, SUCCESS
 */
uint8_t getNVSIP(const char* key, ip_addr_t* address){
  uint8_t retVal;
  uint32_t tempIp;

  retVal = get_nvs_config(key, DATA_U32, &tempIp);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error getting ip from nvs: %d", retVal);
    return retVal;
  }

  ip_addr_set_ip4_u32(address, tempIp);
  return SUCCESS;

}
/*-------------------- Mac ----------------------------------*/

uint8_t get_mac(uint8_t* mac){
    if(mac == NULL){
        ESP_LOGE(Tag, "Null mac buffer - get mac");
        return ACTION_NULL_DATA_ERROR;
    }

    return esp_efuse_mac_get_default(mac);
}


/*-------------------- Network Connections ----------------------------------*/

/*
 * Function: get_network_connections()
 * ----------------------------
 * 
 *  returns: Connection
 */
uint8_t get_network_connections(){
  return Network_Connections;
}

/*
 * Function: setNetworkConnection()
 * ----------------------------
 *   sets new connection bit
 *
 *   returns: SUCCESS, NETWORK_ERROR
 */
void setNetworkConnection(uint8_t connection){
    Network_Connections |= connection;
    setActiveMedium();
}

/*
 * Function: clearNetworkConnection()
 * ----------------------------
 *   clears old connection bit
 *
 *   returns: SUCCESS, NETWORK_ERROR
 */
void clearNetworkConnection(uint8_t connection){
  Network_Connections &= ~connection;
  setActiveMedium();
}

/*
 * Function: check_network_connection()
 * ----------------------------
 *   checks if the connection is connected
 *
 *   returns: true/false -ish
 */
uint8_t check_network_connection(uint8_t connection){
  return Network_Connections & connection;
}

/*
 * Function: get_device_connected()
 * ----------------------------
 * 
 *  returns: Device_Connected
 */
uint8_t get_device_connected(){
  return Device_Connected;
}

/*
 * Function: get_wifi_error()
 * ----------------------------
 *  Returns Wifi Errors in 
 *  the following order
 * 
 *  Bad WiFi
 *  Bad SSID
 *  Bad Pass
 * 
 *  returns: Wifi Error Code
 */
uint8_t get_wifi_error(){
  if(Bad_Wifi){
    return BAD_WIFI_ERROR;
  } else if(Bad_SSID){
    return BAD_SSID_ERROR;
  } else if(Bad_Password){
    return BAD_PASS_ERROR;
  }

  return WIFI_OK;
}

/*----------------------------- Active Medium -----------------------------------------*/
/*
 * Function: setActiveMedium()
 * ----------------------------
 * 
 *  returns: Sets the active medium through priority
 */
void setActiveMedium(){
  if(check_network_connection(CONNECTION_WIFI)){
    Active_Medium = MEDIUM_WIFI;
  } else if(check_network_connection(CONNECTION_AP)){
    Active_Medium = MEDIUM_AP;
  } else {
    Active_Medium = MEDIUM_NONE;
  }
}

/*
 * Function: get_active_medium()
 * ----------------------------
 * 
 *  returns: Active_Medium
 */
uint8_t get_active_medium(){
  return Active_Medium;
}

/*
 * Function: get_ez_connect_running()
 * ----------------------------
 * 
 *  returns: EZ_Connect_Running
 */
uint8_t get_ez_connect_running(){
  return EZ_Connect_Running;
}

/*
 * Function: get_ez_found_channel()
 * ----------------------------
 * 
 *  returns: EZ_Connect_Running
 */
uint8_t get_ez_found_channel(){
  return EZ_Found_Channel;
}

/*------------------------- Event Handlers -----------------------------------*/

/*
 * Function: blizzardConnectWifi()
 * ----------------------------
 * Starts up EZ connect and trys to establish connection
 * 
 * if there is no valid SSID - start up EZ connect
 * 
 * 
 */
void blizzardConnectWifi(){
  if(!EZ_Connect_Running && !Has_SSID){
    startEZConnect();
  } else if(EZ_Connect_Running){
    if(EZ_Connect_Got_SSID){
      esp_wifi_connect();
    }
  } else {
    esp_wifi_connect();
  }
}

/*
 * Function: blizzardEventHandler(void *ctx, system_event_t *event)
 * ----------------------------
 *  ESP system callback that controlls the connection loop. the idea
 *  is that the BB will always be in a state of connection. If one thing fails
 *  it is this loop's job to reconnect or connect to something else.
 * 
 */
void blizzardEventHandler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    static uint8_t retry = RETRY_ATTEMPTS;

    if(event_base == WIFI_EVENT){
      switch (event_id){
        case WIFI_EVENT_SCAN_DONE: ESP_LOGI(Tag, "Wifi event: WIFI_EVENT_SCAN_DONE"); break;
        case WIFI_EVENT_STA_START:
          ESP_LOGI(Tag, "Wifi event: WIFI_EVENT_STA_START");

          blizzardConnectWifi();

        break;
        case WIFI_EVENT_STA_STOP:
          ESP_LOGI(Tag,"WIFI_EVENT_STA_STOP");
          clearNetworkConnection(CONNECTION_WIFI);
          xEventGroupSetBits(Wifi_Event_Group, Wifi_Disconnected_Bit);
          xEventGroupClearBits(Wifi_Event_Group, Wifi_Connected_Bit);
        break;
        case WIFI_EVENT_STA_CONNECTED: 
          ESP_LOGI(Tag, "Wifi event: WIFI_EVENT_STA_CONNECTED"); 
          break;
        case WIFI_EVENT_STA_DISCONNECTED:
          clearNetworkConnection(CONNECTION_WIFI);
          xEventGroupSetBits(Wifi_Event_Group, Wifi_Disconnected_Bit);
          xEventGroupClearBits(Wifi_Event_Group, Wifi_Connected_Bit);
          wifi_event_sta_disconnected_t* disconnected = (wifi_event_sta_disconnected_t*) event_data;
          switch(disconnected->reason){
            case WIFI_REASON_UNSPECIFIED: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_UNSPECIFIED"); break;
            case WIFI_REASON_AUTH_EXPIRE: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_AUTH_EXPIRE"); break;
            case WIFI_REASON_AUTH_LEAVE: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_AUTH_LEAVE"); break;
            case WIFI_REASON_ASSOC_EXPIRE: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_ASSOC_EXPIRE"); break;
            case WIFI_REASON_ASSOC_TOOMANY: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_ASSOC_TOOMANY"); break;
            case WIFI_REASON_NOT_AUTHED: 
              ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_NOT_AUTHED"); 
              Bad_Wifi = true;
              return;
            case WIFI_REASON_NOT_ASSOCED: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_NOT_ASSOCED"); break;
            case WIFI_REASON_ASSOC_LEAVE: 
              ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_ASSOC_LEAVE"); 
              return;
            case WIFI_REASON_ASSOC_NOT_AUTHED: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_ASSOC_NOT_AUTHED"); break;
            case WIFI_REASON_DISASSOC_PWRCAP_BAD: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_DISASSOC_PWRCAP_BAD"); break;
            case WIFI_REASON_DISASSOC_SUPCHAN_BAD: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_DISASSOC_SUPCHAN_BAD"); break;
            case WIFI_REASON_IE_INVALID: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_IE_INVALID"); break;
            case WIFI_REASON_MIC_FAILURE: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_MIC_FAILURE"); break;
            case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT"); break;
            case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT"); break;
            case WIFI_REASON_IE_IN_4WAY_DIFFERS: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_IE_IN_4WAY_DIFFERS"); break;
            case WIFI_REASON_GROUP_CIPHER_INVALID: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_GROUP_CIPHER_INVALID"); break;
            case WIFI_REASON_PAIRWISE_CIPHER_INVALID: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_PAIRWISE_CIPHER_INVALID"); break;
            case WIFI_REASON_AKMP_INVALID: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_AKMP_INVALID"); break;
            case WIFI_REASON_UNSUPP_RSN_IE_VERSION: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_UNSUPP_RSN_IE_VERSION"); break;
            case WIFI_REASON_INVALID_RSN_IE_CAP: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_INVALID_RSN_IE_CAP"); break;
            case WIFI_REASON_802_1X_AUTH_FAILED: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_802_1X_AUTH_FAILED"); break;
            case WIFI_REASON_CIPHER_SUITE_REJECTED: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_CIPHER_SUITE_REJECTED"); break;
            case WIFI_REASON_INVALID_PMKID: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_INVALID_PMKID"); break;
            case WIFI_REASON_BEACON_TIMEOUT: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_BEACON_TIMEOUT"); break;
            case WIFI_REASON_NO_AP_FOUND: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_NO_AP_FOUND"); break;
            case WIFI_REASON_AUTH_FAIL: 
              ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_AUTH_FAIL"); 
              Bad_Password = true;
              return;
            case WIFI_REASON_ASSOC_FAIL: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_ASSOC_FAIL"); break;
            case WIFI_REASON_HANDSHAKE_TIMEOUT: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_HANDSHAKE_TIMEOUT"); break;
            case WIFI_REASON_CONNECTION_FAIL: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_CONNECTION_FAIL"); break;
            case WIFI_REASON_AP_TSF_RESET: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_AP_TSF_RESET"); break;
            case WIFI_REASON_ROAMING: ESP_LOGE(Tag, "Wifi disconnected: WIFI_REASON_ROAMING"); break;
            default:
              ESP_LOGE(Tag, "Wifi disconnected: reason %d", disconnected->reason);
              Bad_Wifi = true;
              return;
          }

          /*
              If you got here it is a general purpose reconnect
              Ether the driver is still looking for the AP or
              some other error occurred
          */
          //just in case
          if(retry > RETRY_ATTEMPTS){
              retry = RETRY_ATTEMPTS;
          }

          if(retry--){
            ESP_LOGI(Tag, "Trying to reconnect...");
            esp_wifi_connect();
          } else {
            ESP_LOGI(Tag, "Stop trying to reconnect");
            EZ_Connect_Got_SSID = false;
            retry = RETRY_ATTEMPTS;

            switch(disconnected->reason){
              case WIFI_REASON_AUTH_EXPIRE:
                Bad_Password = true;
              break;
              case WIFI_REASON_NO_AP_FOUND:
                Bad_SSID = true;
              break;
              default:
                Bad_Wifi = true;
              break;
            }

            uint8_t retVal;
            uint8_t clear = '\0';

            //clear SSID
            retVal = set_nvs_config(SSID_KEY, DATA_STRING, &clear);
            if(retVal != SUCCESS){
              ESP_LOGE(Tag, "Could not clear SSID in NVS from EZ Connect"); 
              break;
            }

            ESP_LOGI(Tag, "Reset EZ Connect");
            EZ_Connect_Needs_Reset = true;
          }

        break;
      case WIFI_EVENT_STA_AUTHMODE_CHANGE: ESP_LOGI(Tag, "Wifi event: WIFI_EVENT_STA_AUTHMODE_CHANGE"); break;
      case WIFI_EVENT_STA_WPS_ER_SUCCESS: ESP_LOGI(Tag, "Wifi event: WIFI_EVENT_STA_WPS_ER_SUCCESS"); break;
      case WIFI_EVENT_STA_WPS_ER_FAILED: ESP_LOGE(Tag, "Wifi event: WIFI_EVENT_STA_WPS_ER_FAILED"); break;
      case WIFI_EVENT_STA_WPS_ER_TIMEOUT: ESP_LOGE(Tag, "Wifi event: WIFI_EVENT_STA_WPS_ER_TIMEOUT"); break;
      case WIFI_EVENT_STA_WPS_ER_PIN: ESP_LOGI(Tag, "Wifi event: WIFI_EVENT_STA_WPS_ER_PIN"); break;
      case WIFI_EVENT_STA_WPS_ER_PBC_OVERLAP: ESP_LOGI(Tag, "Wifi event: WIFI_EVENT_STA_WPS_ER_PBC_OVERLAP"); break;
      case WIFI_EVENT_AP_START: 
        ESP_LOGI(Tag, "Wifi event: WIFI_EVENT_AP_START"); 
        setNetworkConnection(CONNECTION_AP);
        xEventGroupSetBits(Wifi_Event_Group, Wifi_Connected_Bit);
        xEventGroupClearBits(Wifi_Event_Group, Wifi_Disconnected_Bit);
        break;
      case WIFI_EVENT_AP_STOP: 
        ESP_LOGE(Tag, "Wifi event: WIFI_EVENT_AP_STOP"); 
        clearNetworkConnection(CONNECTION_AP);
        xEventGroupSetBits(Wifi_Event_Group, Wifi_Disconnected_Bit);
        xEventGroupClearBits(Wifi_Event_Group, Wifi_Connected_Bit);
        break;
      case WIFI_EVENT_AP_STACONNECTED: 
        ESP_LOGI(Tag, "Wifi event: WIFI_EVENT_AP_STACONNECTED"); 
        Device_Connected = true;
        break;
      case WIFI_EVENT_AP_STADISCONNECTED: 
        ESP_LOGE(Tag, "Wifi event: WIFI_EVENT_AP_STADISCONNECTED"); 
        Device_Connected = false;
        break;
      case WIFI_EVENT_AP_PROBEREQRECVED: ESP_LOGI(Tag, "Wifi event: WIFI_EVENT_AP_PROBEREQRECVED"); break;
      case WIFI_EVENT_FTM_REPORT: ESP_LOGI(Tag, "Wifi event: WIFI_EVENT_FTM_REPORT - Device Connected?"); break;
      /*case 17:
        ESP_LOGI(Tag, "Device Connected");
        Device_Connected = true;
        break;*/
      case WIFI_EVENT_STA_BSS_RSSI_LOW: ESP_LOGE(Tag, "Wifi event: WIFI_EVENT_STA_BSS_RSSI_LOW"); break;
      case WIFI_EVENT_ACTION_TX_STATUS: ESP_LOGI(Tag, "Wifi event: WIFI_EVENT_ACTION_TX_STATUS"); break;
      case WIFI_EVENT_ROC_DONE: ESP_LOGI(Tag, "Wifi event: WIFI_EVENT_ROC_DONE"); break;
      case WIFI_EVENT_STA_BEACON_TIMEOUT: ESP_LOGE(Tag, "Wifi event: WIFI_EVENT_STA_BEACON_TIMEOUT"); break;
      default:
        ESP_LOGE(Tag, "Other connection wifi event %d", event_id);
        break;
      }
    } else if(event_base == IP_EVENT){
      switch (event_id){
        case IP_EVENT_STA_GOT_IP: 
          ESP_LOGI(Tag, "IP event: IP_EVENT_STA_GOT_IP"); 
          Bad_SSID = false;
          Bad_Password = false;
          Bad_Wifi = false;

          setNetworkConnection(CONNECTION_WIFI);
          xEventGroupSetBits(Wifi_Event_Group, Wifi_Connected_Bit);
          xEventGroupClearBits(Wifi_Event_Group, Wifi_Disconnected_Bit);
          retry = RETRY_ATTEMPTS;
          break;
        case IP_EVENT_STA_LOST_IP: ESP_LOGE(Tag, "IP event: IP_EVENT_STA_LOST_IP"); break;
        case IP_EVENT_AP_STAIPASSIGNED: ESP_LOGI(Tag, "IP event: IP_EVENT_AP_STAIPASSIGNED"); break;
        case IP_EVENT_GOT_IP6: ESP_LOGI(Tag, "IP event: IP_EVENT_GOT_IP6"); break;
        case IP_EVENT_ETH_GOT_IP: ESP_LOGI(Tag, "IP event: IP_EVENT_ETH_GOT_IP"); break;
        case IP_EVENT_PPP_GOT_IP: ESP_LOGI(Tag, "IP event: IP_EVENT_PPP_GOT_IP"); break;
        case IP_EVENT_PPP_LOST_IP: ESP_LOGI(Tag, "IP event: IP_EVENT_PPP_LOST_IP"); break;
        default: ESP_LOGE(Tag, "Other connection ip event %d", event_id); break;
      }
    } else if(event_base == SC_EVENT){
      switch (event_id){
        case SC_EVENT_SCAN_DONE: ESP_LOGI(Tag, "SC event: SC_EVENT_SCAN_DONE"); break;
        case SC_EVENT_FOUND_CHANNEL: 
          EZ_Found_Channel = true;
          ESP_LOGI(Tag, "SC event: SC_EVENT_FOUND_CHANNEL"); 
          break;
        case SC_EVENT_GOT_SSID_PSWD: 
          ESP_LOGI(Tag, "SC event: SC_EVENT_GOT_SSID_PSWD"); 
          smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
          uint8_t retVal;
          uint8_t ssid[MAX_EZ_CONNECT_SSID_SIZE] = { 0 };
          uint8_t password[MAX_EZ_CONNECT_PASS_SIZE] = { 0 };

          memcpy(ssid, evt->ssid, sizeof(evt->ssid));
          memcpy(password, evt->password, sizeof(evt->password));

          //To Disable @Full Check turn bypass to TRUE
          if(ezConnectDetectAtFull(ssid, password, FALSE)){
            ESP_LOGI(Tag, "Got SSID: %s", ssid);
            ESP_LOGI(Tag, "Got PASSWORD: %s", password);

            retVal = set_nvs_config(SSID_KEY, DATA_STRING, ssid);
            if(retVal != SUCCESS){
              ESP_LOGE(Tag, "Could not set SSID in NVS from EZ Connect"); 
              break;
            }

            retVal = set_nvs_config(PASS_KEY, DATA_STRING, password);
            if(retVal != SUCCESS){
              ESP_LOGE(Tag, "Could not set PASS in NVS from EZ Connect"); 
              break;
            }

            EZ_Connect_Got_SSID = true;
            reconnect_wifi();
          } else {
            ESP_LOGE(Tag, "Not From @Full, SSID: %s", ssid);
            ESP_LOGE(Tag, "Not From @Full, PASSWORD: %s", password);
          }
          break;
        case SC_EVENT_SEND_ACK_DONE: 
          ESP_LOGI(Tag, "SC event: SC_EVENT_SEND_ACK_DONE - EZ Connect Done"); 

          if(EZ_Connect_Running){
            EZ_Connect_Running = false;
            EZ_Connect_Got_SSID = false;
            EZ_Found_Channel = false;
            esp_smartconfig_stop();
          }
          break;
        default:
          ESP_LOGE(Tag, "Other connection sc event %d", event_id);
          break;
      }
    } else {
      ESP_LOGE(Tag, "Other connection base event");
    }

}

/*
 * Function: ezConnectDetectAtFull(uint8_t* ssid, uint8_t* pass, uint8_t bypass)
 * ----------------------------
 * Checks if the ez connect is coming from @Full
 * 
 * returns 0 on no, 1 on yes
 * 
 */
uint8_t ezConnectDetectAtFull(uint8_t* ssid, uint8_t* pass, uint8_t bypass){
  uint8_t i = 0, lookahead = 0, isFromAtFull = 0;

  if(bypass) return bypass;

  //Blizzard Parse Out SSID
  for(i = 0; i < MAX_EZ_CONNECT_SSID_SIZE; i++){
    if(i != 0){
      if(lookahead == 'B'){
        if(ssid[i] == '\0'){
          ssid[i-1] = '\0';
          isFromAtFull = 1;
          break;
        }
      }
    }
    lookahead = ssid[i];
  }

  //Blizzard Parse Out Pass
  if(isFromAtFull){
    isFromAtFull = 0;
    for(i = 0; i < MAX_EZ_CONNECT_PASS_SIZE; i++){
      if(i != 0){
        if(lookahead == 'L'){
          if(pass[i] == '\0'){
            pass[i-1] = '\0';
            isFromAtFull = 1;
            break;
          }
        }
      }
      lookahead = pass[i];
    }
  }
  return isFromAtFull;
}


/*--------------------------------- Debug ------------------------------------*/
/*
 * Function: dump_network_information(void)
 * ----------------------------
 *  shows all connection information 
 * 
 */
void dump_network_information()
{
  esp_netif_ip_info_t ip;

  // esp_netif_config_t cfg = ESP_NETIF_DEFAULT_WIFI_STA();
  // esp_netif_t *ESP_NETIF_IF_STA = esp_netif_new(&cfg);

  // esp_netif_config_t cfg1 = ESP_NETIF_DEFAULT_WIFI_AP();
  // esp_netif_t *ESP_NETIF_IF_AP = esp_netif_new(&cfg);

  printf("~~Connections~~\n");
  printf("CURRENT CONNECTION: %s\n", medium_to_string(Active_Medium));
  printf("WIFI: %s\n", check_network_connection(CONNECTION_WIFI) ? "Connected" : "Disconnected");
  printf("WIFI AP: %s\n", check_network_connection(CONNECTION_AP) ? "Connected" : "Disconnected");
  printf("~~~~~~~~~~~\n");

  memset(&ip, 0, sizeof(esp_netif_ip_info_t));
  if (esp_netif_get_ip_info(ESP_NETIF_IF_STA, &ip) == 0) {
      printf("~~~~WIFI~~~\n");
      printf("IP:"IPSTR"\n", IP2STR(&ip.ip));
      printf("Netmask:"IPSTR"\n", IP2STR(&ip.netmask));
      printf("Gateway:"IPSTR"\n", IP2STR(&ip.gw));
      printf("~~~~~~~~~~~\n");
  }

  memset(&ip, 0, sizeof(esp_netif_ip_info_t));
  if (esp_netif_get_ip_info(ESP_NETIF_IF_AP, &ip) == 0) {
      printf("~~~~~AP~~~~\n");
      printf("IP:"IPSTR"\n", IP2STR(&ip.ip));
      printf("Netmask:"IPSTR"\n", IP2STR(&ip.netmask));
      printf("Gateway:"IPSTR"\n", IP2STR(&ip.gw));
      printf("~~~~~~~~~~~\n");
  }

  // memset(&ip, 0, sizeof(esp_netif_ip_info_t));
  // if (esp_netif_get_ip_info(ESP_IF_ETH, &ip) == 0) {
  //     printf("~~~~ETH~~~~\n");
  //     printf("IP:"IPSTR"\n", IP2STR(&ip.ip));
  //     printf("Netmask:"IPSTR"\n", IP2STR(&ip.netmask));
  //     printf("Gateway:"IPSTR"\n", IP2STR(&ip.gw));
  //     printf("~~~~~~~~~~~\n");
  // }
}

/*
 * Function: medium_to_string(uint8_t medium)
 * ----------------------------
 *  takes a medium and returns a string representation
 * 
 */
const char * medium_to_string(uint8_t medium){

  if(medium == MEDIUM_ACTIVE){
    medium = Active_Medium;
  }

  switch(medium){
    case MEDIUM_ETHERNET:
      return "Ethernet";
    case MEDIUM_WIFI:
      return "Wi-Fi";
    case MEDIUM_AP:
      return "AP";
    default:
      return "None";
  }

  return "Well this is hawkward";
}

/*-------------------------------------------- Backlog -----------------------------------*/
/* Save for later?
 * Function: set_network_defaults()
 * ----------------------------
 *  
 *  Sets network defaults based on light bytes ip addressing 
 *  formula - subnet goes to 255.0.0.0
 * 
 *  ip: a.b.c.d
 *  mac: u:v:w:x:y:z
 *  oem: h-l
 *  a = 2
 *  b = x + h + l
 *  c = y
 *  d = z
 * 
 *  returns: SUCCESS, other wifi related problems
 */
/*uint8_t set_network_defaults(void){
  esp_err_t err;
  ip_addr_t ips[5];
  uint8_t retVal, mac[MAX_MAC_SIZE];

  err = action_get_mac(mac);
  if(err != ESP_OK){
    ESP_LOGE(Tag, "Error getting mac - set network defaults: %s", errorToString(err));
    return err;
  }

  //Ethernet
  IP_ADDR4(&(ips[0]), 2, (mac[3] + ART_OEM_HI + ART_OEM_LO), mac[4], mac[5]);
  IP_ADDR4(&(ips[1]), 2, (mac[3] + ART_OEM_HI + ART_OEM_LO), mac[4], 1);

  //Wifi
  IP_ADDR4(&(ips[2]), 3, (mac[3] + ART_OEM_HI + ART_OEM_LO), mac[4], mac[5]);
  IP_ADDR4(&(ips[3]), 3, (mac[3] + ART_OEM_HI + ART_OEM_LO), mac[4], 1);

  //Netmask
  IP_ADDR4(&(ips[4]), 0xFF, 0xFF, 0xFF, 0);

  // Ethernet Defaults 
  retVal = change_ip(MEDIUM_ETHERNET, IP_IP, &(ips[0]));
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting eth ip - set network defaults: %d", retVal);
    return retVal;
  }

  retVal = change_ip(MEDIUM_ETHERNET, IP_NETMASK, &(ips[4]));
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting eth netmask - set network defaults: %d", retVal);
    return retVal;
  }

  retVal = change_ip(MEDIUM_ETHERNET, IP_GATEWAY, &(ips[1]));
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting eth gateway - set network defaults: %d", retVal);
    return retVal;
  }

  retVal = change_dhcp_enable(DISABLE, MEDIUM_ETHERNET);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error disabling ethernet dhcp - set network defaults: %d", retVal);
    return retVal;
  }

  // Wifi Defaults 
  retVal = change_ip(MEDIUM_WIFI, IP_IP, &(ips[2]));
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting wifi ip - set network defaults: %d", retVal);
    return retVal;
  }

  retVal = change_ip(MEDIUM_WIFI, IP_NETMASK, &(ips[4]));
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting wifi netmask - set network defaults: %d", retVal);
    return retVal;
  }

  retVal = change_ip(MEDIUM_WIFI, IP_GATEWAY, &(ips[3]));
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting wifi gateway - set network defaults: %d", retVal);
    return retVal;
  }

  retVal = change_dhcp_enable(DISABLE, MEDIUM_WIFI);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error disabling wifi dhcp - set network defaults: %d", retVal);
    return retVal;
  }

  return SUCCESS;

}*/

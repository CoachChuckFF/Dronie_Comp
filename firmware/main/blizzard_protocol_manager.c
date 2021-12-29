/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Protocol Manager -> blizzard_protocol_manager.c
 *
 * This is the high level manager for all of the lighting protocols:
 * Artnet sACN and DMX. It starts each sub manager and can controls the
 * active protocol; The Active protocol only dictates how DMX is received - 
 * either Artnet or sACN.
 * 
 */

#include "blizzard_protocol_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "blizzard_nvs_manager.h"
#include "blizzard_dmx_manager.h"
#include "blizzard_rdm_manager.h"
#include "blizzard_artnet_manager.h"
#include "blizzard_sacn_manager.h"
#include "blizzard_show_manager.h"

static const char *Tag = "PROTOCOL";

uint8_t Protocol = PROTOCOL_NONE;

/*
 * Function: init_protocol_manager(void)
 * ----------------------------
 *  starts up the protocol manager and sets it to user defined defaults
 *
 *  returns: PROTOCOL_GET_ACTIVE_PROTOCOL_ERROR PROTOCOL_INIT_ERROR 
 */
uint8_t init_protocol_manager(void) {
  uint8_t retVal = SUCCESS;

  ESP_LOGI(Tag, "Protocols are getting spiffy...");

  retVal = get_nvs_config(ACTIVE_PROTOCOL_KEY, DATA_U8, &Protocol);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error getting active protocol - protocol init");
    return retVal;
  }

  /*---------------------- DMX SETUP -----------------------------------------*/
  retVal = init_dmx_manager();
  if(retVal != SUCCESS){
    return retVal;
  }
  start_dmx(SEND);

  /*---------------------- RDM SETUP -----------------------------------------*/
  retVal = init_rdm_manager();
  if(retVal != SUCCESS){
    return retVal;
  }

  /*------------------ ARNTET SETUP ------------------------------------------*/
  retVal = init_artnet_manager();
  if(retVal != SUCCESS){
    return retVal;
  }

  retVal = start_artnet();
  if(retVal != SUCCESS){
    return retVal;
  }

  /*------------------ SACN SETUP --------------------------------------------*/
  retVal = init_sacn_manager();
  if(retVal != SUCCESS){
    return retVal;
  }

  retVal = start_sacn();
  if(retVal != SUCCESS){
    return retVal;
  }
  
  /*------------------ GENERAL SETUP -----------------------------------------*/
  retVal = change_active_protocol(Protocol);
  if(retVal != SUCCESS){
    return retVal;
  }

  ESP_LOGI(Tag, "Protocols are ready for prom!");
  return SUCCESS;
}
 
/*
* Function: change_active_protocol(uint8_t protocol)
* ----------------------------
*  changes the active protocol and starts and stops 
*  Artnet or sACN accordingly
*
*  returns: PROTOCOL_UNKOWN_ERROR NVS Errors SUCCESS
*/
uint8_t change_active_protocol(uint8_t protocol){
uint8_t retVal = SUCCESS;

if(protocol == PROTOCOL_REVERT){
  protocol = get_active_protocol();
}

switch(protocol) {
  case PROTOCOL_ARTNET:
    stop_sacn();
    retVal = start_artnet();
    if(retVal != SUCCESS){
      return retVal;
    }
    start_dmx(SEND);
  break;
  case PROTOCOL_SACN:
    stop_artnet();
    retVal = start_sacn();
    if(retVal != SUCCESS){
      return retVal;
    }
    start_dmx(SEND);
  break;
  case PROTOCOL_READ:
    // stop_artnet();
    // stop_sacn();
    start_dmx(RECEIVE);
    protocol = get_active_protocol();
  break;
  default:
    ESP_LOGE(Tag, "Not a valid protocol: %d", protocol);
    return PROTOCOL_UNKOWN_ERROR;
}

retVal = set_nvs_config(ACTIVE_PROTOCOL_KEY, DATA_U8, &protocol);
if(retVal != SUCCESS){
  ESP_LOGE(Tag, "Error setting active protocol");
  return retVal;
}

Protocol = protocol;

return retVal;
}

void start_listening(){
  stop_show();
  change_active_protocol(PROTOCOL_READ);
}

void stop_listening(){
  change_active_protocol(PROTOCOL_REVERT);
}

/*
* Function: get_active_protocol()
* ----------------------------
*  returns: Active Protocol
*/
uint8_t get_active_protocol(){
  return Protocol;
}
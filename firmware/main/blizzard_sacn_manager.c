/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Protocol Manager -> blizzard_sacn_manager.c
 *
 */

#include "blizzard_sacn_manager.h"
#include "blizzard_simple_ui_manager.h"
#include "blizzard_show_manager.h"
#include "blizzard_global_defines.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/udp.h"
#include "lwip/ip4_addr.h"
#include "lwip/ip6_addr.h"
#include "esp_wifi.h"
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
// #include "tcpip_adapter.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "blizzard_nvs_manager.h"
#include "blizzard_dmx_manager.h"

static const char *Tag = "sACN";

uint16_t Sacn_Universe; 
SACNDataPacket Packet;
bool Enabled = false;
int Socket = -1;

const uint8_t Sacn_Id[] = SACN_ID;

/*---------------------------- Protocol Functions ----------------------------*/

/*
 * Function: init_sacn_manager()
 * ----------------------------
 *  inits the sacn manager
 *
 *  returns: SUCCESS
 */
uint8_t init_sacn_manager(){
  uint8_t retVal = SUCCESS;

  ESP_LOGI(Tag, "sACN manager strapping up...");

  retVal = get_nvs_config(SACN_UNIVERSE_KEY, DATA_U16, &Sacn_Universe);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error getting sACN universe");
    return retVal;
  }

  ESP_LOGI(Tag, "sACN strapped!");
  return retVal;
}

/*
 * Function: tick_sACN()
 * ----------------------------
 *  looks for incoming packets
 *
 *  returns: void
 */
void tick_sacn(){
  if(Enabled){

    struct sockaddr_in6 raddr; // Large enough for both IPv4 or IPv6
    socklen_t socklen = sizeof(raddr);

    if(recvfrom(Socket, (uint8_t*) &Packet, sizeof(SACNDataPacket), 0,
                        (struct sockaddr *)&raddr, &socklen) > 0){
                          receivesACN();
                        }
  }
}

/*
 * Function: start_sacn()
 * ----------------------------
 *  binds the manager to the udp port
 *
 *  returns: SACN_NEW_UDP_ERROR SACN_BIND_ERROR   
 */
uint8_t start_sacn(){

  if(!Enabled){
    Enabled = true;

    createMulticastSocket();

    ESP_LOGI(Tag, "sACN started");
  }

  return SUCCESS;
}

/*
 * Function: stop_sacn()
 * ----------------------------
 *  unbinds the sACN manager
 *
 *  returns: void
 */
void stop_sacn(void){
  if(Enabled){
    Enabled = false;
    shutdown(Socket, 0);
    close(Socket);
    //TODO stop socket
  }

  ESP_LOGI(Tag, "sACN stopped");
}

/*
 * Function: createMulticastSocket()
 * ----------------------------
 *  creates the multicast Socket
 *
 *  returns: void
 */
void createMulticastSocket()
{
  Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //IPPROTO_UDP
  if(Socket < 0){
    ESP_LOGE(Tag, "Could not open sACN Socket");
    return;
  }

  //Bind
  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(SACN_PORT);
  if (bind(Socket, (struct sockaddr *) &serv_addr, (socklen_t)sizeof(serv_addr)) < 0){
      ESP_LOGE(Tag, "Could not bind sACN Socket");
      return;
  }

  //Stop Blocking
  if(fcntl(Socket, F_SETFL, fcntl(Socket, F_GETFL, 0) | O_NONBLOCK) != 0){
    ESP_LOGE(Tag, "Failed to stop sACN blocking");
    return;
  }

  //TODO get sacn universe
  struct ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = universeToIP();
  mreq.imr_interface.s_addr = INADDR_ANY;
  if (setsockopt(Socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0){
      ESP_LOGE(Tag, "Could not add multicast membership");
      return;
  }

}

/*
 * Function: change_sacn_universe(uint16_t universe)
 * ----------------------------
 *  tryies to set the sacn universe
 *
 *  returns: SACN_LARGE_UNIVERSE_ERROR SUCCESS
 */
uint8_t change_sacn_universe(uint16_t universe){
  uint8_t retVal = SUCCESS;

  if(universe > MAX_SACN_UNIVERSE){
    ESP_LOGE(Tag, "Error setting sACN universe: universe %d is too big. max: %d", universe, MAX_SACN_UNIVERSE);
    return SACN_LARGE_UNIVERSE_ERROR;
  }

  retVal = set_nvs_config(SACN_UNIVERSE_KEY, DATA_U16, &universe);

  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting sACN universe: %d", retVal);
    return retVal;
  }

  Sacn_Universe = universe;

  //restart sACN
  if(Enabled){
    stop_sacn();
    start_sacn();
  }

  ESP_LOGI(Tag, "sACN universe set to %d", Sacn_Universe);

  return retVal;
}

uint16_t get_sacn_universe(){
  return Sacn_Universe;
}

/*Internal*/

/*
 * Function: uint32_t universeToIP()
 * ----------------------------
 *  callback for sACN messages
 *
 *   returns: multicast ip from sACN universe
 */
uint32_t universeToIP(){
  uint32_t ip = SACN_BASE_IP;
  uint32_t b0,b1,b2,b3, final;

  if(get_sacn_universe() == 0){
    ip += 1;
  } else {
    ip += get_sacn_universe();
  }

  b0 = (ip & 0x000000ff) << 24u;
  b1 = (ip & 0x0000ff00) << 8u;
  b2 = (ip & 0x00ff0000) >> 8u;
  b3 = (ip & 0xff000000) >> 24u;

  final = b0 | b1 | b2 | b3;

  return final;
}

/*
 * Function: receivesACN()
 * ----------------------------
 *  callback for sACN messages
 *
 *   returns: void
 */
void receivesACN(){
  uint8_t i;

  for(i = 0; i < sizeof(Sacn_Id); i++)
  {
    if((Packet._acn_id)[i] != Sacn_Id[i])
    {
      ESP_LOGE(Tag, "Invalid sACN packet: Bad ID - Naughty");
      return;
    }
  }

  if(Enabled && get_show_state() == SHOW_STATE_STOP){
    parsesACNDataPacket();
  }
}

/*
 * Function: parsesACNDataPacket()
 * ----------------------------
 *  parses in the data from the sACN packet
 *
 *  returns: void
 */
uint8_t parsesACNDataPacket(){
  uint8_t retVal = SUCCESS;

  retVal = copy_to_dmx(((uint8_t*)Packet._dmx_data) + 1);

  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "sACN dmx parse error");
  }

  #ifdef CONFIG_DEVICE_SOC
    tick_data();
  #endif

  return retVal;
}

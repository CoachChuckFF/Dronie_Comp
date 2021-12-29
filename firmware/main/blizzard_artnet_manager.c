/* Copyright (C) initDiscoveryPacket Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Protocol Manager -> blizzard_artnet_manager.c
 *
 */

#include "blizzard_artnet_manager.h"
#include "blizzard_dronie_manager.h"
#include "blizzard_battery_manager.h"
#include "blizzard_simple_ui_manager.h"
#include "blizzard_ws2812_manager.h"
//TODO remove
#include "blizzard_debuggers.h"
#include "blizzard_dmx_manager.h"
#include "blizzard_nvs_manager.h"
#include "blizzard_protocol_manager.h"
#include "blizzard_sacn_manager.h"
#include "blizzard_network_manager.h"
#include "blizzard_ota_manager.h"
#include "blizzard_json_manager.h"
#include "blizzard_show_manager.h"
#include "blizzard_rdm_manager.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/udp.h"
#include "lwip/ip4_addr.h"
#include "lwip/ip6_addr.h"
#include "esp_wifi.h"

#include "blizzard_global_defines.h"

static const char *Tag = "ARTNET";

uint8_t Enabled;
uint8_t Net;
uint8_t Subnet;
uint8_t Universe;

ArtQueType Que[CONFIG_ARTNET_QUE_SIZE + 1];
uint8_t Art_Que_Count;
uint8_t Art_Que_Head_Index;
uint8_t Art_Que_Tail_Index;

SemaphoreHandle_t Mutex;

//return packets
ArtnetPollReplyPacket Poll_Reply;
DroniePacketReply Dronie_Reply;

const uint8_t Art_Id[] = ART_ID;
struct udp_pcb *Art_Udp = NULL;

/*---------------------------- Protocol Functions ----------------------------*/
/*
 * Function: init_artnet_manager()
 * ----------------------------
 *  Builds the ARTNET struct
 *
 *   returns: ARTNET_GET_MUTEX_ERROR NVS errors SUCCESS
 */
 uint8_t init_artnet_manager(){
  uint8_t retVal = SUCCESS;

  Art_Udp = NULL;
  Enabled = DISABLE;

  Art_Que_Count = 0;
  Art_Que_Head_Index = 1;
  Art_Que_Tail_Index = 0;

  ESP_LOGI(Tag, "Artnet is dragon-born");


  retVal = get_nvs_config(ARTNET_NET_KEY, DATA_U8, &Net);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error getting artnet net");
    return retVal;
  }

  retVal = get_nvs_config(ARTNET_SUBNET_KEY, DATA_U8, &Subnet);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error getting artnet subnet");
    return retVal;
  }

  retVal = get_nvs_config(ARTNET_UNIVERSE_KEY, DATA_U8, &Universe);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error getting artnet universe");
    return retVal;
  }

  Mutex = xSemaphoreCreateMutex();
  if( Mutex == NULL ){
    ESP_LOGE(Tag, "Error getting artnet universe");
    return ARTNET_GET_MUTEX_ERROR;
  }

  ESP_LOGI(Tag, "Artnet FUS RO DAH!");
  return SUCCESS;
 }

 /*
  * Function: start_artnet()
  * ----------------------------
  *  binds the udp
  *
  *  returns: ARTNET_NEW_UDP_ERROR ARTNET_BIND_ERROR SUCCESS
  */
uint8_t start_artnet(){
  err_t retVal;
  Enabled = ENABLE;

  if(Art_Udp == NULL){

    Art_Udp = udp_new();
    if(Art_Udp == NULL){
      ESP_LOGE(Tag, "Error getting new udp struct - artnet");
      return ARTNET_NEW_UDP_ERROR;
    }
    
    retVal = udp_bind(Art_Udp, IP_ADDR_ANY, ART_PORT);
    if(retVal != ERR_OK){
      ESP_LOGI(Tag, "Artnet bind failed: %d", retVal);
      return ARTNET_BIND_ERROR;
    }

    udp_recv(Art_Udp, receiveArtnet, NULL);
  }

  ESP_LOGI(Tag, "Artnet started");
  return SUCCESS;
}

/*
* Function: stop_artnet()
* ----------------------------
*   unbinds udp
*
*   returns: void
*/
void stop_artnet() {
  Enabled = DISABLE;

  Art_Que_Count = 0;
  Art_Que_Head_Index = 1;
  Art_Que_Tail_Index = 0;

  ESP_LOGI(Tag, "Artnet stopped");
}

/*
* Function: tick_artnet()
* ----------------------------
*   increments merge timout and sync ticks
*   as well as send poll reply packets to 
*   ip addresses in Merge if something has
*   changed if talk to me is set correctly
*
*   returns: void
*/
void tick_artnet(void){

  //TODO delete and restart socket on connection

  if(!emptyQue()){
    handlePackets();
  }

}


/*
 * Function: change_artnet_net(uint8_t net)
 * ----------------------------
 *  tries to set the artnet net
 *
 *  returns: ARTNET_NET_ERROR SUCCESS
 */
uint8_t change_artnet_net(uint8_t net){
  uint8_t retVal = SUCCESS;
 
  net &= ~0x80; //clears top bit

  retVal = set_nvs_config(ARTNET_NET_KEY, DATA_U8, &net);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting artnet net: %d", retVal);
    return retVal;
  }

  Net = net;

  ESP_LOGI(Tag, "Artnet net set to %d", Net);

  return retVal;
}

/*
 * Function: get_artnet_net()
 * ----------------------------
 *
 *  returns: Net
 */
uint8_t get_artnet_net(){
  return Net;
}

/*
 * Function: change_artnet_subnet(uint8_t subnet)
 * ----------------------------
 *  tries to set the artnet subnet
 *
 *  returns: ARTNET_SUBNET_ERROR SUCCESS
 */
uint8_t change_artnet_subnet(uint8_t subnet){
  uint8_t retVal = SUCCESS;

  subnet &= ~0xF0; //clears top 4 bits

  retVal = set_nvs_config(ARTNET_SUBNET_KEY, DATA_U8, &subnet);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting artnet subnet: %d", retVal);
    return retVal;
  }

  Subnet = subnet;

  ESP_LOGI(Tag, "Artnet subnet set to %d", Subnet);

  return retVal;
}

/*
 * Function: get_artnet_subnet()
 * ----------------------------
 *
 *  returns: Subnet
 */
uint8_t get_artnet_subnet(){
  return Subnet;
}

/*
 * Function: change_artnet_universe(uint8_t universe)
 * ----------------------------
 *  tries to set the artnet universe
 *
 *  returns: ARTNET_SUBNET_ERROR SUCCESS
 */
uint8_t change_artnet_universe(uint8_t universe){
  uint8_t retVal = SUCCESS;

  universe &= ~0xF0; //clears top 4 bits

  retVal = set_nvs_config(ARTNET_UNIVERSE_KEY, DATA_U8, &universe);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error setting artnet universe: %d", retVal);
    return retVal;
  }

  Universe = universe;

  ESP_LOGI(Tag, "Artnet universe set to %d", Universe);

  return retVal;
}

/*
 * Function: get_artnet_universe()
 * ----------------------------
 *
 *  returns: Universe
 */
uint8_t get_artnet_universe(){
  return Universe;
}

/*
 * Function: build_artnet_universe()
 * ----------------------------
 *
 *  returns: Uses Net/Subnet/Universe -> universe
 */
uint16_t build_artnet_universe(){
  uint16_t retVal = 0;

  retVal |= ((uint16_t)Net) << 8;
  retVal |= ((uint16_t)Subnet) << 4;
  retVal |= ((uint16_t)Universe);

  return retVal;
}

/*
* Function: receiveArtnet(...)
* ----------------------------
*   handler for incoming udp packets, it will parse and call
*   appropriate functions
*
*   returns: void
*/
void receiveArtnet(void *arg,
                  struct udp_pcb *pcb,
                  struct pbuf *packet,
                  const ip_addr_t *address,
                  u16_t port)
{  
  pushToQue(packet, address);

  pbuf_free(packet);
}
 
uint8_t pushToQue(struct pbuf *packet, const ip_addr_t *address){
  bool overwrite = false;

  if( xSemaphoreTake( Mutex, ( TickType_t ) CONFIG_ARTNET_PUSH_WAIT ) == pdTRUE ) {
    
    if(++Art_Que_Count > CONFIG_ARTNET_QUE_SIZE){
      Art_Que_Count = CONFIG_ARTNET_QUE_SIZE;
      overwrite = true;
    }

    //wrap around
    if(++Art_Que_Tail_Index >= CONFIG_ARTNET_QUE_SIZE){
      Art_Que_Tail_Index = 0;
    }

    if(overwrite){
      if(++Art_Que_Head_Index >= CONFIG_ARTNET_QUE_SIZE){
        Art_Que_Head_Index = 0;
      }
    }

    Que[Art_Que_Tail_Index].len = (packet->len > CONFIG_ARTNET_MAX_PACKET_SIZE) ? CONFIG_ARTNET_MAX_PACKET_SIZE : packet->len;
    
    memcpy(Que[Art_Que_Tail_Index].packet, packet->payload, Que[Art_Que_Tail_Index].len);
    memcpy(&(Que[Art_Que_Tail_Index].address), address, sizeof(ip_addr_t));
    

    xSemaphoreGive( Mutex );

    return SUCCESS;
  }

  return ARTNET_QUE_PUSH_ERROR;
}

bool emptyQue(){
  return (!Art_Que_Count) ? true : false;
}

ArtQueType* popFromQue(){
  uint8_t popIndex;

  if(Art_Que_Count){
    if( xSemaphoreTake( Mutex, ( TickType_t ) CONFIG_ARTNET_POP_WAIT ) == pdTRUE ) {

      popIndex = Art_Que_Head_Index;

      if(++Art_Que_Head_Index >= CONFIG_ARTNET_QUE_SIZE){
        Art_Que_Head_Index = 0;
      }

      Art_Que_Count--;

      memcpy(&(Que[CONFIG_ARTNET_QUE_SIZE]), &(Que[popIndex]), sizeof(ArtQueType));

      xSemaphoreGive( Mutex );

      return &(Que[CONFIG_ARTNET_QUE_SIZE]);
    }
  }

  return NULL;
}


/*
* Function: receiveArtnet(...)
* ----------------------------
*   handler for incoming udp packets, it will parse and call
*   appropriate functions
*
*   returns: void
*/
void handlePackets()
{

  ArtQueType *item;

  item = popFromQue();

  if(item == NULL){
    return;
  }

  uint16_t i, opcode;

  for(i = 0; i < sizeof(Art_Id); i++)
  {
    if(item->packet[i] != Art_Id[i])
    {
      ESP_LOGE(Tag, "Invalid artnet packet");
      return;
    }
  }

  opcode = ((uint16_t*)item->packet)[ART_OPCODE_U16_INDEX];

  switch(opcode)
  {
    case DRONIE_OP_COMMAND:
      if(parseDronieCommandPacket((DronieCommandPacket*)item->packet) != SUCCESS) return;

      ESP_LOGE(Tag, "Dronie Command (%d - %d)", ((DronieCommandPacket*)item->packet)->_command, ((DronieCommandPacket*)item->packet)->_value);

      if(createDronieReplyPacket() != SUCCESS) return;

      sendPacket(&(item->address), sizeof(ArtnetPollReplyPacket), (uint8_t*)&Dronie_Reply);
      break;
    case DRONIE_OP_POLL:

      if(createDronieReplyPacket() != SUCCESS) return;

      sendPacket(&(item->address), sizeof(DroniePacketReply), (uint8_t*)&Dronie_Reply);
      //ESP_LOGI(Tag, "Sent Reply: %04X", Poll_Reply._opcode);
      break;
    case ART_OP_POLL:
      ESP_LOGE(Tag, "Poll Packet");

      if(createArtnetPollReplyPacket() != SUCCESS)  return;

      sendPacket(&(item->address), sizeof(ArtnetPollReplyPacket), (uint8_t*)&Poll_Reply);
      //ESP_LOGI(Tag, "Sent Reply: %04X", Poll_Reply._opcode);
      break;
    default:
      ESP_LOGE(Tag, "Unsupported artnet packet: %04X", opcode);
  }
}

uint8_t parseDronieCommandPacket(DronieCommandPacket* packet){
  if(packet == NULL) return ERROR_ERROR;

  switch(packet->_command){
    case DRONIE_SET_MODE: 
      set_dronie_mode(packet->_value);
    break;
    case DRONIE_SET_LED: 
      switch (packet->_value)
      {
      case 0: set_dronie_led(0, &(packet->_color_0)); break;
      case 1: set_dronie_led(1, &(packet->_color_1)); break;
      case 2: set_dronie_led(2, &(packet->_color_2)); break;
      case 3: set_dronie_led(3, &(packet->_color_3)); break;
      case 4: set_dronie_led(4, &(packet->_color_4)); break;
      default:
        set_dronie_led(0, &(packet->_color_0));
        set_dronie_led(1, &(packet->_color_1));
        set_dronie_led(2, &(packet->_color_2));
        set_dronie_led(3, &(packet->_color_3));
        set_dronie_led(4, &(packet->_color_4));
        break;
      }
      break;
    case DRONIE_SET_OWNER: 
      packet->_owner[sizeof(packet->_owner) - 1] = '\0';
      set_nvs_config(OWNER_KEY, DATA_STRING, packet->_owner);
      break;
    case DRONIE_DOT:
      play_dronie_dot();
      break;
    case DRONIE_DASH:
      play_dronie_dash();
      break;
    case DRONIE_TOUCH_MOTOR:
      touch_dronie_motor();
      break;
    case DRONIE_FACTORY_RESET:
      reset_esp32();
      break;
  }

  return SUCCESS;
}


/*
 * Function: createArtnetPollReplyPacket()
 * ----------------------------
 *   builds the Poll Reply Packet
 *
 *   returns: some errors
 */
uint8_t createDronieReplyPacket(){
  ip_addr_t ip, *fuckingPointer;
  uint16_t i;
  uint8_t retVal = SUCCESS;
  uint8_t mac[MAX_MAC_SIZE];

  memset(&Dronie_Reply, 0, sizeof(Dronie_Reply));

  fuckingPointer = &ip;

  //Id
  for(i = 0; i < sizeof(Art_Id); i++){
    Dronie_Reply._id[i] = Art_Id[i];
  }
  //Opcode
  Dronie_Reply._opcode = DRONIE_OP_POLL_REPLY;

  retVal = get_active_ip(MEDIUM_ACTIVE, IP_IP, fuckingPointer);
  if(retVal != SUCCESS)
  {
    ESP_LOGE(Tag, "Error getting ip - creating art poll reply: %d", retVal);
    return retVal;
  }

  Dronie_Reply._ip[0] = ip_addr_get_ip4_u32(fuckingPointer) & 0xFF;
  Dronie_Reply._ip[1] = (ip_addr_get_ip4_u32(fuckingPointer) >> 8) & 0xFF;
  Dronie_Reply._ip[2] = (ip_addr_get_ip4_u32(fuckingPointer) >> 16) & 0xFF;
  Dronie_Reply._ip[3] = (ip_addr_get_ip4_u32(fuckingPointer) >> 24) & 0xFF;

  //Port
  Dronie_Reply._port = ART_PORT;

  //Version
  Dronie_Reply._version_info_hi = FIRMWARE_VERSION;
  Dronie_Reply._version_info_lo = FIRMWARE_BUILD;

  //OEM
  Dronie_Reply._oem_hi = ART_OEM_HI; //blizzard oem
  Dronie_Reply._oem_lo = ART_OEM_LO;

  //Ubea - we don't use
  Dronie_Reply._ubea_version = 0;


  Dronie_Reply._status_1._port_prog_auth = ART_PORT_PROG_AUTH;
  Dronie_Reply._status_1._boot_status = ART_BOOT_STATUS;
  Dronie_Reply._status_1._rdm_status = ART_RDM_STATUS;
  Dronie_Reply._status_1._ubea_present = ART_UBEA_PRESENT;

  //Blizzard ESTA Code
  Dronie_Reply._esta_man_lo = ART_ESTA_LO;
  Dronie_Reply._esta_man_hi = ART_ESTA_HI;

  //Manufacturer
  memset(Dronie_Reply._man, 0, sizeof(Dronie_Reply._man));
  retVal = get_nvs_config(MAN_KEY, DATA_STRING, Dronie_Reply._man);
  if(retVal != SUCCESS)
  {
    ESP_LOGE(Tag, "Could not get Man");
    return retVal;
  }

  //SN
  memset(Dronie_Reply._sn, 0, sizeof(Dronie_Reply._sn));
  retVal = get_nvs_config(SN_KEY, DATA_STRING, Dronie_Reply._sn);
  if(retVal != SUCCESS)
  {
    ESP_LOGE(Tag, "Could not get SN");
    return retVal;
  }

  //Owner
  memset(Dronie_Reply._node_report._owner, 0, sizeof(Dronie_Reply._node_report._owner));
  retVal = get_nvs_config(OWNER_KEY, DATA_STRING, Dronie_Reply._node_report._owner);
  if(retVal != SUCCESS)
  {
    ESP_LOGE(Tag, "Could not get owner");
    return retVal;
  }

  //State
  Dronie_Reply._node_report._mode = get_dronie_mode();
  Dronie_Reply._node_report._motor = get_dronie_motor_state();
  get_dronie_led(0, &(Dronie_Reply._node_report._led0));
  get_dronie_led(1, &(Dronie_Reply._node_report._led1));
  get_dronie_led(2, &(Dronie_Reply._node_report._led2));
  get_dronie_led(3, &(Dronie_Reply._node_report._led3));
  get_dronie_led(4, &(Dronie_Reply._node_report._led4));

  //Mac
  retVal = get_mac(mac);
  if(retVal != ESP_OK){
    ESP_LOGE(Tag, "Error getting mac - creating art poll reply: %d", retVal);
    return retVal;
  }
  
  Dronie_Reply._mac[5] = mac[0];
  Dronie_Reply._mac[4] = mac[1];
  Dronie_Reply._mac[3] = mac[2];
  Dronie_Reply._mac[2] = mac[3];
  Dronie_Reply._mac[1] = mac[4];
  Dronie_Reply._mac[0] = mac[5];

  return SUCCESS;
}

/*
 * Function: createArtnetPollReplyPacket()
 * ----------------------------
 *   builds the Poll Reply Packet
 *
 *   returns: some errors
 */
uint8_t createArtnetPollReplyPacket(){
  ip_addr_t ip, *fuckingPointer;
  uint16_t i;
  uint8_t retVal = SUCCESS;
  uint8_t mac[MAX_MAC_SIZE];

  fuckingPointer = &ip;

  //Id
  for(i = 0; i < sizeof(Art_Id); i++){
    Poll_Reply._id[i] = Art_Id[i];
  }
  //Opcode
  Poll_Reply._opcode = ART_OP_POLL_REPLY;

  retVal = get_active_ip(MEDIUM_ACTIVE, IP_IP, fuckingPointer);
  if(retVal != SUCCESS)
  {
    ESP_LOGE(Tag, "Error getting ip - creating art poll reply: %d", retVal);
    return retVal;
  }

  Poll_Reply._ip[0] = ip_addr_get_ip4_u32(fuckingPointer) & 0xFF;
  Poll_Reply._ip[1] = (ip_addr_get_ip4_u32(fuckingPointer) >> 8) & 0xFF;
  Poll_Reply._ip[2] = (ip_addr_get_ip4_u32(fuckingPointer) >> 16) & 0xFF;
  Poll_Reply._ip[3] = (ip_addr_get_ip4_u32(fuckingPointer) >> 24) & 0xFF;

  //Port
  Poll_Reply._port = ART_PORT;

  //Version
  Poll_Reply._version_info_hi = FIRMWARE_VERSION;
  Poll_Reply._version_info_lo = FIRMWARE_BUILD;

  //OEM
  Poll_Reply._oem_hi = ART_OEM_HI; //blizzard oem
  Poll_Reply._oem_lo = ART_OEM_LO;

  //Ubea - we don't use
  Poll_Reply._ubea_version = 0;


  Poll_Reply._status_1._port_prog_auth = ART_PORT_PROG_AUTH;
  Poll_Reply._status_1._boot_status = ART_BOOT_STATUS;
  Poll_Reply._status_1._rdm_status = ART_RDM_STATUS;
  Poll_Reply._status_1._ubea_present = ART_UBEA_PRESENT;

  //Blizzard ESTA Code
  Poll_Reply._esta_man_lo = ART_ESTA_LO;
  Poll_Reply._esta_man_hi = ART_ESTA_HI;

  //Long Name
  memset(Poll_Reply._long_name, 0, MAX_LONG_NAME);
  retVal = get_nvs_config(DEVICE_NAME_KEY, DATA_STRING, Poll_Reply._long_name);
  if(retVal != SUCCESS)
  {
    ESP_LOGE(Tag, "Error getting device name - creating art poll reply: %d", retVal);
    return retVal;
  }

  //Mac
  retVal = get_mac(mac);
  if(retVal != ESP_OK){
    ESP_LOGE(Tag, "Error getting mac - creating art poll reply: %d", retVal);
    return retVal;
  }
  
  Poll_Reply._mac[5] = mac[0];
  Poll_Reply._mac[4] = mac[1];
  Poll_Reply._mac[3] = mac[2];
  Poll_Reply._mac[2] = mac[3];
  Poll_Reply._mac[1] = mac[4];
  Poll_Reply._mac[0] = mac[5];

  return SUCCESS;
}

/*
 * Function: artToDMX(uint8_t slots, uint8_t* dmx)
 * ----------------------------
 *
 *   returns: opCode string representation
 */
const char * opCodeToString(uint16_t opCode){
  switch(opCode){
    case ART_OP_DMX:
      return "DMX Packet";
    case ART_OP_SYNC:
      return "Sync Packet";
    case ART_OP_POLL:
      return "Poll Packet";
    case ART_OP_POLL_REPLY:
      return "Poll Reply Packet";
    case ART_OP_IP_PROG:
      return "Prog Packet";
    case ART_OP_IP_PROG_REPLY:
      return "Prog Reply Packet";
    case ART_OP_COMMAND:
      return "Command Packet";
    case ART_OP_FIRMWARE_MASTER:
      return "Firmware Master";
    case ART_OP_FIRMWARE_REPLY:
      return "Firmware Reply";
    case ART_OP_ADDRESS:
      return "Address Packet";
  }

  return "Unkown Packet";
}

/*
 * Function: sendPacket(const ip_addr_t* address, uint16_t size, uint8_t* data)
 * ----------------------------
 *   This sends the reply packet
 *
 *   returns: ARTNET_ALLOC_PACKET_ERROR ARTNET_UDP_CONNECT_ERROR 
 */
uint8_t sendPacket(const ip_addr_t* address, uint16_t size, uint8_t* data)
{
  int retVal, i;
  struct pbuf *sendPacket;

  if(address == NULL){
    ESP_LOGE(Tag, "Null ip - send packet");
    return ARTNET_NULL_IP_ERROR;
  }

  if(data == NULL){
    ESP_LOGE(Tag, "Null data - send packet");
    return ARTNET_NULL_DATA_ERROR;
  }

  sendPacket = pbuf_alloc(PBUF_TRANSPORT, size ,PBUF_RAM);

  if(sendPacket == NULL){
    ESP_LOGE(Tag, "Error getting allocating packet - artnet");
    return ARTNET_ALLOC_PACKET_ERROR;
  }

  retVal = udp_connect(Art_Udp, address, ART_PORT);
  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error connecting to udp port - artnet: %d", retVal);
    return ARTNET_UDP_CONNECT_ERROR;
  }

  for(i = 0; i < size; i++){
    ((uint8_t*)(sendPacket->payload))[i] = data[i];
  }

  retVal = udp_send(Art_Udp, sendPacket);

  if(retVal != SUCCESS){
    ESP_LOGE(Tag, "Error sending udp - artnet: %d", retVal);
    retVal = ARTNET_UDP_SEND_ERROR;
  }

  udp_disconnect(Art_Udp);
  pbuf_free(sendPacket);

  return retVal;
}

/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Protocol Manager -> blizzard_artnet_manager.h
 *
 */

#ifndef BLIZZARD_ARTNET_MANAGER_H
#define BLIZZARD_ARTNET_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif


#include <inttypes.h>
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


/*----------------------- Defines --------------------------------------------*/

#define ART_ID {'A','r','t','-','N','e','t',0}
#define ART_PORT 6454 // or 6454
#define BLIZZARD_OEM 0x1251
 
//Art Poll Reply Info
#define ART_OEM_HI 0x12
#define ART_OEM_LO 0x51
#define ART_INDICATOR_UNKOWN 0x00 //Indicator state unknown
#define ART_INDICATOR_LOCATE 0x01 //Indicator state locate
#define ART_INDICATOR_MUTE 0x02 //Indicator state mute
#define ART_INDICATOR_NORMAL 0x03 //Indicator state normal
#define ART_PORT_PROG_AUTH 0x02 //All or part of Port-Address programmed by network or Web browser
#define ART_BOOT_STATUS 0x00 //Normal firmware boot
#define ART_RDM_STATUS 0x00 //not capable of rdm
#define ART_UBEA_PRESENT 0x00 //ubea is not present
#define ART_ESTA_HI 0x01
#define ART_ESTA_LO 0x04
#define ART_PORT_TYPE (BIT(7) | 0x00)
#define ART_PORT_DISABLE 0x00
#define ART_OUTPUTTING_DATA BIT(7)
#define ART_NO_OUTPUT_DATA 0x00
#define ART_SACN_INPUT_BIT BIT(0)
#define ART_MERGE_LTP_BIT BIT(1)
#define ART_MERGE_BIT BIT(3)

//Lookup Indexes
#define ART_OPCODE_U16_INDEX 4

//Check 
#define DMX_OK 0
#define DMX_NOT_OK 1

//Opcodes
#define ART_OP_DMX 0x5000
#define ART_OP_SYNC 0x5200
#define ART_OP_POLL 0x2000
#define ART_OP_POLL_REPLY 0x2100
#define ART_OP_IP_PROG 0xF800
#define ART_OP_IP_PROG_REPLY 0xF900
#define ART_OP_COMMAND 0x2400
#define ART_OP_SOC_REPLY 0x9696 
#define ART_OP_FIRMWARE_MASTER 0xF200
#define ART_OP_FIRMWARE_REPLY 0xF300
#define ART_OP_ADDRESS 0x6000
#define ART_OP_DIAG_DATA 0x2300
#define ART_BLIZZARD_PROBE 0x6996

#define ART_OP_RDM_TOD_REQUEST 0x8000
#define ART_OP_RDM_TOD_REPLY 0x8100
#define ART_OP_RDM 0x8300



//Poll Reply Codes
#define ART_PROTO_VER 14
#define ART_STYLE_NODE 0x00

//Address Commands
#define ART_ADDRESS_NO_ACTION 0x00
#define ART_ADDRESS_CANCEL_MERGE 0x01
#define ART_ADDRESS_INDICATORS_NORMAL 0x02
#define ART_ADDRESS_INDICATORS_MUTE 0x03
#define ART_ADDRESS_INDICATORS_LOCATE 0x04
#define ART_ADDRESS_MERGE_LTP 0x10
#define ART_ADDRESS_MERGE_HTP 0x50
#define ART_ADDRESS_SET_PROTOCOL_ARTNET 0x60
#define ART_ADDRESS_SET_PROTOCOL_SACN 0x70
#define ART_ADDRESS_CLEAR_DMX 0x90

//Prog Command Info
#define ART_PROG_ENABLE_BIT BIT(7)
#define ART_PROG_DHCP_ENABLE BIT(6)
#define ART_PROG_DEFAULT_ADDRESS_BIT BIT(3)
#define ART_PROG_IP_BIT BIT(2)
#define ART_PROG_NETMASK_BIT BIT(1)
#define ART_PROG_PORT_BIT BIT(0)

#define ART_PROG_AUX_COMMAND_PREAMBLE {'S', 'n', 'w'}
#define ART_PROG_AUX_COMMAND_POSTABMLE {'B', 'a', 'l'}
#define ART_PROG_AUX_COMMAND_N0_OP 0
#define ART_PROG_AUX_COMMAND_CONNECT_TO_WIFI 1
#define ART_PROG_AUX_COMMAND_FORCE_AP 2

#define ART_PRO_AUX_COMMAND_RETURN 1 //Not a success

//Firmware Master Codes
#define ART_FIRMWARE_FIRST 0x00
#define ART_FIRMWARE_CONT 0x01
#define ART_FIRMWARE_LAST 0x02 

#define ART_FIRMWARE_OTA_FIRST ART_FIRMWARE_FIRST
#define ART_SHOW_OTA_FIRST 0x30
#define ART_OTA_CONT ART_FIRMWARE_CONT
#define ART_FIRMWARE_OTA_LAST ART_FIRMWARE_LAST
#define ART_SHOW_OTA_LAST 0x31

#define ART_UBEA_FIRST 0x03
#define ART_UBEA_CONT 0x04
#define ART_UBEA_LAST 0x05
#define ART_FIRMWARE_BLOCK_GOOD 0x00
#define ART_FIRMWARE_ALL_GOOD 0x01
#define ART_FIRMWARE_ALL_FAIL 0xFF
#define ART_FIRMWARE_COMPLETE 0x69
#define ART_OTA_COMPLETE ART_FIRMWARE_COMPLETE
#define ART_FIRMWARE_DATA_BYTE_LENGTH 1024

#define ART_READ_MEMORY 0xC0
#define ART_READ_DMX 0xC1
#define ART_READ_DONE 0xC2


/*----------------------- Structs --------------------------------------------*/

typedef struct ArtnetDataPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint8_t _sequence;
  uint8_t _physical;
  uint8_t _universe_lo; //low byte
  uint8_t _universe_hi; //high 7 bits
  uint8_t _slot_hi; //2 - 512 has to be even
  uint8_t _slot_lo; //2 - 512 has to be even
  uint8_t _dmx_data[MAX_DMX_SLOTS - 1]; // no start bit
}__attribute__((packed)) ArtnetDataPacket;

typedef struct ArtnetPollPacket {
  uint8_t _id[8];  
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint8_t _talk_to_me;
  uint8_t _priority;
}__attribute__((packed)) ArtnetPollPacket;



typedef struct ArtnetPollReplyPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint8_t _ip[4];
  uint16_t _port; //always 0x1936
  uint8_t _version_info_hi; //high byte of firmware version
  uint8_t _version_info_lo; //low byte of firmware version
  uint8_t _net_switch;
  uint8_t _sub_switch;
  uint8_t _oem_hi;
  uint8_t _oem_lo;
  uint8_t _ubea_version; //set to 0
  union {
    struct {
      uint8_t _ubea_present: 1;
      uint8_t _rdm_status: 1;
      uint8_t _boot_status: 1;
      uint8_t _reserverd: 1;
      uint8_t _port_prog_auth: 2;
      uint8_t _indicator_state: 2;
    };
    uint8_t val;
  } _status_1;
  uint8_t _esta_man_lo;
  uint8_t _esta_man_hi;
  uint8_t _short_name[18];
  uint8_t _long_name[64];
  union {
    struct {
      uint8_t _magic[8];
      uint16_t _fixture_type;
      uint8_t _battery_state;
      uint8_t _battery_level;
      uint8_t _show_internal_state;
      uint8_t _show_state;
      uint8_t _show_name[16];
      uint8_t _show_on_loop;
      uint8_t _show_on_start;
      uint32_t _show_current_frame;
      uint32_t _show_total_frames;
      uint32_t _show_current_time;
      uint32_t _show_total_time;
      uint8_t _ssid[16];
    };
    uint8_t val[64];
  } _node_report;
  uint8_t _num_ports_hi;
  uint8_t _num_ports_lo;
  uint8_t _port_types[4];
  uint8_t _good_input[4];
  uint8_t _good_output[4];
  uint8_t _sw_in[4]; //universes for ports 0-3
  uint8_t _sw_out[4]; //will not be used
  uint8_t _sw_video;
  uint8_t _sw_macro;
  uint8_t _sw_remote;
  union {
    struct {
      uint8_t _ui_state;
      uint8_t _protocol_state;
      uint8_t _extra;
    };
    uint8_t val[3];
  } _state;
  uint8_t _style;
  uint8_t _mac[6]; //set high bytes to 0 if no info
  uint8_t _bind_ip[4];
  uint8_t _bind_index;
  union {
    struct {
      uint8_t _web_config_support: 1;
      uint8_t _dhcp_dis: 1;
      uint8_t _dhcp_capable: 1;
      uint8_t _large_universe_addressing: 1;
      uint8_t _sacn_switchable: 1;
      uint8_t _squawking: 1;
      uint8_t _reserved: 2;
    };
    uint8_t val;
  } _status_2;
  uint8_t _filler[26]; //zero for now
}__attribute__((packed)) ArtnetPollReplyPacket;

typedef struct ArtnetAddressPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint8_t _net_switch;
  uint8_t _bind_index;
  uint8_t _short_name[18];
  uint8_t _long_name[64];
  uint8_t _sw_in[4];
  uint8_t _sw_out[4];
  uint8_t _sub_switch;
  uint8_t _sw_video;
  uint8_t _command;
}__attribute__((packed)) ArtnetAddressPacket;

typedef struct ArtnetProgPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint16_t _filler_1; //0's
  union {
    struct {
      uint8_t _prog_port: 1; //not used
      uint8_t _prog_subnet_mask: 1;
      uint8_t _prog_ip: 1;
      uint8_t _set_default: 1;
      uint8_t _filler: 2;
      uint8_t _dhcp_en: 1;
      uint8_t _prog_en: 1;
    };
    uint8_t val;
  } _command;
  uint8_t _filler_2;
  uint8_t _prog_ip[4];
  uint8_t _prog_subnet_mask[4];
  uint16_t _prog_port; //not used
  union {
    struct {
      uint8_t _preamble[3];
      uint8_t _command[2];
      uint8_t _postamble[3];
    };
    uint8_t val[8];
  } _aux_command;

}__attribute__((packed)) ArtnetProgPacket;

typedef struct ArtnetProgReplyPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint8_t _filler[4];
  uint8_t _prog_ip[4];
  uint8_t _prog_subnet_mask[4];
  uint16_t _prog_port; //not used
  union {
    struct {
      uint8_t _reserved_1: 3;
      uint8_t _dhcp_en: 1;
      uint8_t _reserved_2: 4;
    };
    uint8_t val;
  } _status;
  uint8_t _spare[7];

}__attribute__((packed)) ArtnetProgReplyPacket;


typedef struct ArtnetCommandPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint8_t _esta_man_hi;
  uint8_t _esta_man_lo;
  uint8_t _length_hi;
  uint8_t _length_lo;
  uint8_t _data[MAX_DMX_SLOTS - 1];
}__attribute__((packed)) ArtnetCommandPacket;

typedef struct ArtnetFirmwareMasterPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint16_t _filler_1;
  uint8_t _type;
  uint8_t _block_id;
  uint8_t _firmware_length_3;
  uint8_t _firmware_length_2;
  uint8_t _firmware_length_1;
  uint8_t _firmware_length_0; //LSB
  uint32_t _block_to_write;
  uint8_t _filler[20-sizeof(uint32_t)];
  uint16_t _data[MAX_DMX_SLOTS - 1];
}__attribute__((packed)) ArtnetFirmwareMasterPacket;

typedef struct ArtnetFirmwareReplyPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint16_t _filler_1;
  uint8_t _type;
  uint32_t _requested_block;
  uint8_t _filler[21-sizeof(uint32_t)];
}__attribute__((packed)) ArtnetFirmwareReplyPacket;

typedef struct ArtnetDiagPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint8_t _filler_1;
  uint8_t _priority;
  uint16_t _filler_2;
  uint8_t _length_hi;
  uint8_t _length_lo;
  uint8_t _data[MAX_DMX_SLOTS - 1];
}__attribute__((packed)) ArtnetDiagPacket;

//honestly we can ignore this packet
typedef struct ArtnetRDMTodRequestPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint8_t _filler[9];
  uint8_t _net; //can ignore for now
  uint8_t _command; //can ignore for now
  uint8_t _addCount; //can ignore for now
  uint8_t _addresses[32]; //can ignore for now
}__attribute__((packed)) ArtnetRDMTodRequestPacket;

typedef struct ArtnetRDMTodResponsePacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint8_t _rdm_version;
  uint8_t _port; //1
  uint8_t _filler[6];
  uint8_t _bind_index; //1
  uint8_t _net; //can ignore for now
  uint8_t _command_response; //0
  uint8_t _address; //can ignore for now
  uint8_t _uid_total_hi;
  uint8_t _uid_total_lo;
  uint8_t _block_count; //1 for now
  uint8_t _uid_count; //in this packet
  uint8_t _tod[32 * 6]; //32 max uids
}__attribute__((packed)) ArtnetRDMTodResponsePacket;

typedef struct ArtnetRDMPacket {
  uint8_t _id[8];
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint8_t _rdm_version;
  uint8_t _filler[8];
  uint8_t _net; //can ignore for now
  uint8_t _command; //0
  uint8_t _address; //can ignore for now
  uint8_t _rdm[257]; //payload
}__attribute__((packed)) ArtnetRDMPacket;

typedef struct SyncType {
  uint8_t _enabled;
  uint32_t _tick;
  uint8_t _dmx[MAX_DMX_SLOTS];
} SyncType;

typedef struct MergeType {
  uint8_t _mode;
  uint32_t _ticks[2];
  ip_addr_t _ips[2];
  uint8_t _dmx[2][MAX_DMX_SLOTS - 1];
} MergeType;
 
typedef struct ArtQueType {
  uint8_t packet[CONFIG_ARTNET_MAX_PACKET_SIZE];
  u16_t len;
  ip_addr_t address;
}__attribute__((packed)) ArtQueType;

// -------------------- DRONIE -----------------

#define DRONIE_OP_POLL 0xCC01
#define DRONIE_OP_POLL_REPLY 0xCC02
#define DRONIE_OP_COMMAND 0xCC03

#define DRONIE_NOP 0x00

#define DRONIE_SET_MODE 0x01 
#define DRONIE_MODE_INCOGNITO 0
#define DRONIE_MODE_DEBUG 1
#define DRONIE_MODE_SPOON 2

#define DRONIE_SET_LED 0x02

#define DRONIE_SET_OWNER 0x03

#define DRONIE_DOT 0x10
#define DRONIE_DASH 0x11

#define DRONIE_TOUCH_MOTOR 0x20

#define DRONIE_FACTORY_RESET 0xFF

typedef struct DronieCommandPacket {
  uint8_t _id[8];  
  uint16_t _opcode;
  uint8_t _command;
  uint8_t _value;
  color_t _color_0;
  color_t _color_1;
  color_t _color_2;
  color_t _color_3;
  color_t _color_4;
  uint8_t _owner[47];
}__attribute__((packed)) DronieCommandPacket;

typedef struct DroniePollPacket {
  uint8_t _id[8];  
  uint16_t _opcode;
  uint16_t _protocol_version; //14
  uint8_t _talk_to_me;
  uint8_t _priority;
}__attribute__((packed)) DroniePollPacket;


typedef struct DroniePacketReply {
  uint8_t _id[8];
  uint16_t _opcode;
  uint8_t _ip[4];
  uint16_t _port; //always 0x1936
  uint8_t _version_info_hi; //high byte of firmware version
  uint8_t _version_info_lo; //low byte of firmware version
  uint8_t _net_switch;
  uint8_t _sub_switch;
  uint8_t _oem_hi;
  uint8_t _oem_lo;
  uint8_t _ubea_version; //set to 0
  union {
    struct {
      uint8_t _ubea_present: 1;
      uint8_t _rdm_status: 1;
      uint8_t _boot_status: 1;
      uint8_t _reserverd: 1;
      uint8_t _port_prog_auth: 2;
      uint8_t _indicator_state: 2;
    };
    uint8_t val;
  } _status_1;
  uint8_t _esta_man_lo;
  uint8_t _esta_man_hi;
  uint8_t _man[18]; //Talon Corp
  uint8_t _sn[64]; //SN
  union {
    struct {
      uint8_t _mode; //Ingocnito, //Debug, //Spoon
      uint8_t _motor; //on/off
      color_t _led0;
      color_t _led1;
      color_t _led2;
      color_t _led3;
      color_t _led4;
      uint8_t _owner[47];
    };
    uint8_t val[64];
  } _node_report;
  uint8_t _num_ports_hi;
  uint8_t _num_ports_lo;
  uint8_t _port_types[4];
  uint8_t _good_input[4];
  uint8_t _good_output[4];
  uint8_t _sw_in[4]; //universes for ports 0-3
  uint8_t _sw_out[4]; //will not be used
  uint8_t _sw_video;
  uint8_t _sw_macro;
  uint8_t _sw_remote;
  union {
    struct {
      uint8_t _ui_state;
      uint8_t _protocol_state;
      uint8_t _extra;
    };
    uint8_t val[3];
  } _state;
  uint8_t _style;
  uint8_t _mac[6]; //set high bytes to 0 if no info
  uint8_t _bind_ip[4];
  uint8_t _bind_index;
  union {
    struct {
      uint8_t _web_config_support: 1;
      uint8_t _dhcp_dis: 1;
      uint8_t _dhcp_capable: 1;
      uint8_t _large_universe_addressing: 1;
      uint8_t _sacn_switchable: 1;
      uint8_t _squawking: 1;
      uint8_t _reserved: 2;
    };
    uint8_t val;
  } _status_2;
  uint8_t _filler[26]; //zero for now
}__attribute__((packed)) DroniePacketReply;


 

/*----------------------- Functions ------------------------------------------*/

uint8_t init_artnet_manager(void);
uint8_t start_artnet(void);
void stop_artnet(void);

void tick_artnet(void);

uint8_t change_artnet_merge(uint8_t merge);
uint8_t get_artnet_merge(void);
uint8_t change_artnet_net(uint8_t net);
uint8_t get_artnet_net(void);
uint8_t change_artnet_subnet(uint8_t subnet);
uint8_t get_artnet_subnet(void);
uint8_t change_artnet_universe(uint8_t universe);
uint8_t get_artnet_universe(void);
uint16_t build_artnet_universe(void);

/*Internal*/
void receiveArtnet(void *arg,
                  struct udp_pcb *pcb,
                  struct pbuf *p,
                  const ip_addr_t *addr,
                  u16_t port);
                
uint8_t pushToQue(struct pbuf *packet, const ip_addr_t *address);
bool emptyQue();
ArtQueType* popFromQue();

void handlePackets();

uint8_t parseDronieCommandPacket(DronieCommandPacket* packet);
uint8_t createDronieReplyPacket(void);


uint8_t parseArtnetDataPacket(ArtnetDataPacket* packet);
uint8_t parseArtnetAddressPacket(ArtnetAddressPacket* packet, const ip_addr_t *address);
uint8_t parseArtnetProgPacket(ArtnetProgPacket* packet);
uint8_t parseArtnetCommandPacket(ArtnetCommandPacket* packet);
uint8_t parseArtnetFirmwareMasterPacket(ArtnetFirmwareMasterPacket* packet);

uint8_t sendPacket(const ip_addr_t *address, uint16_t size, uint8_t* data);

uint8_t createArtnetPollReplyPacket(void);
uint8_t createArtnetProgReplyPacket(void);
void createArtnetTODReplyPacket();
void createArtnetRDMReplyPacket();
void createArtnetFirmwareReplyPacket(void);
void createArtnetCommandReplyPacket(void);
//command and firmware packets are populated in the parsing of them

uint8_t checkMergeNSync(const ip_addr_t* address, uint8_t* dmx);

uint8_t artToDMX(uint16_t slots, uint8_t* dmx);
const char * opCodeToString(uint16_t opCode);

#ifdef __cplusplus
}
#endif

#endif

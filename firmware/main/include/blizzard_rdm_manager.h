/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Protocol Manager -> blizzard_rdm_manager.h
 */

#ifndef BLIZZARD_RDM_MANAGER_H
#define BLIZZARD_RDM_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>
#include "rdm_defs.h"

#include "blizzard_artnet_manager.h"


/*----------------------- RDM Structs ------------------------------------*/

typedef struct RdmBuffer
{
  uint8_t _data[RDM_MAX_BYTES]; /**< The RDM message bytes. */
  uint16_t  _data_len;            /**< The length of the RDM message. */
}__attribute__((packed)) RdmBuffer;

typedef struct RdmUid
{
  uint16_t _man; //manufacturer uid
  uint32_t  _id; //device uid
}__attribute__((packed)) RdmUid;

typedef struct RdmPacket
{
    uint8_t _start_code; //0xCC
    uint8_t _sub_start_code; //0x01
    uint8_t _length; //everything minus the checksum (-2)
    uint16_t _dest_uid_man; //Dest uid manufacturer
    uint32_t _dest_uid_id; //Dest uid id
    uint16_t _source_uid_man; //Source uid manufacturer
    uint32_t _source_uid_id; //Source uid id
    uint8_t _transaction_number; //just keeps incrementing
    uint8_t _port_id_response_type; //m>s port | s>m response
    uint8_t _message_count; //0 from controller | on response, use GET_QUEUED_MESSAGES
    uint16_t _sub_device; //keep 0 for now
    // ------- Payload ----- //
    uint8_t _command_class; //GET_COMMAND - main command (get set or discover)
    uint16_t _pid; //STATUS_MESSAGES - parameter id (what to get)
    uint8_t _pd_length; //1
    uint8_t _pd[RDM_MAX_PDL]; //this is variable
    // ------- End Length ------ //
    uint8_t _check_hi; //checksum high
    uint8_t _check_lo; //checksum low
    uint8_t _needs_response; //0 or 1
    
}__attribute__((packed)) RdmPacket;

typedef struct RdmDiscoveryPacket
{
    uint8_t _start_code; //0xCC
    uint8_t _sub_start_code; //0x01
    uint8_t _length; //everything minus the checksum (-2)
    uint16_t _dest_uid_man; //Dest uid manufacturer
    uint32_t _dest_uid_id; //Dest uid id
    uint16_t _source_uid_man; //Source uid manufacturer
    uint32_t _source_uid_id; //Source uid id
    uint8_t _transaction_number; //just keeps incrementing
    uint8_t _port_id; //0x01 - 0xFF
    uint8_t _message_count; //0x00
    uint16_t _sub_device; //0x0000
    // ------- Payload ----- //
    uint8_t _command_class; //DISCOVERY_COMMAND
    uint16_t _pid; //DISC_UNIQUE_BRANCH
    uint8_t _pd_length; //0x0C
    uint8_t _pd[RDM_MAX_PDL]; //48bit lower bound //48bit upper bound
    // ------- End Length ------ //
    uint8_t _check_hi; //checksum high
    uint8_t _check_lo; //checksum low
    uint8_t _needs_response; //0 or 1
    
}__attribute__((packed)) RdmDiscoveryPacket;

#define RDM_DISCOVERY_RESPONSE_OPTIONAL_PREAMBLE_LENGTH 7
typedef struct RdmDiscoveryResponsePacket
{
    uint8_t _preamble[7]; //0xFE
    uint8_t _start_code; //0xAA
    uint8_t _mid_1_msb_AA; 
    uint8_t _mid_1_msb_55; 
    uint8_t _mid_0_lsb_AA; 
    uint8_t _mid_0_lsb_55;
    uint8_t _uid_3_msb_AA;
    uint8_t _uid_3_msb_55;
    uint8_t _uid_2_AA;
    uint8_t _uid_2_55;
    uint8_t _uid_1_AA;
    uint8_t _uid_1_55;
    uint8_t _uid_0_lsb_AA;
    uint8_t _uid_0_lsb_55;
    uint8_t _csum_1_msb_AA;
    uint8_t _csum_1_msb_55;
    uint8_t _csum_0_lsb_AA;
    uint8_t _csum_0_lsb_55;
    
}__attribute__((packed)) RdmDiscoveryResponsePacket;

typedef struct RdmDiscoveryDecodedResponsePacket
{
    uint8_t _preamble[7]; //0xFE
    uint8_t _start_code; //0xAA
    uint8_t _mid_1; 
    uint8_t _mid_0; 
    uint8_t _uid_3;
    uint8_t _uid_2;
    uint8_t _uid_1;
    uint8_t _uid_0;
    uint8_t _csum_1;
    uint8_t _csum_0;
    
}__attribute__((packed)) RdmDiscoveryDecodedResponsePacket;

/*----------------------- Discovery Defines -----------------------------------*/
#define RDM_DISCOVERY_IDLE 0
#define RDM_DISCOVERY_RUNNING 1
#define RDM_DISCOVERY_ERROR 2

#define RDM_TEST_UID 0x29B4FF6BA98F
#define RDM_TEST_UID_MAN 0x29B4
#define RDM_TEST_UID_ID 0xFF6BA98F

#define RDM_TEST_UID 0x29B4FF6BA98F
#define RDM_TEST_UID_MAN 0x29B4
#define RDM_TEST_UID_ID 0xFF6BA98F

/*----------------------- RDM Functions -----------------------------------*/

uint8_t init_rdm_manager();

void tick_rdm();

uint8_t rdm_discovery(ArtnetRDMTodResponsePacket* returnPacket);
uint16_t rdm_send_artnet_command(RdmPacket* command, uint16_t expectedLength, uint8_t readDelay);
uint16_t rdm_send_command(RdmPacket* command, uint16_t expectedLength, uint8_t readDelay);

void checkBranch(RdmUid low, RdmUid high, uint8_t *length);
RdmUid divideUID(RdmUid uid);

RdmPacket* get_rdm_command();
RdmPacket* get_rdm_response();

uint8_t get_discovery_status();
uint8_t get_rdm_command_ready();
uint8_t get_rdm_response_ready();
void setRdmCommandReady(uint8_t ready);
void setRdmResponseReady(uint8_t ready);

void prepareToLaunch(volatile RdmPacket* packet);
void setChecksum(RdmPacket* packet);
bool checkChecksum(RdmPacket* packet);
uint16_t calcChecksum(RdmPacket* packet);
uint8_t checkDiscoveryResponse(uint16_t bytesRead, uint8_t* reply, uint8_t* uid);
bool checkDiscoveryChecksum(RdmDiscoveryDecodedResponsePacket* decoded, RdmDiscoveryResponsePacket* response);
uint16_t calcDiscoveryChecksum(RdmDiscoveryResponsePacket* packet);

void setSourceUid(RdmPacket* packet);
void initDiscoveryPacket(RdmUid* lower, RdmUid* upper);
void initDiscoveryClearPacket(void);
void initDiscoveryMutePacket(RdmUid* device);
void initCommandListPacket(RdmUid* device);

#ifdef __cplusplus
}
#endif

#endif // ifndef DMX_UART_H

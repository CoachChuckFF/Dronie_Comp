/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Protocol Manager -> blizzard_rdm_manager.c
 * 
 * For this product, dmx will only be outputting. This files sets up the 
 * DMX serial communications and lets the driver in modified driver go to work.
 */

#include "blizzard_rdm_manager.h"
#include "blizzard_dmx_manager.h"
#include "blizzard_network_manager.h"
#include "blizzard_artnet_manager.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "blizzard_helpers.h"
#include "blizzard_global_defines.h"

static const char *Tag = "RDM";

static RdmUid UIDs[RDM_MAX_UID_COUNT];
static RdmPacket RDM_Command;
static RdmDiscoveryDecodedResponsePacket RDM_Discover_Decoded;
static uint16_t RDM_DISCOVERY_CATCH = 0;
static uint8_t Command_Ready = FALSE;
static uint8_t Response_Ready = FALSE;
static uint8_t Discovery_Status = 0;
static uint8_t UID_Count = 0;


/*---------------------------- Main Uses ----------------------------------*/
/*
 * Function: init_rdm_manager
 * ----------------------------
 *  sets up RDM for use
 */ 
uint8_t init_rdm_manager() {
	ESP_LOGI(Tag, "RDM is lost");

    //clear uids
    memset((uint8_t *)UIDs, 0x00, sizeof(RdmUid) * RDM_MAX_UID_COUNT);

	ESP_LOGI(Tag, "RDM is pursuing it's personal legend");
    return SUCCESS;
}

/*
 * Function: tick_rdm
 * ----------------------------
 *  handles commands, responses and discovery
 */ 
void tick_rdm(){

//    if(!Tick_Tock++){
//     //    initDiscoveryPacket(NULL, NULL);
//     //    ESP_LOGI(Tag, "%d", rdm_send_command(&RDM_Command, 24));
//         if(UID_Count == 0){
//             rdm_discovery();
//         } else {
//             initCommandListPacket(&(UIDs[(Tock_Tick++ % UID_Count)]));
//             ESP_LOGE(Tag, "%d", rdm_send_command(&RDM_Command, 0, 1));
//             hex_dumper((uint8_t *)(&RDM_Command), sizeof(RdmPacket));
//         }
//     }
}

/*
 * Function: rdm_discovery(ArtnetRDMTodResponsePacket* returnPacket)
 * ----------------------------
 *  runs rdm discovery and returns the response in the return packet
 */ 
uint8_t rdm_discovery(ArtnetRDMTodResponsePacket* returnPacket){
    RdmUid high;
    RdmUid low;
    uint8_t length = 0, i = 0;

    Discovery_Status = 1;

    memset((uint8_t *)UIDs, 0x00, sizeof(RdmUid) * RDM_MAX_UID_COUNT);

    initDiscoveryClearPacket();
    rdm_send_command(&RDM_Command, 0, 0);

    //set inital branches
    memset((uint8_t *)&high, 0xFF, sizeof(RdmUid));
    memset((uint8_t *)&low, 0x00, sizeof(RdmUid));

    //catch case for recursion
    RDM_DISCOVERY_CATCH = 0;

    //recursion 
    checkBranch(low, high, &length);

    Discovery_Status = 0;

    //Set return packet
    returnPacket->_block_count = 1;
    returnPacket->_uid_total_lo = length;
    returnPacket->_uid_count = length;

    //set uid array
    for(i = 0; i < length * sizeof(RdmUid); i++){
        returnPacket->_tod[i] = ((uint8_t *)UIDs)[i];
    }

    UID_Count = length;
    ESP_LOGE(Tag, "RDM Devices: %d", UID_Count);
    // for(i = 0; i < length; i++){
    //     hex_dumper(UIDs + (i * 6), 6);
    // }

    return length;
}

/*
 * Function: rdm_send_command(RdmPacket* command);
 * ----------------------------
 *  sends command and reads response
 */ 
// CC 01 18 29 B4 FF 6B A9 8F 12 51 AC 67 B2 57 56 
// 7C 00 20 50 00 00 00 00 08 25 00 00 00 00 00 00 

// CC 01 18 29 B4 FF 6B A9 8F 00 00 00 00 45 01 60 
// 00 00 20 50 00 00 00 00 00 00 00 00 00 00 00 00 

// CC 01 18 29 B4 FF 6B A9 8F 00 00 00 00 00 00 00 
// 01 00 00 00 20 00 50 00 00 00 00 00 00 00 00 00 

// CC 01 18 29 B4 FF 6B A9 8F 12 51 AC 67 B2 57 00 
// 01 00 00 00 20 00 50 00 07 54 00 00 00 00 00 00 
uint16_t rdm_send_artnet_command(RdmPacket* command, uint16_t expectedLength, uint8_t readDelay){
    uint16_t retVal = 0, i = 0;
    uint8_t temp = 0;

    setSourceUid(command);
    setChecksum(command);
    // ESP_LOGE(Tag, "-------- INCOMING --------");
    // hex_dumper(command, 30);
    retVal = rdm_send_command(command, expectedLength, readDelay);

    //DMX reads the break as a bit
    for(i = 0; i < sizeof(RdmPacket) - 1; i++){
        temp = ((uint8_t *)(command))[i + 1];
        ((uint8_t *)(command))[i] = temp;
    }

    // ESP_LOGE(Tag, "           OUTGOING         ");
    // hex_dumper(command, retVal);
    // ESP_LOGE(Tag, "Artnet RDM Command Return Length %d", retVal);
    // ESP_LOGE(Tag, "---------------------------");

    return retVal;
}

/*
 * Function: rdm_send_command(RdmPacket* command);
 * ----------------------------
 *  sends command and reads response
 */ 
uint16_t rdm_send_command(RdmPacket* command, uint16_t expectedLength, uint8_t readDelay){
    uint16_t retSize = 0;
    uint8_t tick = 0xFF;

    if(command == NULL) return 0;

    //Copy to main buffer
    memcpy(((uint8_t*)&RDM_Command), ((uint8_t*)command), sizeof(RdmPacket));

    //use this as source UID
    // if(shouldSetSource){
    //     setSourceUid(&RDM_Command);
    // }

    setRdmCommandReady(TRUE);

    //spin wait for response - max 255ms
    while (!Response_Ready && tick--){vTaskDelay(1);}
    if(Response_Ready){
        memset(((uint8_t *) command), 0, sizeof(RdmPacket));
        if(expectedLength == 0) expectedLength = sizeof(RdmPacket);
        vTaskDelay(readDelay);
        retSize = uart_read_bytes(DMX_UART, ((uint8_t *)command), expectedLength, 13);
    }

    Response_Ready = FALSE;
    Command_Ready = FALSE;
    rebootDMXDriver();
    vTaskDelay(5); //let it breathe

    return retSize;
}

/*---------------------------- BST Discovery ----------------------------------*/
/*
 * Function: checkBranch(RdmUid low, RdmUid high, uint8_t *length)
 * ----------------------------
 *  increments the counter everytime there is a hit - when there is a collision call both sides
 */ 
void checkBranch(RdmUid low, RdmUid high, uint8_t *length){
    uint16_t bytesRead = 0;
    uint8_t checkStatus = 0;

    //Recursion Catch
    if(RDM_DISCOVERY_CATCH++ > 21){return;}

    initDiscoveryPacket(&low, &high);
    bytesRead = rdm_send_command(&RDM_Command, sizeof(RdmPacket), 5);
    checkStatus = checkDiscoveryResponse(bytesRead, ((uint8_t *)&RDM_Command), ((uint8_t*)&(UIDs[length[0]])));

    vTaskDelay(100);

    switch (checkStatus){
    case 1: //Got UID
        ESP_LOGE(Tag, "Discovery Got UID -- Check %d", RDM_DISCOVERY_CATCH);
        length[0]++;
        //mute and re-send
        initDiscoveryMutePacket(&(UIDs[length[0] - 1]));
        rdm_send_command(&RDM_Command, sizeof(RdmPacket), 1);
        checkBranch(low, high, length);

        break;
    case 2: //Collision
    case 3: //Collision - not 24
        ESP_LOGE(Tag, "Discovery Collision -- Check %d", RDM_DISCOVERY_CATCH);
        checkBranch(low, divideUID(high), length);
        checkBranch(divideUID(high), high, length);
        break;
    }
}

/*
 * Function: RdmUid divideUID(RdmUid oldUid)
 * ----------------------------
 *  returns half the value of RDMUid
 */ 
RdmUid divideUID(RdmUid oldUid){
    RdmUid newUid;
    uint64_t* bytes = ((uint64_t *) &oldUid);
    bytes[0] = bytes[0] >> 1ULL;
    newUid = ((RdmUid*)bytes)[0];
    return newUid;
}   


/*---------------------------- Getters/Setters ----------------------------------*/
/*
 * Function: RdmPacket* get_command_packet()
 * ----------------------------
 *  returns the command buffer
 */ 
RdmPacket* get_rdm_command(){
    return &RDM_Command;
}

// /*
//  * Function: RdmPacket* get_response_packet()
//  * ----------------------------
//  *  returns the response buffer
//  */ 
// RdmPacket* get_rdm_response(){
//     return &RDM_Response;
// }

/*
 * Function: get_rdm_command_ready
 * ----------------------------
 *  gets command ready flag
 */ 
uint8_t get_rdm_command_ready(){
    return Command_Ready;
}

/*
 * Function: get_rdm_response_ready
 * ----------------------------
 *  gets response ready flag
 */ 
uint8_t get_rdm_response_ready(){
    return Response_Ready;
}

/*
 * Function: setRdmCommandReady
 * ----------------------------
 *  sets command ready flag
 */ 
void setRdmCommandReady(uint8_t ready){
    Command_Ready = ready;
}

/*
 * Function: setRdmResponseReady
 * ----------------------------
 *  sets response ready flag
 */ 
void setRdmResponseReady(uint8_t ready){
    Response_Ready = ready;
}

/*---------------------------- Checks ----------------------------------*/
/*
 * Function: checkDiscoveryResponse(uint16_t bytesRead)
 * ----------------------------
 *  returns a list of rdm ids and a count of devices
 */ 
uint8_t checkDiscoveryResponse(uint16_t bytesRead, uint8_t* reply, uint8_t* uid){
    uint8_t i, feCount = 0;
    RdmDiscoveryResponsePacket* response = (RdmDiscoveryResponsePacket*)reply;

    if(bytesRead == 0) return 0;
    // if(bytesRead > sizeof(RdmDiscoveryResponsePacket) || bytesRead < (sizeof(RdmDiscoveryResponsePacket) - RDM_DISCOVERY_RESPONSE_OPTIONAL_PREAMBLE_LENGTH)) return 3;

    //TODO - can do without all 7 FE's
    for(i = 0; i < RDM_DISCOVERY_RESPONSE_OPTIONAL_PREAMBLE_LENGTH; i++){
        if((reply)[i] == 0xFE){
            feCount++;
        } else {
            break;
        }
    }

    reply -= (7-feCount); //adjust to lower FEs

    response = (RdmDiscoveryResponsePacket*)(reply);

    //Decode Packet
    memset(((uint8_t *)&RDM_Discover_Decoded), 0x00, sizeof(RdmDiscoveryDecodedResponsePacket));

    RDM_Discover_Decoded._start_code = response->_start_code;

    RDM_Discover_Decoded._mid_1 = ((response->_mid_1_msb_AA | 0xAA) & (response->_mid_1_msb_55 | 0x55));
    RDM_Discover_Decoded._mid_0 = ((response->_mid_0_lsb_AA | 0xAA) & (response->_mid_0_lsb_55 | 0x55));

    RDM_Discover_Decoded._uid_3 = ((response->_uid_3_msb_AA | 0xAA) & (response->_uid_3_msb_55 | 0x55));
    RDM_Discover_Decoded._uid_2 = ((response->_uid_2_AA | 0xAA) & (response->_uid_2_55 | 0x55));
    RDM_Discover_Decoded._uid_1 = ((response->_uid_1_AA | 0xAA) & (response->_uid_1_55 | 0x55));
    RDM_Discover_Decoded._uid_0 = ((response->_uid_0_lsb_AA | 0xAA) & (response->_uid_0_lsb_55 | 0x55));

    RDM_Discover_Decoded._csum_1 = ((response->_csum_1_msb_AA | 0xAA) & (response->_csum_1_msb_55 | 0x55));
    RDM_Discover_Decoded._csum_0 = ((response->_csum_0_lsb_AA | 0xAA) & (response->_csum_0_lsb_55 | 0x55));
    //Check Packet

    // hex_dumper((uint8_t*) &RDM_Discover_Decoded, sizeof(RdmDiscoveryDecodedResponsePacket));

    if(RDM_Discover_Decoded._start_code != 0xAA) return 2;

    if(!checkDiscoveryChecksum(&RDM_Discover_Decoded, response)) return 2;

    //Add to RdmUID TODO look at
    uid[0] = RDM_Discover_Decoded._mid_1;
    uid[1] = RDM_Discover_Decoded._mid_0;
    uid[2] = RDM_Discover_Decoded._uid_3;
    uid[3] = RDM_Discover_Decoded._uid_2;
    uid[4] = RDM_Discover_Decoded._uid_1;
    uid[5] = RDM_Discover_Decoded._uid_0;

    return 1;
}

/*
 * Function: setChecksum(RdmPacket* packet)
 * ----------------------------
 *  sets the correct checksum for the given packet
 */ 
void setChecksum(RdmPacket* packet){
    uint16_t checksum = calcChecksum(packet);

    packet->_check_hi = (uint8_t)((checksum & 0xFF00) >> 8);
    packet->_check_lo = (uint8_t)(checksum & 0x00FF);
}

/*
 * Function: prepareToLaunch(RdmPacket* packet)
 * ----------------------------
 *  called by the uart driver to make sure checksum is in the right place!
 */ 
void prepareToLaunch(volatile RdmPacket* packet){
    uint8_t length = packet->_length;

    //Set the checksum in right place
    ((uint8_t*) packet)[length] = packet->_check_hi;
    ((uint8_t*) packet)[length+1] = packet->_check_lo;
}

/*
 * Function: bool checkChecksum(RdmPacket* packet)
 * ----------------------------
 *  checks to see if packet has the correct checksum
 */ 
bool checkChecksum(RdmPacket* packet){
    return calcChecksum(packet) == ((((uint16_t)packet->_check_hi) << 8) | ((uint16_t)packet->_check_lo));
}

/*
 * Function: bool checkChecksum(RdmPacket* packet)
 * ----------------------------
 * The Checksum field is the unsigned, modulo 0x10000, 16-bit additive checksum of the entire packet’s slot data, 
 * including START Code. The checksum is an additive sum of the 8-bit fields into a 16-bit response value.
 * If the checksum field in the packet does not match the calculated checksum, 
 * then the packet shall be discarded and no response sent.
 */ 
uint16_t calcChecksum(RdmPacket* packet){
    uint16_t sum = 0, i = 0;
    for (i = 0; i < packet->_length; i++)
        sum += ((uint8_t *)packet)[i];
    return sum;
}

/*
 * Function: bool checkChecksum(RdmPacket* packet)
 * ----------------------------
 *  checks to see if packet has the correct checksum
 */ 
bool checkDiscoveryChecksum(RdmDiscoveryDecodedResponsePacket* decoded, RdmDiscoveryResponsePacket* response){
    return calcDiscoveryChecksum(response) == ((((uint16_t)decoded->_csum_1) << 8) | ((uint16_t)decoded->_csum_0));
}

/*
 * Function: bool checkChecksum(RdmPacket* packet)
 * ----------------------------
 * The Checksum field is the unsigned, modulo 0x10000, 16-bit additive checksum of the entire packet’s slot data, 
 * including START Code. The checksum is an additive sum of the 8-bit fields into a 16-bit response value.
 * If the checksum field in the packet does not match the calculated checksum, 
 * then the packet shall be discarded and no response sent.
 */ 
uint16_t calcDiscoveryChecksum(RdmDiscoveryResponsePacket* packet){
    uint16_t sum = 0, i = 0;
    //RDM_DISCOVERY_RESPONSE_OPTIONAL_PREAMBLE_LENGTH + 1 for startbit
    for (i = RDM_DISCOVERY_RESPONSE_OPTIONAL_PREAMBLE_LENGTH + 1; i < sizeof(RdmDiscoveryResponsePacket) - 4; i++)
        sum += ((uint8_t *)packet)[i];
    return sum;
}


/*---------------------------- Premade Packets ----------------------------------*/
/*
 * Function: setSourceUid(RdmPacket* packet)
 * ----------------------------
 *  sets up the sending source UID
 */ 
void setSourceUid(RdmPacket* packet){
    //mac as _source_uid_id
    packet->_source_uid_man = to_right_endian_16(BLIZZARD_OEM);
    get_mac(((uint8_t*)packet) + 11u);
}

/*
 * Function: initDiscoveryPacket
 * ----------------------------
 *  sets up a test packet for RDM use
 */ 
void initDiscoveryPacket(RdmUid* lower, RdmUid* upper){
    memset(((uint8_t*)&RDM_Command), 0x00, sizeof(RdmPacket));

    RDM_Command._start_code = 0xCC;
    RDM_Command._sub_start_code = 0x01;
    RDM_Command._length = 0x19 + 11;

    RDM_Command._dest_uid_man = 0xFFFF;
    RDM_Command._dest_uid_id = 0xFFFFFFFF;

    setSourceUid(&RDM_Command);

    RDM_Command._transaction_number = 0x00;
    RDM_Command._port_id_response_type = 0x01;
    RDM_Command._message_count = 0x00;
    RDM_Command._sub_device = 0x0000;

    RDM_Command._command_class = E120_DISCOVERY_COMMAND;
    RDM_Command._pid = to_right_endian_16(E120_DISC_UNIQUE_BRANCH);
    RDM_Command._pd_length = 0x0C;

    //lower bound
    if(lower == NULL){
        memset(RDM_Command._pd, 0x00, 6);
    } else {
        RDM_Command._pd[0] = (((uint8_t*)lower)[0]);
        RDM_Command._pd[1] = (((uint8_t*)lower)[1]);
        RDM_Command._pd[2] = (((uint8_t*)lower)[2]);
        RDM_Command._pd[3] = (((uint8_t*)lower)[3]);
        RDM_Command._pd[4] = (((uint8_t*)lower)[4]);
        RDM_Command._pd[5] = (((uint8_t*)lower)[5]);
    }


    //upper bound
    if(lower == NULL){
        memset(RDM_Command._pd + 6u, 0xFF, 6);
    } else {
        RDM_Command._pd[6] = (((uint8_t*)upper)[0]);
        RDM_Command._pd[7] = (((uint8_t*)upper)[1]);
        RDM_Command._pd[8] = (((uint8_t*)upper)[2]);
        RDM_Command._pd[9] = (((uint8_t*)upper)[3]);
        RDM_Command._pd[10] = (((uint8_t*)upper)[4]);
        RDM_Command._pd[11] = (((uint8_t*)upper)[5]);
    }


    RDM_Command._needs_response = 1;

    setChecksum(&RDM_Command);
    prepareToLaunch(&RDM_Command);

    // hex_dumper((uint8_t *)&RDM_Command, sizeof(RDM_Command));    
}

/*
 * Function: initRDMTestClearPacket
 * ----------------------------
 *  sets up a test packet for clear discovery
 */ 
void initDiscoveryMutePacket(RdmUid* device){
    memset(((uint8_t*)&RDM_Command), 0x00, sizeof(RdmPacket));

    RDM_Command._start_code = 0xCC;
    RDM_Command._sub_start_code = 0x01;
    RDM_Command._length = 0x19 - 1;

    RDM_Command._dest_uid_man = device->_man;
    RDM_Command._dest_uid_id = device->_id;

    setSourceUid(&RDM_Command);

    RDM_Command._transaction_number = 0x00;
    RDM_Command._port_id_response_type = 0x01;
    RDM_Command._message_count = 0x00;
    RDM_Command._sub_device = 0x0000;

    RDM_Command._command_class = E120_DISCOVERY_COMMAND;
    RDM_Command._pid = to_right_endian_16(E120_DISC_MUTE);
    RDM_Command._pd_length = 0x00;

    RDM_Command._needs_response = 1;

    setChecksum(&RDM_Command);
    prepareToLaunch(&RDM_Command);

    // hex_dumper((uint8_t *)&RDM_Command, sizeof(RDM_Command));    
}

/*
 * Function: initRDMTestClearPacket
 * ----------------------------
 *  sets up a test packet for clear discovery
 */ 
void initDiscoveryClearPacket(){
    memset(((uint8_t*)&RDM_Command), 0x00, sizeof(RdmPacket));

    RDM_Command._start_code = 0xCC;
    RDM_Command._sub_start_code = 0x01;
    RDM_Command._length = 0x19 - 1;

    RDM_Command._dest_uid_man = 0xFFFF;
    RDM_Command._dest_uid_id = 0xFFFFFFFF;

    setSourceUid(&RDM_Command);

    RDM_Command._transaction_number = 0x00;
    RDM_Command._port_id_response_type = 0x01;
    RDM_Command._message_count = 0x00;
    RDM_Command._sub_device = 0x0000;

    RDM_Command._command_class = E120_DISCOVERY_COMMAND;
    RDM_Command._pid = to_right_endian_16(E120_DISC_UN_MUTE);
    RDM_Command._pd_length = 0x00;

    RDM_Command._needs_response = 1;

    setChecksum(&RDM_Command);
    prepareToLaunch(&RDM_Command);

    // hex_dumper((uint8_t *)&RDM_Command, sizeof(RDM_Command));    
}

/*
 * Function: initCommandListPacket
 * ----------------------------
 *  gets the pid's of the device
 */ 
void initCommandListPacket(RdmUid* device){
    memset(((uint8_t*)&RDM_Command), 0x00, sizeof(RdmPacket));

    RDM_Command._start_code = 0xCC;
    RDM_Command._sub_start_code = 0x01;
    RDM_Command._length = 0x19 - 1;

    RDM_Command._dest_uid_man = device->_man;
    RDM_Command._dest_uid_id = device->_id;

    setSourceUid(&RDM_Command);

    RDM_Command._transaction_number = 0x00;
    RDM_Command._port_id_response_type = 0x01;
    RDM_Command._message_count = 0x00;
    RDM_Command._sub_device = 0x0000;

    RDM_Command._command_class = E120_GET_COMMAND;
    RDM_Command._pid = to_right_endian_16(E120_SUPPORTED_PARAMETERS);
    RDM_Command._pd_length = 0x00;

    RDM_Command._needs_response = 1;

    setChecksum(&RDM_Command);
    prepareToLaunch(&RDM_Command);

    // hex_dumper((uint8_t *)&RDM_Command, sizeof(RDM_Command));    
}
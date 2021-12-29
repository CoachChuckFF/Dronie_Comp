/* Artnet 4 Op Code Defines
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * This file is for debug purposes only it just defines all of the
 * codes Artnet 4 uses so I can see what is being reveived
 *
 * All used codes or future to use codes are in blizzard_artnet_manager.h
 *
 * Protocol Manager -> artnet_codes.h
 *
 */

#ifndef ARTNET_CODES_H
#define ARTNET_CODES_H

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------- OP CODES ---------------------------------------*/

#define ART_OP_NZS 0x5100
#define ART_OP_SYNC 0x5200
#define ART_OP_TOD_REQUEST 0x8000
#define ART_OP_TOD_DATA 0x8100
#define ART_OP_TOD_CONTROL 0x8200
#define ART_OP_RDM 0x8300
#define ART_OP_RDM_SUB 0x8400
#define ART_OP_VIDEO_SETUP 0xA010
#define ART_OP_VIDEO_PALETTE 0xA020
#define ART_OP_VIDEO_DATA 0xA040
#define ART_OP_MAC_MASTER 0xF000 //depreciated
#define ART_OP_MAC_SLAVE 0xF100 //depreciated
#define ART_OP_FILE_TN_MASTER 0xF400
#define ART_OP_FILE_FN_MASTER 0xF500
#define ART_OP_FILE_FN_REPLY 0xF600
#define ART_OP_MEDIA 0x9000
#define ART_OP_MEDIA_PATCH 0x9100
#define ART_OP_MEDIA_CONTROL 0x9200
#define ART_OP_MEDIA_CONTROL_REPLY 0x9300
#define ART_OP_TIME_CODE 0x9700
#define ART_OP_TIME_SYNC 0x9800
#define ART_OP_TRIGGER 0x9900
#define ART_OP_DIRECTORY 0x9A00
#define ART_OP_DIRECTORY_REPLY 0x9B00

/*--------------------------- NODE REPORT CODES ------------------------------*/

#define ART_REP_DEBUG 0x0000 //Booted in debug mode (Only used in development)
#define ART_REP_POWER_OK 0x0001 //Power On Tests successful
#define ART_REP_POWER_FAIL 0x0002 //Hardware tests failed at Power On
#define ART_REP_SOCKET_WR1 0x0003 //Last UDP from Node failed due to truncated length, Most likely caused by a collision.
#define ART_REP_UDP_FAIL 0x0005 //Unable to open Udp Socket in last transmission attempt
#define ART_REP_SHORT_NAME_OK 0x0006 //Confirms that Short Name programming via ArtAddress, was successful.
#define ART_REP_LONG_NAME_OK 0x0007 //Confirms that Long Name programming via ArtAddress, was successful.
#define ART_REP_DMX_ERROR 0x0008 //DMX512 receive errors detected.
#define ART_REP_DMX_UDP_FULL 0x0009 //Ran out of internal DMX transmit buffers
#define ART_REP_DMX_RX_FULL 0x000A //Ran out of internal DMX Rx buffers.
#define ART_REP_SWITCH_ERROR 0x000B //Rx Universe switches conflict.
#define ART_REP_DMX_SHORT 0x000D //DMX output short detected. See GoodOutput field.
#define ART_REP_FIRMWARE_FAIL 0x000E //Last attempt to upload new firmware failed.
#define ART_REP_USER_FAIL 0x000F //User changed switch settings when address locked by remote programming. User changes ignored.
#define ART_REP_FACTORY_RESET 0x0010 //Factory reset has occured

/*--------------------------- STYLE CODES ------------------------------------*/

#define ART_STYLE_CONTROLLER 0x01
#define ART_STYLE_MEDIA 0x02
#define ART_STYLE_ROUTE 0x03
#define ART_STYLE_BACKUP 0x04
#define ART_STYLE_CONFIG 0x05
#define ART_STYLE_VISUAL 0x06

#ifdef __cplusplus
}
#endif

#endif

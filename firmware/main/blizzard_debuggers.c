// /* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
//  * Unauthorized copying of this file, via any medium is strictly prohibited
//  * Proprietary and confidential
//  * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
//  *
//  * Blizzard Reserves -> blizzard_generators.c
//  *
//  * 
//  */

// #include "blizzard_reserves.h"
// #include "blizzard_debuggers.h"
// #include <stdint.h>
// #include <stdlib.h> 
// #include <stdio.h>

// /*
//  * Function: hex_dumper(uint8_t* hex, uint32_t size)
//  * ----------------------------
//  * Generates a random u8 inclusive of min and max into the generated slot
//  * Returns: SUCCESS or FAIL
//  */
// uint8_t hex_dumper(uint8_t* hex, uint32_t size){
    
//     uint32_t i;

//     if(hex == NULL){
//         return RESERVES_ERROR;
//     }

//     if(size == 0){
//         return RESERVES_ERROR;
//     }

//     printf("--------------------\n");
//     for(i = 0; i < size; i++){
//         if(i % 16 == 0){
//             printf("\n");
//         }
//         printf("%02X ", hex[i]);
//     }
//     printf("\n--------------------\n");

//     return RESERVES_SUCCESS;
// }

// /*
//  * Function: bool to_right_endian_16(uint16_t* hex)
//  * ----------------------------
//  * Changes the u16 so it can be assigned to a u16 in a struct
//  */ 
// uint16_t to_right_endian_16(uint16_t hex){
//     uint16_t dummy = 0;

//     dummy = (hex >> 8u) & 0x00FF;
//     dummy |= (hex << 8u) & 0xFF00;

//     return dummy;
// }

// /*
//  * Function: bool to_right_endian_32(uint16_t* hex)
//  * ----------------------------
//  * Changes the u32 so it can be assigned to a u32 in a struct
//  */ 
// uint32_t to_right_endian_32(uint32_t hex){
//     uint32_t b0,b1,b2,b3;

//     b0 = (hex & 0x000000FF) << 24u;
//     b1 = (hex & 0x0000FF00) << 8u;
//     b2 = (hex & 0x00FF0000) >> 8u;
//     b3 = (hex & 0xFF000000) >> 24u;

//     return b0 | b1 | b2 | b3;
// }
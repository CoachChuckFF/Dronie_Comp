/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Blizzard Reserves -> blizzard_generators.c
 *
 * 
 */

#include "blizzard_reserves.h"
#include "blizzard_generators.h"
#include <stdint.h>
#include <stdlib.h> 

/*
 * Function: generate_random_number(uint32_t max_size, uint32_t min_size)
 * ----------------------------
 * Generates a random u8 inclusive of min and max into the generated slot
 * Returns: SUCCESS or FAIL
 */
uint8_t generate_random_u8(uint8_t* generated, uint8_t min, uint8_t max){
    // blizzert(max_size > min_size);

    if(generated == NULL){
        return RESERVES_ERROR;
    }

    if(min > max){
        return RESERVES_ERROR;
    }

    *generated = rand() % (max - min + 1) + min;

    return RESERVES_SUCCESS;
}

/*
 * Function: generate_random_string(char* generated, uint8_t char_count)
 * ----------------------------
 * Generates a random string of length 'char_count' given and places it in the generated slot
 * The length of the generated string is char_count + 1 because the generator will add a null
 * terminator. Therefor the biggest char_count is 0xFF - 1
 * The string needs to be freed from the caller. The length is returned in the length pointer.
 * 
 */
uint8_t generate_random_string(char* generated, uint8_t char_count){
    uint8_t i;
    char rng;

    if(generated == NULL){
        return RESERVES_ERROR;
    }

    if(char_count == 0){
        return RESERVES_ERROR;
    }

    if(char_count == 0xFF){
        return RESERVES_ERROR;
    }

    for(i = 0; i < char_count; i++){
        if(generate_random_u8((uint8_t*) &rng, ' ', '~') == RESERVES_ERROR){
            return RESERVES_ERROR;
        }
        generated[i] = rng;
    }

    generated[char_count] = '\0'; //null terminate

    return RESERVES_SUCCESS;
}

/*
 * Function: generate_garbage(uint8_t* generated, uint8_t length)
 * ----------------------------
 * Generates garbage data into the generated slot of length length
 * Returns: SUCCESS, FAILURE
 */
uint8_t generate_garbage(uint8_t* generated, uint8_t length){
    uint8_t rng;
    uint8_t i;

    if(generated == NULL){
        return RESERVES_ERROR;
    }

    if(length == 0){
        return RESERVES_ERROR;
    }

    for(i = 0; i < length; i++){
        if(generate_random_u8(&rng, 0, 0xFF) == RESERVES_ERROR){
            return RESERVES_ERROR;
        }

        generated[i] = rng;
    }

    return RESERVES_SUCCESS;
}
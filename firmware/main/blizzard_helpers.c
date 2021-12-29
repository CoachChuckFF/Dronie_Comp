/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Blizzard Helpers -> blizzard_helpers.c
 *
 * 
 */

#include "blizzard_helpers.h"
#include "blizzard_errors.h"
#include "blizzard_global_defines.h"
#include "esp_log.h"
#include <stdint.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>

static const char *Tag = "HELPERS";

/*
 * Function: blz_generate_u32(uint32_t min, uint32_t max)
 * ----------------------------
 */
uint32_t blz_generate_u32(uint32_t min, uint32_t max){
    return blz_min_u32(max, blz_max_u32(min, esp_random()));
}

/*
 * Function: generate_random_number(uint32_t max_size, uint32_t min_size)
 * ----------------------------
 * Generates a random u8 inclusive of min and max into the generated slot
 * Returns: SUCCESS or FAIL
 */
uint8_t generate_random_u8(uint8_t* generated, uint8_t min, uint8_t max){
    // blizzert(max_size > min_size);

    if(generated == NULL){
        return HELPER_ERRORS;
    }

    if(min > max){
        return HELPER_ERRORS;
    }

    *generated = rand() % (max - min + 1) + min;

    return SUCCESS;
}

uint8_t generate_random_u16(uint16_t* generated, uint16_t min, uint16_t max){
    // blizzert(max_size > min_size);

    if(generated == NULL){
        return HELPER_ERRORS;
    }

    if(min > max){
        return HELPER_ERRORS;
    }

    *generated = rand() % (max - min + 1) + min;

    return SUCCESS;
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
        return HELPER_ERRORS;
    }

    if(char_count == 0){
        return HELPER_ERRORS;
    }

    if(char_count == 0xFF){
        return HELPER_ERRORS;
    }

    for(i = 0; i < char_count; i++){
        if(generate_random_u8((uint8_t*) &rng, ' ', '~') == HELPER_ERRORS){
            return HELPER_ERRORS;
        }
        generated[i] = rng;
    }

    generated[char_count] = '\0'; //null terminate

    return SUCCESS;
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
        return HELPER_ERRORS;
    }

    if(length == 0){
        return HELPER_ERRORS;
    }

    for(i = 0; i < length; i++){
        if(generate_random_u8(&rng, 0, 0xFF) == HELPER_ERRORS){
            return HELPER_ERRORS;
        }

        generated[i] = rng;
    }

    return SUCCESS;
}

/*
 * Function: blz_min_uX(uintX_t a, uintX_t b)
 * ----------------------------
 * Grabs the min of 2 uints
 */
uint8_t blz_min_u8(uint8_t a, uint8_t b){return (a > b) ? b : a;}
uint16_t blz_min_u16(uint16_t a, uint16_t b){return (a > b) ? b : a;}
uint32_t blz_min_u32(uint32_t a, uint32_t b){return (a > b) ? b : a;}

/*
 * Function: blz_max_uX(uintX_t a, uintX_t b)
 * ----------------------------
 * Grabs the max of 2 uints
 */
uint8_t blz_max_u8(uint8_t a, uint8_t b){return (a > b) ? a : b;}
uint16_t blz_max_u16(uint16_t a, uint16_t b){return (a > b) ? a : b;}
uint32_t blz_max_u32(uint32_t a, uint32_t b){return (a > b) ? a : b;}

uint8_t blz_strlen(char * string, uint8_t max){
    uint8_t len = 0;

    if(string == NULL) return 0;

    for(uint8_t i = 0; i < max; i++){
        if(string[i] == '\0') break;
        len++;
    }

    return len;
}

/*
 * Function: blz_is_char_safe(char c)
 * ----------------------------
 * TRUE if its a safe char
 */
uint8_t blz_is_char_safe(char c){
    if(c >= '0' && c <= '9') return TRUE; //Numbers
    if(c >= 'A' && c <= 'Z') return TRUE; //Capital Letters
    if(c >= 'a' && c <= 'z') return TRUE; //Lower-Case Letters

    switch(c){
        case '/': return TRUE;
        case '_': return TRUE;
        case '.': return TRUE;
        case '\0': return TRUE;
    }

    return FALSE;
}

/*
 * Function: blz_str_to_safe(char * buffer)
 * ----------------------------
 * Sets 2 strings into a buffer
 */
void blz_str_to_safe(char * string, uint8_t len){
    for(uint8_t i = 0; i < len; i++){
        if(!blz_is_char_safe(string[i])){
            string[i] = '_';
        }
    }

    string[len - 1] = '\0';
}

/*
 * Function: blz_set_filepath(char * buffer, char * fp)
 * ----------------------------
 * Sets 2 strings into a buffer
 */
uint8_t blz_set_filepath(char * buffer, char * fp, uint8_t count){
    uint8_t workingLength = sizeof(SHOW_FILEPATH_PREAMBLE) + MAX_FILENAME_LENGTH + sizeof(SHOW_FILETYPE) - 1;
    uint8_t fpLen = 0;
    uint8_t pointer = 0;

    if(buffer == NULL){
        ESP_LOGE(Tag, "Null buffer - set filepath");
        return HELPER_ERRORS;
    } 

    fpLen = blz_strlen(fp, MAX_FILENAME_LENGTH);
    if(fpLen == 0){
        ESP_LOGE(Tag, "Empty filepath - set filepath");
        return HELPER_ERRORS;
    }

    //Clear out buffer
    memset(buffer + pointer, 0, workingLength); //Clear FP

    //Set directory path
    memcpy(buffer + pointer, SHOW_FILEPATH_PREAMBLE, sizeof(SHOW_FILEPATH_PREAMBLE) - 1); //Add directory path
    pointer += sizeof(SHOW_FILEPATH_PREAMBLE) - 1;

    //Add filename
    memcpy(buffer + pointer, fp, fpLen);
    pointer += fpLen;

    if(count != SET_FP_NO_CHANGE){
        sprintf(buffer + pointer, "_%03d", count);
        pointer += sizeof(SHOW_COUNTER) - 1;
    }

    memcpy(buffer + pointer, SHOW_FILETYPE, sizeof(SHOW_FILETYPE)); //Directory

    blz_str_to_safe(buffer, workingLength);

    return SUCCESS;
}


/*
 * Function: hex_dumper(uint8_t* hex, uint32_t size)
 * ----------------------------
 * Generates a random u8 inclusive of min and max into the generated slot
 * Returns: SUCCESS or FAIL
 */
uint8_t hex_dumper(uint8_t* hex, uint32_t size){
    
    uint32_t i;

    if(hex == NULL){
        return HELPER_ERRORS;
    }

    if(size == 0){
        return HELPER_ERRORS;
    }

    printf("--------------------\n");
    for(i = 0; i < size; i++){
        if(i % 16 == 0){
            printf("\n");
        }
        printf("%02X ", hex[i]);
    }
    printf("\n------------------\n");

    return SUCCESS;
}

/*
 * Function: bool to_right_endian_16(uint16_t* hex)
 * ----------------------------
 * Changes the u16 so it can be assigned to a u16 in a struct
 */ 
uint16_t to_right_endian_16(uint16_t hex){
    uint16_t dummy = 0;

    dummy = (hex >> 8u) & 0x00FF;
    dummy |= (hex << 8u) & 0xFF00;

    return dummy;
}

/*
 * Function: bool to_right_endian_32(uint16_t* hex)
 * ----------------------------
 * Changes the u32 so it can be assigned to a u32 in a struct
 */ 
uint32_t to_right_endian_32(uint32_t hex){
    uint32_t b0,b1,b2,b3;

    b0 = (hex & 0x000000FF) << 24u;
    b1 = (hex & 0x0000FF00) << 8u;
    b2 = (hex & 0x00FF0000) >> 8u;
    b3 = (hex & 0xFF000000) >> 24u;

    return b0 | b1 | b2 | b3;
}
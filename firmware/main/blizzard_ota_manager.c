/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * OTA Manager -> blizzard_ota_manager.c
 * 
 */

#include "blizzard_ota_manager.h"
#include "blizzard_show_manager.h"
#include "blizzard_dmx_manager.h"
#include "blizzard_artnet_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "blizzard_global_defines.h"

static const char *Tag = "OTA";
 
// const esp_partition_t *Boot_Partition;
// const esp_partition_t *Running_Partition;
const esp_partition_t *Update_Partition;
const esp_partition_t *Show_Partition;
esp_ota_handle_t Ota_Handle;
uint32_t Bytes_Written = 0;
uint32_t Current_Block = 0;
uint16_t OTA_Tick = 0;
uint8_t OTA_State = OTA_STATE_IDLE;

/*---------------------------- Initializers ----------------------------------*/
/*
 * Function: init_ota_manager
 * ----------------------------
 *  
 *  Inits the ota manager
 * 
 *  returns: 
 */ 
uint8_t init_ota_manager() {
	ESP_LOGI(Tag, "OTA Manager is taping up");
	ESP_LOGI(Tag, "OTA is going to #JDILN");
	return SUCCESS;
}

/*
 * Function: get_ota_state
 * ----------------------------
 * 
 *  returns: The OTA state
 */ 
uint8_t get_ota_state(){
    return OTA_State;
}

/*
 * Function: tick_ota
 * ----------------------------
 * 
 *  Timeout for OTA
 */ 
void tick_ota(){
    if(OTA_Tick){
        OTA_Tick--;
    } else {
        switch (OTA_State)
        {
        case OTA_STATE_SHOW:
            ESP_LOGE(Tag, "Show OTA Timeout");
            end_show_ota();
            break;
        case OTA_STATE_FIRMWARE:
            ESP_LOGE(Tag, "Firmware OTA Timeout");
            end_firmware_ota();
            break;
        default:
            ;;
            break; //IDLE
        }
    }
}

/*---------------------------- Controllers ----------------------------------*/
/*
 * Function: start_firmware_ota()
 * ----------------------------
 *  
 *  Starts the ota process by getting the correct partition information and 
 *  inits the ota handle
 * 
 *  returns: OTA_NULL_PARTITION_ERROR, OTA_START_ERROR, SUCCESS
 */ 
uint8_t start_firmware_ota(){
    esp_err_t err;

    //TODO check if we need these
    // Boot_Partition = esp_ota_get_boot_partition();
    // if(Boot_Partition == NULL){
    //     ESP_LOGE(Tag, "Failed to get boot partition - start Firmware OTA");
    //     return OTA_NULL_PARTITION_ERROR;
    // }

    // Running_Partition = esp_ota_get_running_partition();
    // if(Running_Partition == NULL){
    //     ESP_LOGE(Tag, "Failed to get running partition - start Firmware OTA");
    //     return OTA_NULL_PARTITION_ERROR;
    // }

    Update_Partition = esp_ota_get_next_update_partition(NULL);
    if(Update_Partition == NULL){
        ESP_LOGE(Tag, "Failed to get update partition - start Firmware OTA");
        return OTA_NULL_PARTITION_ERROR;
    }

    err = esp_ota_begin((const esp_partition_t *) Update_Partition, OTA_SIZE_UNKNOWN, &Ota_Handle); //erases the whole partition
    if (err != ESP_OK) {
        ESP_LOGE(Tag, "OTA begin failed: %s", esp_err_to_name(err));
        return OTA_START_ERROR;
    }

    Current_Block = 0;
    Bytes_Written = 0;
    OTA_Tick = OTA_TIMEOUT;
    OTA_State = OTA_STATE_FIRMWARE;

    ESP_LOGI(Tag, "Firmware OTA Started");
    return SUCCESS;
}

uint8_t read_show_data(uint32_t offset, uint16_t length, uint8_t* buffer){
    readShow(offset, buffer, length);
    return ART_READ_DONE;
}

uint8_t read_dmx_data(uint8_t* buffer){
    copy_from_dmx(buffer);
    return ART_READ_DONE;
}

/*
 * Function: start_show_ota()
 * ----------------------------
 *  
 *  Erases the entire show parition to be able to write to it!
 * 
 *  returns: OTA_NULL_PARTITION_ERROR, OTA_START_ERROR, SUCCESS
 */ 
uint8_t start_show_ota(){
    esp_err_t retVal;

    Show_Partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, 
        ESP_PARTITION_SUBTYPE_ANY,
        "show"
    );

    if(Show_Partition == NULL){
        ESP_LOGE(Tag, "Could not find show partition");
        return SHOW_PARTITION_ERROR;
    }

    //wipe the memory
    retVal = esp_partition_erase_range(Show_Partition, 0, Show_Partition->size);
    if(retVal != ESP_OK){
        ESP_LOGE(Tag, "Error erasing show partition - start show ota: %d", retVal);
        return SHOW_WRITE_ERROR;
    }

    Current_Block = 0;
    Bytes_Written = 0;
    OTA_Tick = OTA_TIMEOUT;
    OTA_State = OTA_STATE_SHOW;

    ESP_LOGI(Tag, "Show OTA Started");
    return SUCCESS;
}

/*
 * Function: write_ota(uint32_t bin_length, uint8_t *buffer, uint32_t packet_length)
 * ----------------------------
 *  
 *  Sequentially writes the next bytes into the partition
 *  the amount of bytes will always be the size of the Art-Net
 *  Firmware Master packet buffer - 512 u16s
 * 
 *  returns: OTA_NULL_BUFFER_ERROR, OTA_WRITE_ERROR, SUCCESS
 */ 
int write_ota(uint32_t block, uint32_t bin_length, uint8_t *buffer, uint32_t packet_length){
    esp_err_t retVal;

    if(buffer == NULL){
        ESP_LOGE(Tag, "Buffer is null - write OTA");
        return -1;
    }

    if(Current_Block != block){
        ESP_LOGE(Tag, "Block does not match %d =/ %d - write OTA", Current_Block, block);
        return Current_Block;
    }

    /*if(Bytes_Written > bin_length){
        ESP_LOGE(Tag, "Ota write failed: ota overflow %d of %d", Bytes_Written, bin_length);
        return OTA_WRITE_ERROR; 
    }*/

    switch (OTA_State)
    {
    case OTA_STATE_FIRMWARE:
        retVal = esp_ota_write(Ota_Handle, (const void *)buffer, packet_length);
        break;
    case OTA_STATE_SHOW:
        retVal = esp_partition_write(Show_Partition, Bytes_Written, buffer, packet_length);
        break;
    default:
        ESP_LOGE(Tag, "Invalid OTA state when writing OTA: %d", OTA_State);
        OTA_State = OTA_STATE_IDLE;
        return -1;
    }


    if (retVal != ESP_OK) {
        ESP_LOGE(Tag, "OTA write failed: %s", esp_err_to_name(retVal));
        return -1; 
    }

    Bytes_Written += packet_length;
    if(Bytes_Written < bin_length){
        Current_Block++;
        OTA_Tick = OTA_TIMEOUT;
        ESP_LOGI(Tag, "Bytes written: %d out of %d - next Block %d", Bytes_Written, bin_length, Current_Block);
    } else {
        ESP_LOGI(Tag, "All bytes written");
    }

    return Current_Block;
}

/*
 * Function: end_firmware_ota
 * ----------------------------
 *  
 *  Ends the ota process and sets the new boot partition
 * 
 *  returns: OTA_END_ERROR, OTA_SET_BOOT_PARTITION_ERROR, SUCCESS
 */ 
uint8_t end_firmware_ota(){
    esp_err_t err;

    err = esp_ota_end(Ota_Handle);
    if (err != ESP_OK) {
        ESP_LOGE(Tag, "Ota end failed: %s", esp_err_to_name(err));
        return OTA_END_ERROR;
    }
    
    err = esp_ota_set_boot_partition(Update_Partition);
    if (err != ESP_OK) {
        ESP_LOGE(Tag, "Ota set boot partiton failed: %s", esp_err_to_name(err));
        return OTA_SET_BOOT_PARTITION_ERROR;
    }

    OTA_State = OTA_STATE_IDLE;

    ESP_LOGI(Tag, "Firmware OTA ended");
    return SUCCESS;
}

/*
 * Function: end_show_ota
 * ----------------------------
 *  
 *  Changes state back to idle
 * 
 *  returns: SUCCESS
 */ 
uint8_t end_show_ota(){
    OTA_State = OTA_STATE_IDLE;

    ESP_LOGI(Tag, "Ota ended");
    return SUCCESS;
}
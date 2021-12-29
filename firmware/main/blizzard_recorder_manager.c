/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * OTA Manager -> blizzard_ota_manager.c
 * 
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "blizzard_global_defines.h"
#include "blizzard_show_manager.h"
#include "blizzard_dmx_manager.h"

#include "blizzard_global_defines.h"
#include "blizzard_show_manager.h"
#include "blizzard_dmx_manager.h"

#include "blizzard_recorder_manager.h"
#include "blizzard_protocol_manager.h"
#include "blizzard_simple_ui_manager.h"
#include "blizzard_simple_indication_manager.h"
#include "blizzard_show_manager.h"
#include <string.h>

static const char *Tag = "RECORDER";

const esp_partition_t *Show_Partition;
uint32_t Bytes_Recorded = 0;
uint32_t Frames_Recorded = 0;
uint32_t Time_Total = 0;
uint32_t Time = 0;
uint32_t Recorder_Tick = 0;
uint8_t Recorder_State = RECORDER_STATE_IDLE;
uint8_t Frame_Buffer[sizeof(BlizzardPlaybackFileFrame) + 512];
uint8_t Temp_DMX[512];

const uint8_t Recorded_file_name[16] = RECORDED_FILE_NAME;


/*---------------------------- Initializers ----------------------------------*/
/*
 * Function: init_recorder_manager
 * ----------------------------
 *  
 *  Inits the recorder manager
 * 
 *  returns: 
 */ 
uint8_t init_recorder_manager() {
	ESP_LOGI(Tag, "Recorder is practicing Hot Cross Buns");

	ESP_LOGI(Tag, "Recorder is shredding Through the Fire and the Flames");
	return SUCCESS;
}

/*
 * Function: start_recording
 * ----------------------------
 *  
 *  Wipes the show memory, writes the first frame and kicks off the tick
 *  
 */ 
void start_recording(void){
    uint8_t retVal;

    switch(Recorder_State){
        case RECORDER_STATE_IDLE:

            stop_show();

            Show_Partition = esp_partition_find_first(
                ESP_PARTITION_TYPE_DATA, 
                ESP_PARTITION_SUBTYPE_ANY,
                "show"
            );

            if(Show_Partition == NULL){
                ESP_LOGE(Tag, "Could not find show partition");
            }

            //wipe the memory
            retVal = esp_partition_erase_range(Show_Partition, 0, Show_Partition->size);
            if(retVal != ESP_OK){
                ESP_LOGE(Tag, "Error erasing show partition - start show ota: %d", retVal);
            }

            Frames_Recorded = 0;
            Bytes_Recorded = BPF_STARTING_DMX_OFFSET; //have to write metadata at the end
            Recorder_Tick = RECORDER_COUNTDOWN_TICK;

            start_listening();

            set_prerecording_ui_state();

            Recorder_State = RECORDER_STATE_RECORDING;
            break;
        default:
            return;
    }
}

/*
 * Function: stop_recording
 * ----------------------------
 *  
 *  writes the final frame and stops recording
 *  
 */ 
void stop_recording(void){
    switch(Recorder_State){
        case RECORDER_STATE_RECORDING:

            writeFrame();
            finishHeader();
            writeEnd();
            stop_listening();

            esp_restart();

            Recorder_State = RECORDER_STATE_IDLE;
            break;
        default:
            return;
    }
}

/*
 * Function: tick_recorder
 * ----------------------------
 * 
 *  Timeout for OTA
 */ 
void tick_recorder(){
    
    switch(Recorder_State){
        case RECORDER_STATE_RECORDING:
            if(Frames_Recorded == 0){
                Recorder_Tick--;

                //set countdown colors
                if(Recorder_Tick > 2500) change_indicator_color(BLIZZARD_YELLOW);
                else if (Recorder_Tick > 2000) change_indicator_color(BLIZZARD_BLACK);
                else if (Recorder_Tick > 1500) change_indicator_color(BLIZZARD_YELLOW);
                else if (Recorder_Tick > 1000) change_indicator_color(BLIZZARD_BLACK);
                else if (Recorder_Tick > 500) change_indicator_color(BLIZZARD_YELLOW);
                else change_indicator_color(BLIZZARD_BLACK);

                if(Recorder_Tick == 1){
                    copy_from_dmx(Temp_DMX);
                    writeFirstDMX();
                } else if(!Recorder_Tick){ //this is done because we need the first frame delay to be 1ms
                    setBuffer();
                    Time_Total++;
                    Frames_Recorded = 2;
                    Recorder_Tick = RECORDER_CHECK_FRAME_TICK;
                    set_recording_ui_state();
                }
            } else {
                Time_Total++;
                Time++;
                if(!(Recorder_Tick--)){
                    if(checkFrame()){
                        if(!hasEnoughSpace()){
                            stop_recording();
                            ESP_LOGE(Tag, "Ran out of space to record");
                            return;
                        }
                        writeFrame();
                        setBuffer();
                        Time = 0;
                        Frames_Recorded++;
                    }
                    Recorder_Tick = RECORDER_CHECK_FRAME_TICK;
                }
            }
            break;
        default:
            return;
    }
}

uint8_t get_recorder_state(){
    return Recorder_State;
}

/*---------------------------- Controllers -----------------------------------*/
/*
 * Function: writeFirstDMX
 * ----------------------------
 *  
 *  writes the starting DMX buffer into flash
 *  
 */ 
void writeFirstDMX(void){
    uint8_t retVal;

    retVal = esp_partition_write(Show_Partition, Bytes_Recorded, Temp_DMX, 512);
    if(retVal != ESP_OK){
        ESP_LOGE(Tag, "Error writing starting DMX frame");
    }

    Bytes_Recorded += 512;
}

/*
 * Function: checkFrame
 * ----------------------------
 *  
 *  checks for any changes
 *  
 */ 
bool checkFrame(void){
    uint16_t i = 0;

    for(i = 1; i <= 512; i++)
        if(Temp_DMX[i - 1] != get_dmx_value(i)) 
            return true;

    return false;
}

/*
 * Function: hasEnoughSpace
 * ----------------------------
 *  
 *  makes sure there is enough space for more
 *  
 */
bool hasEnoughSpace(void){
    return ((Bytes_Recorded + (BPF_MAX_FRAME_SIZE * 3)) < SHOW_BYTE_SIZE);
}

/*
 * Function: setBuffer
 * ----------------------------
 *  
 *  sets the current frame in the buffer
 *  
 */ 
void setBuffer(void){
    uint8_t magic[] = BPF_FRAME_MAGIC;
    BlizzardPlaybackFileFrame *frame = (BlizzardPlaybackFileFrame*) Frame_Buffer;
    BlizzardPlaybackFileFrameDiff diff;
    uint16_t diffs = 0, i = 1;

    //copy over header
    memcpy(Frame_Buffer, magic, sizeof(magic));

    for(i = 1; i <= 512; i++){
        if(Temp_DMX[i-1] != get_dmx_value(i)){
            diff._address = i;
            diff._value = get_dmx_value(i);
            memcpy(
                (Frame_Buffer + 
                sizeof(BlizzardPlaybackFileFrame) + 
                (sizeof(BlizzardPlaybackFileFrameDiff) * diffs++)),
                &diff,
                sizeof(BlizzardPlaybackFileFrameDiff)
            );
            frame->_diff_count = diffs;
        }
        if(diffs >= 128){
            frame->_diff_count = 0xFFFF;
            break;
        }
    }

    copy_from_dmx(Temp_DMX);
}

/*
 * Function: writeFrame
 * ----------------------------
 *  
 *  reads the frame in the buffer and then writes it to memory
 *  
 */ 
void writeFrame(void){
    BlizzardPlaybackFileFrame *frame = (BlizzardPlaybackFileFrame*) Frame_Buffer;
    uint8_t retVal;
    uint16_t length;

    frame->_delay = Time;

    length = (frame->_diff_count != 0xFFFF) ? 
        (frame->_diff_count * sizeof(BlizzardPlaybackFileFrameDiff)) :
        512;

    length += sizeof(BlizzardPlaybackFileFrame);

    retVal = esp_partition_write(Show_Partition, Bytes_Recorded, Frame_Buffer, length);
    if(retVal != ESP_OK){
        ESP_LOGE(Tag, "Error writing frame");
    }

    Bytes_Recorded += length;
}


/*
 * Function: finishHeader
 * ----------------------------
 *  
 *  Writes the header with totals
 *  
 */ 
void finishHeader(void){
    uint8_t magic[] = BPF_HEAD_MAGIC;
    uint8_t retVal;
    uint32_t initalDelay = 1;

    retVal = esp_partition_write(Show_Partition, BPF_MAGIC_OFFSET, magic, sizeof(magic));
    if(retVal != ESP_OK){
        ESP_LOGE(Tag, "Error writing header metadata - magic");
    }

    retVal = esp_partition_write(Show_Partition, BPF_TOTAL_FRAMES_OFFSET, &Frames_Recorded, sizeof(uint32_t));
    if(retVal != ESP_OK){
        ESP_LOGE(Tag, "Error writing header metadata - frames");
    }

    retVal = esp_partition_write(Show_Partition, BPF_TOTAL_TIME_OFFSET, &Time_Total, sizeof(uint32_t));
    if(retVal != ESP_OK){
        ESP_LOGE(Tag, "Error writing header metadata - time");
    }

    //1ms delay, the second frame is a 0 change frame,
    retVal = esp_partition_write(Show_Partition, BPF_FIRST_DELAY_OFFSET, &initalDelay, sizeof(uint32_t));
    if(retVal != ESP_OK){
        ESP_LOGE(Tag, "Error writing header metadata - delay");
    }

    retVal = esp_partition_write(Show_Partition, BPF_NAME_OFFSET, Recorded_file_name, sizeof(Recorded_file_name));
    if(retVal != ESP_OK){
        ESP_LOGE(Tag, "Error writing header metadata - name");
    }
}

/*
 * Function: writeEnd
 * ----------------------------
 *  
 *  Writes the end magic bytes
 *  
 */ 
void writeEnd(void){
    uint8_t magic[] = BPF_TAIL_STOP_CODE;
    uint8_t retVal;

    retVal = esp_partition_write(Show_Partition, Bytes_Recorded, magic, sizeof(magic));
    if(retVal != ESP_OK){
        ESP_LOGE(Tag, "Error writing end of recording");
    }
}

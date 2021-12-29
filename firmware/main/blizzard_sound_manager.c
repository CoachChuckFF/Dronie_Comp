



/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * UI Manager -> blizzard_button_manager.c
 * 
 */

#include "blizzard_sound_manager.h"
#include "blizzard_global_defines.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spi_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_partition.h"
#include "driver/i2s.h"
#include "driver/adc.h"
// #include "esp_adc_cal.h"
#include "esp_rom_sys.h"

#include "sound_chirp.h"
#include "sound_dot.h"
#include "sound_dash.h"
#include "sound_cheep.h"
#include "sound_ohhi.h"

/*---------------------------------------------------------------
                            SPEAKER CONFIG
---------------------------------------------------------------*/

#define SPEAKER_PIN 25
#define SPEAKER_PIN_BIT (1ULL<<SPEAKER_PIN)
//i2s number
#define SPEAKER_I2S_NUM (0)
//i2s sample rate
#define SPEAKER_I2S_SAMPLE_RATE   (16000)
//i2s data bits
#define SPEAKER_I2S_SAMPLE_BITS   (16)
//I2S read buffer length
#define SPEAKER_I2S_READ_LEN      (16 * 1024)
//I2S data format
#define SPEAKER_I2S_FORMAT        (I2S_CHANNEL_FMT_RIGHT_LEFT)
//I2S channel number
#define EXAMPLE_I2S_CHANNEL_NUM   ((SPEAKER_I2S_FORMAT < I2S_CHANNEL_FMT_ONLY_RIGHT) ? (2) : (1))
//I2S built-in ADC unit
#define I2S_ADC_UNIT              ADC_UNIT_1
//I2S built-in ADC channel
#define I2S_ADC_CHANNEL           ADC1_CHANNEL_0
//sector size of flash
#define FLASH_SECTOR_SIZE         (0x1000)
//flash read / write address
#define FLASH_ADDR                (0x200000)

static const char *Tag = "SOUND";

uint8_t Speaker_Buffer[SPEAKER_I2S_READ_LEN];

uint8_t Ready = FALSE;
uint8_t Playing = FALSE;
uint8_t Clip_Index = CLIP_CHIRP;


void playSound(void *arg){
    uint32_t offset;
    uint32_t clipSize;
    const uint8_t* audioTable = NULL;
    size_t bytes;

    i2s_stop(SPEAKER_I2S_NUM);

    while(1){
        if(Ready){
            Playing = TRUE;
            Ready = FALSE;

            switch(Clip_Index){
                case CLIP_CHIRP: audioTable = chirp_table; clipSize = sizeof(chirp_table); break;
                case CLIP_DOT: audioTable = dot_table; clipSize = sizeof(dot_table); break;
                case CLIP_OHHI: audioTable = ohhi_table; clipSize = sizeof(ohhi_table); break;
                default: clipSize = 0; break;
            }

            offset = 0;
            i2s_start(SPEAKER_I2S_NUM);
            i2s_zero_dma_buffer(SPEAKER_I2S_NUM);

            while (offset < clipSize && Playing && audioTable != NULL) {
                // memset(Speaker_Buffer, 0, sizeof(Speaker_Buffer));
                int play_len = ((clipSize - offset) > (4 * 1024)) ? (4 * 1024) : (clipSize - offset);
                int i2s_wr_len = scaleSound(Speaker_Buffer, (uint8_t*)(audioTable + offset), play_len);
                i2s_write(SPEAKER_I2S_NUM, Speaker_Buffer, i2s_wr_len, &bytes, portMAX_DELAY);
                offset += play_len;
            }

            vTaskDelay(250);
            i2s_zero_dma_buffer(SPEAKER_I2S_NUM);
            vTaskDelay(250);
            i2s_stop(SPEAKER_I2S_NUM);
            i2s_zero_dma_buffer(SPEAKER_I2S_NUM);
            Playing = FALSE;
        } else {
            vTaskDelay(10);
        }
    }

    vTaskDelete(NULL);
}

uint8_t init_sound_manager() {

    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
        .sample_rate =  SPEAKER_I2S_SAMPLE_RATE,
        .bits_per_sample = SPEAKER_I2S_SAMPLE_BITS,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .channel_format = SPEAKER_I2S_FORMAT,
        .tx_desc_auto_clear = true,
        .intr_alloc_flags = 0,
        .dma_buf_count = 2,
        .dma_buf_len = 1024,
        .use_apll = 1,
    };
    ESP_LOGI(Tag, "Tweet Tweet");

    //install and start i2s driver
    assert(i2s_driver_install(SPEAKER_I2S_NUM, &i2s_config, 0, NULL) == SUCCESS);
    //init DAC pad
    assert(i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN) == SUCCESS);
    //init ADC pad
    // assert(i2s_set_adc_mode(I2S_ADC_UNIT, I2S_ADC_CHANNEL) == SUCCESS);

    assert(i2s_set_clk(SPEAKER_I2S_NUM, 16000, SPEAKER_I2S_SAMPLE_BITS, 1) == SUCCESS);

    xTaskCreate(playSound, "play_sound", 1024 * 2, NULL, 5, NULL);

    ESP_LOGI(Tag, "Beep Boop");

    return SUCCESS;
}

void play_sound(uint8_t clip){    
    stop_sound();

    switch(clip){
        case CLIP_CHIRP: Clip_Index = clip; Ready = TRUE; ESP_LOGE(Tag, "[Sound] Queued Chrip"); break;
        case CLIP_DOT: Clip_Index = clip; Ready = TRUE; ESP_LOGE(Tag, "[Sound] Queued Moon"); break;
        case CLIP_OHHI: Clip_Index = clip; Ready = TRUE; ESP_LOGE(Tag, "[Sound] Queued Oh Hi"); break;
    }

}

void stop_sound(){
    Playing = FALSE;
    Ready = FALSE;
}

uint8_t get_playing(){
    return Playing || Ready;
}

int scaleSound(uint8_t* d_buff, uint8_t* s_buff, uint32_t len)
{
    uint32_t j = 0;
#if (SPEAKER_I2S_SAMPLE_BITS == 16)
    for (int i = 0; i < len; i++) {
        d_buff[j++] = 0;
        d_buff[j++] = s_buff[i];
    }
    return (len * 2);
#else
    for (int i = 0; i < len; i++) {
        d_buff[j++] = 0;
        d_buff[j++] = 0;
        d_buff[j++] = 0;
        d_buff[j++] = s_buff[i];
    }
    return (len * 4);
#endif
}

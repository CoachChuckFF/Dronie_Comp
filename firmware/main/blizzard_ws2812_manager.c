/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, November 2018
 *
 * Protocol Manager -> blizzard_ws8212_driver.c
 *
 * The ws2812 manager is able to control X amount of WS2812 compatible leds.
 * It uses a modified spi protocol only useing the MOSI pin. The frequency is set 
 * 4x the frequency a WS2812 led requires.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "blizzard_ws2812_manager.h"

static const char *Tag = "WS2812";

static led_t* Leds = NULL;
static spi_device_handle_t Spi;
static uint8_t Running = 0;
static uint16_t Led_Length = 0;
static uint32_t Led_Buffer_Size = 0;
static double Brightness = 1;


static void ws2812_spi_post_transfer_callback(spi_transaction_t *t)
{
    spi_transaction_t trans;

    if(Running){  
        memset(&trans, 0, sizeof(spi_transaction_t));

        trans.length=Led_Buffer_Size*8;
        trans.tx_buffer = Leds;
        trans.rxlength = 0;
        trans.rx_buffer = NULL;
        trans.user=(void*)3;

        //send payload
        spi_device_queue_trans(Spi, &trans, portMAX_DELAY);
    }
}

uint8_t init_ws2812_manager(uint16_t length)
{
    esp_err_t retVal;

    ESP_LOGI(Tag, "5050's are doubling down...");

    Led_Buffer_Size = (length * sizeof(led_t)) + (RESET_LED_LENGTH * sizeof(uint32_t));

    //allocate leds buffer
    Leds = (led_t*) heap_caps_malloc(Led_Buffer_Size, MALLOC_CAP_DMA); 
    if(Leds == NULL){
		ESP_LOGE(Tag, "Error configuring gpio - dmx init: %d", 53);
		return 53; //TODO change
	}

    memset(Leds, 0, Led_Buffer_Size);

    Led_Length = length;

    spi_bus_config_t buscfg={
        .miso_io_num=WS_MISO_PIN,
        .mosi_io_num=WS_MOSI_PIN,
        .sclk_io_num=WS_CLK_PIN,
        .quadwp_io_num=(-1),
        .quadhd_io_num=(-1),
        .max_transfer_sz=Led_Buffer_Size,
    };

    spi_device_interface_config_t devcfg={
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .input_delay_ns = 0,
        .flags = SPI_DEVICE_NO_DUMMY,
        .clock_speed_hz = 3.333*1000*1000,     //3.3MHz
        .mode=0,                               //SPI mode 0
        .spics_io_num=WS_CS_PIN,               //CS pin
        .queue_size=1,                         //We want to be able to queue 7 transactions at a time
        .post_cb=ws2812_spi_post_transfer_callback,  //Specify post-transfer callback to handle D/C line
    };

    retVal = spi_bus_initialize(VSPI_HOST, &buscfg, 1);
    if(retVal != ESP_OK){
		ESP_LOGE(Tag, "Error configuring spi bus - ws2812 init: %d", retVal);
		return 57;
	}

    retVal = spi_bus_add_device(VSPI_HOST, &devcfg, &Spi);
    if(retVal != ESP_OK){
		ESP_LOGE(Tag, "Error adding spi bus - ws2812 init: %d", retVal);
		return 58;
	}

    Running = 1;
    set_all_leds(0x00, 0x00, 0x00);

    //Start
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));

    trans.length=Led_Buffer_Size*8;
    trans.tx_buffer = Leds;
    trans.rxlength = 0;
    trans.rx_buffer = NULL;
    trans.user=(void*)3;

    spi_device_queue_trans(Spi, &trans, portMAX_DELAY);
    set_all_leds(0x00, 0x00, 0x00);

    ESP_LOGI(Tag, "Jackpot!");

    return 0;
}

uint8_t redrum_ws2812_manager(){
    if(Running){
        Running = 0;
        vTaskDelay(1); //let current transfer finish
        //ESP_ERROR_CHECK(spi_bus_remove_device(Spi));
        //ESP_ERROR_CHECK(spi_bus_free(VSPI_HOST));
        heap_caps_free((void*)Leds);
    }
    return 0;
}

void set_all_leds(uint8_t red, uint8_t green, uint8_t blue){
    uint16_t i;

    for(i = 0; i < Led_Length; i++){
        set_led(i, red, green, blue);
    }
}

void set_all_leds_bright(uint8_t red, uint8_t green, uint8_t blue, double brightness){
    uint16_t i;

    for(i = 0; i < Led_Length; i++){
        set_led_bright(i, red, green, blue, brightness);
    }
}

void set_all_leds_color(color_t color){
    set_all_leds(color.red, color.green, color.blue);
}

void set_all_leds_color_bright(color_t color, double brightness){
    set_all_leds_bright(color.red, color.green, color.blue, brightness);
}

void set_led(uint16_t index, uint8_t red, uint8_t green, uint8_t blue){

    if(index > Led_Length - 1){
        return;
    }

    if(Brightness < 1 && Brightness >= 0){
        red = (uint8_t)(ceil(((double)red) * Brightness));
        green = (uint8_t)(ceil(((double)green) * Brightness));
        blue = (uint8_t)(ceil(((double)blue) * Brightness));
    }

    ledValueTransform(red, Leds[index].red);
    ledValueTransform(green, Leds[index].green);
    ledValueTransform(blue, Leds[index].blue);

}

void set_led_bright(uint16_t index, uint8_t red, uint8_t green, uint8_t blue, double brightness){
    if(index > Led_Length - 1){
        return;
    }

    if(brightness < 1 && brightness >= 0){
        red = (uint8_t)(floor(((double)red) * brightness));
        green = (uint8_t)(floor(((double)green) * brightness));
        blue = (uint8_t)(floor(((double)blue) * brightness));
    }

    ledValueTransform(red, Leds[index].red);
    ledValueTransform(green, Leds[index].green);
    ledValueTransform(blue, Leds[index].blue);
}

void set_led_color(uint16_t index, color_t color){
    set_led(index, color.red, color.green, color.blue);
}

void set_led_color_bright(uint16_t index, color_t color, double brightness){
    set_led_bright(index, color.red, color.green, color.blue, brightness);
}


void ledValueTransform(uint8_t value, uint8_t* dest){
    dest[3] = (value & 0x01) ? 0x0E : 0x08;
    dest[3] |= (value & 0x02) ? 0xE0 : 0x80;
    dest[2] = (value & 0x04) ? 0x0E : 0x08;
    dest[2] |= (value & 0x08) ? 0xE0 : 0x80;
    dest[1] = (value & 0x10) ? 0x0E : 0x08;
    dest[1] |= (value & 0x20) ? 0xE0 : 0x80;
    dest[0] = (value & 0x40) ? 0x0E : 0x08;
    dest[0] |= (value & 0x80) ? 0xE0 : 0x80;
}
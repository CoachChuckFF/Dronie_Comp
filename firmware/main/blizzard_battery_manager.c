/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Action Manager -> blizzard_action_manager.c
 *
 */


#include "blizzard_battery_manager.h"

#include <math.h>
#include "blizzard_network_manager.h"
#include "blizzard_simple_button_manager.h"
#include "blizzard_dmx_manager.h"
#include "blizzard_simple_indication_manager.h"
#include "blizzard_ws2812_manager.h"
#include "blizzard_global_defines.h"
#include "esp32/ulp.h"
#include "ulp_sleep.h"
#include "esp_sleep.h"
#include "blizzard_simple_ui_manager.h"
#include "driver/adc.h"


static const char *Tag = "BATTERY";

extern const uint8_t ulp_sleep_bin_start[] asm("_binary_ulp_sleep_bin_start");
extern const uint8_t ulp_sleep_bin_end[] asm("_binary_ulp_sleep_bin_end");

uint8_t init_battery_manager(){

    gpio_config_t io_conf;

    ESP_LOGI(Tag, "Battery is jucing up...");

    //turn all unused pins to input
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.pin_bit_mask = UNUSED_PIN_MASK;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 1;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    //turn gpio39 dac (battery level)
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(BATTERY_LEVEL_GPIO, BATTERY_LEVEL_ATTEN);

    //sets pgood and charge pins
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.pin_bit_mask = (BATTERY_SOURCE_PIN_BIT | BATTERY_CHARGE_PIN_BIT); 
    io_conf.mode = GPIO_MODE_INPUT;
    //they have external pullups
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 0;
    gpio_config(&io_conf);

    //CHeck Low Battery
    get_battery_level();

    ESP_LOGI(Tag, "Battery Anger!");

    //check battery voltage if too low turn off
    return SUCCESS;
}

void tick_battery(){
    get_battery_level();
}

//goes to low power mode
void go_the_fuck_to_sleep(){

    stopWifi();
    stop_dmx();
    uart_driver_delete(DMX_UART);
    sleep_animation();

    power_indicator(false);
    redrum_ws2812_manager();

    //load in the ulp binary
    ESP_ERROR_CHECK(ulp_load_binary(0, ulp_sleep_bin_start, (ulp_sleep_bin_end - ulp_sleep_bin_start) / sizeof(uint32_t)));

    //enable adc for ulp
    adc1_ulp_enable();

    //set ulp wakeup period
    ulp_set_wakeup_period(0, 20000); //TODO make macro (us)

    //set ulp varables
    ulp_detect_high = BUTTON_PRESSED_HIGH_LEVEL;
    ulp_detect_low = BUTTON_PRESSED_LOW_LEVEL;
    ulp_wakeup_count = ULP_WAKEUP_COUNT; //TODO make macro
    ulp_held_count = 0; //clears value

    //disable logging
    esp_deep_sleep_disable_rom_logging();

    ESP_LOGI(Tag, "Going quietly into this dark night...");

    ESP_ERROR_CHECK(ulp_run(&ulp_entry - RTC_SLOW_MEM) );
    ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());

    esp_deep_sleep_start();
}


bool battery_is_charging(void){
    return (gpio_get_level(BATTERY_CHARGE_PIN) == LOW);
}

bool battery_has_valid_input(void){
    return (gpio_get_level(BATTERY_SOURCE_PIN) == LOW);
}

uint8_t get_battery_state(){
    if(battery_is_charging()){
        return BATTERY_STATE_CHARGING;
    }

    if(battery_has_valid_input()){
        return BATTERY_STATE_DONE_CHARGING;
    }

    return BATTERY_STATE_NOT_CHARGING;
}

uint8_t get_battery_level(void){
    static double average[BATTERY_AVERAGE_RESOLUTION];
    static uint32_t tick = 0;
    uint8_t i, level = 0;
    uint16_t ave;
    double movingAverage = 0;
    //if too low power down device

    average[tick++ % BATTERY_AVERAGE_RESOLUTION] = (double)adc1_get_raw(BATTERY_LEVEL_GPIO);

    if(tick < BATTERY_AVERAGE_RESOLUTION){
        return 0xFF; //invalid number
    }

    for(i = 0; i < BATTERY_AVERAGE_RESOLUTION; i++){
        movingAverage += (double)average[i];
    }

    movingAverage /= ((double)BATTERY_AVERAGE_RESOLUTION);

    ave = (uint16_t)(floor(movingAverage));

    set_ui_crit(false);

    if(ave > 2900){ //4.2
        level = 100;
    } else if(ave > 2830){ //4.1
        level = 94;
    } else if(ave > 2750){ //4.0
        level = 85;
    } else if(ave > 2700){ //3.9
        level = 76;
    } else if(ave > 2650){ //3.8
        level = 66;
    } else if(ave > 2550){ //3.7
        level = 54;
    } else if(ave > 2470){ //3.6
        level = 21;
    } else if(ave > 2400){ //3.5
        level = 5;
        //set state to crit
        set_ui_crit(true);

    } else {
        if(!battery_has_valid_input()){
            danger_battery_animation();
            go_the_fuck_to_sleep();
        }
    }

    return level;
}

/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * UI Manager -> blizzard_indication_manager.c
 * 
 */

#include "blizzard_simple_indication_manager.h"
#include "blizzard_ws2812_manager.h"
#include "blizzard_dmx_manager.h"
#include "blizzard_nvs_manager.h"
#include "blizzard_network_manager.h"
#include "blizzard_colors.h"
#include "driver/gpio.h"
#include <math.h>
#include <stdio.h> 
#include <string.h> 

#define PI 3.14159265

static const char *Tag = "INDICATION";
color_t Current_Color;
uint8_t QC_Active;

/*---------------------------- Initializers ----------------------------------*/
/*
 * Function: init_indication_manager
 * ----------------------------
 *  
 *  Inits the indication manager by configuring the led gpios
 * 
 *  returns: INDICATION_CONFIG_ERROR, SUCCESS
 */ 
uint8_t init_indication_manager() {
    esp_err_t retVal;
    gpio_config_t ioConfig = {
		.intr_type = GPIO_PIN_INTR_DISABLE,
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask = LED_ENABLE_PIN_BIT,
		.pull_down_en = DISABLE,
		.pull_up_en = DISABLE
	};

    ESP_LOGI(Tag, "Indicators charging...");

    retVal = gpio_config(&ioConfig);
	if(retVal != ESP_OK){
		ESP_LOGE(Tag, "Error configuring gpio - init indication manager: %d", retVal);
		return DMX_GPIO_ERROR;
	}

    power_indicator(true);

    retVal = init_ws2812_manager(1);
    if(retVal != SUCCESS){
        ESP_LOGE(Tag, "Init indicators failed - ws2812: %d", retVal);
        return INDICATION_CONFIG_ERROR;
    }

    //Grab QC Flag
    retVal = get_nvs_config(QC_KEY, DATA_U8, &QC_Active);
    if(retVal != SUCCESS){
        ESP_LOGE(Tag, "No QC Flag: %d", retVal);
    }


    ESP_LOGI(Tag, "Indicators Charged!");

    return SUCCESS;
}

/*---------------------------- Controllers ----------------------------------*/

/*
 * Function: tick_indicators()
 * ----------------------------
 *  
 *  Sets the correct color depending on some flags
 *  Errors > Actions > Connection
 * 
 *  returns: the color it was changed to or an error
 */ 
void tick_indicator(uint8_t state){

    if(QC_Active){
        if(check_network_connection(CONNECTION_WIFI)){
            state = UI_QC_STATE_CONNECTED;
        } else {
            state = UI_QC_STATE_CONNECTING;
        }
    }

    switch(state){
        case UI_IDLE_STATE_WIFI_EZ_CONNECTING:
            handleAnimation(UI_IDLE_STATE_WIFI_EZ_CONNECTING_ANIMATION, UI_IDLE_STATE_WIFI_EZ_CONNECTING_COLOR);
        break;
        case UI_IDLE_STATE_WIFI_CONNECTING:
            handleAnimation(UI_IDLE_STATE_WIFI_CONNECTING_ANIMATION, UI_IDLE_STATE_WIFI_CONNECTING_COLOR);
        break;
        case UI_IDLE_STATE_WIFI_CONNECTED:
            handleAnimation(UI_IDLE_STATE_WIFI_CONNECTED_ANIMATION, UI_IDLE_STATE_WIFI_CONNECTED_COLOR);
        break;
        case UI_IDLE_STATE_WIFI_ERROR_BAD_PASS:
            handleAnimation(UI_IDLE_STATE_WIFI_ERROR_BAD_PASS_ANIMATION, UI_IDLE_STATE_WIFI_ERROR_BAD_PASS_COLOR);
        break;
        case UI_IDLE_STATE_WIFI_ERROR_BAD_SSID:
            handleAnimation(UI_IDLE_STATE_WIFI_ERROR_BAD_SSID_ANIMATION, UI_IDLE_STATE_WIFI_ERROR_BAD_SSID_COLOR);
        break;
        case UI_IDLE_STATE_WIFI_ERROR_BAD_WIFI: //generic something wrong signal
            handleAnimation(UI_IDLE_STATE_WIFI_ERROR_BAD_WIFI_ANIMATION, UI_IDLE_STATE_WIFI_ERROR_BAD_WIFI_COLOR);
        break;
        case UI_IDLE_STATE_AP_IDLE:
            handleAnimation(UI_IDLE_STATE_AP_IDLE_ANIMATION, UI_IDLE_STATE_AP_IDLE_COLOR);
        break;
        case UI_IDLE_STATE_AP_DEVICE_CONNECTED:
            handleAnimation(UI_IDLE_STATE_AP_DEVICE_CONNECTED_ANIMATION, UI_IDLE_STATE_AP_DEVICE_CONNECTED_COLOR);
        break;
        case UI_IDLE_STATE_MUTE:
            handleAnimation(UI_IDLE_STATE_MUTE_ANIMATION, UI_IDLE_STATE_MUTE_COLOR);
        break;
        case UI_IDLE_STATE_LOCATE:
            handleAnimation(UI_IDLE_STATE_LOCATE_ANIMATION, UI_IDLE_STATE_LOCATE_COLOR);
        break;
        case UI_BATTERY_STATE_LEVEL_CRIT:
            handleAnimation(UI_BATTERY_STATE_LEVEL_CRIT_ANIMATION, UI_BATTERY_STATE_LEVEL_CRIT_COLOR);
        break;
        case UI_BATTERY_STATE_LEVEL_5:
            handleAnimation(UI_BATTERY_STATE_LEVEL_5_ANIMATION, UI_BATTERY_STATE_LEVEL_5_COLOR);
        break;
        case UI_BATTERY_STATE_LEVEL_4:
            handleAnimation(UI_BATTERY_STATE_LEVEL_4_ANIMATION, UI_BATTERY_STATE_LEVEL_4_COLOR);
        break;
        case UI_BATTERY_STATE_LEVEL_3:
            handleAnimation(UI_BATTERY_STATE_LEVEL_3_ANIMATION, UI_BATTERY_STATE_LEVEL_3_COLOR);
        break;
        case UI_BATTERY_STATE_LEVEL_2:
            handleAnimation(UI_BATTERY_STATE_LEVEL_2_ANIMATION, UI_BATTERY_STATE_LEVEL_2_COLOR);
        break;
        case UI_BATTERY_STATE_LEVEL_1:
            handleAnimation(UI_BATTERY_STATE_LEVEL_1_ANIMATION, UI_BATTERY_STATE_LEVEL_1_COLOR);
        break;
        case UI_BATTERY_STATE_CHARGING:
            handleAnimation(UI_BATTERY_STATE_CHARGING_ANIMATION, UI_BATTERY_STATE_CHARGING_COLOR);
        break;
        case UI_BATTERY_STATE_DONE_CHARGING:
            handleAnimation(UI_BATTERY_STATE_DONE_CHARGING_ANIMATION, UI_BATTERY_STATE_DONE_CHARGING_COLOR);
        break;
        case UI_USER_STATE_UNDEFINED:
            handleAnimation(UI_USER_STATE_UNDEFINED_ANIMATION, UI_USER_STATE_UNDEFINED_COLOR);
        break;
        case UI_USER_STATE_RECORD:
            handleAnimation(UI_USER_STATE_RECORD_ANIMATION, UI_USER_STATE_RECORD_COLOR);
        break;
        case UI_USER_STATE_RECORD_CONFIRM:
            handleAnimation(UI_USER_STATE_RECORD_CONFIRM_ANIMATION, UI_USER_STATE_RECORD_CONFIRM_COLOR);
        break;
        case UI_USER_STATE_PRE_RECORDING:
            ;;
        break;
        case UI_USER_STATE_RECORDING:
            handleAnimation(UI_USER_STATE_RECORDING_ANIMATION, UI_USER_STATE_RECORDING_COLOR);
        break;
        case UI_USER_STATE_LISTEN:
            handleAnimation(UI_USER_STATE_LISTEN_ANIMATION, UI_USER_STATE_LISTEN_COLOR);
        break;  
        case UI_USER_STATE_LISTEN_CONFIRM:
            handleAnimation(UI_USER_STATE_LISTEN_CONFIRM_ANIMATION, UI_USER_STATE_LISTEN_CONFIRM_COLOR);
        break;
        case UI_USER_STATE_LISTENING:
            // handleAnimation(UI_USER_STATE_LISTENING_ANIMATION, UI_USER_STATE_LISTENING_COLOR);
            handleAnimation(ANIMATION_SOLID, get_dmx_color(1));
        break;
        case UI_USER_STATE_REBOOT:
            handleAnimation(UI_USER_STATE_REBOOT_ANIMATION, UI_USER_STATE_REBOOT_COLOR);
        break;
        case UI_USER_STATE_START_AP:
            handleAnimation(UI_USER_STATE_START_AP_ANIMATION, UI_USER_STATE_START_AP_COLOR);
        break;     
        case UI_USER_STATE_START_AP_CONFIRM:
            handleAnimation(UI_USER_STATE_START_AP_CONFIRM_ANIMATION, UI_USER_STATE_START_AP_CONFIRM_COLOR);
        break;
        case UI_USER_STATE_START_WIFI:
            handleAnimation(UI_USER_STATE_START_WIFI_ANIMATION, UI_USER_STATE_START_WIFI_COLOR);
        break;
        case UI_USER_STATE_START_WIFI_CONFIRM:
            handleAnimation(UI_USER_STATE_START_WIFI_CONFIRM_ANIMATION, UI_USER_STATE_START_WIFI_CONFIRM_COLOR);
        break;  
        case UI_USER_STATE_FACTORY_RESET:
            handleAnimation(UI_USER_STATE_FACTORY_RESET_ANIMATION, UI_USER_STATE_FACTORY_RESET_COLOR);
        break;
        case UI_USER_STATE_FACTORY_RESET_CONFIRM:
            handleAnimation(UI_USER_STATE_FACTORY_RESET_CONFIRM_ANIMATION, UI_USER_STATE_FACTORY_RESET_CONFIRM_COLOR);
        break;
        case UI_TEMP_STATE_ACTION:
            handleAnimation(ANIMATION_SOLID, BLIZZARD_WHITE);
        break;
        case UI_USER_STATE_CONTROL:
            handleAnimation(ANIMATION_SOLID, get_dmx_color(1));
        break;  
        case UI_SECRET_STATE_CLUE_1:
            handleAnimation(ANIMATION_S1, BLIZZARD_WHITE);
        break;
        case UI_SECRET_STATE_CLUE_2:
            handleAnimation(ANIMATION_S2, BLIZZARD_RED);
        break;
        case UI_SECRET_STATE_CLUE_3:
            handleAnimation(ANIMATION_S3, BLIZZARD_BLUE);
        break; 
        case UI_SECRET_STATE_RAINBOW:
            handleAnimation(ANIMATION_RAINBOW, DONT_CARE_COLOR);
        break;
        case UI_QC_STATE_CONNECTING:
            handleAnimation(UI_QC_STATE_CONNECTING_ANIMATION, DONT_CARE_COLOR);
        break;
        case UI_QC_STATE_CONNECTED:
            handleAnimation(UI_QC_STATE_CONNECTED_ANIMATION, DONT_CARE_COLOR);
        break;
        default:
            handleAnimation(ANIMATION_BREATH_SLOW, BLIZZARD_ERROR);
        break;
    }
}

color_t* get_current_color(void){
    return &Current_Color;
}

void change_indicator(uint8_t red, uint8_t green, uint8_t blue){
    Current_Color.red = red;
    Current_Color.green = green;
    Current_Color.blue = blue;

    set_all_leds(red, green, blue);
}

void change_indicator_bright(uint8_t red, uint8_t green, uint8_t blue, double brightness){
    Current_Color.red = red;
    Current_Color.green = green;
    Current_Color.blue = blue;

    set_all_leds_bright(red, green, blue, brightness);
}

void change_indicator_color(color_t color){
    memcpy(&Current_Color, &color, sizeof(color_t));

    set_all_leds_color(color);
}

void change_indicator_color_bright(color_t color, double brightness){
    memcpy(&Current_Color, &color, sizeof(color_t));

    set_all_leds_color_bright(color, brightness);
}

/*---------------------------- Handlers ----------------------------------*/

void handleAnimation(uint8_t animation, color_t color){
    static uint32_t tick = 0;
    static uint32_t tock = 0;
    static uint8_t prevAnimation = 0;

    if(animation != prevAnimation){
        prevAnimation = animation;
        tick = 0;
        tock = 0;
    } else {
        tock++;
    }

    switch(animation){
        case ANIMATION_BREATH_SLOW:
            if(!(tock % 13)){
                tickBreath(tick++, color);
            }
        break;
        case ANIMATION_BREATH:
            if(!(tock % 8)){
                tickBreath(tick++, color);
            }
        break;
        case ANIMATION_BREATH_FAST:
            if(!(tock % 3)){
                tickBreath(tick++, color);
            }
        break;
        case ANIMATION_BLINK_SLOW:
            if(!(tock % 1300)){
                tickFlash(tick++, color);
            }
        break;
        case ANIMATION_BLINK:
            if(!(tock % 800)){
                tickFlash(tick++, color);
            }
        break;
        case ANIMATION_BLINK_FAST:
            if(!(tock % 300)){
                tickFlash(tick++, color);
            }
        break;
        case ANIMATION_DIT_5:
            if(!(tock % 300)){
                tickDits(tick++, 5, color);
            }
        break;
        case ANIMATION_DIT_4:
            if(!(tock % 300)){
                tickDits(tick++, 4, color);
            }
        break;
        case ANIMATION_DIT_3:
            if(!(tock % 300)){
                tickDits(tick++, 3, color);
            }
        break;
        case ANIMATION_DIT_2:
            if(!(tock % 300)){
                tickDits(tick++, 2, color);
            }
        break;
        case ANIMATION_DIT_1:
            if(!(tock % 300)){
                tickDits(tick++, 1, color);
            }
        break;
        case ANIMATION_BATTERY_CRIT:
            if(!(tock % 50)){
                tickDits(tick++, 1, color);
            }
        break;
        case ANIMATION_S1:
            if(!(tock % CLUE_TICK)){
                tickMorseCode(CLUE_1, sizeof(CLUE_1), color);
            }
        break;
        case ANIMATION_S2:
            if(!(tock % CLUE_TICK)){
                tickMorseCode(CLUE_2, sizeof(CLUE_2), color);
            }
        break;  
        case ANIMATION_S3:
            if(!(tock % CLUE_TICK)){
                tickMorseCode(CLUE_3, sizeof(CLUE_3), color);
            }
        break; 
        case ANIMATION_RAINBOW:
            if(!(tock % 2)){
                tickRainbow(tick++);
            }   
        break;   
        case ANIMATION_QC_CONNECTING:
            if(!(tock % 200)){
                tickSwap(tick++, BLIZZARD_ORANGE, BLIZZARD_PURPLE);
            }
        break; 
        case ANIMATION_QC_CONNECTED:
            if(!(tock % 300)){
                tickSwap(tick++, BLIZZARD_BLUE, BLIZZARD_WHITE);
            }
        break; 
        default: //Solid
            change_indicator_color(color);
        break;
    }

}

void tickBreath(uint32_t tick, color_t color){
    tick = tick & 0xFF;
    double brightness = 0.0;

    brightness = sin((((double)tick)/128.0/2.) * PI);
    brightness = (brightness < 0) ? 0.01 : brightness + 0.01;

    set_all_leds_color_bright(color, brightness);
}

void tickBreathIndex(uint32_t tick, color_t color, uint8_t index){
    tick = (tick + index * 20) & 0xFF;
    double brightness = 0.0;

    brightness = sin((((double)tick)/128.0/2.) * PI);
    brightness = (brightness < 0) ? 0.01 : brightness + 0.01;

    set_led_color_bright(index, color, brightness);
}

void tickFlash(uint32_t tick, color_t color){
    tickSwap(tick, BLIZZARD_BLACK, color);
}

void tickSwap(uint32_t tick, color_t one, color_t two){
    change_indicator_color((tick % 2) ? one : two);
}

void tickDits(uint32_t tick, uint8_t level, color_t color){
    static uint8_t state = 0;
    static uint8_t count = 0;

    if(!count){
        count = level*2;
        state = 0;
    }

    if(state == 0){ //dits
        change_indicator_color((tick % 2) ? BLIZZARD_BLACK : color);
        count--;

        if(!count){
            state = 1;
            count = BATTERY_RESET_PERIOD;
        }
    } else { //break
        change_indicator_color(BLIZZARD_BLACK);
        count--;
    }
}

void tickMorseCode(char * code, uint16_t length, color_t color){
    static uint8_t count = 0;
    static uint32_t tick = 0;
    static bool space = true;
    static bool dah = false;

    if(count){
        if(--count == 0){ //between symbols
            change_indicator_color(BLIZZARD_BLACK);
        } else {
            change_indicator_color_bright(
                (space) ? BLIZZARD_BLACK : color,
                (dah) ? 0.1 : 1.
            );
        }
    } else { //get next char
        space = true; //assume a space char
        dah = false;
        switch(code[tick]){
            case '-':
                count = 4;
                space = false;
                dah = true;
            break;
            case '.':
                count = 2;
                space = false;
            break;
            case ' ':
                count = 3;
            break;
            case '?':
                count = 7;
            break;
            case '\0':
                count = 13;
            break;
            default:
                count = 0;
        }

        if(tick + 1 >= length){
            tick = 0;
        } else {
            tick++;
        }
    }
}

void tickRainbow(uint32_t tick){
    uint8_t tock = (uint8_t)(tick&0xFF);

    if(tock < 85){
        change_indicator_bright(tock*3, (0xFF-(tock*3)), 0, 0.69);
    } else if(tock < 170){
        tock-=85;
        change_indicator_bright((0xFF-(tock*3)), 0, tock*3, 0.69);
    } else {
        tock-=170;
        change_indicator_bright(0, tock*3, (0xFF-(tock*3)), 0.69);
    }

}

color_t get_dmx_color(uint16_t address){
    double dimmer = 0;
    volatile uint8_t* dmx = get_dmx();
    color_t color;

    if(address > MAX_DMX_SLOTS-4 || address == 0){
        ESP_LOGE(Tag, "Address out of bounds - get_dmx_color: %d", address);

        color.red = 0;
        color.green = 0;
        color.blue = 0;
    } else {
        dimmer = ((double) dmx[address])/255.;
        color.red = ((uint8_t)(((double)dmx[address+1]) * dimmer));
        color.green = ((uint8_t)(((double)dmx[address+2]) * dimmer));
        color.blue = ((uint8_t)(((double)dmx[address+3]) * dimmer));
    }

    return color;
} 

void danger_battery_animation(){
    uint8_t tick = 3;
    
    while(--tick){
        change_indicator_color(BLIZZARD_ULTRA_DIM_RED);
        vTaskDelay(100);
        change_indicator_color(BLIZZARD_BLACK);
        vTaskDelay(100);
    }
}

void sleep_animation(){
    uint8_t tock = 202-15;
    uint8_t tick = 30;
    color_t color;
    uint8_t r,g,b;

    while(tick++ < 102){
        if(tick <= 55){
            r = tick;
            g = (tick <= 46) ? tick : 46;
            b = 55 - tick;
        } else {
            r = 55;
            g = (102 - tick);
            b = 0;
        }

        color.red = r;
        color.green = g;
        color.blue = b;
        tickBreath(tock--, color);
        vTaskDelay(10);
        tickBreath(tock--, color);
        vTaskDelay(10);
    }

    power_indicator(false);
}

void morgenstimmung_animation(){
    uint8_t tick = 0;
    color_t color;
    uint8_t r,g,b;


    while(tick++ < 48){
        if(tick < 8){
            r = 255-tick*5;
            g = 255-tick*5;
            b = 255-tick*2;
        } else {
            r = 255-tick*5;
            g = 255-tick*5;
            b = 255-tick-(8);
        }

        color.red = r;
        color.green = g;
        color.blue = b;
        change_indicator_color(color);
        vTaskDelay(20);
    }

    tick = 4;

    while(--tick){
        change_indicator_color(BLIZZARD_BLACK);
        vTaskDelay(100);
        change_indicator_color(color);
        vTaskDelay(100);
    }

}

uint8_t power_indicator(bool on){
    uint8_t retVal;

    if(!on){
        change_indicator(0,0,0);
    }

    retVal = gpio_set_level(LED_ENABLE_PIN, (on) ? HIGH : LOW);
	if(retVal != ESP_OK){
		ESP_LOGE(Tag, "Error setting GPIO level - init indication manager: %d", retVal);
		return DMX_SET_PIN_ERROR;
	}

    return SUCCESS;
}

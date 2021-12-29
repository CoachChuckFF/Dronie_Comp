/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * UI Manager -> blizzard_button_manager.c
 * 
 */

#include "blizzard_simple_button_manager.h"
#include "blizzard_global_defines.h"
#include <driver/adc.h>

static const char *Tag = "BUTTON";


/*---------------------------- Initializers ----------------------------------*/
/*
 * Function: init_button_manager
 * ----------------------------
 *  
 *  Inits the button manager by configuring the button gpio
 * 
 *  returns: BUTTON_CONFIG_ERROR, SUCCESS
 */ 
uint8_t init_button_manager() {
    esp_err_t err;
    gpio_config_t io_conf;

    ESP_LOGI(Tag, "Button Manager is. like. totally getting ready...");

    //turn gpio36 dac (button level)
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(BUTTON_LEVEL_GPIO, BUTTON_LEVEL_ATTEN);

    //sets pgood and charge pins
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.pin_bit_mask = (BUTTON_PIN_BIT); 
    io_conf.mode = GPIO_MODE_INPUT;
    //they have external pullups
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 0;

    err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(Tag, "Init button failed: %s", esp_err_to_name(err));
        return BUTTON_CONFIG_ERROR;
    }

    ESP_LOGI(Tag, "Button Manager is going to lit city!");

    return SUCCESS;
}

/*---------------------------- Controllers ----------------------------------*/

uint8_t getPressed(void){
    uint16_t buttonLevel = adc1_get_raw(BUTTON_LEVEL_GPIO);

    return (buttonLevel >= BUTTON_PRESSED_HIGH_LEVEL || buttonLevel <= BUTTON_PRESSED_LOW_LEVEL);
}

uint8_t getStablePressed(){
    static bool wasLastPressed = false;
    static uint8_t stableCount = 0;
    static uint8_t pressedState = BUTTON_UNSTABLE;

    bool currentlyPressed = getPressed();


    //Stabalize Button
    if(wasLastPressed == currentlyPressed){
        if(pressedState == BUTTON_UNSTABLE){
            if(stableCount++ > BUTTON_STABLE_COUNT){
                pressedState = (currentlyPressed) ? BUTTON_PRESSED : BUTTON_RELEASED;
            }
        }
    } else {
        pressedState = BUTTON_UNSTABLE;
        stableCount = 0;
    }

    wasLastPressed = currentlyPressed;

    return pressedState;
}

uint8_t tick_button(){
    static uint8_t state = BUTTON_STATE_IDLE;
    static uint32_t timeout = 0;
    uint8_t pressed = getStablePressed();

    uint8_t buttonAction = BUTTON_ACTION_NOTHING;

    if(pressed != BUTTON_UNSTABLE){
        
        switch(state){
            case BUTTON_STATE_IDLE:
                if(pressed == BUTTON_PRESSED){
                    timeout = BUTTON_HOLD_TIMEOUT;
                    state = BUTTON_STATE_START_TAP;
                }
            break;
            case BUTTON_STATE_START_TAP:
                if(timeout == 0){
                    buttonAction = BUTTON_ACTION_HOLD;
                    state = BUTTON_STATE_HOLD;
                } else if(pressed == BUTTON_RELEASED){
                    timeout = BUTTON_TAP_TIMEOUT;
                    state = BUTTON_STATE_END_TAP;
                }     
            break;
            case BUTTON_STATE_HOLD:
                if(pressed == BUTTON_RELEASED){
                    buttonAction = BUTTON_ACTION_HELD;
                    timeout = BUTTON_COOLDOWN;
                    state = BUTTON_STATE_DONE;
                }
            break;
            case BUTTON_STATE_END_TAP:
                if(timeout == 0){
                    buttonAction = BUTTON_ACTION_TAP;
                    timeout = BUTTON_COOLDOWN;
                    state = BUTTON_STATE_DONE;
                } else if(pressed == BUTTON_PRESSED){
                    timeout = BUTTON_DOUBLE_TAP_TIMEOUT;
                    state = BUTTON_STATE_START_TAP_2;
                }
            break;
            case BUTTON_STATE_START_TAP_2:
                if(timeout == 0){
                    state = BUTTON_STATE_DOUBLE_TAP;
                } else if(pressed == BUTTON_RELEASED){
                    timeout = BUTTON_DOUBLE_TAP_TIMEOUT;
                    state = BUTTON_STATE_END_TAP_2;
                }
            break;
            case BUTTON_STATE_END_TAP_2:
                if(timeout == 0){
                    state = BUTTON_STATE_DOUBLE_TAP;
                } else if(pressed == BUTTON_PRESSED){
                    timeout = BUTTON_DOUBLE_TAP_TIMEOUT;
                    state = BUTTON_STATE_START_TAP_3;
                }
            break;
            case BUTTON_STATE_DOUBLE_TAP:
                if(pressed == BUTTON_RELEASED){
                    buttonAction = BUTTON_ACTION_DOUBLE_TAP;
                    timeout = BUTTON_COOLDOWN;
                    state = BUTTON_STATE_DONE;
                }
            break;
            case BUTTON_STATE_NOTHING:
                if(pressed == BUTTON_RELEASED){
                    timeout = BUTTON_COOLDOWN;
                    state = BUTTON_STATE_DONE;
                }
            break;
            case BUTTON_STATE_START_TAP_3:
                if(timeout == 0){
                    state = BUTTON_STATE_NOTHING;
                } else if(pressed == BUTTON_RELEASED){
                    timeout = BUTTON_DOUBLE_TAP_TIMEOUT;
                    state = BUTTON_STATE_END_TAP_3;
                }
            break;
            case BUTTON_STATE_END_TAP_3:
                if(timeout == 0){
                    timeout = BUTTON_COOLDOWN;
                    state = BUTTON_STATE_DONE;
                    buttonAction = BUTTON_ACTION_TOGGLE_SHOW;
                } else if(pressed == BUTTON_PRESSED){
                    timeout = BUTTON_DOUBLE_TAP_TIMEOUT;
                    state = BUTTON_STATE_START_TAP_4;
                }
            break;
            case BUTTON_STATE_START_TAP_4:
                if(timeout == 0){
                    state = BUTTON_STATE_NOTHING;
                } else if(pressed == BUTTON_RELEASED){
                    timeout = BUTTON_DOUBLE_TAP_TIMEOUT;
                    state = BUTTON_STATE_END_TAP_4;
                }
            break;
            case BUTTON_STATE_END_TAP_4:
                if(timeout == 0){
                    state = BUTTON_STATE_NOTHING;
                } else if(pressed == BUTTON_PRESSED){
                    timeout = BUTTON_DOUBLE_TAP_TIMEOUT;
                    state = BUTTON_STATE_START_TAP_5;
                }
            break;
            case BUTTON_STATE_START_TAP_5:
                if(timeout == 0){
                    state = BUTTON_STATE_NOTHING;
                } else if(pressed == BUTTON_RELEASED){
                    buttonAction = BUTTON_ACTION_SLEEP;
                    timeout = BUTTON_COOLDOWN;
                    state = BUTTON_STATE_DONE;
                }
            break;
            default:
            break;
        }
    }

    if(timeout != 0){
        timeout--;
    } else if(state == BUTTON_STATE_DONE){
        state = BUTTON_STATE_IDLE;
    }

    return buttonAction;

}

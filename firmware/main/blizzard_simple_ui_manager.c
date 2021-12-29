/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * UI Manager -> blizzard_ui_manager.c
 * 
 */

#include "blizzard_simple_ui_manager.h"
#include "blizzard_simple_button_manager.h"
#include "blizzard_simple_indication_manager.h"
#include "blizzard_recorder_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "blizzard_simple_button_manager.h"
#include "blizzard_simple_indication_manager.h"
#include "blizzard_ws2812_manager.h"

#include "blizzard_global_defines.h"
#include "blizzard_network_manager.h"
#include "blizzard_battery_manager.h"
#include "blizzard_protocol_manager.h"
#include "blizzard_dmx_manager.h"
#include "blizzard_nvs_manager.h"
#include "blizzard_show_manager.h"

static const char *Tag = "UI";

UIState State = UI_USER_STATE_IDLE;
UIState Temp_State = UI_USER_STATE_IDLE;
UIState Idle_State = UI_USER_STATE_IDLE;
uint32_t User_TO = 0;
uint8_t Alt_View_Count = 0;
bool Hold = false;

//Flags
bool Crit = false;
bool Locate = false;
bool Mute = false;

/*---------------------------- Initializers ----------------------------------*/
/*
 * Function: init_ui_manager
 * ----------------------------
 *  
 *  Inits the indication manager by configuring the led gpios
 * 
 *  returns: INDICATION_CONFIG_ERROR, SUCCESS
 */ 
uint8_t init_ui_manager() {
    esp_err_t err;

    ESP_LOGI(Tag, "UI is thinks about life...");

    err = init_button_manager();
    if(err != SUCCESS){
        ESP_LOGE(Tag, "Error starting button manager: %d", err);
        return err;
    }

    err = init_indication_manager();
    if(err != SUCCESS){
        ESP_LOGE(Tag, "Error starting indication manager: %d", err);
        return err;
    }

    State = UI_USER_STATE_IDLE;

    ESP_LOGI(Tag, "UI continues to think about life");

    return SUCCESS;
}    


/*---------------------------- Tick Tock ----------------------------------*/
/*
 * Function: tick_data
 * ----------------------------
 *  
 *  flashes data indicator when data is recieved - only when device is in idle
 * 
 *  returns: void
 */ 
void tick_data(){
    if(State == UI_USER_STATE_IDLE &&
    (Idle_State == UI_IDLE_STATE_AP_DEVICE_CONNECTED ||
    Idle_State == UI_IDLE_STATE_WIFI_CONNECTED)){
        change_indicator_color((get_show_state() == SHOW_STATE_STOP) ?
            BLIZZARD_DATA_COLOR : BLIZZARD_RED
        );
    }
}

/*
 * Function: tick_ui
 * ----------------------------
 *  
 *  read button action and instructs indicator
 * 
 *  returns: void
 */ 
void tick_ui(){
    uint8_t action = tick_button();


    // if(action != BUTTON_ACTION_NOTHING){
    //     ESP_LOGI(Tag, "Button Action: %02X\n", action);
    // }
    //if timer still running, handle action
    //else show idle state
    if(User_TO || action != BUTTON_ACTION_NOTHING){
        if(User_TO != 0){
            User_TO--;
        }

        if(action != BUTTON_ACTION_NOTHING){
            handleAction(action); //handles button action
        } else if(State == UI_USER_STATE_RECORDING){
            User_TO = UI_USER_TICK_TIMEOUT;
        } else if(State == UI_USER_STATE_PRE_RECORDING){
            User_TO = UI_USER_TICK_TIMEOUT;
        } else if(State == UI_USER_STATE_LISTENING){
            User_TO = UI_USER_TICK_TIMEOUT;
        }

        if(State >= UI_USER_STATE_BATTERY && State < UI_USER_STATE_CONTROL){
            handleBatteryLogic(); //if battery view, show battery ticks - has a timeout
        } else if(State >= UI_USER_STATE_CONTROL){
            handleViewLogic(); //keeps TO constant for static views
        }
    } else {
        handleIdleLogic(); //show idle animations
    }

    tick_indicator((User_TO) ? ((Hold) ? Temp_State : State) : Idle_State);

}

/*---------------------------- Flags ----------------------------------*/

uint8_t get_current_ui_state(){
    return State;
}

void set_listening_ui_state(){
    State = UI_USER_STATE_LISTENING;
    User_TO = UI_USER_TICK_TIMEOUT;

}

void set_idle_ui_state(){
    State = UI_USER_STATE_IDLE;
    User_TO = 0;
}

void set_prerecording_ui_state(){
    State = UI_USER_STATE_PRE_RECORDING;
    User_TO = UI_USER_TICK_TIMEOUT;
}

void set_recording_ui_state(){
    if(State == UI_USER_STATE_PRE_RECORDING){
        State = UI_USER_STATE_RECORDING;
    }
}

void set_ui_crit(bool crit){
    Crit = crit;
}

bool get_ui_crit(void){
    return Crit;
}

void set_ui_normal(){
    Locate = false;
    Mute = false;
}

void set_ui_locate(bool locate){
    Locate = locate;
    Mute = false;
}

bool get_ui_locate(void){
    return Locate;
}

void set_ui_mute(bool mute){
    Mute = mute;
    Locate = false;
}

bool get_ui_mute(){
    return Mute;
}

/*---------------------------- Handlers ----------------------------------*/

void handleInputFeedback(color_t color, uint16_t spin){
    color_t* oldColor = get_current_color();
    uint8_t olRed, olGreen, olBlue;

    olRed = oldColor->red;
    olGreen = oldColor->green;
    olBlue = oldColor->blue;

    change_indicator_color(color);

    while (spin--){
        vTaskDelay(1);
    }

    change_indicator(olRed, olGreen, olBlue);
}

void handleAction(uint8_t action){


    if(action == BUTTON_ACTION_SLEEP){
        go_the_fuck_to_sleep();
        return;
    }

    if(action == BUTTON_ACTION_TOGGLE_SHOW){
        if(get_show_ok()){
            toggle_show();
        }

        handleInputFeedback(
            (get_show_state() == SHOW_STATE_PLAY) ? 
            UI_PLAY_COLOR : UI_STOP_COLOR,
            UI_USER_PLAY_STOP_FEEDBACK_TIMEOUT
        );

        return;
    }

    Locate = false; //stops locating device on user action
    Hold = false; //Stops Temp State
    User_TO = UI_USER_TICK_TIMEOUT; //Resets the Timeout

    // if(action == BUTTON_ACTION_TAP){
    //     handleInputFeedback(UI_FORWARD_COLOR, UI_USER_INPUT_FEEDBACK_TIMEOUT);
    // } else if(action == BUTTON_ACTION_DOUBLE_TAP){
    //     handleInputFeedback(UI_REVERSE_COLOR, UI_USER_INPUT_FEEDBACK_TIMEOUT);
    // }

    switch(State){
        case UI_USER_STATE_IDLE:
            if(action == BUTTON_ACTION_DOUBLE_TAP){
                //printf("To Battery\n");
                User_TO = UI_BATTERY_TIMEOUT;
                State = UI_USER_STATE_BATTERY;
                return;
            }else if(action == BUTTON_ACTION_TAP){
                State = UI_USER_STATE_RECORD;
            }
        break;
        case UI_USER_STATE_RECORD:
            if(action == BUTTON_ACTION_TAP){
                State = UI_USER_STATE_LISTEN;
            } else if(action == BUTTON_ACTION_HELD){ 
                State = UI_USER_STATE_RECORD_CONFIRM;
                return;
            } else if(action == BUTTON_ACTION_DOUBLE_TAP){
                State = UI_USER_STATE_IDLE;
                User_TO = 0;
                return;
            } else if (action == BUTTON_ACTION_HOLD){
                Hold = true;
                Temp_State = UI_USER_STATE_RECORD_CONFIRM;
            }
        break;
        case UI_USER_STATE_RECORD_CONFIRM:
            if(action == BUTTON_ACTION_HELD){ 
                start_recording();
                State = UI_USER_STATE_PRE_RECORDING;
                return;
            } else if(action == BUTTON_ACTION_DOUBLE_TAP){
                State = UI_USER_STATE_RECORD;
            } else if(action == BUTTON_ACTION_HOLD){
                Hold = true;
                Temp_State = UI_TEMP_STATE_ACTION;
            }
        break;
        case UI_USER_STATE_PRE_RECORDING:
            ;;//cannot be stopped
        break;
        case UI_USER_STATE_RECORDING:
            if(action == BUTTON_ACTION_DOUBLE_TAP){ 
                stop_recording();
                State = UI_USER_STATE_IDLE;
                User_TO = 0;
            }
        break;
        case UI_USER_STATE_LISTEN:
            if(action == BUTTON_ACTION_TAP){
                if(get_connection() == CONNECTION_INTERNAL){
                    State = UI_USER_STATE_START_WIFI;
                } else {
                    State = UI_USER_STATE_START_AP;
                }
            } else if(action == BUTTON_ACTION_HELD){ 
                State = UI_USER_STATE_LISTEN_CONFIRM;
                return;
            } else if(action == BUTTON_ACTION_DOUBLE_TAP){
                State = UI_USER_STATE_IDLE;
                User_TO = 0;
                return;
            } else if (action == BUTTON_ACTION_HOLD){
                Hold = true;
                Temp_State = UI_USER_STATE_LISTEN_CONFIRM;
            }
        break;
        case UI_USER_STATE_LISTEN_CONFIRM:
            if(action == BUTTON_ACTION_HELD){ 
                start_listening();
                State = UI_USER_STATE_LISTENING;
                return;
            } else if(action == BUTTON_ACTION_DOUBLE_TAP){
                State = UI_USER_STATE_LISTEN;
            } else if(action == BUTTON_ACTION_HOLD){
                Hold = true;
                Temp_State = UI_TEMP_STATE_ACTION;
            }
        break;
        case UI_USER_STATE_LISTENING:
            if(action == BUTTON_ACTION_DOUBLE_TAP){ 
                stop_listening();
                State = UI_USER_STATE_IDLE;
                User_TO = 0;
            }
        break;
        case UI_USER_STATE_REBOOT:
            if(action == BUTTON_ACTION_TAP){ 
                //printf("To Factory Reset\n");
                State = UI_USER_STATE_FACTORY_RESET;
            } else if(action == BUTTON_ACTION_HELD){ 
                power_indicator(false);
                esp_restart();
            } else if(action == BUTTON_ACTION_DOUBLE_TAP){
                //printf("Normal Operation\n");
                User_TO = 0; //Idle state
                State = UI_USER_STATE_IDLE;
                return;
            } else if(action == BUTTON_ACTION_HOLD){
                Hold = true;
                Temp_State = UI_TEMP_STATE_ACTION;
            }
        break;
        case UI_USER_STATE_START_WIFI:
            if(action == BUTTON_ACTION_TAP){
                //printf("To Reboot\n");
                State = UI_USER_STATE_REBOOT;
            } else if(action == BUTTON_ACTION_HELD){ 
                //printf("To Start Wifi Confirm\n");
                State = UI_USER_STATE_START_WIFI_CONFIRM;
                return;
            } else if(action == BUTTON_ACTION_DOUBLE_TAP){
                //printf("Normal Operation\n");
                State = UI_USER_STATE_IDLE;
                User_TO = 0;
                return;
            } else if (action == BUTTON_ACTION_HOLD){
                Hold = true;
                Temp_State = UI_USER_STATE_START_WIFI_CONFIRM;
            }
        break;
        case UI_USER_STATE_START_WIFI_CONFIRM:
            if(action == BUTTON_ACTION_HELD){ 
                //printf("Starting WiFi...\n");
                change_connection(CONNECTION_EXTERNAL);
                State = UI_USER_STATE_IDLE;
                User_TO = 0;
                return;
            } else if(action == BUTTON_ACTION_DOUBLE_TAP){
                //printf("To Start Wifi\n");
                State = UI_USER_STATE_START_WIFI;
            } else if(action == BUTTON_ACTION_HOLD){
                Hold = true;
                Temp_State = UI_TEMP_STATE_ACTION;
            }
        break;
        case UI_USER_STATE_START_AP:
            if(action == BUTTON_ACTION_TAP){
                //printf("To Reboot\n");
                State = UI_USER_STATE_REBOOT;
            } else if(action == BUTTON_ACTION_HELD){ 
                //printf("To Start AP Confirm\n");
                State = UI_USER_STATE_START_AP_CONFIRM;
                return;
            } else if(action == BUTTON_ACTION_DOUBLE_TAP){
                //printf("Normal Operation\n");
                State = UI_USER_STATE_IDLE;
                User_TO = 0;
                return;
            } else if (action == BUTTON_ACTION_HOLD){
                Hold = true;
                Temp_State = UI_USER_STATE_START_AP_CONFIRM;
            }
        break;
        case UI_USER_STATE_START_AP_CONFIRM:
            if(action == BUTTON_ACTION_HELD){ 
                change_connection(CONNECTION_INTERNAL);
                State = UI_USER_STATE_IDLE;
                User_TO = 0;
                return;
            } else if(action == BUTTON_ACTION_DOUBLE_TAP){
                State = UI_USER_STATE_START_AP;
            } else if(action == BUTTON_ACTION_HOLD){
                Hold = true;
                Temp_State = UI_TEMP_STATE_ACTION;
            }
        break;
        case UI_USER_STATE_FACTORY_RESET:
            if(action == BUTTON_ACTION_TAP){ 
                State = UI_USER_STATE_RECORD;
            } else if(action == BUTTON_ACTION_HELD){
                //printf("To Factory Reset Confirm\n");
                State = UI_USER_STATE_FACTORY_RESET_CONFIRM;
                return;
            } else if(action == BUTTON_ACTION_DOUBLE_TAP){
                //printf("Normal Operation\n");
                State = UI_USER_STATE_IDLE;
                User_TO = 0;
                return;
            } else if (action == BUTTON_ACTION_HOLD){
                Hold = true;
                Temp_State = UI_USER_STATE_FACTORY_RESET_CONFIRM;
            }
        break;
        case UI_USER_STATE_FACTORY_RESET_CONFIRM:
            if(action == BUTTON_ACTION_HELD){ 
                //printf("Factory Resetting...\n");
                power_indicator(false);
                reset_esp32();
            } else if(action == BUTTON_ACTION_DOUBLE_TAP){
                //printf("To Factory Reset\n");
                State = UI_USER_STATE_FACTORY_RESET;
            } else if(action == BUTTON_ACTION_HOLD){
                Hold = true;
                Temp_State = UI_TEMP_STATE_ACTION;
            }
        break;
        default: //Battery State
            if(action == BUTTON_ACTION_TAP){
                //printf("To Idle\n");
                State = UI_USER_STATE_IDLE;
                User_TO = 0;
                Alt_View_Count = 0;
            } else if(action == BUTTON_ACTION_DOUBLE_TAP){
                Alt_View_Count++;

                if(Alt_View_Count == CONTROL_DT_COUNT){
                    State = UI_USER_STATE_CONTROL;
                } else if(Alt_View_Count == CLUE_1_DT_COUNT){
                    State = UI_SECRET_STATE_CLUE_1;
                } else if(Alt_View_Count == CLUE_2_DT_COUNT){
                    State = UI_SECRET_STATE_CLUE_2;
                } else if(Alt_View_Count > CLUE_2_DT_COUNT){
                    State = UI_USER_STATE_BATTERY;
                    Alt_View_Count = 0;
                }
            }
        break;
    }
}

void handleBatteryLogic(){
    uint8_t batteryLevel = 0xFF;


    if(State == UI_USER_STATE_BATTERY){
        if(battery_has_valid_input()){
            if(battery_is_charging()){
                State = UI_BATTERY_STATE_CHARGING;
            } else {
                State = UI_BATTERY_STATE_DONE_CHARGING;
            }
        } else {
            while(batteryLevel == 0xFF){
                batteryLevel = get_battery_level();
            }//TODO Check

            
            if(batteryLevel > BATTERY_LEVEL_5){
                State = UI_BATTERY_STATE_LEVEL_5;
            } else if(batteryLevel > BATTERY_LEVEL_4){
                State = UI_BATTERY_STATE_LEVEL_4;
            }else if(batteryLevel > BATTERY_LEVEL_3){
                State = UI_BATTERY_STATE_LEVEL_3;
            }else if(batteryLevel > BATTERY_LEVEL_2){
                State = UI_BATTERY_STATE_LEVEL_2;
            }else {
                State = UI_BATTERY_STATE_LEVEL_1;
            }
        }
    }
}

void handleViewLogic(){
    uint16_t i, fib = 1, oldFib = 1;
    User_TO = UI_USER_TICK_TIMEOUT;

    if(State == UI_SECRET_STATE_CLUE_2){
        for(i = 1; i <= 512; i++){
            if(i == fib){
                if(get_dmx_value(i) == 0xFF){
                    fib += oldFib;
                    oldFib = i;
                } else {
                    return;
                }
            } else if(get_dmx_value(i) == 0x00){
                continue;
            } else {
                return;
            }
        }
        State = UI_SECRET_STATE_CLUE_3;
    } else if(UI_SECRET_STATE_CLUE_3){
        for(i = 1; i <= 512; i++){
            if(get_dmx_value(i) != 69){
                return;
            }
            State = UI_SECRET_STATE_RAINBOW;
        }
    }
}

void handleIdleLogic(){
    State = UI_USER_STATE_IDLE;
    Hold = false;

    if(Locate){
        Idle_State = UI_IDLE_STATE_LOCATE;
    } else if(Crit && !battery_has_valid_input()){
        Idle_State = UI_BATTERY_STATE_LEVEL_CRIT;
    } else if (Mute){
        Idle_State = UI_IDLE_STATE_MUTE;
    } else {

//         if(get_dmx_value(512) == 69){
//             for(i = 1; i < 512; i++){
//                 if(get_dmx_value(i) != 69){
//                     goto SKIP_RAINBOW_CHECK;
//                 }
//             }
//             Idle_State = UI_SECRET_STATE_RAINBOW;
//             return;
//         }

// SKIP_RAINBOW_CHECK:

        switch(get_connection()){
            case CONNECTION_INTERNAL:
                if(get_device_connected()){
                    Idle_State = UI_IDLE_STATE_AP_DEVICE_CONNECTED;
                } else {
                    Idle_State = UI_IDLE_STATE_AP_IDLE;
                }
            break;
            case CONNECTION_EXTERNAL:
                if(check_network_connection(CONNECTION_WIFI)){
                    Idle_State = UI_IDLE_STATE_WIFI_CONNECTED;
                } else {
                    switch(get_wifi_error()){
                        case BAD_WIFI_ERROR:
                            Idle_State = UI_IDLE_STATE_WIFI_ERROR_BAD_WIFI; 
                        break;
                        case BAD_SSID_ERROR:
                            Idle_State = UI_IDLE_STATE_WIFI_ERROR_BAD_SSID; 
                        break;
                        case BAD_PASS_ERROR:
                            Idle_State = UI_IDLE_STATE_WIFI_ERROR_BAD_PASS; 
                        break;
                        default: //Wifi ok
                            if(get_ez_connect_running()){
                                Idle_State = UI_IDLE_STATE_WIFI_EZ_CONNECTING;
                            } else {
                                Idle_State = UI_IDLE_STATE_WIFI_CONNECTING;    
                            }
                        break;
                    } 
                }
            break;
        }
    }
}



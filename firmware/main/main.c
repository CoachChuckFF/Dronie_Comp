/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, February 2018
 *
 * main.c
 *
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "blizzard_ws2812_manager.h"
#include "blizzard_global_defines.h"
#include "blizzard_helpers.h"
#include "blizzard_colors.h"
#include "blizzard_recorder_manager.h"
#include "blizzard_simple_ui_manager.h"
#include "blizzard_timer_manager.h"
#include "blizzard_dronie_manager.h"
#include "blizzard_dmx_manager.h"
#include "blizzard_nvs_manager.h"
#include "blizzard_sound_manager.h"
#include "blizzard_battery_manager.h"
#include "blizzard_network_manager.h"
#include "blizzard_protocol_manager.h"
#include "blizzard_ota_manager.h"
#include "blizzard_show_manager.h"
#include "blizzard_artnet_manager.h"
#include "blizzard_ws2812_manager.h"
#include "blizzard_rdm_manager.h"
#include "blizzard_sacn_manager.h"
#include "blizzard_simple_indication_manager.h"

static const char *Tag = "Main";

uint32_t Tock = 1;

uint32_t MS_Tick = 0;
uint32_t Slow_Tick = 0;

uint32_t Incognito_Timer = 3 * 1000;
uint32_t Debug_Timer = 0;
uint32_t Spoon_Timer = 0;

color_t Dronie_LED_Buffer[DRONIE_LED_COUNT];

const char Morse_Code[] = "- ---/- .... ./-- --- --- -.";
uint8_t Morse_Code_Index = 0xFF;
uint16_t Morse_Code_Time_Unit = 200;


void startYourEngines(){
  //Core
  assert(init_nvs_manager() == SUCCESS);
  assert(init_ws2812_manager(5) == SUCCESS);
  assert(init_dronie_manager() == SUCCESS);
  assert(init_sound_manager() == SUCCESS);

  //Structure
  assert(init_timer_manager() == SUCCESS);
  assert(init_network_manager() == SUCCESS); //Network can be pokey

  assert(init_artnet_manager() == SUCCESS);
  assert(start_artnet() == SUCCESS);

  ESP_LOGE(Tag, "Start your engines for another");
  ESP_LOGE(Tag, "Christian Krueger Health Production!");
}

uint8_t TEST_SOUND = 0;
void incognitoLoop(){
  set_all_leds(0, 0, 0);

  if(!Incognito_Timer){
    if(!get_playing()){

      switch(MS_Tick % 5){
        case 0: Incognito_Timer = 45 * 1000; break;
        case 1: Incognito_Timer = 1 * 60 * 1000; break;
        case 2: Incognito_Timer = 2 * 60 * 1000; break;
        case 3: Incognito_Timer = 5 * 60 * 1000; break;
        case 4: Incognito_Timer = 8 * 60 * 1000; break;
      }
      play_sound(CLIP_CHIRP);
      set_dronie_motor_state(HIGH);
    } 
  } else {
    if(!get_playing()){
      set_dronie_motor_state(LOW);
      Incognito_Timer--;  
    }
  }
}

void connectingLoop(){
  tickBreath(Slow_Tick, BLIZZARD_BRIGHT_GREEN);
}


void debugLoop(){

  //Sets LED to current Color
  for(int i = 0; i < DRONIE_LED_COUNT; i++){
    get_dronie_led(i, Dronie_LED_Buffer + i);
    tickBreathIndex(Slow_Tick, Dronie_LED_Buffer[i], DRONIE_LED_COUNT - 1- i);
  }

  if(!Debug_Timer){
    if(!get_playing()){
      Debug_Timer = 13000;
      play_sound(CLIP_DOT);
    } 
  } else {
    if(!get_playing()){
      Debug_Timer--;  
    }
  }

}

void editLoop(){
  //Sets LED to current Color
  for(uint8_t i = 0; i < DRONIE_LED_COUNT; i++){
    get_dronie_led(i, Dronie_LED_Buffer + i);
    set_led_color(i, Dronie_LED_Buffer[i]);
  }

}

void spoonLoop(){
  tickRainbow(Slow_Tick);

  
  if(!Spoon_Timer){
    if(!get_playing()){
      set_dronie_motor_state(HIGH);
      Spoon_Timer = 30*1000*60;
      play_sound(CLIP_OHHI);
    } 
  } else {
    if(!get_playing()){
      set_dronie_motor_state(LOW);
      Spoon_Timer--;  
    }
  }
}

void app_main(void){

  startYourEngines();


  while(1){
    if(check_tick()){
      MS_Tick++;
      if(MS_Tick % 5 == 0) Slow_Tick++;

      tick_network();
      tick_artnet();
      tick_dronie_motor();

      switch (get_dronie_mode())
      {
        case DRONIE_MODE_DEBUG: 
          if(get_dronie_timeout()){
            editLoop();
          } else {
            debugLoop();
          }
          break;
        case DRONIE_MODE_SPOON: spoonLoop(); break;
        default: 
          if(get_ez_found_channel()){
            connectingLoop();
          } else {
            incognitoLoop();
          }
         break;
      }
      
      if(!(++Tock % (BATTERY_TICK_DELAY))){
        tick_dronie();
      }

      clear_tick();
    } else {
      vTaskDelay(1);
    }
  }
}

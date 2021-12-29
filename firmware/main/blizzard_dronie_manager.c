/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * UI Manager -> blizzard_button_manager.c
 * 
 */

#include "blizzard_dronie_manager.h"
#include "blizzard_colors.h"
#include "blizzard_ws2812_manager.h"
#include "blizzard_global_defines.h"
#include "blizzard_sound_manager.h"
#include "blizzard_artnet_manager.h"
#include "blizzard_nvs_manager.h"
#include "driver/gpio.h"


static const char *Tag = "DRONIE";

uint8_t Dronie_Mode;
uint8_t Dronie_Motor_State;
uint8_t Edit_Timeout = 0;
uint16_t Motor_Timeout = 0;

color32_t Dronie_LEDs[5];

uint8_t init_dronie_manager() {
    gpio_config_t ioConfig = {
		.intr_type = GPIO_PIN_INTR_DISABLE,
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask = DRONIE_MOVE_PIN_BIT,
		.pull_down_en = DISABLE,
		.pull_up_en = DISABLE
	};

    ESP_LOGI(Tag, "Robo Bird");

	Dronie_Mode = DRONIE_MODE_INCOGNITO;

	get_nvs_config(LED0_KEY, DATA_U32, ((uint8_t*) Dronie_LEDs) + (sizeof(color32_t) * 0));
	get_nvs_config(LED1_KEY, DATA_U32, ((uint8_t*) Dronie_LEDs) + (sizeof(color32_t) * 1));
	get_nvs_config(LED2_KEY, DATA_U32, ((uint8_t*) Dronie_LEDs) + (sizeof(color32_t) * 2));
	get_nvs_config(LED3_KEY, DATA_U32, ((uint8_t*) Dronie_LEDs) + (sizeof(color32_t) * 3));
	get_nvs_config(LED4_KEY, DATA_U32, ((uint8_t*) Dronie_LEDs) + (sizeof(color32_t) * 4));

	Dronie_Motor_State = LOW;
    gpio_set_level(DRONIE_MOVE_PIN, LOW);

    assert(gpio_config(&ioConfig) == ESP_OK);

    return SUCCESS;
}

void touch_dronie_motor(){
	if(Dronie_Mode == DRONIE_MODE_DEBUG){
		Motor_Timeout = 800;
		set_dronie_motor_state(HIGH);
    	ESP_LOGI(Tag, "[Dronie] Touch Motor");
	}
}

void tick_dronie_motor(){
	if(Motor_Timeout != 0){
		Motor_Timeout--;
		if(Motor_Timeout == 0){
			set_dronie_motor_state(LOW);
    		ESP_LOGI(Tag, "[Dronie] Touch Motor Timeout");

		}
	}
}

void tick_dronie(){
	if(Edit_Timeout != 0){
		Edit_Timeout--;
	}
}

uint8_t get_dronie_timeout(){
	return Edit_Timeout;
}

uint8_t get_dronie_mode(){
	return Dronie_Mode;
}

void set_dronie_mode(uint8_t mode){
	Edit_Timeout = 0;

	if(mode != Dronie_Mode){
		set_dronie_motor_state(LOW);
		stop_sound();
		set_all_leds(0x00, 0x00, 0x00);
		

		switch(mode){
			case DRONIE_MODE_DEBUG: Dronie_Mode = mode; ESP_LOGE(Tag, "Set to Debug"); break;
			case DRONIE_MODE_SPOON: Dronie_Mode = mode; ESP_LOGE(Tag, "Set to Spoon"); break;
			default: Dronie_Mode = DRONIE_MODE_INCOGNITO; ESP_LOGE(Tag, "Set to Incognito"); break;
		}
	}
}

void get_dronie_led(uint8_t index, color_t *ret){
	if(index >= DRONIE_LED_COUNT) index = DRONIE_LED_COUNT - 1;

	ret->red = Dronie_LEDs[index]._color.red;
	ret->green = Dronie_LEDs[index]._color.green;
	ret->blue = Dronie_LEDs[index]._color.blue;
}

void set_dronie_led(uint8_t index, color_t *set){
	if(index >= DRONIE_LED_COUNT) index = DRONIE_LED_COUNT - 1;

	if(Dronie_Mode == DRONIE_MODE_DEBUG){
		Edit_Timeout = DRONIE_EDIT_TIMEOUT;

		Dronie_LEDs[index]._color.red = set->red;
		Dronie_LEDs[index]._color.green = set->green;
		Dronie_LEDs[index]._color.blue = set->blue;

		switch (index)
		{
			case 0: set_nvs_config(LED0_KEY, DATA_U32, ((uint8_t*) Dronie_LEDs) + (sizeof(color32_t) * index)); break;
			case 1: set_nvs_config(LED1_KEY, DATA_U32, ((uint8_t*) Dronie_LEDs) + (sizeof(color32_t) * index)); break;
			case 2: set_nvs_config(LED2_KEY, DATA_U32, ((uint8_t*) Dronie_LEDs) + (sizeof(color32_t) * index)); break;
			case 3: set_nvs_config(LED3_KEY, DATA_U32, ((uint8_t*) Dronie_LEDs) + (sizeof(color32_t) * index)); break;
			case 4: set_nvs_config(LED4_KEY, DATA_U32, ((uint8_t*) Dronie_LEDs) + (sizeof(color32_t) * index)); break;
		}

		ESP_LOGE(Tag, "[Dronie] LED(%d) Set", index);

	}
}

uint8_t get_dronie_motor_state(void){
	return Dronie_Motor_State;
}

void set_dronie_motor_state(uint8_t state){
	if(state == Dronie_Motor_State) return;

	ESP_LOGI(Tag, "[Dronie] Motor State set (%s)", (state) ? "HIGH" : "LOW");
	Dronie_Motor_State = state;
    gpio_set_level(DRONIE_MOVE_PIN, Dronie_Motor_State);
}

void play_dronie_dot(){
	// if(Dronie_Mode == DRONIE_MODE_DEBUG){
	// 	Edit_Timeout = DRONIE_EDIT_TIMEOUT;
	// 	ESP_LOGE(Tag, "[Dronie] Dot");
	// 	play_sound(CLIP_DOT);
	// }
}

void play_dronie_dash(){
	// if(Dronie_Mode == DRONIE_MODE_DEBUG){
	// 	Edit_Timeout = DRONIE_EDIT_TIMEOUT;
	// 	ESP_LOGE(Tag, "[Dronie] Dash");
	// 	play_sound(CLIP_DASH);
	// }
}


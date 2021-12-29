/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Timer Manager -> blizzard_timer_manager.c
 *
 */

#include "blizzard_timer_manager.h"

#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "esp_log.h"

static const char *Tag = "TIMER MANAGER";

volatile bool Tick = false;

/*
 * Function: init_timer_manager()
 * ----------------------------
 *  starts the timer that will set a flag every interval determined in
 *  blizzard_timer_manager.h
 *
 *  returns: TIMER_INIT_ERROR, TIMER_PAUSE_ERROR, TIMER_SET_COUNTER_ERROR,
 *  TIMER_ALARM_ERROR, TIMER_INTERRUPT_ENABLE_ERROR, TIMER_ISR_REGISTER_ERROR,
 *  TIMER_START_ERROR, SUCCESS
 */
uint8_t init_timer_manager()
{
  esp_err_t err;
  timer_config_t config;
  int timer_group = TIMER_GROUP_0;
  int timer_idx = TIMER_0;

  ESP_LOGI(Tag, "Tick tock...");

  Tick = false;

  config.alarm_en = TIMER_ALARM_EN;
  config.auto_reload = TIMER_AUTORELOAD_EN;
  config.counter_dir = TIMER_COUNT_UP;
  config.divider = TIMER_DIVIDER;
  config.intr_type = TIMER_INTR_LEVEL;
  config.counter_en = TIMER_PAUSE;

  /*Configure timer*/
  err = timer_init(timer_group, timer_idx, &config);
  if (err != ESP_OK) {
      ESP_LOGE(Tag, "Init timer failed - init timer: %s", esp_err_to_name(err));
      return TIMER_INIT_ERROR;
  }

  /*Stop timer counter*/
  err = timer_pause(timer_group, timer_idx);
  if (err != ESP_OK) {
    ESP_LOGE(Tag, "Pause timer failed - init timer: %s", esp_err_to_name(err));
    return TIMER_PAUSE_ERROR;
  }

  /*Load counter value */
  err = timer_set_counter_value(timer_group, timer_idx, 0x00000000ULL);
  if (err != ESP_OK) {
    ESP_LOGE(Tag, "Set counter failed - init timer: %s", esp_err_to_name(err));
    return TIMER_SET_COUNTER_ERROR;
  }

  /*Set alarm value*/
  err = timer_set_alarm_value(timer_group, timer_idx, (TIMER_MS_TIME * TIMER_SCALE_MS - TIMER_FINE_ADJ));
  if (err != ESP_OK) {
    ESP_LOGE(Tag, "Set alarm failed - init timer: %s", esp_err_to_name(err));
    return TIMER_ALARM_ERROR;
  }

  /*Enable timer interrupt*/
  err = timer_enable_intr(timer_group, timer_idx);
  if (err != ESP_OK) {
    ESP_LOGE(Tag, "Set interrupt enable failed - init timer: %s", esp_err_to_name(err));
    return TIMER_INTERRUPT_ENABLE_ERROR;
  }
  
  /*Set ISR handler*/
  err = timer_isr_register(timer_group, timer_idx, timer_group0_isr, (void*) timer_idx, ESP_INTR_FLAG_IRAM, NULL);
  if (err != ESP_OK) {
    ESP_LOGE(Tag, "Set isr register failed - init timer: %s", esp_err_to_name(err));
    return TIMER_ISR_REGISTER_ERROR;
  }

  /*Start timer counter*/
  err = timer_start(timer_group, timer_idx);
  if (err != ESP_OK) {
    ESP_LOGE(Tag, "Start timer failed - init timer: %s", esp_err_to_name(err));
    return TIMER_START_ERROR;
  }

  ESP_LOGI(Tag, "On the clock!!!");

  return SUCCESS;
}

/*
 * Function: IRAM_ATTR timer_group0_isr(void *para)
 * ----------------------------
 *  isr for when the timer goes off - simply sets the Tick flag and resets
 *  the timer
 *
 *  returns: void
 */
void IRAM_ATTR timer_group0_isr(void *para)
{
  int timer_idx = (int) para;
  uint32_t intr_status = TIMERG0.int_st_timers.val;
  if((intr_status & BIT(timer_idx)) && timer_idx == TIMER_0) {

    TIMERG0.hw_timer[timer_idx].update = 1;

    TIMERG0.int_clr_timers.t0 = 1;

    Tick = true;

    TIMERG0.hw_timer[timer_idx].config.alarm_en = TIMER_ALARM_EN;
  }
}

bool check_tick(){
  return Tick;
}

void clear_tick(){
  Tick = false;
}

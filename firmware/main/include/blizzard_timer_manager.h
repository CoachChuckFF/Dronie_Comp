/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Timer Manager-> blizzard_timer_manager.h
 */

#ifndef BLIZZARD_TIMER_MANAGER_H
#define BLIZZARD_TIMER_MANAGER_H

#include <stdio.h>

#include "blizzard_global_defines.h"

#define TIMER_DIVIDER 16000
#define TIMER_MAG_MS 1000
#define TIMER_SCALE_SEC    (TIMER_BASE_CLK / TIMER_DIVIDER)  /*!< used to calculate counter value */
#define TIMER_SCALE_MS    (TIMER_BASE_CLK / TIMER_DIVIDER / TIMER_MAG_MS)  /*!< used to calculate counter value */
#define TIMER_FINE_ADJ   (1.4 * (TIMER_BASE_CLK / TIMER_DIVIDER) / 1000000) /*!< used to compensate alarm value */
#define TIMER_MS_TIME 1 //ms

uint8_t init_timer_manager(void);
void IRAM_ATTR timer_group0_isr(void *para);

bool check_tick(void);
void clear_tick(void);

#endif

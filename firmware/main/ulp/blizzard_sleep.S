/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Battery Manager -> sleep.S
 *
 */

#include "soc/rtc_cntl_reg.h"
#include "soc/soc_ulp.h"

	/* Configurations */
	/* ADC1 channel 0, GPIO36 */
	.set adc_channel, 0
	.set sample_factor, 3
	.set sample_count, (1 << sample_factor)

	/* Define variables, which go into .bss section (zero-initialized data) */
	.bss

	/* High level detection voltage - ulp_detect_high */
	.global detect_high
detect_high:
	.long 0

	/* Low level detection voltage - ulp_detect_low */
	.global detect_low
detect_low:
	.long 0

	/* Wakeup count, amount of cycles to wakeup - ulp_wakeup_count */
	.global wakeup_count
wakeup_count:
	.long 0

	/* Held count (internal) - ulp_held_count */
	.global held_count
held_count:
	.long 0

	/* Last Read - ulp_last_read*/
	.global last_read
last_read:
	.long 0

	/* Code goes into .text section */
	.text
	.global entry

entry:
	/* R0 - adc measurements/held count */
	/* R1 - sample factor/wakeup count */
	/* R2 - adc channel/aux */
	/* R3 - samples per cycle/thresholds */

	/* R0 - clear accumulator for averaging adc Reads */
	MOVE R0, 0

	/* stage will act as our averaging counter */
	STAGE_RST

measure:
	/* Read adc into R1 */
	ADC R1, 0, adc_channel + 1
	ADD R0, R0, R1
	STAGE_INC 1

	/* loop back to measure until it goes through 2^sample_factor samples */
	JUMPS measure, sample_count, lt

	/* Average the Reading */
	RSH R0, R0, sample_factor

	/* Save Reading to memory */
	MOVE R3, last_read
	ST R0, R3, 0

	/* Button Push Detection High */
	MOVE R3, detect_high
	LD R3, R3, 0
	SUB R3, R3, R0 
	JUMP detected, ov

	/* Button Push Detection Low */
	MOVE R3, detect_low
	LD R3, R3, 0
	SUB R3, R0, R3 
	JUMP detected, ov

	/* Exit */
	/* Reset held count to 0 and halt */
	MOVE R0, held_count
	MOVE R2, 0
	ST R2, R0, 0
	HALT


	.global detected
detected:

	/* Increment held_count */
	MOVE R2, held_count
	LD R2, R2, 0
	MOVE R0, R2
	ADD R0, R0, 1

	/* Load in wakeup count */
	MOVE R1, wakeup_count
	LD R1, R1, 0

	/* Compare held count to wakeup count*/
	SUB R1, R1, R0
	JUMP wake_up, ov

	/* Exit */
	/* Save held count and halt */
	MOVE R1, held_count
	ST R0, R1, 0
	HALT

	.global wake_up
wake_up:

	/* Wake up the SoC, end program */
	WAKE
	WRITE_RTC_FIELD(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN, 0)
	HALT

/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Protocol Manager -> blizzard_dmx_manager.h
 *
 * The blizzard dmx manager library supports output and input of dmx using the UART
 * UART serial output of the ESP32 microcontroller.  dmx manager uses
 * UART2 for output and input.  This means that hardware serial
 * can still be used for USB communication.
 * (do not use UART1 because it is connected to flash - depending on the model)
 */

#ifndef BLIZZARD_DMX_MANAGER_H
#define BLIZZARD_DMX_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>
#include "driver/uart.h"
#include "soc/uart_reg.h"

/*----------------------- UART Information -----------------------------------*/

#define DMX_UART UART_NUM_2
#define DMX_DATA_BAUD 248000
#define DMX_BREAK_BAUD 88000

/*----------------------- Uart Driver Defines --------------------------------*/

//#define TOUT_CHECK //if something goes screwing try this
//#define NUKE_WITHOUT_CABLE //if a time out happends blow away the DMX array

#define CURRENT_SLOT_RESET 0xBEEF
#define CURRENT_SLOT_LAST_CALL 0xCACE

#define DMX_FULL_THRESH 32
#define DMX_EMPTY_THRESH 16
#define DMX_TOUT_THRESH 30
#define DMX_FIFO_SIZE 64 //keep this as is

#define DMX_IDLE_SIZE 89
#define DMX_BREAK_SIZE 55
#define DMX_MAB_SIZE 8

/*----------------------- Uart Driver Masks ----------------------------------*/

#define DMX_ALL_INTERRUPT_MASK 0x7FFFF //UART_INTR_MASK There are 18-0 = 19 intr bits

#ifdef TOUT_CHECK
	#define DMX_RECEIVE_INTERRUPT_MASK 	UART_RXFIFO_FULL_INT_ENA_M \
																    | UART_RXFIFO_TOUT_INT_ENA_M \
																		| UART_BRK_DET_INT_ENA_M
#else
	#define DMX_RECEIVE_INTERRUPT_MASK 	UART_RXFIFO_FULL_INT_ENA_M \
																		| UART_BRK_DET_INT_ENA_M
#endif


#define DMX_SEND_INTERRUPT_MASK UART_TXFIFO_EMPTY_INT_ENA_M

/*----------------------- Functions ------------------------------------------*/

uint8_t init_dmx_manager(void);
uint8_t start_dmx(uint8_t direction);
uint8_t stop_dmx(void);

void tick_dmx(uint16_t range);

uint8_t get_dmx_value(uint16_t channel);
volatile uint8_t* get_dmx(void);
uint8_t copy_to_dmx(uint8_t* temp_dmx);
uint8_t copy_from_dmx(uint8_t* temp_dmx);
void blackout(void);

//used in uart.c
void clearOutBuffers(void);
void initDMXDriver(void);
void resetDMXDriver(void);
void rebootDMXDriver(void);

#ifdef __cplusplus
}
#endif

#endif // ifndef DMX_UART_H

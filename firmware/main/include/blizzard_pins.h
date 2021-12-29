/* Copyright (C) Blizzard Lighting LLC. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Christian Krueger <christian@blizzardlighting.com>, January 2018
 *
 * Global Defines-> blizzard_pins.h
 *
 */

#ifndef BLIZZARD_PINS_DEFINES_H
#define BLIZZARD_PINS_DEFINES_H

#include "blizzard_keys.h"

#ifdef __cplusplus
extern "C" {
#endif
     
#ifdef CONFIG_DEVICE_SOC
    #define UNUSED_PIN_MASK ((1ULL<<32) | (1ULL<<33) | (1ULL<<25) | (1ULL<<26) | (1ULL<<13) | (1ULL<<4) | (1ULL<<18) | (1ULL<<19) | (1ULL<<21))

    #define BATTERY_LEVEL_PIN 39
    #define BATTERY_SOURCE_PIN 34
    #define BATTERY_CHARGE_PIN 35

    #define BUTTON_PIN 36

    #define DMX_RX_PIN 27
    #define DMX_SEL_PIN 14
    #define DMX_TX_PIN 12

    #define LED_DATA_PIN 23
    #define LED_ENABLE_PIN 22

    #define BATTERY_LEVEL_PIN_BIT (1ULL<<BATTERY_LEVEL_PIN)
    #define BATTERY_SOURCE_PIN_BIT (1ULL<<BATTERY_SOURCE_PIN)
    #define BATTERY_CHARGE_PIN_BIT (1ULL<<BATTERY_CHARGE_PIN)

    #define BUTTON_PIN_BIT (1ULL<<BUTTON_PIN)

    #define DMX_RX_PIN_BIT (1ULL<<DMX_RX_PIN)
    #define DMX_SEL_PIN_BIT (1ULL<<DMX_SEL_PIN)
    #define DMX_TX_PIN_BIT (1ULL<<DMX_TX_PIN)

    #define LED_DATA_PIN_BIT (1ULL<<LED_DATA_PIN)
    #define LED_ENABLE_PIN_BIT (1ULL<<LED_ENABLE_PIN)
#else

    #define UNUSED_PIN_MASK 0

    #define ROTARY_KNOB_CW_PIN 39
    #define ROTARY_KNOB_CCW_PIN 36
    #define ROTARY_KNOB_BUTTON_PIN 34

    #define DMX_RX_PIN 35
    #define DMX_SEL_PIN 33
    #define DMX_TX_PIN 32

    #define SPI_MOSI_PIN 13
    #define SPI_MISO_PIN 12
    #define SPI_CLK_PIN 14
    #define SPI_CS_1_PIN 15 //WDMX
    #define SPI_CS_2_PIN 2 //Display
    #define SPI_CS_3_PIN 4 //Driver

    #define WDMX_FUNCTION_PIN 16

    #define LED_DATA_PIN 5

    #define ETHERNET_RXD0_PIN 25
    #define ETHERNET_RXD1_PIN 26
    #define ETHERNET_CRS_PIN 27
    #define ETHERNET_MDC_PIN 23
    #define ETHERNET_CLK_PIN 0
    #define ETHERNET_POW_PIN 17
    #define ETHERNET_MIDO_PIN 18
    #define ETHERNET_TXEN_PIN 21
    #define ETHERNET_TXD0_PIN 19
    #define ETHERNET_TXD1_PIN 22

    #define SPI_FLASH_SD2_PIN 9
    #define SPI_FLASH_SD3_PIN 10
    #define SPI_FLASH_CMD_PIN 11
    #define SPI_FLASH_CLK_PIN 6
    #define SPI_FLASH_SD0_PIN 7
    #define SPI_FLASH_SD1_PIN 8                     

    #define ROTARY_KNOB_CW_PIN_BIT (1ULL<<ROTARY_KNOB_CW_PIN)
    #define ROTARY_KNOB_CCW_PIN_BIT (1ULL<<ROTARY_KNOB_CCW_PIN)
    #define ROTARY_KNOB_BUTTON_PIN_BIT (1ULL<<ROTARY_KNOB_BUTTON_PIN)

    #define DMX_RX_PIN_BIT (1ULL<<DMX_RX_PIN)
    #define DMX_SEL_PIN_BIT (1ULL<<DMX_SEL_PIN)
    #define DMX_TX_PIN_BIT (1ULL<<DMX_TX_PIN)

    #define SPI_MOSI_PIN_BIT (1ULL<<SPI_MOSI_PIN)
    #define SPI_MISO_PIN_BIT (1ULL<<SPI_MISO_PIN)
    #define SPI_CLK_PIN_BIT (1ULL<<SPI_CLK_PIN)
    #define SPI_CS_1_PIN_BIT (1ULL<<SPI_CS_1_PIN)
    #define SPI_CS_2_PIN_BIT (1ULL<<SPI_CS_2_PIN)
    #define SPI_CS_3_PIN_BIT (1ULL<<SPI_CS_3_PIN)

    #define WDMX_FUNCTION_PIN_BIT (1ULL<<WDMX_FUNCTION_PIN)

    #define LED_DATA_PIN_BIT (1ULL<<LED_DATA_PIN)

    #define ETHERNET_RXD0_PIN_BIT (1ULL<<ETHERNET_RXD0_PIN)
    #define ETHERNET_RXD1_PIN_BIT (1ULL<<ETHERNET_RXD1_PIN)
    #define ETHERNET_CRS_PIN_BIT (1ULL<<ETHERNET_CRS_PIN)
    #define ETHERNET_MDC_PIN_BIT (1ULL<<ETHERNET_MDC_PIN)
    #define ETHERNET_CLK_PIN_BIT (1ULL<<ETHERNET_CLK_PIN)
    #define ETHERNET_POW_PIN_BIT (1ULL<<ETHERNET_POW_PIN)
    #define ETHERNET_MIDO_PIN_BIT (1ULL<<ETHERNET_MIDO_PIN)
    #define ETHERNET_TXEN_PIN_BIT (1ULL<<ETHERNET_TXEN_PIN)
    #define ETHERNET_TXD0_PIN_BIT (1ULL<<ETHERNET_TXD0_PIN)
    #define ETHERNET_TXD1_PIN_BIT (1ULL<<ETHERNET_TXD1_PIN)

    //legacy
    #define BATTERY_LEVEL_PIN 39
    #define BATTERY_SOURCE_PIN 34
    #define BATTERY_CHARGE_PIN 35
    #define BUTTON_PIN 36
    #define LED_ENABLE_PIN 22
    #define BATTERY_LEVEL_PIN_BIT (1ULL<<BATTERY_LEVEL_PIN)
    #define BATTERY_SOURCE_PIN_BIT (1ULL<<BATTERY_SOURCE_PIN)
    #define BATTERY_CHARGE_PIN_BIT (1ULL<<BATTERY_CHARGE_PIN)
    #define BUTTON_PIN_BIT (1ULL<<BUTTON_PIN)
    #define LED_ENABLE_PIN_BIT (1ULL<<LED_ENABLE_PIN)

#endif


 
#ifdef __cplusplus
}
#endif

#endif
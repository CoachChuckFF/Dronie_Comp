#! /bin/bash

# Linux Only (Also this is a hack - never 'chmod 777')
# sudo chmod 777 /dev/ttyUSB0
# sudo chmod 777 /dev/ttyUSB1

idf.py erase_flash

cp main/modified_drivers/uart.c $IDF_PATH/components/driver/
cp main/modified_drivers/queue.c $IDF_PATH/components/freertos/

idf.py build

idf.py flash

# Change device in file if error
python flash_bpf.py

idf.py monitor


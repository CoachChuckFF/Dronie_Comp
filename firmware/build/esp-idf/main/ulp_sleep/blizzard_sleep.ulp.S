       
       
       
 .set adc_channel, 0
 .set sample_factor, 3
 .set sample_count, (1 << sample_factor)
 .bss
 .global detect_high
detect_high:
 .long 0
 .global detect_low
detect_low:
 .long 0
 .global wakeup_count
wakeup_count:
 .long 0
 .global held_count
held_count:
 .long 0
 .global last_read
last_read:
 .long 0
 .text
 .global entry
entry:
 MOVE R0, 0
 STAGE_RST
measure:
 ADC R1, 0, adc_channel + 1
 ADD R0, R0, R1
 STAGE_INC 1
 JUMPS measure, sample_count, lt
 RSH R0, R0, sample_factor
 MOVE R3, last_read
 ST R0, R3, 0
 MOVE R3, detect_high
 LD R3, R3, 0
 SUB R3, R3, R0
 JUMP detected, ov
 MOVE R3, detect_low
 LD R3, R3, 0
 SUB R3, R0, R3
 JUMP detected, ov
 MOVE R0, held_count
 MOVE R2, 0
 ST R2, R0, 0
 HALT
 .global detected
detected:
 MOVE R2, held_count
 LD R2, R2, 0
 MOVE R0, R2
 ADD R0, R0, 1
 MOVE R1, wakeup_count
 LD R1, R1, 0
 SUB R1, R1, R0
 JUMP wake_up, ov
 MOVE R1, held_count
 ST R0, R1, 0
 HALT
 .global wake_up
wake_up:
 WAKE
 REG_WR ((((0x3ff48000 + 0x18)) - 0x3ff48000) / 4), ((24) + (((((((0x1) >> (0)) & 1) + (((0x1) >> (0 + 1)) & 1)) + ((((0x1) >> (0 + 2)) & 1) + (((0x1) >> (0 + 2 + 1)) & 1))) + (((((0x1) >> (0 + 4)) & 1) + (((0x1) >> (0 + 4 + 1)) & 1)) + ((((0x1) >> (0 + 4 + 2)) & 1) + (((0x1) >> (0 + 4 + 2 + 1)) & 1))))) - 1), (24), ((((0) & 0x1)) & 0xff)
 HALT

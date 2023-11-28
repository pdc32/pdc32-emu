#ifndef REGISTERS_H
#define REGISTERS_H

enum registers {
    REG_DRIVE_SERIAL = 0,
    REG_RTC = 1,
    REG_UNUSED = 2,
    REG_KBD = 3,
    REG_UART = 4,
    REG_DRAM_DATA = 5,
    REG_DRAM_ADDR = 6,
    REG_CACHE_DATA = 7,
    REG_SHIFT_LEFT_A = 8,
    REG_A_AND_B = 9,
    REG_SHIFT_RIGHT_A = 10,
    REG_A_XOR_B = 11,
    REG_A_OR_B = 12,
    REG_A_PLUS_B = 13,
    REG_STATE = 14,
    REG_LITERAL = 15
};

#endif

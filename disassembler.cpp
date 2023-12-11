#include <iostream>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstring>

#include "registers.h"
using namespace std;

#include "instructions.h"

constexpr uint32_t program_len = 32768; // 96 KiB total
uint32_t program[program_len];

void load_program(const char* filename) {
    FILE *fp = fopen(filename, "rb");

    if (fp == nullptr) {
        cerr << "COULD NOT OPEN PROGRAM FILE " << filename << endl;
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    const size_t fileLen = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    for (size_t i = 0; i < fileLen/3; i++) {
        uint8_t bytes[3];
        fread(bytes, 1, 3, fp);

        const uint32_t word = (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
        program[i] = word;
    }
    fclose(fp);
}

char *buses[16] = {
    "serial_drive",
    "rtc",
    "unused",
    "kbd",
    "uart",
    "dram_data",
    "dram_addr",
    "cache_data",
    "a<<1",
    "a&b",
    "a>>1",
    "a^b",
    "a|b",
    "a+b",
    "state",
    "literal"
};

void print_instruction(uint32_t instr) {
    uint8_t full_instr = instr >> 16;
    uint8_t group = full_instr & 0xF0;
    uint8_t type = full_instr & 0xF;
    uint16_t data = instr & 0xFFFF;
    char group_letter = '?';
    bus_register bus_selector = (bus_register)(data & 15);
    const char *bus_selector_string = buses[bus_selector];

    if(group == GROUP_A) group_letter = 'A';
    else if(group == GROUP_B) group_letter = 'B';
    else if(group == GROUP_C) group_letter = 'C';
    else if(group == GROUP_D) group_letter = 'D';

    if(group_letter != '?') {
        printf("%c%d 0x%04X", group_letter, type, data);
        if(full_instr == A0_SET_DRAM_DATA) printf(" ; MOV DRAM_DATA, BUS");
        if(full_instr == A1_SET_CARRY_IN) printf(" ; MOV CIN, BUS");
        if(full_instr == A2_RETURN) printf(" ; RET");
        if(full_instr == A3_JLEQ) printf(" ; JLEQ");
        if(full_instr == A4_JGEQ) printf(" ; JGEQ");
        if(full_instr == A5_JMP) printf(" ; JMP");
        if(full_instr == A6_INC_DRAM_ADDR) printf(" ; INC DRAM_ADDR");
        if(full_instr == A7_SET_DRAM_ADDR) printf(" ; MOV DRAM_ADDR, BUS");
        if(full_instr == A8_JNE) printf(" ; JNE");
        if(full_instr == A9_SET_A) printf(" ; MOV A, BUS");
        if(full_instr == A10_SET_HIGH) printf(" ; MOV HIGH, LITERAL");
        if(full_instr == A11_WRITE_DRAM) printf(" ; WRITE DRAM");
        if(full_instr == A12_SET_BUS) printf(" ; MOV BUS, %s", bus_selector_string);
        if(full_instr == A13_AUTO_RET) printf(" ; MOV AUTORET, BUS");
        if(full_instr == A14_SET_B) printf(" ; MOV B, BUS");
        if(full_instr == A15_SET_ALU) printf(" ; MOV ALU, BUS");

        if(full_instr == B0_TIMER_SPEAKER_OFV) printf(" ; MOV TMRSPK_OVF, BUS");
        if(full_instr == B1_UART_OFV) printf(" ; MOV UART_OVF, BUS");
        if(full_instr == B2_UART_CONFIG) printf(" ; MOV UART_CFG, BUS");
        if(full_instr == B3_UART_TX) printf(" ; MOV UART_TX, BUS");
        if(full_instr == B5_KBD_TX) printf(" ; MOV KBD_TX, BUS");
        if(full_instr == B6_DATA_ADDR_RTC) printf(" ; MOV RTC_DATA_ADDR, BUS");
        if(full_instr == B7_OUT_DEBUG_COMMAND_RTC) printf(" ; MOV RTC_COMMAND, BUS");
        if(full_instr == B8_JL) printf(" ; JL");
        if(full_instr == B9_TIMER_SPEAKER_FUNCTION) printf(" ; MOV TMRSPK_CFG, BUS");
        if(full_instr == B10_JG) printf(" ; JG");
        if(full_instr == B11_JE) printf(" ; JE");
        if(full_instr == B12_SET_CACHE_DATA) printf(" ; MOV CACHE_DATA, BUS");
        if(full_instr == B13_SET_CACHE_ADDR) printf(" ; MOV CACHE_ADDR, BUS");
        if(full_instr == B14_ON_OFF_ATX) printf(" ; MOV ON_OFF, BUS");
        if(full_instr == B15_WRITE_CACHE) printf(" ; WRITE CACHE");

        if(full_instr == C0_TIMER) printf(" ; MOV TIMER_SELECT, BUS");
        if(full_instr == C1_TIME) printf(" ; MOV TIMER_TIME, BUS");
        if(full_instr == C2_DRIVE_SERIAL_DATA) printf(" ; MOV DRIVE_DATA, BUS");
        if(full_instr == C3_DRIVE_SERIAL_ADDR) printf(" ; MOV DRIVE_ADDR, BUS");
        if(full_instr == C4_DRIVE_SERIAL_FUNCTION) printf(" ; MOV DRIVE_CFG, BUS");
        if(full_instr == C7_VGA_TEXT_COLOR) printf(" ; MOV VGA_TXT_COLOR, BUS");
        if(full_instr == C8_VGA_WRITE_VRAM) printf(" ; WRITE VRAM");
        if(full_instr == C9_VGA_FUNCTION) printf(" ; MOV VGA_CFG, BUS");
        if(full_instr == C10_VGA_TEXT_BLINK) printf(" ; MOV VGA_TXT_BLINK, BUS");
        if(full_instr == C11_VGA_PIXEL_COLOR) printf(" ; MOV VGA_PIX_COLOR, BUS");
        if(full_instr == C12_VGA_TEXT_WRITE) printf(" ; WRITE VGA_TEXT");
        if(full_instr == C13_VGA_TEXT_CHAR) printf(" ; MOV VGA_TXT_CHAR, BUS");
        if(full_instr == C14_VGA_PIXEL_POS) printf(" ; MOV VGA_PIX_POS, BUS");
        if(full_instr == C15_VGA_TEXT_POS) printf(" ; MOV VGA_TXT_POS, BUS");
    } else {
        printf("'0x%06X", instr);
    }
}

int main(int argc, char **argv) {
    if (argc >= 2) {
        bool help = false;

        help |= strcmp(argv[1], "-h") == 0;
        help |= strcmp(argv[1], "-H") == 0;
        help |= strcmp(argv[1], "--help") == 0;
        help |= strcmp(argv[1], "/?") == 0;

        if (help) {
            cout << "usage: " << argv[0] << " [-h] [program.bin]" << endl;
            return 0;
        }
        load_program(argv[1]);
    } else {
        load_program("firmware/PDC32.firmware");
    }

    for(size_t i=0;i<program_len;i++) {
        printf("p%04X: ", i);
        print_instruction(program[i]);
        printf("\n");
    }

    return 0;
}

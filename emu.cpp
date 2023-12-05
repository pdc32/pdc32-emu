#include <iostream>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
using namespace std;

#include "instructions.h"
#include "registers.h"
#include "vga.h"
#include "spk.h"
#include "alu.h"
#include "tmr.h"
#include "pwr.h"
#include "uart.h"
#include "eep.h"

constexpr uint32_t program_len = 32768; // 32 K * 3 bytes
constexpr uint32_t cache_len = 32768; // 128 KiB
constexpr uint32_t dram_len = 8388608; // 32 MiB

// 4Mhz (baseclock) / 4 (clocks per instruction) / 60hz (display fps)
constexpr uint32_t instructions_per_display_update = 16666;

uint32_t data_literal = 0; // Data from instructions

uint32_t cache_addr = 0;
uint32_t cache_data = 0;
uint32_t cache[cache_len];

uint32_t dram_addr = 0;
uint32_t dram_data = 0;
uint32_t dram[dram_len];

uint32_t program[program_len];
uint16_t program_counter = 0;
uint16_t return_address = 0;
bool save_return = false;

bus_register bus_selector = REG_LITERAL;
bool debug = false;

void jump_to(uint16_t addr) {
    if(addr > program_len) {
        cerr << "At " << program_counter-1 << " trying to jump to invalid address " << addr << endl;
        exit(1);
    }
    if(save_return) {
        return_address = program_counter;
    }
    program_counter = addr;
}

uint32_t get_state() {
    return alu_get_state() |
        (spk_ovf() ? spk_ovf_bit : 0) |
        (pwr_get_state() << pwr_state_offset) |
        (uart_state() << uart_state_offset) |
        (vga_get_mode() << vga_mode_offset) |
        keyboard_rx() << keyboard_rx_offset |
        tmr_busy() << tmr_busy_offset | 
        tmr_ovf() << tmr_ovf_offset;
}

uint32_t bus() {
    switch (bus_selector) {
        case REG_DRIVE_SERIAL:
            return eep_read();
        case REG_RTC:
            return 0; // TODO: implement
        case REG_UNUSED:
            return 0;
        case REG_KBD:
            return keyboard_get_data();
        case REG_UART:
            return uart_read();
        case REG_DRAM_DATA:
            return dram[dram_addr % dram_len];
        case REG_DRAM_ADDR:
            return dram_addr;
        case REG_CACHE_DATA:
            return cache[cache_addr % cache_len];
        case REG_SHIFT_LEFT_A:
            return alu_get_a() << 1;
        case REG_A_AND_B:
            return alu_get_a() & alu_get_b();
        case REG_SHIFT_RIGHT_A:
            return alu_get_a() >> 1;
        case REG_A_XOR_B:
            return alu_get_a() ^ alu_get_b();
        case REG_A_OR_B:
            return alu_get_a() | alu_get_b();
        case REG_A_PLUS_B:
            return alu_get_sum();
        case REG_STATE:
            return get_state();
        case REG_LITERAL:
            return data_literal;
    }
    cerr << "MISSING IMPLEMENTATION FOR REGISTER: " << bus_selector << endl;
    exit(1);
}

void handleInstruction(const uint32_t instruction) {
    const uint8_t type = (instruction & 0xFF0000) >> 16;
    const uint16_t data = instruction & 0xFFFF;
    data_literal = (data_literal & 0xFFFF0000) | data;

    if(type == A0_SET_DRAM_DATA) {
        dram_data = bus();
        if(debug) printf("A0 %x ; DRAM_DATA = %x\n", bus(), bus());
    } else if(type == A1_SET_CARRY_IN) {
        alu_set_carry_in(bus() & alu_carry_in_bit);
        if(debug) printf("A1 %x ; CARRY_IN = %d\n", bus() & alu_carry_in_bit, bus() & alu_carry_in_bit);
    } else if(type == A2_RETURN) {
        program_counter = return_address;
        if(debug) printf("A2 ; RET\n");
    } else if(type == A3_JLEQ) {
        if(alu_less_or_equal_than()) jump_to(bus() % program_len);
        if(debug) printf("A3 %x ; JLEQ %x\n", bus(), bus());
    } else if(type == A4_JGEQ) {
        if(alu_greater_or_equal_than()) jump_to(bus() % program_len);
        if(debug) printf("A4 %x ; JGEQ %x\n", bus(), bus());
    } else if(type == A5_JMP) {
        jump_to(bus() % program_len);
        if(debug) printf("A5 %x ; JMP %x\n", bus(), bus());
    } else if(type == A6_INC_DRAM_ADDR) {
        dram_addr++;
        if(debug) printf("A6 ; INC DRAM_ADDR\n");
    } else if(type == A7_SET_DRAM_ADDR) {
        dram_addr = bus() % dram_len;
        if(debug) printf("A7 %x ; DRAM_ADDR = %x\n", bus(), bus());
    } else if(type == A8_JNE) {
        if(alu_not_equal_than()) jump_to(bus() % program_len); else if(debug) printf("JNE failed %x %x\n", alu_get_a(), alu_get_b());
        if(debug) printf("A8 %x ; JNE %x\n", bus(), bus());
    } else if(type == A9_SET_A) {
        alu_set_a(bus());
        if(debug) printf("A9 %x ; A = %x\n", bus(), bus());
    } else if(type == A10_SET_HIGH) {
        data_literal = (data_literal & 0xFFFF) | data << 16;
        if(debug) printf("A10 %x ; HIGH = %x\n", data, data);
    } else if(type == A11_WRITE_DRAM) {
        dram[dram_addr] = dram_data;
        if(debug) printf("A11 ; WRITE_DRAM\n");
    } else if(type == A12_SET_BUS) {
        bus_selector = (bus_register)(data & 15);
        if(debug) printf("A12 %d ; BUS = %d\n", bus_selector, bus_selector);
    } else if(type == A13_AUTO_RET) {
        save_return = bus() & 0x8000;
        if(debug) printf("A13 %d ; AUTORET = %x\n", bus() & 0x8000, bus() & 0x8000);
    } else if(type == A14_SET_B) {
        alu_set_b(bus());
        if(debug) printf("A14 %x ; B = %x\n", bus(), bus());
    } else if(type == A15_SET_ALU) {
        alu_set_flags(bus() & alu_flags_bits);
        if(debug) printf("A15 %x ; ALU_FLAGS = %x\n", bus() & alu_flags_bits, bus() & alu_flags_bits);
    } else if(type == B0_TIMER_SPEAKER_OFV) {
        if(debug) printf("B0 %x ; SPK_OVF = %x\n", bus(), bus());
        spk_b0_timer_ovf(bus());
    } else if(type == B1_UART_OFV) {
        if(debug) printf("B1 %x ; UART_OVF = %x\n", bus(), bus());
        uart_b1_set_ovf(bus());
    } else if(type == B2_UART_CONFIG) {
        if(debug) printf("B2 %x ; UART_CFG = %x\n", bus(), bus());
        uart_b2_config(bus());
    } else if(type == B3_UART_TX) {
        if(debug) printf("B3 %x ; UART_TX = %x\n", bus(), bus());
        uart_b3_tx(bus());
    } else if(type == B5_KBD_TX) {
        if(debug) printf("B5! %x ; KBD_TX\n", bus());
        uint8_t command = bus() & 0xff;
        keyboard_B5_send(command);
    } else if(type == B6_DATA_ADDR_RTC) {
        if(debug) printf("B6! %x ; RTC_ADDR_DATA\n", bus());
        // TODO: implement
    } else if(type == B7_OUT_DEBUG_COMMAND_RTC) {
        if(debug) printf("B7! %x ; RTC_CMD\n", bus());
        // TODO: implement
    } else if(type == B8_JL) {
        if(debug) printf("B8 %x ; JL %x\n", bus(), bus());
        if(alu_less_than()) jump_to(bus() % program_len);
    } else if(type == B9_TIMER_SPEAKER_FUNCTION) {
        const bool off = bus() & spk_off_bit;
        const bool mute = bus() & spk_mute_bit;
        spk_b9_timer_config(!off, mute);
        if(debug) printf("B9 %x ; TMR_SPK = %x\n", bus(), bus());
    } else if(type == B10_JG) {
        if(debug) printf("B10 %x ; JG %x\n", bus(), bus());
        if(alu_greater_than()) jump_to(bus() % program_len);
    } else if(type == B11_JE) {
        if(debug) printf("B11 %x ; JE %x\n", bus(), bus());
        if(alu_equal_than()) jump_to(bus() % program_len); else if(debug) printf("JE failed %x %x\n", alu_get_a(), alu_get_b());
    } else if(type == B12_SET_CACHE_DATA) {
        if(debug) printf("B12 %x ; CACHE_DATA = %x\n", bus(), bus());
        cache_data = bus();
    } else if(type == B13_SET_CACHE_ADDR) {
        if(debug) printf("B13 %x ; CACHE_ADDR = %x\n", bus(), bus());
        cache_addr = bus() % cache_len;
    } else if(type == B14_ON_OFF_ATX) {
        if(debug) printf("B14 %x ; PWR = %x\n", bus(), bus());
        pwr_B14_set_power_on(bus() & pwr_bit);
    } else if(type == B15_WRITE_CACHE) {
        if(debug) printf("B15 ; CACHE_WRITE\n");
        cache[cache_addr] = cache_data;
    } else if(type == C0_TIMER) {
        if(debug) printf("C0 %x ; TMR_SELECT = %x\n", bus(), bus());
        tmr_C0_select_timer(bus() >> 24);
    } else if(type == C1_TIME) {
        if(debug) printf("C1 %x ; TMR_SET = %x\n", bus(), bus());
        tmr_C1_set_time(bus() & 0xFFFFFF);
    } else if(type == C2_DRIVE_SERIAL_DATA) {
        if(debug) printf("C2! %x ; SERIAL MEM DATA\n", bus());
        // TODO: implement
    } else if(type == C3_DRIVE_SERIAL_ADDR) {
        if(debug) printf("C3! %x ; SERIAL MEM ADDR\n", bus());
        // TODO: implement
    } else if(type == C4_DRIVE_SERIAL_FUNCTION) {
        if(debug) printf("C4! %x ; SERIAL MEM FCN\n", bus());
        // TODO: implement
    } else if(type == C7_VGA_TEXT_COLOR) {
        if(debug) printf("C7 %x ; VGA_COLOR %x\n", bus(), bus());
        uint8_t fg = (bus() >> 16) & 0xff;
        uint8_t bg = (bus() >> 24) & 0xff;
        vga_C7_text_color(fg, bg);
    } else if(type == C8_VGA_WRITE_VRAM) {
        if(debug) printf("C8! ; VGA_WRITE_RAM\n");
        // TODO: implement
    } else if(type == C9_VGA_FUNCTION) {
        if(debug) printf("C9 %x ; VGA_FUNCTION %d\n", bus(), bus());
        vga_C9_set_mode(bus() & vga_C9_mode_bits);
    } else if(type == C10_VGA_TEXT_BLINK) {
        if(debug) printf("C10 %x ; VGA_BLINK %x\n", bus(), bus());
        vga_C10_blink(bus() & vga_blink_bit);
    } else if(type == C11_VGA_PIXEL_COLOR) {
        if(debug) printf("C11! %x ; VGA_COLOR\n", bus());
        // TODO: implement
    } else if(type == C12_VGA_TEXT_WRITE) {
        if(debug) printf("C12 ; VGA_TEXT_WRITE\n");
        vga_C12_text_write();
    } else if(type == C13_VGA_TEXT_CHAR) {
        if(debug) printf("C13 %x ; VGA_CHAR\n", bus());
        vga_C13_set_char(bus() & 0xff);
    } else if(type == C14_VGA_PIXEL_POS) {
        if(debug) printf("C14! %x ; VGA_PIXEL_POS\n", bus());
        // TODO: implement
    } else if(type == C15_VGA_TEXT_POS) {
        if(debug) printf("C15 %x ; VGA_TEXT_POS\n", bus());
        uint8_t row = (bus() >> 7) & 0x1f;
        uint8_t col = bus() & 0x7f;
        vga_C15_text_position(row, col);
    } else {
        cerr << "UNKNOWN INSTRUCTION " << hex << instruction << " at 0x" << program_counter-1 << endl;
        exit(1);
    }
}

void loadProgram(const char* filename) {
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

void dump_memory() {
    FILE *dram_fp = fopen("build/dram.bin", "wb");
    fwrite(dram, 4, dram_len, dram_fp);
    fclose(dram_fp);

    FILE *cache_fp = fopen("build/cache.bin", "wb");
    fwrite(cache, 4, cache_len, cache_fp);
    fclose(cache_fp);
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
        loadProgram(argv[1]);
    } else {
        loadProgram("firmware/PDC32.firmware");
    }

    bool quit = false;

    display_init();
    spk_init();
    eep_init();
    uint32_t executed_instructions = 0;
    while(!quit) {
        if(handle_events()) quit = true;

        for(uint32_t i=0; i<instructions_per_display_update; i++) {
            if(debug) {
                printf("%x: ", program_counter);
            }
            const uint32_t instruction = program[program_counter++];
            handleInstruction(instruction);
            spk_process();
            executed_instructions++;
            tmr_process();
        }
        display_update(&executed_instructions);
    }
    eep_teardown();
    display_teardown();
    if(debug) {
        dump_memory();
    }
    return 0;
}

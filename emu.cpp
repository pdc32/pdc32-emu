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

constexpr uint32_t program_len = 32768; // 32 K * 3 bytes
constexpr uint32_t cache_len = 131072; // 128 KiB
constexpr uint32_t dram_len = 33554432; // 32 MiB

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

constexpr uint32_t power_on_bit = 1<<11;
bool power_on = false;

uint8_t uart_data_bits = 8;

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
    return alu_get_state() | (spk_ovf() ? spk_ovf_bit : 0) | power_on_bit | (vga_get_mode() << vga_mode_offset);
}

uint32_t bus() {
    switch (bus_selector) {
        case REG_DRIVE_SERIAL:
            return 0; // TODO: implement
        case REG_RTC:
            return 0; // TODO: implement
        case REG_UNUSED:
            return 0;
        case REG_KBD:
            return 0; // TODO: implement
        case REG_UART:
            return 0; // TODO: implement
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
    } else if(type == A1_SET_CARRY_IN) {
        alu_set_carry_in(bus() & alu_carry_in_bit);
    } else if(type == A2_RETURN) {
        program_counter = return_address;
    } else if(type == A3_JLEQ) {
        if(alu_less_or_equal_than()) jump_to(bus() % program_len); else if(debug) printf("JLEQ failed %x %x\n", alu_get_a(), alu_get_b());
    } else if(type == A4_JGEQ) {
        if(alu_greater_or_equal_than()) jump_to(bus() % program_len); else if(debug) printf("JGEQ failed %x %x\n", alu_get_a(), alu_get_b());
    } else if(type == A5_JMP) {
        jump_to(bus() % program_len);
    } else if(type == A6_INC_DRAM_ADDR) {
        dram_addr++;
    } else if(type == A7_SET_DRAM_ADDR) {
        dram_addr = bus() % dram_len;
    } else if(type == A8_JNE) {
        if(alu_not_equal_than()) jump_to(bus() % program_len); else if(debug) printf("JNE failed %x %x\n", alu_get_a(), alu_get_b());
    } else if(type == A9_SET_A) {
        alu_set_a(bus());
    } else if(type == A10_SET_HIGH) {
        data_literal = (data_literal & 0xFFFF) | data << 16;
    } else if(type == A11_WRITE_DRAM) {
        dram[dram_addr] = dram_data;
    } else if(type == A12_SET_BUS) {
        bus_selector = (bus_register)(data & 15);
    } else if(type == A13_AUTO_RET) {
        save_return = bus() & 0x8000;
    } else if(type == A14_SET_B) {
        alu_set_b(bus());
    } else if(type == A15_SET_ALU) {
        alu_set_flags(bus() & alu_flags_bits);
    } else if(type == B0_TIMER_SPEAKER_OFV) {
        spk_b0_timer_ovf(bus());
    } else if(type == B1_UART_OFV) {
        auto speed = 24000000 / bus();
        cout << "UART OFV: speed=" << speed << " bps (* might need more math)" << endl;
    } else if(type == B2_UART_CONFIG) {
        auto word = bus();
        bool parity_odd = (word >> 16) & 0b1;
        uint8_t databits = 6 + ((word >> 17) & 0b11);
        bool parity_enabled = (word >> 19) & 0b1;
        uint8_t stopbits = 1 + (word >> 20);

        uart_data_bits = databits;

        cout << "UART CONFIG:"
            << " databits=" << unsigned(databits)
            << " parity=" << (parity_enabled ? (parity_odd ? "ODD" : "EVEN") : "OFF")
            << " stopbits=" << unsigned(stopbits)
            << endl;
    } else if(type == B3_UART_TX) {
        uint16_t mask = (1 << (uart_data_bits + 1)) - 1; // lower bits
        uint16_t word = bus() & mask;
        char data[5]; // in case uart_data_bits gets corrupted to something invalid, the bus might be 4 hexa chars + NUL
                      //
        if (uart_data_bits <= 8) {
            data[0] = (char) bus() & 0xFF;
            data[1] = '\0';
        } else {
            snprintf(data, 4, "%x", word);
        }

        cout << "UART TX: (" << unsigned(uart_data_bits) << " bits) " << (char *) data << endl;
    } else if(type == B5_KBD_TX) {
        // TODO: implement
    } else if(type == B6_DATA_ADDR_RTC) {
        // TODO: implement
    } else if(type == B7_OUT_DEBUG_COMMAND_RTC) {
        // TODO: implement
    } else if(type == B8_JL) {
        if(alu_less_than()) jump_to(bus() % program_len); else if(debug) printf("JL failed %x %x\n", alu_get_a(), alu_get_b());
    } else if(type == B9_TIMER_SPEAKER_FUNCTION) {
        const bool off = bus() & spk_off_bit;
        const bool mute = bus() & spk_mute_bit;
        spk_b9_timer_config(!off, mute);
    } else if(type == B10_JG) {
        if(alu_greater_than()) jump_to(bus() % program_len); else if(debug) printf("JG failed %x %x\n", alu_get_a(), alu_get_b());
    } else if(type == B11_JE) {
        if(alu_equal_than()) jump_to(bus() % program_len); else if(debug) printf("JE failed %x %x\n", alu_get_a(), alu_get_b());
    } else if(type == B12_SET_CACHE_DATA) {
        cache_data = bus();
    } else if(type == B13_SET_CACHE_ADDR) {
        cache_addr = bus() % cache_len;
    } else if(type == B14_ON_OFF_ATX) {
        power_on = bus() & (1<<11);
        cout << "Power status changed: " << power_on << endl;
    } else if(type == B15_WRITE_CACHE) {
        cache[cache_addr] = cache_data;
    } else if(type == C0_TIMER) {
        // TODO: implement
    } else if(type == C1_TIME) {
        // TODO: implement
    } else if(type == C2_DRIVE_SERIAL_DATA) {
        // TODO: implement
    } else if(type == C3_DRIVE_SERIAL_ADDR) {
        // TODO: implement
    } else if(type == C4_DRIVE_SERIAL_FUNCTION) {
        // TODO: implement
    } else if(type == C7_VGA_TEXT_COLOR) {
        uint8_t fg = (bus() >> 16) & 0xff;
        uint8_t bg = (bus() >> 24) & 0xff;
        vga_C7_text_color(fg, bg);
    } else if(type == C8_VGA_WRITE_VRAM) {
        // TODO: implement
    } else if(type == C9_VGA_FUNCTION) {
        vga_C9_set_mode(bus() & vga_C9_mode_bits);
    } else if(type == C10_VGA_TEXT_BLINK) {
        vga_C10_blink(bus() & vga_blink_bit);
    } else if(type == C11_VGA_PIXEL_COLOR) {
        // TODO: implement
    } else if(type == C12_VGA_TEXT_WRITE) {
        uint8_t c = bus() & 0xff;
        vga_C12_text_write(c);
    } else if(type == C13_VGA_TEXT_CHAR) {
        // TODO: implement
    } else if(type == C14_VGA_PIXEL_POS) {
        // TODO: implement
    } else if(type == C15_VGA_TEXT_POS) {
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
    uint32_t executed_instructions = 0;
    while(!quit) {
        if(handle_events()) quit = true;

        for(uint32_t i=0; i<instructions_per_display_update; i++) {
            if(debug) {
                printf("%x %x %x %x %x\n", program_counter, program[program_counter]>>16, program[program_counter] & 0xFFFF, alu_get_a(), alu_get_b());
            }
            const uint32_t instruction = program[program_counter++];
            handleInstruction(instruction);
            spk_process();
            executed_instructions++;
        }
        display_update(&executed_instructions);
    }
    display_teardown();
    return 0;
}

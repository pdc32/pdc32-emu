#include <iostream>
#include <bitset>

#include "rtc.h"

using namespace std;


// Access to registers
#define SECONDS (rtc_ds1387.data[REG_SECONDS])
#define SECONDS_ALARM   (rtc_ds1387.data[REG_SECONDS_ALARM])
#define MINUTES (rtc_ds1387.data[MINUTES])
#define MINUTES_ALARM (rtc_ds1387.data[MINUTES_ALARM])
#define HOURS (rtc_ds1387.data[HOURS])
#define HOURS_ALARM (rtc_ds1387.data[HOURS_ALARM])
#define DAY_OF_THE_WEEK (rtc_ds1387.data[DAY_OF_THE_WEEK])
#define DAY_OF_THE_MONTH (rtc_ds1387.data[DAY_OF_THE_MONTH])
#define MONTH (rtc_ds1387.data[MONTH])
#define YEAR (rtc_ds1387.data[YEAR])
#define REGISTER_A (rtc_ds1387.data[REGISTER_A])
#define REGISTER_B (rtc_ds1387.data[REGISTER_B])
#define REGISTER_C (rtc_ds1387.data[REGISTER_C])
#define REGISTER_D (rtc_ds1387.data[REGISTER_D])

// Pin mapping - All negated by definition except ALE
#define OER 0b00000001
#define AS0 0b00000010
#define AS1 0b00000100
#define WER 0b00001000
#define RD  0b00010000
#define WR  0b00100000
#define ALE 0b01000000
#define CS  0b10000000

/*
#define OER 0b10000000
#define AS0 0b01000000
#define AS1 0b00100000
#define WER 0b00010000
#define RD  0b00001000
#define WR  0b00000100
#define ALE 0b00000010
#define CS  0b00000001
*/
struct RTC_DS1387 rtc_ds1387;

void ds1387_init(){
    # std::cout << "RTC: Init" << std::endl;
    rtc_ds1387.nvram_file = fopen("rtc_nvram.bin", "wb");
    if(rtc_ds1387.nvram_file == nullptr) {
		std::cerr << "RTC: Cannot open nvram file" << std::endl;
		exit(1);
	}
    fread(&(rtc_ds1387.regs_nvram[USER_NVRAM]), 1, USER_NVRAM_SIZE, rtc_ds1387.nvram_file);
}

void ds1387_set_cmd(uint32_t bus){
    uint8_t connected_lines = bus & 0x00FF;

    // RTC operation. Get Calendar, Registers and user nram data
    if (~connected_lines & CS) {
        # std::cout << "RTC: Operation" << std::endl;
        // RTC Address strobe
        if (connected_lines & WR && connected_lines & RD  && connected_lines & ALE) {
            # std::cout << "RTC: Address strobe" << std::endl;
            rtc_ds1387.reg_index = rtc_ds1387.data_reg;
            return;
        }
        // Write choosen register
        if (~connected_lines & WR && connected_lines & RD  && connected_lines & ALE) {
            # std::cout << "RTC: Write choosen register" << std::endl;
            rtc_ds1387.regs_nvram[rtc_ds1387.reg_index] = rtc_ds1387.data_reg;
            return;
        }
        // Read choosen register
        if (connected_lines & WR && ~connected_lines & RD  && connected_lines & ALE) {
            # std::cout << "RTC: Write choosen register" << std::endl;
            rtc_ds1387.data_reg = rtc_ds1387.regs_nvram[rtc_ds1387.reg_index];
            return;
        }
    }
    else 
        if (~connected_lines & WER && ~connected_lines & OER)
            std::cerr << "RTC doesn't support write and read signals at same time" << std::endl;
        else
            if (~connected_lines & WER) {
                rtc_ds1387.static_ram[rtc_ds1387.ram_address] = rtc_ds1387.data_reg;
                # std::cout << "RTC: write to static ram" << std::endl;
            }
            else {
                // AS0 latches the lower eight bits of static ram address
                if (~connected_lines & AS0) {
                    rtc_ds1387.ram_address = rtc_ds1387.ram_address & 0xFF00;
                    rtc_ds1387.ram_address = rtc_ds1387.ram_address | rtc_ds1387.data_reg;

                    return;
                }
                // AS1 latches the upper four bits of static ram address
                if (~connected_lines & AS1) {
                    uint16_t upper_byte = 0x00FF & rtc_ds1387.data_reg;
                    upper_byte = upper_byte << 8;
                    rtc_ds1387.ram_address = rtc_ds1387.ram_address & 0x00FF;
                    rtc_ds1387.ram_address = rtc_ds1387.ram_address | upper_byte;
                    return;
                }
                if (~connected_lines & OER) {
                    rtc_ds1387.data_reg = rtc_ds1387.static_ram[rtc_ds1387.ram_address];
                    # std::cout << "RTC: Reading from static ram" << std::endl;
                    return;
                }
                 if (~connected_lines & WER) {
                    rtc_ds1387.static_ram[rtc_ds1387.ram_address] = rtc_ds1387.data_reg;
                    # std::cout << "RTC: Writing to static ram" << std::endl;
                    return;
                }

            }
}

uint8_t ds1387_get_data(){
    return rtc_ds1387.data_reg;
}

void ds1387_set_data(uint32_t bus){
    rtc_ds1387.data_reg = bus & 0x00FF;
}

void ds1387_tick(){

}

void ds1387_write_nvram (const uint8_t address, const uint8_t value) {
    rtc_ds1387.regs_nvram[address] = value;
    fwrite(&(rtc_ds1387.regs_nvram[USER_NVRAM]), 1, USER_NVRAM_SIZE, rtc_ds1387.nvram_file);
}

uint8_t ds1387_read_nvram (uint8_t address) {
    return rtc_ds1387.regs_nvram[address];
}

void ds1387_teardown(){
    fclose(rtc_ds1387.nvram_file);
}
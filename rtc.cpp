#include <iostream>
#include <bitset>
#include <chrono>
#include <ctime>

#include "rtc.h"

using namespace std;
using namespace std::chrono;


// Access to registers
#define SECONDS rtc_ds1387.regs_nvram[REG_SECONDS]
#define SECONDS_ALARM   rtc_ds1387.regs_nvram[REG_SECONDS_ALARM]
#define MINUTES rtc_ds1387.regs_nvram[REG_MINUTES]
#define MINUTES_ALARM rtc_ds1387.regs_nvram[REG_MINUTES_ALARM]
#define HOURS rtc_ds1387.regs_nvram[REG_HOURS]
#define HOURS_ALARM rtc_ds1387.regs_nvram[REG_HOURS_ALARM]
#define DAY_OF_THE_WEEK rtc_ds1387.regs_nvram[REG_DAY_OF_THE_WEEK]
#define DAY_OF_THE_MONTH rtc_ds1387.regs_nvram[REG_DAY_OF_THE_MONTH]
#define MONTH rtc_ds1387.regs_nvram[REG_MONTH]
#define YEAR rtc_ds1387.regs_nvram[REG_YEAR]
#define REGISTER_A rtc_ds1387.regs_nvram[REG_REGISTER_A]
#define REGISTER_B rtc_ds1387.regs_nvram[REG_REGISTER_B]
#define REGISTER_C rtc_ds1387.regs_nvram[REG_REGISTER_C]
#define REGISTER_D rtc_ds1387.regs_nvram[REG_REGISTER_D]

// Pin mapping - All negated by definition except ALE
#define OER 0b00000001
#define AS0 0b00000010
#define AS1 0b00000100
#define WER 0b00001000
#define RD  0b00010000
#define WR  0b00100000
#define ALE 0b01000000
#define CS  0b10000000

//#define DEBUG_RTC true

struct RTC_DS1387 rtc_ds1387;

void ds1387_init(){
    std::cout << "RTC: Init" << std::endl;
    rtc_ds1387.nvram_file = fopen("res/rtc_nvram.bin", "rb");
    rtc_ds1387.old_value = 0xFF;

    if(rtc_ds1387.nvram_file == nullptr) {
		std::cerr << "RTC: Cannot open nvram file. New one will be created" << std::endl;
	}
    else {
        fread(&rtc_ds1387.regs_nvram, 1, USER_NVRAM_SIZE, rtc_ds1387.nvram_file);
        fclose(rtc_ds1387.nvram_file);
    }

}

void update_time_regs(){
    system_clock::time_point now = system_clock::now();
    time_t tt = system_clock::to_time_t(now);
    tm local_tm = *localtime(&tt);

    SECONDS = local_tm.tm_sec;
    MINUTES = local_tm.tm_min;
    HOURS = local_tm.tm_hour;
    DAY_OF_THE_WEEK = local_tm.tm_wday;
    DAY_OF_THE_MONTH = local_tm.tm_mday;
    MONTH = local_tm.tm_mon + 1;
    YEAR = uint8_t(local_tm.tm_year % 100);    
}

void ds1387_set_cmd(uint32_t bus){
    uint8_t connected_lines = bus & 0xFF;
    uint8_t changes, failing_edges, rising_edges;

    changes = rtc_ds1387.old_value ^ connected_lines;
    rising_edges = changes & connected_lines;
    failing_edges = changes & ~connected_lines;

    rtc_ds1387.old_value = connected_lines;

    
    // RTC operation. Get Calendar, Registers and user nram data
    if (~connected_lines & CS) {
        #ifdef DEBUG_RTC
        std::cout << "RTC: Operation " << std::bitset<8>(connected_lines) << std::endl;
        #endif

        // RTC Address strobe
        if (connected_lines & WR && connected_lines & RD  && failing_edges & ALE) {
        
            rtc_ds1387.reg_address = rtc_ds1387.data_in;

            #ifdef DEBUG_RTC
                std::cout << "RTC: Address strobe " << std::bitset<8>(rtc_ds1387.data_in) << std::endl;
                switch (rtc_ds1387.reg_address)
                {
                case REG_REGISTER_A:
                    std::cout << "            Access REGISTER A " << std::endl;
                    break;
                case REG_REGISTER_B:
                    std::cout << "            Access REGISTER B " << std::endl;
                    break;
                case REG_REGISTER_C:
                    std::cout << "            Access REGISTER C " << std::endl;
                    break;
                case REG_REGISTER_D:
                    std::cout << "            Access REGISTER D " << std::endl;
                    break;                                
                default:
                    break;
                }
            #endif

            return;
        }

        // Write choosen register
        if (rising_edges & WR && connected_lines & RD) {
            #ifdef DEBUG_RTC
                std::cout << "RTC: Write choosen register " \
                    << std::bitset<8>(rtc_ds1387.reg_address) \
                    << ":: 0x" << std::hex << int(rtc_ds1387.reg_address) \
                    << std::endl << "    value 0x" << std::hex << int(rtc_ds1387.data_in) \
                    << std::endl;
            #endif

            rtc_ds1387.regs_nvram[rtc_ds1387.reg_address] = rtc_ds1387.data_in;
            return;
        }
        // Read choosen register
        if (connected_lines & WR && connected_lines & ~RD) {

            if (rtc_ds1387.reg_address <= YEAR) {
                update_time_regs();
            };

            rtc_ds1387.data_out = rtc_ds1387.regs_nvram[rtc_ds1387.reg_address];

            #ifdef DEBUG_RTC
                std::cout << "RTC: Read choosen register " \
                << std::bitset<8>(rtc_ds1387.reg_address) \
                << ":: 0x" << std::hex << int(rtc_ds1387.reg_address) \
                << std::endl << "    value 0x" << std::hex << int(rtc_ds1387.data_out) \
                << std::endl;
            #endif

            return;
        }
    }
    else 
        if (~connected_lines & WER && ~connected_lines & OER) {
            std::cerr << "RTC doesn't support write and read signals at same time" << std::endl;
        } else {
            // AS0 latches the lower eight bits of static ram address
            if (rising_edges & AS0) {
                rtc_ds1387.ram_address = rtc_ds1387.ram_address & 0xFF00;
                rtc_ds1387.ram_address = rtc_ds1387.ram_address | rtc_ds1387.data_in;
#ifdef DEBUG_RTC
                std::cout << "RTC: AS0 Latch" << std::endl;
#endif
                return;
            }
            // AS1 latches the upper four bits of static ram address
            if (rising_edges & AS1) {
                uint16_t upper_byte = 0x00FF & rtc_ds1387.data_in;
                upper_byte = upper_byte << 8;
                rtc_ds1387.ram_address = rtc_ds1387.ram_address & 0x00FF;
                rtc_ds1387.ram_address = rtc_ds1387.ram_address | upper_byte;
#ifdef DEBUG_RTC
                std::cout << "RTC: AS1 Latch" << std::endl;
#endif
                return;
            }
            if (~connected_lines & WER) {
                rtc_ds1387.static_ram[rtc_ds1387.ram_address] = rtc_ds1387.data_in;
#ifdef DEBUG_RTC
                std::cout << "RTC: Writing to static ram" << std::endl;
#endif
                return;
            }
            if (~connected_lines & OER) {
                rtc_ds1387.data_out = rtc_ds1387.static_ram[rtc_ds1387.ram_address];
#ifdef DEBUG_RTC
                std::cout << "RTC: Reading from static ram" << std::endl;
#endif
                return;
            }
        }
}

uint32_t ds1387_get_data(){
    #ifdef DEBUG_RTC
        std::cout << "RTC: Read data latch " \
            << std::bitset<8>(rtc_ds1387.data_out) \
            << ":: 0x" << std::hex << int(rtc_ds1387.data_out) \
            << std::endl;
    #endif    
    return rtc_ds1387.data_out;
}

void ds1387_set_data(uint32_t bus){
    rtc_ds1387.data_in = bus & 0xFF;
    #ifdef DEBUG_RTC
        std::cout << "RTC: Set data latch " \
        << std::bitset<8>(rtc_ds1387.data_in) \
        << ":: 0x" << std::hex << int(rtc_ds1387.data_in) \
        << std::endl;
    #endif
}

void ds1387_teardown(){
    rtc_ds1387.nvram_file = fopen("res/rtc_nvram.bin", "wb");
    fwrite(&rtc_ds1387.regs_nvram, 1, USER_NVRAM_SIZE, rtc_ds1387.nvram_file);
    fclose(rtc_ds1387.nvram_file);
}

#ifndef RTC_H
#define RTC_H


#define DS1387_REGS_SIZE 0x40
#define DS1387_RAM_SIZE 4096


// Registers
#define REG_SECONDS 0x00
#define REG_SECONDS_ALARM 0x01
#define REG_MINUTES 0x02
#define REG_MINUTES_ALARM 0x03
#define REG_HOURS 0x04
#define REG_HOURS_ALARM 0x05
#define REG_DAY_OF_THE_WEEK 0x06
#define REG_DAY_OF_THE_MONTH 0x07
#define REG_MONTH 0x08
#define REG_YEAR 0x09
#define REG_REGISTER_A 0x0A
#define REG_REGISTER_B 0x0B
#define REG_REGISTER_C 0x0C
#define REG_REGISTER_D 0x0D
//User RAM. Begin
#define USER_NVRAM 0x0E
#define USER_NVRAM_SIZE 50

#include <cstdint>

struct RTC_DS1387
{
	uint8_t reg_index;
	uint8_t regs_nvram[DS1387_REGS_SIZE];

	uint16_t ram_address;
	uint8_t static_ram[DS1387_RAM_SIZE];

    FILE* nvram_file;

	uint8_t data_reg;
};

void ds1387_init();
void ds1387_set_cmd(uint32_t bus);
void ds1387_set_data(uint32_t bus);
uint8_t ds1387_get_data();


#endif

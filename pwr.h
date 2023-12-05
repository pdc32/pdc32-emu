#ifndef PWR_H
#define PWR_H

#include <cstdint>

constexpr uint32_t pwr_state_offset = 11;
constexpr uint32_t pwr_bit = 1<<pwr_state_offset;
void pwr_B14_set_power_on(bool val);
void pwr_button_press(bool val);
uint32_t pwr_get_state();

#endif

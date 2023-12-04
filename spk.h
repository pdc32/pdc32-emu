#ifndef SPK_H
#define SPK_H

#include <cstdint>

void spk_b0_timer_ovf(uint32_t bus);
void spk_b9_timer_config(bool on, bool mute);
void spk_process();
int spk_init();
bool spk_ovf();
constexpr uint32_t spk_ovf_bit = 16;
constexpr uint32_t spk_off_bit = 0x10;
constexpr uint32_t spk_mute_bit = 0x20;

#endif

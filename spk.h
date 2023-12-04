#ifndef SPK_H
#define SPK_H

#include <cstdint>

void spk_b0_timer_ovf(uint32_t bus);
void spk_b9_timer_config(bool on, bool mute);
void spk_process();
int spk_init();

#endif

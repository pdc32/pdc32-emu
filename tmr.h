#ifndef TMR_H
#define TMR_H

#include <cstdint>

#include "emu.h"

void tmr_C0_select_timer(uint8_t timer);
void tmr_C1_set_time(uint32_t time);
void tmr_process();
uint8_t tmr_busy();
uint8_t tmr_ovf();

constexpr uint32_t tmr_busy_offset = 15;
constexpr uint32_t tmr_ovf_offset = 24;
constexpr uint32_t instructions_per_timer_update = instructions_per_second / 14400; // 1.843 MHz / 128 (TMR multiplexing)

#endif

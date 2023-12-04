#ifndef TMR_H
#define TMR_H

#include <cstdint>

void tmr_C0_select_timer(uint8_t timer);
void tmr_C1_set_time(uint32_t time);
void tmr_process();
uint8_t tmr_busy();
uint8_t tmr_ovf();

constexpr uint32_t tmr_busy_offset = 15;
constexpr uint32_t tmr_ovf_offset = 24;

#endif

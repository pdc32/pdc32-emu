#ifndef EMU_H
#define EMU_H

uint64_t get_tick_count();
void emu_reset();
constexpr uint32_t instructions_per_second = 4000000 / 4;

#endif

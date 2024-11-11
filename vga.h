#ifndef VGA_H
#define VGA_H

#include "emu.h"

constexpr uint32_t frames_per_second = 15;
constexpr uint32_t keyboard_update_hz = 240;
constexpr uint32_t ms_per_frame = 1000 / frames_per_second;

// 4Mhz (baseclock) / 4 (clocks per instruction)
constexpr uint32_t instructions_per_display_update = instructions_per_second / frames_per_second;
constexpr uint32_t instructions_per_event_checking = instructions_per_second / keyboard_update_hz;

// SDL input/output functions

// initialize SDL, open window, setup palette, etc
int display_init();

// render text buffer into VRAM, blit onto the screen
void display_update();

// shut it all down
void display_teardown();

// handle mouse/kbd/quit events, returns non zero on exit
int handle_events();

// ----------------------------------

// implementation of PDC32 VGA opcodes
void vga_C7_text_color(uint8_t fg, uint8_t bg);
void vga_C8_write_vram();
void vga_C9_set_mode(uint8_t mode);
void vga_C10_blink(bool enable);
void vga_C11_pixel_color(uint8_t color);
void vga_C12_text_write();
void vga_C13_set_char(uint8_t character);
void vga_C14_pixel_position(uint16_t pos_y, uint16_t pos_x);
void vga_C14_pixel_position(uint32_t abs_pos);

void vga_C15_text_position(uint8_t row, uint8_t col);


uint8_t vga_get_mode();

constexpr uint32_t vga_blink_bit = 1<<12;
constexpr uint32_t vga_C9_mode_bits = 3;
constexpr uint32_t vga_mode_offset = 12;


// ----------------------------------

void keyboard_B5_send(uint8_t command);
void keyboard_queue(const char* codes);
uint32_t keyboard_get_data();
uint8_t keyboard_rx_state();

constexpr uint32_t keyboard_rx_offset = 7;
constexpr uint32_t keyboard_tx_offset = 8;

#endif // VGA_H

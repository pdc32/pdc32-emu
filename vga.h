#ifndef VGA_H
#define VGA_H

// SDL input/output functions

// initialize SDL, open window, setup palette, etc
int display_init();

// render text buffer into VRAM, blit onto the screen
void display_update(uint32_t *executed_instructions);

// shut it all down
void display_teardown();

// handle mouse/kbd/quit events, returns non zero on exit
int handle_events();

// ----------------------------------

// implementation of PDC32 VGA opcodes
void vga_C7_text_color(uint8_t fg, uint8_t bg);
void vga_C12_text_write();
void vga_C15_text_position(uint8_t row, uint8_t col);
void vga_C9_set_mode(uint8_t mode);
void vga_C10_blink(bool enable);
void vga_C13_set_char(uint8_t character);

uint8_t vga_get_mode();

constexpr uint32_t vga_blink_bit = 1<<12;
constexpr uint32_t vga_C9_mode_bits = 3;
constexpr uint32_t vga_mode_offset = 12;


// ----------------------------------

void keyboard_B5_send(uint8_t command);
void keyboard_queue(const char* codes);
uint8_t keyboard_get_byte();

#endif // VGA_H

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
void vga_C12_text_write(uint8_t c);
void vga_C15_text_position(uint8_t row, uint8_t col);

constexpr uint32_t blink_bit = 1<<12;
void vga_C10_blink(bool enable);

#endif // VGA_H

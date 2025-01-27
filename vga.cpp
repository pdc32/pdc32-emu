// alecu was here
//

#include <SDL2/SDL.h>
#include <cstdlib>
#include <iostream>
#include <bitset>
#include <queue>

#include "vga.h"

#include "eep.h"
#include "keyboard/scancodes.cpp"

#include "pwr.h"
#include "emu.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

constexpr uint8_t text_columns = 80;
constexpr uint8_t text_rows = 30;
constexpr uint8_t char_height = 16;
constexpr uint16_t screen_width = 640;
constexpr uint16_t screen_height = 480;
uint32_t framebuffer[screen_width*screen_height];

struct vga_character {
    uint8_t character;
    uint8_t fg;
    uint8_t bg;
};

vga_character text_vram[text_rows][text_columns];
uint8_t charset_rom[256][char_height];

uint32_t pixel_abs_pos;
uint8_t pixel_color;

uint8_t text_cursor_col;
uint8_t text_cursor_row;
uint8_t text_color_fg;
uint8_t text_color_bg;
uint8_t vga_mode = 0;
uint8_t vga_char = 0;

bool glitchy_video;
bool enable_blink = false;


uint32_t pallete[256];

void init_pdc32_palette(uint32_t colors[256]) {
    //std::cerr << "PAL" << std::endl;
    for (int j=0; j<256; j++) {
        uint8_t v = j;
        // por ahora se toman los bits: BBGGGRRR
        // esta distribucion está completamente inventada
        // habría que checkear con el autor
        uint8_t red = (v & 0x07) << 5;
        uint8_t green = ((v >> 3) & 0x07) << 5;
        uint8_t blue = ((v >> 6) & 0x03) << 6;
        uint8_t alpha = 255;
        pallete[j] = red | green<<8 | blue << 16 | alpha << 24;
            //std::cerr << std::bitset<8>(red) << ", " << std::bitset<8>(green) << ", " << std::bitset<8>(blue) << std::endl;
    }
    //std::cerr << "/PAL" << std::endl;
}

void vga_C7_text_color(uint8_t fg, uint8_t bg) {
    text_color_fg = fg;
    text_color_bg = bg;
}

void vga_C8_write_vram(){
    framebuffer[pixel_abs_pos] = pallete[pixel_color];
}

uint8_t vga_get_mode() {
    return vga_mode;
}

void vga_C9_set_mode(uint8_t mode) {
    vga_mode = mode;
}

void vga_C10_blink(bool enable) {
    enable_blink = enable;
}

void vga_C11_pixel_color(uint8_t color){
    pixel_color = color;
}

void vga_C12_text_write() {
    if (text_cursor_row >= text_rows || text_cursor_col >= text_columns) {
        std::cerr << "Skipping out of bounds character <" << (int)vga_char
            << ">, row: " << (int)text_cursor_row
            << ", col: " << (int)text_cursor_col << std::endl;
        return;
    }
    text_vram[text_cursor_row][text_cursor_col].character = vga_char;
    text_vram[text_cursor_row][text_cursor_col].fg = text_color_fg;
    text_vram[text_cursor_row][text_cursor_col].bg = text_color_bg;
}

void vga_C13_set_char(uint8_t character) {
    vga_char = character;
}

void vga_C14_pixel_position(uint32_t abs_pos){
    pixel_abs_pos = abs_pos;
}

void vga_C15_text_position(uint8_t row, uint8_t col) {
    if (row >= text_rows) {
        std::cerr << "row too large: " << (int)row << std::endl;
    }
    text_cursor_row = row;
    if (col >= text_columns) {
        std::cerr << "col too large: " << (int)col << std::endl;
    }
    text_cursor_col = col;
}

SDL_Window* win;
SDL_Renderer* ren;
SDL_Texture* tex;
SDL_Texture* power_button_tex;
SDL_Texture* paste_button_tex;
SDL_Texture* load_button_tex;
SDL_Texture* store_button_tex;
SDL_Texture* reload_button_tex;
SDL_Rect power_button_rect = {screen_width-16,0,16,16};
SDL_Rect activity_led_rect = {screen_width-16,20,16,16};
SDL_Rect load_button_rect = {screen_width-16,40,16,16};
SDL_Rect store_button_rect = {screen_width-16,60,16,16};
SDL_Rect reload_button_rect = {screen_width-16,80,16,16};
SDL_Rect paste_button_rect = {screen_width-16, 100,16,16};
bool blink_status = false;
SDL_Texture* led_off_tex;
SDL_Texture* led_on_tex;


void vga_update_framebuffer(uint32_t *framebuffer) {
    if (vga_mode==0x02 || vga_mode==0x03) {
        int row_start = 0;
        for (int row=0; row < text_rows; row++) {
        int col_offset = row_start;
        for (int col=0; col < text_columns; col++) {
            uint8_t c = text_vram[row][col].character;

            uint8_t fg = text_vram[row][col].fg;
            uint8_t bg = text_vram[row][col].bg;

            if(enable_blink && blink_status) {
                if((text_vram[row][col].fg & 0x80) == 0) {
                    fg = 0;
                }
                if((text_vram[row][col].bg & 0x80) == 0) {
                    bg = 0;
                }
            }

            int offset = col_offset;
            for (int y=0; y < char_height; y++) {
            int byte = charset_rom[c][y];
            for (int x=0; x < 8; x++) {
                uint8_t color = ((byte >> x) & 1 ) ? fg : bg;
                framebuffer[offset + 7-x] = pallete[color];

                if(glitchy_video && col % 2 == 0 && x == 7 && rand()%2000 == 0) {
                    framebuffer[offset + 7-x] = pallete[127];
                }
            }
            offset += text_columns * 8;
            }
            col_offset += 8;
            }
        row_start += text_columns * 8 * char_height;
        }
    }
}

const char* format_as_hex_pairs(const char* sig_code) {
    static char hexbuf[40];
    unsigned char* code = (unsigned char*) sig_code;
    int n = 0;
    while(code[n] != 0) {
        snprintf(hexbuf + n*2, 3, "%02x", code[n]);
        n++;
    }

    return hexbuf;
}

bool power_button_pressed = false;
#include "char_to_scancode.h"

std::queue<const char*> keycodes_paste;

#ifndef __EMSCRIPTEN__
#define EMSCRIPTEN_KEEPALIVE
#endif

extern "C" {
    void EMSCRIPTEN_KEEPALIVE keyboard_paste_text(char* text) {
        for(size_t i=0;i<strlen(text);i++) {
            SDL_Scancode scancode0 = SDL_SCANCODE_UNKNOWN, scancode1 = SDL_SCANCODE_UNKNOWN;
            char_to_scancode(text[i], &scancode0, &scancode1);

            if(scancode0 != SDL_SCANCODE_UNKNOWN) {
                keycodes_paste.push(ps2_map.at(scancode0).key_make);
                if(scancode1 != SDL_SCANCODE_UNKNOWN) {
                    keycodes_paste.push(ps2_map.at(scancode1).key_make);
                    keycodes_paste.push(ps2_map.at(scancode1).key_break);
                }
                keycodes_paste.push(ps2_map.at(scancode0).key_break);
            }
        }
    }
}

int handle_events()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)){
        if(e.type == SDL_QUIT) {
            return 1;
        }
        if(e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            if(e.key.repeat != 0) continue;
            auto found = ps2_map.find(e.key.keysym.scancode);
            if (found == ps2_map.end()) {
                std::cerr << "Key not found: " << e.key.keysym.scancode << std::endl;
            } else {
                const PS2_scancode& code = ps2_map.at(e.key.keysym.scancode);
                if(e.type == SDL_KEYDOWN) {
                    //std::cerr << "Key press: " << format_as_hex_pairs(code.key_make) << std::endl;
                    keyboard_queue(code.key_make);
                } else { // SDL_KEYUP
                    //std::cerr << "Key release: " << format_as_hex_pairs(code.key_break) << std::endl;
                    keyboard_queue(code.key_break);
                }
            }
        }
        if(e.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            if (mouseX >= power_button_rect.x && mouseX <= power_button_rect.x + power_button_rect.w &&
                mouseY >= power_button_rect.y && mouseY <= power_button_rect.y + power_button_rect.h) {
                power_button_pressed = true;
                pwr_button_press(true);
            }

            if (mouseX >= store_button_rect.x && mouseX <= store_button_rect.x + store_button_rect.w &&
                mouseY >= store_button_rect.y && mouseY <= store_button_rect.y + store_button_rect.h) {
                eep_download();
            }
            if (mouseX >= load_button_rect.x && mouseX <= load_button_rect.x + load_button_rect.w &&
                mouseY >= load_button_rect.y && mouseY <= load_button_rect.y + load_button_rect.h) {
                eep_upload();
            }
            if (mouseX >= reload_button_rect.x && mouseX <= reload_button_rect.x + reload_button_rect.w &&
                mouseY >= reload_button_rect.y && mouseY <= reload_button_rect.y + reload_button_rect.h) {
                eep_reload();
            }

#ifndef __EMSCRIPTEN__
            if (mouseX >= paste_button_rect.x && mouseX <= paste_button_rect.x + paste_button_rect.w &&
                mouseY >= paste_button_rect.y && mouseY <= paste_button_rect.y + paste_button_rect.h) {
                keyboard_paste_text(SDL_GetClipboardText());
            }
#endif

        }
        if(e.type == SDL_MOUSEBUTTONUP) {
            if(power_button_pressed) {
                power_button_pressed = false;
                pwr_button_press(false);
            }
        }
    }

    /*
    int keycode = keyboard_get_byte();
    if (keycode) {
        std::cerr << "Byte in keyboard queue: " << std::hex << keycode << std::endl;
    }
    */
    
    return 0;
}

void load_rom() {
    FILE *fp = fopen("font/pdc32.font", "rb");
    if(fp == nullptr) {
        std::cerr << "Cannot open font file" << std::endl;
        exit(1);
    }
    fread(charset_rom, 1, 4096, fp);
    fclose(fp);
}

int display_init() {
// based on sdl2-examples, https://github.com/xyproto/sdl2-examples/tree/main/c%2B%2B11
// Copyright 2023 Alexander F. Rødseth
    using std::cerr;
    using std::endl;

    load_rom();

    auto sdl_init_flags = SDL_INIT_EVERYTHING;
#ifdef __EMSCRIPTEN__
    sdl_init_flags &= ~SDL_INIT_HAPTIC;
#endif

    if (SDL_Init(sdl_init_flags) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    win = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) {
        std::cerr << "SDL_CreateRenderer Error" << SDL_GetError() << std::endl;
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
    if (tex == nullptr) {
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_Surface *power_button = SDL_LoadBMP("res/power.bmp");
    if (power_button == nullptr) {
        std::cerr << "SDL_LoadBMP (res/power.bmp) Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_Surface *load_button = SDL_LoadBMP("res/load.bmp");
    if (load_button == nullptr) {
        std::cerr << "SDL_LoadBMP (res/load.bmp) Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_Surface *store_button = SDL_LoadBMP("res/store.bmp");
    if (store_button == nullptr) {
        std::cerr << "SDL_LoadBMP (res/store.bmp) Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_Surface *reload_button = SDL_LoadBMP("res/reload.bmp");
    if (reload_button == nullptr) {
        std::cerr << "SDL_LoadBMP (res/reload.bmp) Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_Surface *paste_button = SDL_LoadBMP("res/paste.bmp");
    if (paste_button == nullptr) {
        std::cerr << "SDL_LoadBMP (res/paste.bmp) Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_Surface *led_off = SDL_LoadBMP("res/led_off.bmp");
    if (led_off == nullptr) {
        std::cerr << "SDL_LoadBMP (res/led_off.bmp) Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_Surface *led_on = SDL_LoadBMP("res/led_on.bmp");
    if (led_on == nullptr) {
        std::cerr << "SDL_LoadBMP (res/led_on.bmp) Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return EXIT_FAILURE;
    }
    power_button_tex = SDL_CreateTextureFromSurface(ren, power_button);
    load_button_tex = SDL_CreateTextureFromSurface(ren, load_button);
    store_button_tex = SDL_CreateTextureFromSurface(ren, store_button);
    reload_button_tex = SDL_CreateTextureFromSurface(ren, reload_button);
    paste_button_tex = SDL_CreateTextureFromSurface(ren, paste_button);
    led_off_tex = SDL_CreateTextureFromSurface(ren, led_off);
    led_on_tex = SDL_CreateTextureFromSurface(ren, led_on);
    SDL_FreeSurface(power_button);
    SDL_FreeSurface(load_button);
    SDL_FreeSurface(store_button);
    SDL_FreeSurface(paste_button);
    SDL_FreeSurface(led_off);
    SDL_FreeSurface(led_on);

    init_pdc32_palette(pallete);
    return EXIT_SUCCESS;
}

void update_window_title(SDL_Window* window, int frames, Uint32 start_ticks, Uint32 executed_instructions) {
    float avg_fps = frames / ((SDL_GetTicks() - start_ticks) / 1000.0f);
    float avg_ips = executed_instructions / ((SDL_GetTicks() - start_ticks) / 1000.0f);

    char title[100];
    snprintf(title, sizeof(title), "PDC32 Emulator - FPS: %.2f - IPS: %.2f %s", avg_fps, avg_ips, pwr_is_on() ? "" : "- OFF");

    SDL_SetWindowTitle(window, title);
}


void display_update() {
    static Uint32 last_visit = 0;
    static Uint32 last_blink = 0;
    static Uint32 fps_ticks = 0;
    static Sint32 frames = 0;
    static uint64_t last_ticks = get_tick_count();
    if(!pwr_is_on()) {
        memset(text_vram, 0, sizeof(text_vram));
    }
    vga_update_framebuffer(framebuffer);

    SDL_UpdateTexture(tex, NULL, framebuffer, screen_width * 4);
    SDL_RenderCopy(ren, tex, nullptr, nullptr);
    SDL_RenderCopy(ren, power_button_tex, nullptr, &power_button_rect);
    SDL_RenderCopy(ren, load_button_tex, nullptr, &load_button_rect);
    SDL_RenderCopy(ren, store_button_tex, nullptr, &store_button_rect);
    SDL_RenderCopy(ren, reload_button_tex, nullptr, &reload_button_rect);
#ifndef __EMSCRIPTEN__
    SDL_RenderCopy(ren, paste_button_tex, nullptr, &paste_button_rect);
#endif
    SDL_RenderCopy(ren, eep_was_active_last_frame() ? led_on_tex : led_off_tex, nullptr, &activity_led_rect);
    SDL_RenderPresent(ren);

    // Un toque de delay, para que no ejecute mas de 60fps,
    // y poder simular la velocidad real de la PDC32
    while(!SDL_TICKS_PASSED(SDL_GetTicks(), last_visit + ms_per_frame)) {
        SDL_Delay(1);
    }
    if(SDL_TICKS_PASSED(SDL_GetTicks(), last_blink + 125)) {
        blink_status = !blink_status;
        last_blink = SDL_GetTicks();

        if(!keycodes_paste.empty()) {
            // 8 keycodes per second to avoid buffer overruns
            keyboard_queue(keycodes_paste.front());
            keycodes_paste.pop();
        }
    }

    frames++;
    if (SDL_TICKS_PASSED(SDL_GetTicks(), fps_ticks + 500)) {
        update_window_title(win, frames, fps_ticks, get_tick_count() - last_ticks);

        // Reset variables for the next second
        fps_ticks = SDL_GetTicks();
        frames = 0;
        last_ticks = get_tick_count();
    }

    last_visit = SDL_GetTicks();
}

void display_teardown() {
    SDL_DestroyTexture(power_button_tex);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

std::queue<uint32_t> keycodes_queue;
void keyboard_queue(const char* codes) {
    uint8_t* bytes = (uint8_t*) codes;
    uint8_t b = *bytes++;
    while(b != 0) {
        uint32_t data = b;
        b = *bytes++;

        if (b != 0) {
            data = data << 8 | b;
            b = *bytes++;
        }

        if (b != 0) {
            data = data << 8 | b;
            b = *bytes++;
        }
        keycodes_queue.push(data);
    }
}

uint32_t keyboard_get_data() {
    if (keycodes_queue.empty()) {
        return 0;
    } else {
        uint32_t data = keycodes_queue.front();
        keycodes_queue.pop();
        std::cerr << "Data in bus from keyboard: " << std::hex << data << std::endl;
        return data;
    }
}

uint8_t keyboard_rx_state() {
    return keycodes_queue.empty() == false ? 1 : 0;
}

void keyboard_reset() {
    while(!keycodes_queue.empty()) {
        keycodes_queue.pop();
    }
}

void keyboard_send_ack() {
    keycodes_queue.push(0xFA); // ack
}

void keyboard_B5_send(uint8_t code) {
    switch(code) {
        case 0xFF: // reset
            keyboard_reset();
            keyboard_send_ack();
            break;
        case 0xED: // set LEDs
            keyboard_send_ack();
            break;
        default:
            std::cerr << "PDC32 wants to send to keyboard: " <<  std::hex << (int)code << std::endl;
            keyboard_send_ack();
            break;
    }
}

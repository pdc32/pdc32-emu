// alecu was here
//

#include <SDL2/SDL.h>
#include <cstdlib>
#include <iostream>
#include <bitset>

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

uint8_t text_cursor_col;
uint8_t text_cursor_row;
uint8_t text_color_fg;
uint8_t text_color_bg;
bool glitchy_video;

void vga_C7_text_color(uint8_t fg, uint8_t bg) {
    text_color_fg = fg;
    text_color_bg = bg;
}

void vga_C12_text_write(uint8_t c) {
    text_vram[text_cursor_row][text_cursor_col].character = c;
    text_vram[text_cursor_row][text_cursor_col].fg = text_color_fg;
    text_vram[text_cursor_row][text_cursor_col].bg = text_color_bg;
}

void vga_C15_text_position(uint8_t row, uint8_t col) {
    text_cursor_row = row;
    text_cursor_col = col;
}

SDL_Window* win;
SDL_Renderer* ren;
SDL_Texture* tex;
uint32_t pallete[128];
bool blink_status = false;

void init_pdc32_palette(uint32_t colors[128]) {
    //std::cerr << "PAL" << std::endl;
    for (int j=0; j<128; j++) {
	    uint8_t v = j;
	    // por ahora se toman los bits: BBGGGRR
	    // esta distribucion está completamente inventada
	    // habría que checkear con el autor
	    uint8_t red = (v & 0x03) << 6;
	    uint8_t green = ((v >> 2) & 0x07) << 5;
	    uint8_t blue = ((v >> 5) & 0x03) << 6;
	    uint8_t alpha = 255;
	    pallete[j] = red | green<<8 | blue << 16 | alpha << 24;
            //std::cerr << std::bitset<8>(red) << ", " << std::bitset<8>(green) << ", " << std::bitset<8>(blue) << std::endl;
    }
    //std::cerr << "/PAL" << std::endl;
}

void vga_update_framebuffer(uint32_t *framebuffer) {
    int row_start = 0;
    for (int row=0; row < text_rows; row++) {
	int col_offset = row_start;
	for (int col=0; col < text_columns; col++) {
	    uint8_t c = text_vram[row][col].character;

	    uint8_t fg = text_vram[row][col].fg & 0x7f;
	    uint8_t bg = text_vram[row][col].bg & 0x7f;

		if(blink_status && text_vram[row][col].fg & 0x80) {
			fg = 0;
		}
		if(blink_status && text_vram[row][col].bg & 0x80) {
			bg = 0;
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

int handle_events()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)){
    	if(e.type == SDL_QUIT) {
    		return 1;
    	}
    }
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

void vga_text_test() {
    vga_C15_text_position(15, 40);
    vga_C7_text_color(0b111, 0);
    vga_C12_text_write('P');
    vga_C7_text_color(0b111000, 0);
    vga_C12_text_write('D');
    vga_C7_text_color(0b11001001, 0);
    vga_C12_text_write('C');
    vga_C7_text_color(255, 0);
    vga_C12_text_write('3');
    vga_C12_text_write('2');
    vga_C7_text_color(0b111111, 0);
    vga_C12_text_write(255);
}

int display_init() {
// based on sdl2-examples, https://github.com/xyproto/sdl2-examples/tree/main/c%2B%2B11
// Copyright 2023 Alexander F. Rødseth
    using std::cerr;
    using std::endl;

    load_rom();

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    win = SDL_CreateWindow("PDC32 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, SDL_WINDOW_SHOWN);
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

	tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
    if (tex == nullptr) {
	    std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
		SDL_DestroyRenderer(ren);
		SDL_DestroyWindow(win);
		SDL_Quit();
        return EXIT_FAILURE;
    }

    init_pdc32_palette(pallete);
    return EXIT_SUCCESS;
}

void display_update() {

    static Uint32 last_visit = 0;
    static Uint32 last_blink = 0;
    vga_update_framebuffer(framebuffer);

	SDL_UpdateTexture(tex, NULL, framebuffer, screen_width * 4);
    SDL_RenderCopy(ren, tex, nullptr, nullptr);
    SDL_RenderPresent(ren);

    // Un toque de delay, para que no ejecute mas de 60fps,
    // y poder simular la velocidad real de la PDC32
    while(!SDL_TICKS_PASSED(SDL_GetTicks(), last_visit + 16)) {
        SDL_Delay(1);
    }
	if(SDL_TICKS_PASSED(SDL_GetTicks(), last_blink + 125)) {
		blink_status = !blink_status;
		last_blink = SDL_GetTicks();
	}

    last_visit = SDL_GetTicks();
}

void display_teardown() {
	SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

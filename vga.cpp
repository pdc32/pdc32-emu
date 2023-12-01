// alecu was here
//

#include <SDL2/SDL.h>
#include <cstdlib>
#include <iostream>
#include <bitset>

constexpr uint8_t text_columns = 80;
constexpr uint8_t text_rows = 30;
constexpr uint8_t char_height = 16;

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

void vga_C7_text_color(uint8_t fg, uint8_t bg) {
    text_color_fg = fg;
    text_color_bg = bg;
}

void vga_C12_text_write(uint8_t c) {
    text_vram[text_cursor_row][text_cursor_col].character = c;
    text_vram[text_cursor_row][text_cursor_col].fg = text_color_fg;
    text_vram[text_cursor_row][text_cursor_col].bg = text_color_bg;
    text_cursor_col += 1;
    if (text_cursor_col >= text_columns) {
	text_cursor_col = 0;
	text_cursor_row += 1;
    }

    if (text_cursor_row >= text_rows) {
	// TODO: end of screen reached. Should we scroll?
	// for the time being, we start again at the top
        text_cursor_row = 0;
    }
}

void vga_C15_text_position(uint8_t row, uint8_t col) {
    text_cursor_row = row;
    text_cursor_col = col;
}

SDL_Window* win;
SDL_Renderer* ren;
SDL_Surface* bmp;
SDL_Color colors[256];

void init_pdc32_palette(SDL_Color colors[256]) {
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
	    colors[j] = { red, green, blue, alpha};
            //std::cerr << std::bitset<8>(red) << ", " << std::bitset<8>(green) << ", " << std::bitset<8>(blue) << std::endl;
    }
    //std::cerr << "/PAL" << std::endl;
}

void vga_update_framebuffer(unsigned char* framebuffer) {
    int row_start = 0;
    for (int row=0; row < text_rows; row++) {
	int col_offset = row_start;
	for (int col=0; col < text_columns; col++) {
	    uint8_t c = text_vram[row][col].character;
	    uint8_t fg = text_vram[row][col].fg;
	    uint8_t bg = text_vram[row][col].bg;
	    int offset = col_offset;
	    for (int y=0; y < char_height; y++) {
		int byte = charset_rom[c][y];
		for (int x=0; x < 8; x++) {
		    uint8_t color = ((byte >> x) & 1 ) ? fg : bg;
		    // TODO: blinking
		    framebuffer[offset + 7-x] = color;
		}
		offset += text_columns * 8;
	    }
	    col_offset += 8;
        }
	row_start += text_columns * 8 * char_height;
    }
}

void sdl_loop()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)){
    }
}

void load_rom() {
    FILE *fp = fopen("font/pdc32.font", "rb");
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

    vga_text_test();

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    win = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) {
	std::cerr << "SDL_CreateRenderer Error" << SDL_GetError() << std::endl;
		if (win != nullptr) {
			SDL_DestroyWindow(win);
		}
		SDL_Quit();
        return EXIT_FAILURE;
    }

    bmp = SDL_CreateRGBSurface(0,640,480,8,0,0,0,0);
    if (bmp == nullptr) {
	    std::cerr << "SDL_LoadBMP Error: " << SDL_GetError() << std::endl;
		if (ren != nullptr) {
			SDL_DestroyRenderer(ren);
		}
		if (win != nullptr) {
			SDL_DestroyWindow(win);
		}
		SDL_Quit();
        return EXIT_FAILURE;
    }

    init_pdc32_palette(colors);

    SDL_SetPaletteColors(bmp->format->palette, colors, 0, 255);
    return EXIT_SUCCESS;
}

void display_update() {
    static Uint32 last_visit = 0;
    unsigned char* framebuffer = (unsigned char*)bmp->pixels;
    vga_update_framebuffer(framebuffer);
    
    // TODO: buscar la forma de actualizar la surface sin crearla en cada cuadro
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, bmp);
    if (tex == nullptr) {
	std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
	if (bmp != nullptr) {
		SDL_FreeSurface(bmp);
	}
	if (ren != nullptr) {
		SDL_DestroyRenderer(ren);
	}
	if (win != nullptr) {
		SDL_DestroyWindow(win);
	}
	SDL_Quit();
        return; //EXIT_FAILURE;
    }

    sdl_loop();
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, tex, nullptr, nullptr);
    SDL_RenderPresent(ren);
    SDL_DestroyTexture(tex);

    // Un toque de delay, para que no ejecute mas de 60fps,
    // y poder simular la velocidad real de la PDC32
    while(!SDL_TICKS_PASSED(SDL_GetTicks(), last_visit + 16)) {
    }

    last_visit = SDL_GetTicks();
}

void display_teardown() {
    SDL_FreeSurface(bmp);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

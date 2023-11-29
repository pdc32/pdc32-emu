// alecu was here
//

#include <SDL2/SDL.h>
#include <cstdlib>
#include <iostream>

void init_pdc32_palette(SDL_Color colors[256]) {
    for (int j=0; j<256; j++) {
	    // por ahora se toman los bits: BBGGGRRR
	    // esta distribucion está completamente inventada
	    // habría que checkear con el autor
	    unsigned char red = (j % 0x07) << 5;
	    unsigned char green = ((j >> 3) & 0x07) << 5; 
	    unsigned char blue = ((j >> 6) & 0x03) << 6; 
	    unsigned char alpha = 255;
	    colors[j] = { red, green, blue, alpha};
    }
}

void draw_something_on_framebuffer(unsigned char* framebuffer) {
    for (int n=0; n<640*480; n++) {
	framebuffer[n] = n % 256;
    }
}

void loop()
{
    SDL_Event e;
    bool quit = false;
    while (SDL_PollEvent(&e)){
	if (e.type == SDL_QUIT){
	    quit = true;
	}
	if (e.type == SDL_KEYDOWN){
	    quit = true;
	}
	if (e.type == SDL_MOUSEBUTTONDOWN){
	    quit = true;
	}
    }
    if (quit) {
	SDL_Quit();
    }
}

int main()
{
// based on sdl2-examples, https://github.com/xyproto/sdl2-examples/tree/main/c%2B%2B11
// Copyright 2023 Alexander F. Rødseth
    using std::cerr;
    using std::endl;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        cerr << "SDL_Init Error: " << SDL_GetError() << endl;
        return EXIT_FAILURE;
    }

    SDL_Window* win = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        return EXIT_FAILURE;
    }

    SDL_Renderer* ren
        = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) {
        cerr << "SDL_CreateRenderer Error" << SDL_GetError() << endl;
		if (win != nullptr) {
			SDL_DestroyWindow(win);
		}
		SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Surface* bmp = SDL_CreateRGBSurface(0,640,480,8,0,0,0,0);
    if (bmp == nullptr) {
        cerr << "SDL_LoadBMP Error: " << SDL_GetError() << endl;
		if (ren != nullptr) {
			SDL_DestroyRenderer(ren);
		}
		if (win != nullptr) {
			SDL_DestroyWindow(win);
		}
		SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Color colors[256];
    init_pdc32_palette(colors);

    SDL_SetPaletteColors(bmp->format->palette, colors, 0, 255);

    unsigned char* framebuffer = (unsigned char*)bmp->pixels;
    draw_something_on_framebuffer(framebuffer);
    
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, bmp);
    if (tex == nullptr) {
        cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << endl;
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
        return EXIT_FAILURE;
    }
    SDL_FreeSurface(bmp);

    for (int i = 0; i < 20; i++) {
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, nullptr, nullptr);
        SDL_RenderPresent(ren);
	loop();
        SDL_Delay(100);
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return EXIT_SUCCESS;
}

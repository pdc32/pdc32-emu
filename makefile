all: msg vga.exe
	g++ -std=c++11 emu.cpp -o emu.exe -Wall -O2

CXX?=c++
SDL2FLAGS=$(shell pkg-config sdl2 --cflags --libs)
CXXFLAGS?=-std=c++11 -Wall -pedantic -Werror -Wshadow -Wstrict-aliasing -Wstrict-overflow

.PHONY: all msg clean fullclean


msg:
	@echo '--- C++11 ---'

vga.exe: vga.cpp
	${CXX} ${CXXFLAGS} -O2 -o $@ $< ${SDL2FLAGS}

small: vga.cpp
	${CXX} ${CXXFLAGS} -Os -o vga.exe $< ${SDL2FLAGS}
	-strip vga.exe
	-sstrip vga.exe

debug: vga.cpp
	${CXX} ${CXXFLAGS} -O0 -g -o vga.exe $< ${SDL2FLAGS}

asm: vga.asm

vga.asm: vga.cpp
	${CXX} ${CFLAGS} -S -masm=intel -Og -o vga.asm $< ${SDL2FLAGS}

run: msg vga
	time ./vga.exe

clean:
	rm -f vga.exe *.o vga.asm

fullclean: clean

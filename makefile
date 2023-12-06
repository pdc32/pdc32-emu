# test name for `make test` (overridable by `make test TESTNAME=spk`)
TESTNAME := vga

CXXFLAGS := -std=c++11 -Wall -O2
SDL2FLAGS=$(shell pkg-config sdl2 --cflags --libs)

BUILD_DIR := build

BIN_EXT := .exe

EMU := $(BUILD_DIR)/emu$(BIN_EXT)
ASM := $(BUILD_DIR)/asm$(BIN_EXT)

TEST_BIN := $(BUILD_DIR)/$(TESTNAME)_test.bin

all: $(EMU) $(ASM)

$(EMU): emu.cpp vga.cpp spk.cpp alu.cpp tmr.cpp pwr.cpp uart.cpp eep.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ ${SDL2FLAGS}

$(ASM): asm.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/%.bin: prg/%.pdc $(ASM)
	cat $< | $(ASM) > $@

test: $(EMU) $(TEST_BIN)
	$(EMU) $(TEST_BIN)

boot: $(EMU)
	$(EMU) firmware/PDC32-skipmemcheck.firmware

memcheck: $(EMU)
	$(EMU) firmware/PDC32.firmware

testall: 
	make test TESTNAME=base
	make test TESTNAME=jump_call
	make test TESTNAME=spk
	make test TESTNAME=vga
	make test TESTNAME=uart # crashes

compile_flags.txt: makefile
	echo '$(CXXFLAGS)' | tr ' ' '\n' > $@

emscripten:
	EM_CACHE=$(PWD)/.emscripten_cache/ emmake make BIN_EXT=".html" CXXFLAGS="-sUSE_SDL=2 -sTOTAL_MEMORY=67108864 --preload-file firmware/ --preload-file font/ --preload-file res/"

server:
	cd build/; python3 -m http.server

clean:
	rm -rf $(BUILD_DIR)/*

# test name for `make test` (overridable by `make test TESTNAME=spk`)
TESTNAME := vga

CXX := g++
CXXFLAGS := -std=c++11 -Wall -O2
SDL2FLAGS=$(shell pkg-config sdl2 --cflags --libs)

BUILD_DIR := build

EMU := $(BUILD_DIR)/emu.exe
ASM := $(BUILD_DIR)/asm.exe

TEST_BIN := $(BUILD_DIR)/$(TESTNAME)_test.bin

all: $(EMU) $(ASM)

$(EMU): emu.cpp vga.cpp spk.cpp alu.cpp tmr.cpp pwr.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ ${SDL2FLAGS}

$(ASM): asm.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/%.bin: prg/%.pdc $(ASM)
	cat $< | $(ASM) > $@

test: $(EMU) $(TEST_BIN)
	$(EMU) $(TEST_BIN)

testall: 
	make test TESTNAME=base
	make test TESTNAME=jump_call
	make test TESTNAME=spk
	make test TESTNAME=vga
	make test TESTNAME=uart # crashes

compile_flags.txt: makefile
	echo '$(CXXFLAGS)' | tr ' ' '\n' > $@

clean:
	rm -rf $(BUILD_DIR)/*

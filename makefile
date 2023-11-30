CXX := g++
CXXFLAGS := -std=c++11 -Wall -O2

TEST_SOURCE := "prg/uart_test.pdc"

all: emu.exe asm.exe

emu.exe: emu.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

asm.exe: asm.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

compile_flags.txt: makefile
	echo '$(CXXFLAGS)' | tr ' ' '\n' > $@

test: asm.exe emu.exe 
	cat $(TEST_SOURCE) | ./asm.exe > test.bin 
	./emu.exe test.bin

clean:
	rm -rf emu.exe asm.exe compile_flags.txt test.bin

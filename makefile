CXX := g++
CXXFLAGS := -std=c++11 -Wall -O2

all: emu.exe asm.exe

emu.exe: emu.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

asm.exe: asm.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -rf emu.exe asm.exe

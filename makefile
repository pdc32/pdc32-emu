CXX := g++
CXXFLAGS := -std=c++11 -Wall -O2

all: emu.exe

emu.exe: emu.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -rf emu.exe

#include <iostream>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

#include "instructions.h"
#include "registers.h"

constexpr uint32_t programLen = 65536; // 64 KiB
uint32_t program[programLen];

uint32_t toInstruction(const string& type, const string& argument) {
    // cerr << "toInstruction: "  << type << " " << argument << endl;

    uint32_t instruction = 0;
    if(type[0] == 'A') {
        instruction = GROUP_A << 16;
    } else if(type[0] == 'B') {
        instruction = GROUP_B << 16;
    } else if(type[0] == 'C') {
        instruction = GROUP_C << 16;
    } else if(type[0] == 'D') {
        instruction = GROUP_D << 16;
    }
    const string num = type.substr(1);
    instruction |= stoi(num) << 16;

    if(!argument.empty()) {
        instruction |= stoi(argument);
    }
    return instruction;
}


uint32_t loadProgramFromStdin(uint16_t startAddr) {
    uint32_t pc = 0;

    string line;
    while(getline(cin, line) && pc <= programLen) {
        if (line.length() == 0) continue; // skip empty lines

        istringstream lineStream(line);
        string instructionType;
        string instructionArgument;
        lineStream >> instructionType >> instructionArgument;
        program[startAddr + pc] = toInstruction(instructionType, instructionArgument);
        pc++;
    }

    return pc;
}

void writeProgramBinaryToStdout(uint16_t baseAddr, uint16_t programLength) {
    for (uint16_t addr = baseAddr; addr < baseAddr + programLength; addr++) {
        uint32_t word = program[addr];

        char b0 = (word >>  0) & 0xFF;
        char b1 = (word >>  8) & 0xFF;
        char b2 = (word >> 16) & 0xFF;

        cout << b0 << b1 << b2;
    }
}

int main() {
    uint32_t pc = loadProgramFromStdin(0);
    writeProgramBinaryToStdout(0, pc);
    return 0;
}

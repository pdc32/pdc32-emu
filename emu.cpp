#include <iostream>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
using namespace std;

constexpr uint32_t programLen = 65536; // 64 KiB
constexpr uint32_t cacheLen = 131072; // 128 KiB
constexpr uint32_t dramLen = 33554432; // 32 MiB

// TODO: add name to more instructions
enum instruction {
    A0_SET_DRAM_DATA = 0x71,
    A1_SET_CARRY_IN = 0x71,
    A2_RETURN = 0x72,
    A3_JLEQ = 0x73,
    A4_JGEQ = 0x74,
    A5_JMP = 0x75,
    A6_INC_DRAM_ADDR = 0x76,
    A7_SET_DRAM_ADDR = 0x77,
    A8_JNE = 0x78,
    A9_SET_A = 0x79,
    A10_SET_HIGH = 0x7A,
    A11_WRITE_DRAM = 0x7B,
    A12_SET_BUS = 0x7C,
    A13_CALL = 0x7D,
    A14_SET_B = 0x7E,
    A15_SET_ALU = 0x7F,
    B0 = 0xB0,
    B1 = 0xB1,
    B2 = 0xB2,
    B3 = 0xB3,
    B4 = 0xB4,
    B5 = 0xB5,
    B6 = 0xB6,
    B7_OUT_DEBUG = 0xB7,
    B8_JL = 0xB8,
    B9 = 0xB9,
    B10_JG = 0xBA,
    B11_JE = 0xBB,
    B12_SET_CACHE_DATA = 0xBC,
    B13_SET_CACHE_ADDR = 0xBD,
    B14 = 0xBE,
    B15_WRITE_CACHE = 0xBF,
    C0 = 0xD0,
    C1 = 0xD1,
    C2 = 0xD2,
    C3 = 0xD3,
    C4 = 0xD4,
    C5 = 0xD5,
    C6 = 0xD6,
    C7 = 0xD7,
    C8 = 0xD8,
    C9 = 0xD9,
    C10 = 0xDA,
    C11 = 0xDB,
    C12 = 0xDC,
    C13 = 0xDD,
    C14 = 0xDE,
    C15 = 0xDF,
};

uint32_t dataLiteral = 0;

uint32_t getLiteral() {
    return dataLiteral;
}

uint32_t cacheAddr = 0;
uint32_t cacheData = 0;
uint32_t cache[cacheLen];

uint32_t getCache() {
    return cache[cacheAddr % cacheLen];
}

uint32_t program[programLen];
uint16_t programCounter = 0;

uint32_t (*bus)() = getLiteral;

bool carryIn = false;
uint32_t a = 0;
uint32_t b = 0;

uint32_t getAdder() {
    const uint32_t cin = carryIn ? 1 : 0;
    return cin + a + b;
}

void setBus(const uint16_t data) {
    // TODO: implement more registers
    if(data == 15) {
        bus = getLiteral;
    } else if(data == 7){
        bus = getCache;
    } else if(data == 13){
        bus = getAdder;
    } else {
        cerr << "MISSING IMPLEMENTATION FOR REGISTER: " << data << endl;
        exit(1);
    }
}

bool aLessThanb() {
    return a < b; // TODO: include extra bits that could override this
}

void handleInstruction(const uint32_t instruction) {
    const uint8_t type = (instruction & 0xFF0000) >> 16;
    const uint16_t data = instruction & 0xFFFF;
    dataLiteral = dataLiteral & 0xFFFF0000 | data;

    // TODO: add more instructions
    if(type == A12_SET_BUS) {
        setBus(data);
    } else if(type == A10_SET_HIGH) {
        dataLiteral = dataLiteral & 0xFFFF | data << 16;
    } else if(type == B13_SET_CACHE_ADDR) {
        cacheAddr = bus();
    } else if(type == B12_SET_CACHE_DATA) {
        cacheData = bus();
    } else if(type == B15_WRITE_CACHE) {
        cache[cacheAddr % cacheLen] = cacheData;
    } else if(type == A1_SET_CARRY_IN) {
        carryIn = bus() & 8; // Bit 3 is used for carry in
    } else if(type == A9_SET_A) {
        a = bus();
    } else if(type == A14_SET_B) {
        b = bus();
    } else if(type == B8_JL) {
        if(aLessThanb()) programCounter = data;
    } else if(type == B7_OUT_DEBUG) {
        cout << "OUT PARALLEL " << bus() << endl;
    } else if(type == A5_JMP) {
        programCounter = data;
    } else {
        cerr << "UNKNOWN TYPE " << hex << (int)type << endl;
        exit(1);
    }
}

uint32_t toInstruction(const string& type, const string& argument) {
    uint32_t instruction = 0xF00000;
    // Set just one low bit of the highest nibble depending on the instruction class
    if(type[0] == 'A') {
        instruction &= ~0x800000;
    } else if(type[0] == 'B') {
        instruction &= ~0x400000;
    } else if(type[0] == 'C') {
        instruction &= ~0x200000;
    } else if(type[0] == 'D') {
        instruction &= ~0x100000;
    }
    const string num = type.substr(1);
    instruction |= stoi(num) << 16;

    if(!argument.empty()) {
        instruction |= stoi(argument);
    }
    return instruction;
}

void loadProgram(const string &str, uint16_t startAddr) {
    istringstream inputStream(str);

    string line;
    while(getline(inputStream, line)) {
        istringstream lineStream(line);
        string instructionType;
        string instructionArgument;
        lineStream >> instructionType >> instructionArgument;
        program[startAddr++] = toInstruction(instructionType, instructionArgument);
    }
}

int main() {
    // Program copied from https://www.youtube.com/watch?v=Wf5YIInCZp8&t=94s
    loadProgram(R"program(A12 15
A10 0
B13 1
B12 0
B15
B13 1
A1 0
A12 7
A9
A12 15
A10 4
A14 0
B8 25
B13 2
A12 7
A9
A12 15
A10 0
A14 1
A12 13
B7
B12
A12 15
B15
A5 0
B13 1
A12 7
A9
A12 15
A10 0
A14 1
A12 13
B12
B15
A12 15
A5 5)program", 0);

    while(true) {
        const uint32_t instruction = program[programCounter++];
        handleInstruction(instruction);
    }
}

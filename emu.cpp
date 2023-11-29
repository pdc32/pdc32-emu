#include <iostream>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
using namespace std;

#include "instructions.h"
#include "registers.h"

constexpr uint32_t programLen = 65536; // 64 KiB
constexpr uint32_t cacheLen = 131072; // 128 KiB
constexpr uint32_t dramLen = 33554432; // 32 MiB

uint32_t dataLiteral = 0;

uint32_t getLiteral() {
    return dataLiteral;
}

uint32_t cacheAddr = 0;
uint32_t cacheData = 0;
uint32_t cache[cacheLen];

uint32_t dramAddr = 0;
uint32_t dramData = 0;
uint32_t dram[dramLen];

uint32_t getCache() {
    return cache[cacheAddr % cacheLen];
}

uint32_t getDramData() {
    return dram[dramAddr % dramLen];
}

uint32_t getDramAddr() {
    return dramAddr;
}

uint32_t program[programLen];
uint16_t programCounter = 0;

uint32_t (*bus)() = getLiteral;

bool carryIn = false;
uint32_t aluFlags = 0;
uint32_t a = 0;
uint32_t b = 0;

uint32_t getAdd() {
    const uint32_t cin = carryIn ? 1 : 0;
    return cin + a + b;
}
uint32_t getShiftLeft() {
    return a << 1;
}
uint32_t getShiftRight() {
    return a >> 1;
}
uint32_t getXor() {
    return a ^ b;
}
uint32_t getOr() {
    return a ^ b;
}
uint32_t getAnd() {
    return a & b;
}

void setBus(const uint8_t reg) {
    switch(reg) {
        case REG_DRIVE_SERIAL:
            // TODO: implement
            break;
        case REG_RTC:
            // TODO: implement
            break;
        case REG_UNUSED:
            bus = nullptr;
            break;
        case REG_KBD:
            // TODO: implement
            break;
        case REG_UART:
            // TODO: implement
            break;
        case REG_DRAM_DATA:
            bus = getDramData;
            break;
        case REG_DRAM_ADDR:
            bus = getDramAddr;
            break;
        case REG_CACHE_DATA:
            bus = getCache;
            break;
        case REG_SHIFT_LEFT_A:
            bus = getShiftLeft;
            break;
        case REG_A_AND_B:
            bus = getAnd;
            break;
        case REG_SHIFT_RIGHT_A:
            bus = getShiftRight;
            break;
        case REG_A_XOR_B:
            bus = getXor;
            break;
        case REG_A_OR_B:
            bus = getOr;
            break;
        case REG_A_PLUS_B:
            bus = getAdd;
            break;
        case REG_STATE:
            // TODO: implement
            break;
        case REG_LITERAL:
            bus = getLiteral;
            break;
        default:
            cerr << "MISSING IMPLEMENTATION FOR REGISTER: " << reg << endl;
            exit(1);
    }
}

bool lessThan() {
    if (a!=b) {
        return a < b;
    }
    bool greaterThan = aluFlags & 1;
    bool equals = aluFlags & 2;
    if(equals) return false;
    if(greaterThan) return false;
    return true;
}

void handleInstruction(const uint32_t instruction) {
    const uint8_t type = (instruction & 0xFF0000) >> 16;
    const uint16_t data = instruction & 0xFFFF;
    dataLiteral = (dataLiteral & 0xFFFF0000) | data;

    // TODO: add more instructions
    if(type == A12_SET_BUS) {
        setBus(data);
    } else if(type == A10_SET_HIGH) {
        dataLiteral = (dataLiteral & 0xFFFF) | data << 16;
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
    } else if(type == A15_SET_ALU) {
        aluFlags = bus();
    } else if(type == B8_JL) {
        if(lessThan()) programCounter = data;
    } else if(type == B7_OUT_DEBUG_COMMAND_RTC) {
        cout << "OUT PARALLEL " << bus() << endl;
    } else if(type == A5_JMP) {
        programCounter = data;
    } else {
        cerr << "UNKNOWN TYPE " << hex << (int)type << endl;
        exit(1);
    }
}

void loadProgram(const char* filename) {
    FILE *fp = fopen(filename, "rb");

    fseek(fp, 0, SEEK_END);
    size_t fileLen = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    for (int i = 0; i < fileLen/3; i++) {
        uint8_t bytes[3];
        fread(bytes, 1, 3, fp);

        uint32_t word = (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
        program[i] = word;
    }
    fclose(fp);
}

int main() {
    loadProgram("program.bin");

    bool quit = false;
    while(!quit) {
        const uint32_t instruction = program[programCounter++];
        handleInstruction(instruction);
    }
}

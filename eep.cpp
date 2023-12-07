#include "eep.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

constexpr uint32_t eep_internal_len = 2048;
uint32_t eep_internal[eep_internal_len];

constexpr uint32_t eep_external_len = 8192;
uint32_t eep_external[eep_external_len];

uint32_t eep_data = 0;
uint32_t eep_addr = 0;
uint32_t eep_read_data = 0;
uint8_t eep_busy = 0;
uint32_t eep_clear = 0;

const string& internal_filename = "build/eeprom_int.bin";
const string& external_filename = "build/eeprom_ext.bin";

uint32_t eep_state() {
    return eep_busy;
}
uint32_t eep_read() {
    return eep_read_data;
}
void eep_c2_serial_data(uint32_t data) {
    eep_data = data;
}
void eep_c3_serial_addr(uint32_t addr) {
    eep_addr = addr;
}
void eep_c4_serial_function(uint32_t function) {
    if(function == eep_read_internal) {
        eep_read_data = eep_internal[eep_addr % eep_internal_len];
    } else if(function == eep_read_external) {
        eep_read_data = eep_external[eep_addr % eep_external_len];
    }else if(function == eep_write_internal) {
        eep_internal[eep_addr % eep_internal_len] = eep_data;
    } else if(function == eep_write_external) {
        eep_external[eep_addr % eep_external_len] = eep_data;
    } else {
        std::cerr << "UNKNOWN EEPROM OPERATION " << function << std::endl;
        return;
    }
    eep_busy = 1;
    eep_clear = SDL_GetTicks()+1;   // 1 ms per operation
}

void eep_process() {
    if(eep_clear && SDL_GetTicks() >= eep_clear) {
        eep_busy = 0;
        eep_clear = 0;
    }
}

void eep_init() {
    // Load internal memory from file
    std::ifstream internal_file(internal_filename, std::ios::binary);
    if (internal_file) {
        internal_file.read(reinterpret_cast<char*>(eep_internal), sizeof(eep_internal));
        internal_file.close();
    } else {
        // File doesn't exist, initialize to 0xFF
        memset(eep_internal, 0xFF, sizeof(eep_internal));
    }

    // Load external memory from file
    std::ifstream external_file(external_filename, std::ios::binary);
    if (external_file) {
        external_file.read(reinterpret_cast<char*>(eep_external), sizeof(eep_external));
        external_file.close();
    } else {
        // File doesn't exist, initialize to 0xFF
        memset(eep_external, 0xFF, sizeof(eep_external));
    }
}
void eep_teardown() {
    // Store internal memory to file
    std::ofstream internal_file(internal_filename, std::ios::binary);
    if (internal_file) {
        internal_file.write(reinterpret_cast<const char*>(eep_internal), sizeof(eep_internal));
        internal_file.close();
    } else {
        std::cerr << "Error: Could not open internal file for writing." << std::endl;
    }

    // Store external memory to file
    std::ofstream external_file(external_filename, std::ios::binary);
    if (external_file) {
        external_file.write(reinterpret_cast<const char*>(eep_external), sizeof(eep_external));
        external_file.close();
    } else {
        std::cerr << "Error: Could not open external file for writing." << std::endl;
    }
}

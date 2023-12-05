#include "eep.h"

constexpr uint32_t eep_internal_len = 2048;
uint32_t eep_internal[eep_internal_len];

constexpr uint32_t eep_external_len = 8192;
uint32_t eep_external[eep_external_len];

uint32_t eep_data = 0;
uint32_t eep_addr = 0;
uint32_t eep_read_data = 0;

uint32_t eep_state() {
    return 0; // Always non-busy, immediate operations
}
uint32_t eep_read() {
    return eep_read_data;
}
void eep_c2_serial_data(uint32_t data) {
    eep_data = data;
}
void eep_c3_serial_addr(uint32_t addr) {
    eep_data = addr;
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
    }
}

void eep_init() {
    // TODO: Load internal and external memories from file
}
void eep_teardown() {
    // TODO: Store internal and external memories into fiile
}

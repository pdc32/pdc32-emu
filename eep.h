#ifndef EEP_H
#define EEP_H

#include <cstdint>

uint32_t eep_state();
uint32_t eep_read();
void eep_c2_serial_data(uint32_t);
void eep_c3_serial_addr(uint32_t);
void eep_c4_serial_function(uint32_t);
void eep_process();
void eep_init();
void eep_teardown();
bool eep_was_active_last_frame();

void eep_download_web();
void eep_upload_web();

constexpr uint32_t eep_state_offset = 14;
constexpr uint32_t eep_read_internal = 1;
constexpr uint32_t eep_write_internal = 3;
constexpr uint32_t eep_read_external = 4;
constexpr uint32_t eep_write_external = 6;

#endif

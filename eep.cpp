#include "eep.h"
#include "emu.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

constexpr uint32_t eep_internal_len = 8192;
uint8_t eep_internal[eep_internal_len];

constexpr uint32_t eep_external_len = 65536;
uint8_t eep_external[eep_external_len];

uint32_t eep_data = 0;
uint32_t eep_addr = 0;
uint32_t eep_read_data = 0;
uint8_t eep_busy = 0;
uint64_t eep_clear_ticks = 0;
bool eep_active_last_frame = false;

const string& internal_filename = "res/eeprom_int.bin";
const string& external_filename = "res/eeprom_ext.bin";
const string& external_filename_web = "/work/eeprom_ext.bin";

bool eep_was_active_last_frame() {
    bool ret = eep_active_last_frame;
    eep_active_last_frame = false;
    return ret;
}
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
uint32_t read_word(uint32_t addr, bool internal) {
    uint32_t eep_len = internal ? eep_internal_len : eep_external_len;
    if(addr >= eep_len) return 0xFFFFFFFF;
    uint32_t addr0 = (addr + 0) % eep_len;
    uint32_t addr1 = (addr + 1) % eep_len;
    uint32_t addr2 = (addr + 2) % eep_len;
    uint32_t addr3 = (addr + 3) % eep_len;

    if(internal) {
        return eep_internal[addr0] | eep_internal[addr1] << 8 | eep_internal[addr2] << 16 | eep_internal[addr3] << 24;
    }
    return eep_external[addr0] | eep_external[addr1] << 8 | eep_external[addr2] << 16 | eep_external[addr3] << 24;
}

void write_word(uint32_t addr, uint32_t data, bool internal) {
    uint32_t eep_len = internal ? eep_internal_len : eep_external_len;
    if(addr >= eep_len) return;

    uint32_t addr0 = (addr + 0) % eep_len;
    uint32_t addr1 = (addr + 1) % eep_len;
    uint32_t addr2 = (addr + 2) % eep_len;
    uint32_t addr3 = (addr + 3) % eep_len;
    uint32_t data0 = data & 0xFF;
    uint32_t data1 = (data >> 8) & 0xFF;
    uint32_t data2 = (data >> 16) & 0xFF;
    uint32_t data3 = (data >> 24) & 0xFF;

    if(internal) {
        eep_internal[addr0] = data0;
        eep_internal[addr1] = data1;
        eep_internal[addr2] = data2;
        eep_internal[addr3] = data3;
    } else {
        eep_external[addr0] = data0;
        eep_external[addr1] = data1;
        eep_external[addr2] = data2;
        eep_external[addr3] = data3;
    }
}

void eep_c4_serial_function(uint32_t function) {
    if(function == eep_read_internal) {
        eep_read_data = read_word(eep_addr, true);
    } else if(function == eep_read_external) {
        eep_read_data = read_word(eep_addr, false);
    }else if(function == eep_write_internal) {
        write_word(eep_addr, eep_data, true);
    } else if(function == eep_write_external) {
        write_word(eep_addr, eep_data, false);
    } else {
        std::cerr << "UNKNOWN EEPROM OPERATION " << function << std::endl;
        return;
    }
    eep_active_last_frame = true;
    eep_busy = 1;
    eep_clear_ticks = get_tick_count() + instructions_per_second / 40000;   // 0.1 ms per operation (~10 per ms)
}

void eep_process() {
    if(eep_clear_ticks && get_tick_count() >= eep_clear_ticks) {
        eep_busy = 0;
        eep_clear_ticks = 0;
    }
}

void eep_init() {

#ifdef __EMSCRIPTEN__
    EM_ASM(
        FS.mkdir('/work');
        FS.mount(IDBFS, {}, '/work');

        FS.syncfs(true, function (err) {
            Module._eep_load_continue();
        });
    );
#endif

    // Load internal memory from file
    std::ifstream internal_file(internal_filename, std::ios::binary);
    if (internal_file) {
        internal_file.read(reinterpret_cast<char*>(eep_internal), sizeof(eep_internal));
        internal_file.close();
    } else {
        // File doesn't exist, initialize to 0xFF
        memset(eep_internal, 0xFF, sizeof(eep_internal));
    }

#ifndef __EMSCRIPTEN__
    // Load external memory from file
    std::ifstream external_file(external_filename, std::ios::binary);
    if (external_file) {
        external_file.read(reinterpret_cast<char*>(eep_external), sizeof(eep_external));
        external_file.close();
    } else {
        // File doesn't exist, initialize to 0xFF
        memset(eep_external, 0xFF, sizeof(eep_external));
    }
#endif
}

#ifdef __EMSCRIPTEN__
extern "C" {

    void EMSCRIPTEN_KEEPALIVE eep_load_continue() {
        std::ifstream external_file_web(external_filename_web, std::ios::binary);
        if (external_file_web) {
            cout << "EEP: Loaded external memory from IDBFS" << endl;
            external_file_web.read(reinterpret_cast<char*>(eep_external), sizeof(eep_external));
            external_file_web.close();
        } else {
            std::ifstream external_file(external_filename, std::ios::binary);
            if (external_file) {
                cout << "EEP: Loaded default external memory" << endl;
                external_file.read(reinterpret_cast<char*>(eep_external), sizeof(eep_external));
                external_file.close();
            } else {
                // File doesn't exist, initialize to 0xFF
                cout << "EEP: Initialized external memory with 0xFFs" << endl;
                memset(eep_external, 0xFF, sizeof(eep_external));
            }
        }
    }

    void EMSCRIPTEN_KEEPALIVE eep_teardown_web(){
        // Store external memory to web file
        std::ofstream external_file(external_filename_web, std::ios::binary);
        if (external_file) {
            external_file.write(reinterpret_cast<const char*>(eep_external), sizeof(eep_external));
            external_file.close();
        } else {
            std::cerr << "Error: Could not open external file for writing." << std::endl;
        }

        EM_ASM(
            FS.syncfs(function (err) {
            });
        );
    }
}
#endif

void eep_teardown() {
#ifndef __EMSCRIPTEN__
    const SDL_MessageBoxButtonData buttons[] = {
            { 0, 0, "Yes" },
            { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "No" },
    };
    const SDL_MessageBoxData messageboxdata = {SDL_MESSAGEBOX_INFORMATION,
                                        NULL,
                                        "Save memory",
                                        "Do you want to save the contents of the memory?",
                                        SDL_arraysize(buttons),
                                        buttons,
                                        NULL};
    int button_id;
    SDL_ShowMessageBox(&messageboxdata, &button_id);
    if (button_id == 0) {
        return;
    }

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
#endif
}

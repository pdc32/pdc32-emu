#include "pwr.h"
#include "emu.h"
#include <iostream>

bool pwr_button_down = false;
bool pwr_on = true;

void pwr_B14_set_power_on(bool val){
    std::cout << "Power status changed: " << (val ? "ON" : "OFF") << std::endl;
    if(!val) {
        pwr_on = false;
    }
}

void pwr_button_press(bool val){
    std::cout << "Power button status: " << (val ? "PRESSED" : "RELEASED") << std::endl;
    pwr_button_down = val;
    if(val && !pwr_on) {
        pwr_on = true;
        emu_reset();
    }
}

uint32_t pwr_get_state(){
    return !pwr_button_down;
}

bool pwr_is_on() {
    return pwr_on;
}

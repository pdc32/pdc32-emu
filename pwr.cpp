#include "pwr.h"
#include <iostream>

bool pwr_button_down = false;

void pwr_B14_set_power_on(bool val){
    std::cout << "Power status changed: " << (val ? "ON" : "OFF") << std::endl;
}

void pwr_button_press(bool val){
    std::cout << "Power button status: " << (val ? "PRESSED" : "RELEASED") << std::endl;
    pwr_button_down = val;
}

uint32_t pwr_get_state(){
    return !pwr_button_down;
}

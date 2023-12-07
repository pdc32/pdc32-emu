#include "tmr.h"
#include <cstdio>

uint32_t timers[8] = {0};
uint32_t selected_timers = 0;
uint32_t value = 0;

void tmr_C0_select_timer(uint8_t selected){
    //printf("SEL TIMERS: %x", selected);
    for(int i=0;i<8;i++){
        if(selected & (1<<i)) {
            timers[i] = value;
        }
    }
    selected_timers = selected;
}
void tmr_C1_set_time(uint32_t time){
    //printf("SET TIME %x\n", time);
    value = time;
}
void tmr_process(){
    for(int i=0;i<8;i++){
        if(timers[i] != 0xFFFFFF)
            timers[i] ++;
    }
}
uint8_t tmr_busy(){
    bool busy = false;
    for(int i=0;i<8;i++){
        if(selected_timers & (1<<i)) {
            if(timers[i] != 0xFFFFFF) {
                busy = true;
            }
        }
    }
    return busy;
}
uint8_t tmr_ovf(){
    uint8_t ovf = 0;
    for(int i=0;i<8;i++){
        if(timers[i] == 0xFFFFFF)
            ovf |= (1<<i);
    }
    return ovf;
}

#include "tmr.h"
#include <cstdio>

uint32_t timers[8] = {0};
uint8_t selected = 0xFF;

void tmr_C0_select_timer(uint8_t timer){
    selected = timer;
}
void tmr_C1_set_time(uint32_t time){
    //printf("SET TIME %x\n", time);
    for(int i=0;i<8;i++){
        if(selected & (1<<i))
            timers[i] = time;
    }
}
void tmr_process(){
    //printf("SEL TIMERS: ");
    for(int i=0;i<8;i++){
        if(selected & (1<<i)){
//            printf("%x ", timers[i]);
            if(timers[i] != 0xFFFFFF)
                timers[i] ++;
        }
    }
    //printf("\n");
}
uint8_t tmr_busy(){
    bool busy = false;
    for(int i=0;i<8;i++){
        if(selected & (1<<i)){
            if(timers[i] != 0xFFFFFF)
                busy = true;
        }
    }
    return busy;
}
uint8_t tmr_ovf(){
    uint8_t ovf = 0;
    for(int i=0;i<8;i++){
        if(selected & (1<<i)){
            if(timers[i] == 0xFFFFFF)
                ovf |= (1<<i);
        }
    }
    return ovf;
}

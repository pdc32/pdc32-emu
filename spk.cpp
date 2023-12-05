#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#include "spk.h"
using namespace std;

#define M_PI           3.14159265358979323846
constexpr int amplitude = 16384;   // not full amplitude
int frequency = 440;
double phase = 0.0;
SDL_AudioSpec audio_spec;

uint32_t spk_overflow = 0;
int32_t spk_value = 0;
bool spk_on;
bool spk_mute = true;
bool spk_state;

void audioCallback(void* userdata, Uint8* stream, int bufferLength) {
    Uint16* buffer = (Uint16*)stream;

    if (spk_mute){
        for (int i = 0; i < bufferLength/2; ++i) {
            buffer[i] = 0;
        }
        return;
    }

    for (int i = 0; i < bufferLength/2; ++i) {
        double waveSample = sin(2.0 * M_PI * phase);
        buffer[i] = static_cast<Sint16>(amplitude * waveSample);
        phase += frequency * (1.0 / audio_spec.freq);
    }
}

void spk_b0_timer_ovf(uint32_t bus){
    bus &= 0xFFFFFF;

    float freq = 8e6 / bus;
    cout << "SPK frequency: " << freq << " Hz" << endl;
    frequency = freq;
    spk_overflow = bus;
}
void spk_b9_timer_config(bool on, bool mute){
    cout << "SPK config: " << (on ? "ON":"OFF") << ", " << (mute ? "TMR":"SPK") << endl;
    spk_on = on;
    spk_mute = mute;
    if(!spk_on) spk_state = false;
}
void spk_process(){
    // Timer gets decremented 8 times per instruction
    spk_value -= 8;
    if(spk_value < 0){
        spk_value = spk_overflow;

        // Realistic logic that somehow isn't working
        /*if(spk_on) {
            if(spk_mute) {
                if(!spk_state) spk_state = false;
            } else {
                spk_state = !spk_state;
            }
        }*/
        if(spk_on) spk_state = !spk_state;
    }
}
bool spk_ovf() {
    return spk_state && spk_mute;
}

int spk_init(){
    SDL_AudioSpec desired;
    desired.freq = 44100;
    desired.format = AUDIO_S16SYS;
    desired.channels = 1;
    desired.samples = 1024;
    desired.callback = audioCallback;

    SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, &audio_spec, 0);
    if (audioDevice == 0) {
        fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_PauseAudioDevice(audioDevice, 0);
    return 0;
}

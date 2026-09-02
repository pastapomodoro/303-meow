#pragma once
#include "../DSP/Saturation.h"

class VCA
{
public:
    VCA();
    void setVolume(float v);   // 0.0 - 1.0
    float processSample(float input, float noteEnv, float accentEnv, float accentLevel);
    void reset();

private:
    float volume = 0.8f;

    // La saturazione del VCA non e' un effetto opzionale: e' sempre in catena,
    // quindi e' anche la sorgente di aliasing piu' costante di tutto il synth.
    AdaaTanh sat;
};

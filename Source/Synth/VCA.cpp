#include "VCA.h"
#include <cmath>
#include <algorithm>

VCA::VCA() {}

void VCA::setVolume(float v)
{
    volume = std::max(0.0f, std::min(1.0f, v));
}

float VCA::processSample(float input, float noteEnv, float accentEnv)
{
    // Accent adds up to +6dB (factor 2.0) on top of note envelope
    float amp = noteEnv + accentEnv * 1.0f; // accent boosts amplitude
    amp = std::min(amp, 2.0f);

    float out = input * amp * volume;

    // Soft tanh clipping for subtle saturation
    out = std::tanh(out);

    return out;
}

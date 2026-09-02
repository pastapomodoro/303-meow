#include "VCA.h"
#include <cmath>
#include <algorithm>

VCA::VCA() {}

void VCA::setVolume(float v)
{
    volume = std::max(0.0f, std::min(1.0f, v));
}

void VCA::reset()
{
    sat.reset();
}

float VCA::processSample(float input, float noteEnv, float accentEnv, float accentLevel)
{
    float amp = noteEnv + accentEnv * accentLevel;
    amp = std::min(amp, 2.0f);

    // Con amp fino a 2.0 il tanh lavora ben oltre la zona lineare proprio sulle
    // note accentate — cioe' dove il 303 deve mordere. Passare per l'ADAA
    // lascia intatta la curva ma toglie il ripiegamento in banda.
    const double driven = static_cast<double>(input) * amp * volume;
    return static_cast<float>(sat.process(driven));
}

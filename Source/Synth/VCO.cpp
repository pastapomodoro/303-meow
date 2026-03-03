#include "VCO.h"
#include <cmath>

VCO::VCO() {}

void VCO::prepare(double sr)
{
    sampleRate = sr;
    phase = 0.0;
    // Slide time ~70ms => coefficient per sample
    double slideTimeSec = 0.070;
    int    slideSamples = static_cast<int>(slideTimeSec * sampleRate);
    slideCoeff = std::pow(0.001, 1.0 / slideSamples); // approach within 60dB
}

void VCO::reset()
{
    phase = 0.0;
    sliding = false;
}

double VCO::noteToHz(int note, float tuning)
{
    return 440.0 * std::pow(2.0, (note - 69 + tuning / 100.0) / 12.0);
}

void VCO::setNote(int midiNote, float tuning)
{
    tuningCents = tuning;
    targetFreq  = noteToHz(midiNote, tuning);
    if (!sliding)
        currentFreq = targetFreq;
}

void VCO::setSlide(bool enable, int fromNote, int toNote)
{
    sliding     = enable;
    currentFreq = noteToHz(fromNote, tuningCents);
    targetFreq  = noteToHz(toNote,   tuningCents);
}

void VCO::setWaveform(int wf)
{
    waveform = wf;
}

// PolyBLEP correction at discontinuities
double VCO::polyBlep(double t, double dt)
{
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0;
    }
    else if (t > 1.0 - dt)
    {
        t = (t - 1.0) / dt;
        return t * t + t + t + 1.0;
    }
    return 0.0;
}

double VCO::processSaw()
{
    double dt  = phaseInc;
    double out = 2.0 * phase - 1.0;         // naive sawtooth
    out -= polyBlep(phase, dt);             // correct discontinuity at 0
    return out;
}

double VCO::processSquare()
{
    double dt  = phaseInc;
    double out = (phase < 0.5) ? 1.0 : -1.0;
    out += polyBlep(phase, dt);              // rising edge at 0
    out -= polyBlep(std::fmod(phase + 0.5, 1.0), dt); // falling edge at 0.5
    return out;
}

float VCO::processSample()
{
    // Slide: exponential interpolation
    if (sliding)
    {
        double diff = targetFreq - currentFreq;
        if (std::fabs(diff) < 0.01)
        {
            currentFreq = targetFreq;
            sliding = false;
        }
        else
        {
            // Per-sample exponential approach
            currentFreq += (targetFreq - currentFreq) * (1.0 - slideCoeff);
        }
    }

    phaseInc = currentFreq / sampleRate;

    double out = 0.0;
    if (waveform == 0)
        out = processSaw();
    else
        out = processSquare();

    phase += phaseInc;
    if (phase >= 1.0) phase -= 1.0;

    return static_cast<float>(out);
}

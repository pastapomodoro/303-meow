#include "Envelope.h"
#include <cmath>
#include <algorithm>

Envelope::Envelope(Type t) : type(t) {}

double Envelope::computeCoeff(double timeSec, double sr)
{
    // RC-style exponential: reaches ~99.3% in timeSec seconds
    return std::exp(-1.0 / (timeSec * sr));
}

void Envelope::prepare(double sr)
{
    sampleRate  = sr;
    attackCoeff = computeCoeff(0.003, sr);  // 3ms attack

    if (type == Type::Note)
        decayCoeff = computeCoeff(0.300, sr);   // default 300ms, overridden by setDecay
    else
        decayCoeff = computeCoeff(0.200, sr);   // accent: fixed 200ms

    state = State::Idle;
    value = 0.0;
}

void Envelope::setDecay(float decaySec)
{
    // Only meaningful for Note envelope
    decayCoeff = computeCoeff(static_cast<double>(decaySec), sampleRate);
}

void Envelope::trigger()
{
    state = State::Attack;
    // Don't reset value to 0 — allow retriggering from current level
}

void Envelope::reset()
{
    state = State::Idle;
    value = 0.0;
}

float Envelope::processSample()
{
    switch (state)
    {
        case State::Idle:
            value = 0.0;
            break;

        case State::Attack:
        {
            // Exponential attack toward 1.0
            value = 1.0 - (1.0 - value) * attackCoeff;
            if (value >= 0.999)
            {
                value = 1.0;
                state = State::Decay;
            }
            break;
        }

        case State::Decay:
            // RC-style exponential decay toward 0
            value *= decayCoeff;
            if (value < 1e-6)
            {
                value = 0.0;
                state = State::Idle;
            }
            break;
    }

    return static_cast<float>(value);
}

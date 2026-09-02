#pragma once
#include "../DSP/Saturation.h"
#include <vector>
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
//  Delay
//
//  La versione precedente leggeva a campione intero e prendeva il tempo dal
//  parametro senza filtrarlo. Due conseguenze udibili:
//
//   - muovendo il knob Time la testina saltava di campione in campione, con
//     un click a ogni scatto invece della pitch-shift continua che ci si
//     aspetta da un delay;
//   - il feedback rimandava in circolo il segnale tale e quale, quindi ogni
//     ripetizione conservava tutta la banda alta. Con feedback alto le code si
//     accumulavano restando brillanti, ed e' li' che il suono diventava
//     davvero fastidioso.
//
//  Qui la lettura e' interpolata, il tempo e' filtrato, e nel ramo di feedback
//  c'e' un passa-basso: ogni ripetizione perde un po' di alto, come in un BBD
//  o su nastro. E' quello che rende le code lunghe ascoltabili.
// ─────────────────────────────────────────────────────────────────────────────
class Delay
{
public:
    void prepare(double sampleRate, int maxDelayMs = 1000)
    {
        sr = sampleRate;
        size_t bufSize = static_cast<size_t>(sr * maxDelayMs / 1000.0) + 4;
        buffer.assign(bufSize, 0.0f);
        writePos = 0;

        // ~40 ms di costante di tempo: il knob resta reattivo ma la testina
        // non fa mai un salto secco.
        timeCoeff     = 1.0 - std::exp(-1.0 / (0.040 * sr));
        smoothedTime  = timeSeconds;

        damper.prepare(sr, 4500.0);
        fbSat.reset();
    }

    void reset()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writePos     = 0;
        smoothedTime = timeSeconds;
        damper.reset();
        fbSat.reset();
    }

    void setTime(float seconds)    { timeSeconds = std::max(0.001f, seconds); }
    void setFeedback(float fb)     { feedback = std::min(0.95f, std::max(0.0f, fb)); }
    void setMix(float m)           { mix = std::max(0.0f, std::min(1.0f, m)); }

    // Quanto scuriscono le ripetizioni. Piu' basso = coda piu' corta all'orecchio.
    void setDamping(float cutoffHz) { damper.setCutoff(cutoffHz); }

    float processSample(float input)
    {
        if (buffer.size() < 4) return input;

        smoothedTime += timeCoeff * (timeSeconds - smoothedTime);

        const double maxDelay = static_cast<double>(buffer.size() - 3);
        double delaySamples   = smoothedTime * sr;
        delaySamples = std::min(std::max(delaySamples, 1.0), maxDelay);

        // Lettura frazionaria: la parte decimale viene interpolata invece di
        // essere troncata, cosi' spostare il tempo scivola invece di scattare.
        const double readPosF = static_cast<double>(writePos)
                              + static_cast<double>(buffer.size())
                              - delaySamples;
        const size_t i0   = static_cast<size_t>(readPosF) % buffer.size();
        const size_t i1   = (i0 + 1) % buffer.size();
        const float  frac = static_cast<float>(readPosF - std::floor(readPosF));

        const float delayed = buffer[i0] + frac * (buffer[i1] - buffer[i0]);

        // Ramo di feedback: prima si smorza, poi si satura dolcemente. La
        // saturazione qui non e' colore, e' la rete di sicurezza che impedisce
        // al loop di divergere se il feedback sta a 0.95.
        const float damped = damper.process(delayed);
        const float fbSig  = static_cast<float>(fbSat.process(
                                 static_cast<double>(damped) * feedback));

        buffer[writePos] = input + fbSig;
        writePos = (writePos + 1) % buffer.size();

        // Parallel wet/dry mix
        return input * (1.0f - mix) + delayed * mix;
    }

private:
    std::vector<float> buffer;
    size_t   writePos     = 0;
    double   sr           = 44100.0;
    float    timeSeconds  = 0.375f;
    float    feedback     = 0.4f;
    float    mix          = 0.0f;

    double   smoothedTime = 0.375;
    double   timeCoeff    = 1.0;

    OnePoleLP damper;
    AdaaTanh  fbSat;
};

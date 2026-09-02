#include "VCF.h"
#include <cmath>

VCF::VCF() {}

void VCF::prepare(double sr, int /*blockSize*/)
{
    sampleRate = sr;
    reset();
}

void VCF::reset()
{
    for (int i = 0; i < 4; ++i)
    {
        stage[i]     = 0.0;
        stageTanh[i] = 0.0;
    }
    prevInput = 0.0;
    dcX = dcY = 0.0;
}

void VCF::setCutoff(float hz)   { baseCutoff = hz; }
void VCF::setResonance(float r) { resonance  = r;  }
void VCF::setEnvMod(float a)    { envMod     = a;  }
void VCF::setAccentMod(float a) { accentMod  = a;  }

// Pre-warped bilinear coefficient for the Huovilainen model.
// We run the filter at 2x the host sample rate, so we use sampleRate*2.
double VCF::computeCutoffG(double cutoffHz) const
{
    double sr2x = sampleRate * 2.0;
    double f    = juce::jlimit(20.0, sr2x * 0.49, cutoffHz);
    double g    = std::tan(juce::MathConstants<double>::pi * f / sr2x);
    return g / (1.0 + g);
}

// One 4-pole ladder step — called twice per external sample (2x oversampling).
void VCF::filterStep(double input, double g, double res4)
{
    double fb = std::tanh((input - res4 * stage[3]) * 0.5);

    for (int i = 0; i < 4; ++i)
    {
        double stageIn  = (i == 0) ? fb : stageTanh[i - 1];
        double v        = g * (stageIn - stageTanh[i]);
        double newStage = v + stage[i];
        stage[i]        = newStage + v;           // trapezoidal integrator
        stageTanh[i]    = std::tanh(stage[i] * 0.5);
    }
}

float VCF::dcBlock(float input)
{
    double xn = static_cast<double>(input);
    double yn = xn - dcX + 0.9995 * dcY;
    dcX = xn;
    dcY = yn;
    return static_cast<float>(yn);
}

float VCF::processSample(float input, float envInput, float accentInput)
{
    // ── Modulated cutoff (log domain) ─────────────────────────────────────
    double logCutoff  = std::log2(static_cast<double>(
                          juce::jlimit(20.0f, 18000.0f, baseCutoff)));
    double envOctaves = static_cast<double>(envMod)    * 3.0
                      * static_cast<double>(envInput);
    double accOctaves = static_cast<double>(accentMod) * 1.5
                      * static_cast<double>(accentInput);

    double modCutoff = std::pow(2.0, logCutoff + envOctaves + accOctaves);
    modCutoff        = juce::jlimit(20.0, sampleRate * 0.45, modCutoff);

    // Accent subtly boosts resonance
    double res = static_cast<double>(resonance)
               + static_cast<double>(accentInput)
               * static_cast<double>(accentMod) * 0.15;
    res = juce::jlimit(0.0, 0.99, res);

    // ── 2× oversampling via linear interpolation ──────────────────────────
    double g    = computeCutoffG(modCutoff);
    double res4 = res * 4.0;

    // Compensazione risonanza: ad alta res il feedback aspira energia dal
    // fondamentale. +0.5 per unita' di res compensa il calo di livello.
    double inputGain = 1.0 + res * 0.5;
    double inputF    = static_cast<double>(input) * inputGain;

    // First pass: use midpoint between previous and current sample
    double mid = 0.5 * (prevInput + inputF);
    filterStep(mid, g, res4);

    // Second pass: actual sample
    filterStep(inputF, g, res4);
    prevInput = inputF;

    return dcBlock(static_cast<float>(stageTanh[3] * 2.0));
}

#pragma once
#include <JuceHeader.h>

class VCF
{
public:
    VCF();

    void prepare(double sampleRate, int blockSize);
    void setCutoff(float hz);
    void setResonance(float r);      // 0.0 - 1.0
    void setEnvMod(float amount);    // 0.0 - 1.0
    void setAccentMod(float amount); // 0.0 - 1.0

    // Process one sample; envInput and accentInput are 0..1 envelope outputs
    float processSample(float input, float envInput, float accentInput);

    void reset();

private:
    double sampleRate = 44100.0;

    float baseCutoff = 800.0f;
    float resonance  = 0.5f;
    float envMod     = 0.5f;
    float accentMod  = 0.5f;

    // Huovilainen diode ladder state (4 stages)
    double stage[4]     = {0.0, 0.0, 0.0, 0.0};
    double stageTanh[4] = {0.0, 0.0, 0.0, 0.0};
    double prevInput    = 0.0;

    // DC blocker state
    double dcX = 0.0, dcY = 0.0;

    // Internal 2x oversampling helpers
    // g = prewarped cutoff coefficient at 2× sampleRate
    double computeCutoffG(double cutoffHz) const;
    void   filterStep(double input, double g, double res4);
    float  dcBlock(float input);
};

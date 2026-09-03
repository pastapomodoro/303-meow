#pragma once
#include <JuceHeader.h>

class VCO
{
public:
    VCO();

    void prepare(double sampleRate);
    void setNote(int midiNote, float tuningCents = 0.0f);
    void setWaveform(int waveform);   // 0=saw, 1=square
    void setSlide(bool enable, int fromNote, int toNote);

    // Sub-oscillatore un'ottava sotto. In Acid V e' attivo in 72 preset su
    // 156, e chi lo usa lo tiene forte (volume mediano 0.84): e' da li' che
    // arriva il peso in basso che un solo VCO non da'.
    void setSubVolume(float v);       // 0..1
    void reset();

    float processSample();

private:
    double sampleRate = 44100.0;
    double phase = 0.0;
    double phaseInc = 0.0;

    double subPhase = 0.0;
    float  subVol   = 0.0f;

    // Portamento
    double currentFreq = 440.0;
    double targetFreq  = 440.0;
    double slideCoeff  = 1.0;   // per-sample multiplier toward targetFreq
    bool   sliding     = false;

    int    waveform    = 0;      // 0=saw, 1=square
    float  tuningCents = 0.0f;

    static double noteToHz(int note, float tuningCents = 0.0f);

    // PolyBLEP helpers
    static double polyBlep(double t, double dt);
    double processSaw();
    double processSquare();
    double processSub();
};

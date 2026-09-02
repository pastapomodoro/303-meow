#pragma once
#include "VCO.h"
#include "VCF.h"
#include "VCA.h"
#include "Envelope.h"
#include "../DSP/Saturation.h"

class TB303Engine
{
public:
    TB303Engine();

    void prepare(double sampleRate, int blockSize);
    void reset();

    // Parameter setters (called from audio thread via APVTS)
    void setCutoff(float hz);
    void setResonance(float r);
    void setEnvMod(float amount);
    void setDecay(float seconds);
    void setAccentLevel(float level);
    void setVolume(float v);
    void setWaveform(int wf);
    void setTuning(float cents);
    void setDistortion(float d);  // 0.0 - 1.0

    // Events (called from sequencer, on audio thread)
    void noteOn(int midiNote, bool accent, bool slide, int previousNote);
    void noteOff();

    // Render one sample of mono audio
    float processSample();

private:
    VCO      vco;
    VCF      vcf;
    VCA      vca;
    Envelope noteEnv  { Envelope::Type::Note   };
    Envelope accentEnv{ Envelope::Type::Accent  };

    float tuningCents  = 0.0f;
    float accentLevel  = 0.7f;
    float distortion   = 0.0f;

    int  lastNote      = 36;
    bool noteActive    = false;

    // Post-VCA tone shaper: taglia la cresta a 13kHz che il filter drive
    // lascia comunque, senza toccare la banda utile del 303.
    OnePoleLP toneShaper;
};

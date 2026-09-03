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
    void setSubVolume(float v);    // 0.0 - 1.0, ottava sotto

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

    // ── Bass boost ───────────────────────────────────────────────────────
    // Filter303_BassBoost e' attivo in 149 dei 156 preset factory di Acid V,
    // mediana 0.50: praticamente sempre. E' la compensazione del basso che il
    // ladder porta via salendo di risonanza, e senza di essa la linea resta
    // magra qualunque cosa si faccia col cutoff. Shelf a un polo:
    //     y = x + boost * lowpass(x)
    double bassLp    = 0.0;
    double bassCoeff = 0.0;
    static constexpr float kBassBoost = 0.5f;
};

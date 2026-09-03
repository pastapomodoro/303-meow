#include "TB303Engine.h"
#include <cmath>
#include <algorithm>

TB303Engine::TB303Engine() {}

void TB303Engine::prepare(double sampleRate, int blockSize)
{
    vco.prepare(sampleRate);
    vcf.prepare(sampleRate, blockSize);
    vca.setVolume(0.8f);

    noteEnv.prepare(sampleRate);
    accentEnv.prepare(sampleRate);

    // 13 kHz: sopra la banda utile del 303 (il VCF arriva a 20k ma il timbro
    // vive sotto i 5) e sotto la zona in cui il waveshaper concentra la sua
    // sporcizia residua. Abbastanza alto da non sentirsi come un filtro.
    toneShaper.prepare(sampleRate, 13000.0);

    // 180 Hz: sotto la fondamentale delle note piu' basse del 303, quindi
    // aggiunge peso senza impastare la zona in cui vive il timbro.
    bassCoeff = 1.0 - std::exp(-2.0 * juce::MathConstants<double>::pi
                               * 180.0 / sampleRate);
    bassLp    = 0.0;

    noteEnv.setDecay(0.3f);
    reset();
}

void TB303Engine::reset()
{
    vco.reset();
    vcf.reset();
    noteEnv.reset();
    accentEnv.reset();
    vca.reset();
    toneShaper.reset();
    bassLp      = 0.0;
    noteActive  = false;
}

void TB303Engine::setCutoff(float hz)       { vcf.setCutoff(hz);       }
void TB303Engine::setResonance(float r)     { vcf.setResonance(r);     }
void TB303Engine::setEnvMod(float amount)   { vcf.setEnvMod(amount);   }
void TB303Engine::setDecay(float seconds)   { noteEnv.setDecay(seconds);}
void TB303Engine::setVolume(float v)        { vca.setVolume(v);        }
void TB303Engine::setWaveform(int wf)       { vco.setWaveform(wf);     }
void TB303Engine::setTuning(float cents)    { tuningCents = cents;     }
void TB303Engine::setDistortion(float d)    { distortion  = d;         }
void TB303Engine::setSubVolume(float v)     { vco.setSubVolume(v);     }

void TB303Engine::setAccentLevel(float level)
{
    accentLevel = level;
    vcf.setAccentMod(level);
}

void TB303Engine::noteOn(int midiNote, bool accent, bool slide, int previousNote)
{
    if (slide)
        vco.setSlide(true, previousNote, midiNote);
    else
    {
        vco.setSlide(false, midiNote, midiNote);
        vco.setNote(midiNote, tuningCents);
    }

    if (!slide)
    {
        noteEnv.trigger();
        if (accent)
            accentEnv.trigger();
    }

    lastNote   = midiNote;
    noteActive = true;
}

void TB303Engine::noteOff()
{
    // TB-303 uses gate-length-independent envelopes — no action needed here.
    // The decay envelope runs freely regardless of gate.
    noteActive = false;
}

float TB303Engine::processSample()
{
    float envOut    = noteEnv.processSample();
    float accentOut = accentEnv.processSample();

    float vcoOut  = vco.processSample();
    float driven  = vcoOut * (1.0f + distortion * 2.0f);   // drive VCF input
    float vcfOut  = vcf.processSample(driven, envOut, accentOut);

    // Bass boost prima del VCA: e' compensazione del filtro, non un effetto
    // in coda, quindi deve passare per l'inviluppo d'ampiezza come il resto.
    bassLp += bassCoeff * (static_cast<double>(vcfOut) - bassLp);
    const float boosted = vcfOut + kBassBoost * static_cast<float>(bassLp);

    float vcaOut  = vca.processSample(boosted, envOut, accentOut, accentLevel);
    float out     = toneShaper.process(vcaOut);

    return out;
}

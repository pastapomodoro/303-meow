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

    noteEnv.setDecay(0.3f);
    reset();
}

void TB303Engine::reset()
{
    vco.reset();
    vcf.reset();
    noteEnv.reset();
    accentEnv.reset();
    noteActive  = false;
    distDcX = distDcY = 0.0;
}

void TB303Engine::setCutoff(float hz)       { vcf.setCutoff(hz);       }
void TB303Engine::setResonance(float r)     { vcf.setResonance(r);     }
void TB303Engine::setEnvMod(float amount)   { vcf.setEnvMod(amount);   }
void TB303Engine::setDecay(float seconds)   { noteEnv.setDecay(seconds);}
void TB303Engine::setVolume(float v)        { vca.setVolume(v);        }
void TB303Engine::setWaveform(int wf)       { vco.setWaveform(wf);     }
void TB303Engine::setTuning(float cents)    { tuningCents = cents;     }
void TB303Engine::setDistortion(float d)    { distortion  = d;         }

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

    noteEnv.trigger();

    if (accent)
        accentEnv.trigger();

    lastNote   = midiNote;
    noteActive = true;
}

void TB303Engine::noteOff()
{
    // TB-303 uses gate-length-independent envelopes — no action needed here.
    // The decay envelope runs freely regardless of gate.
    noteActive = false;
}

float TB303Engine::applyDistortion(float input) const
{
    if (distortion < 1e-4f)
        return input;
    float drive = 1.0f + distortion * 9.0f;
    float tanhDrive = std::tanh(drive);
    return std::tanh(input * drive) / tanhDrive;
}

float TB303Engine::distDcBlock(float input)
{
    double xn = static_cast<double>(input);
    double yn = xn - distDcX + 0.9995 * distDcY;
    distDcX = xn;
    distDcY = yn;
    return static_cast<float>(yn);
}

float TB303Engine::processSample()
{
    float envOut    = noteEnv.processSample();
    float accentOut = accentEnv.processSample();

    float vcoOut = vco.processSample();
    float vcfOut = vcf.processSample(vcoOut, envOut, accentOut);
    float vcaOut = vca.processSample(vcfOut, envOut, accentOut);

    float out = applyDistortion(vcaOut);
    out = distDcBlock(out);

    return out;
}

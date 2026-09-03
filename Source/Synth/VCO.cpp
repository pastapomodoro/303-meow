#include "VCO.h"
#include <cmath>

VCO::VCO() {}

void VCO::prepare(double sr)
{
    sampleRate = sr;
    phase = 0.0;
    slideCoeff = std::exp(-1.0 / (0.025 * sampleRate));   // τ = 25ms
}

void VCO::reset()
{
    phase    = 0.0;
    subPhase = 0.0;
    sliding  = false;
}

void VCO::setSubVolume(float v)
{
    subVol = juce::jlimit(0.0f, 1.0f, v);
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
    // Rampa lineare. Si e' provato a curvarla come la carica del condensatore
    // del 303: misurato, non serve. Anche con curvatura estrema le armoniche
    // cambiano di meno di 1 dB, perche' il contenuto armonico di una dente di
    // sega lo determina il salto a fine ciclo, non la forma della rampa fra un
    // salto e l'altro. Costava un exp() per campione per nulla.
    double out = 2.0 * phase - 1.0;
    out -= polyBlep(phase, dt);                  // correct discontinuity at 0
    return out;
}

// Sub: onda quadra un'ottava sotto, con la sua fase indipendente.
double VCO::processSub()
{
    const double dt  = phaseInc * 0.5;
    double out = (subPhase < 0.5) ? 1.0 : -1.0;
    out += polyBlep(subPhase, dt);
    out -= polyBlep(std::fmod(subPhase + 0.5, 1.0), dt);

    subPhase += dt;
    if (subPhase >= 1.0) subPhase -= 1.0;
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

    double out = (waveform == 0) ? processSaw() : processSquare();

    // Il sub gira sempre: la sua fase deve restare agganciata anche quando il
    // volume e' a zero, altrimenti riaprendolo entra a fase casuale e si sente
    // uno scalino.
    const double sub = processSub();
    if (subVol > 1.0e-4f)
    {
        // Il denominatore tiene il picco intorno a 1.0 su tutta la corsa del
        // sub (misurato 1.04 a 0.55, 1.06 a fondo corsa). Serve perche' a
        // decidere quanta saturazione entra nel ladder deve restare il knob
        // Distort, non il volume del sub.
        out = (out + static_cast<double>(subVol) * 0.8 * sub)
            / (1.0 + static_cast<double>(subVol) * 0.7);
    }

    phase += phaseInc;
    if (phase >= 1.0) phase -= 1.0;

    return static_cast<float>(out);
}

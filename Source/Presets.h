#pragma once
#include <array>

// ─────────────────────────────────────────────────────────────────────────────
// Full preset: synth params + FX + pattern
// ─────────────────────────────────────────────────────────────────────────────
struct PresetData
{
    const char* name;
    float cutoff, resonance, envMod, decay;
    float accent, volume, distortion, tuning;
    float tempo;
    int   waveform;
    float delayTime, delayFeedback, delayMix;
    float reverbSize, reverbMix;
    int   patternLength;
    struct StepData {
        int  note; bool gate, accent, slide; int octave;
    } steps[16];
};

// ─────────────────────────────────────────────────────────────────────────────
// Synth-only preset: no pattern data
// ─────────────────────────────────────────────────────────────────────────────
struct SynthPresetData
{
    const char* name;
    float cutoff, resonance, envMod, decay;
    float accent, volume, distortion, tuning;
    int   waveform;
    float delayTime, delayFeedback, delayMix;
    float reverbSize, reverbMix;
};

// MIDI note reference:
// C2=36 D2=38 Eb2=39 E2=40 F2=41 F#2=42 G2=43 Ab2=44 A2=45 Bb2=46 B2=47
// C3=48 D3=50 E3=52 F3=53 G3=55 A3=57

// ─────────────────────────────────────────────────────────────────────────────
// 10 Factory Full Presets
//
// Un preset per archetipo. Gli archetipi vengono dall'analisi dei 154 pattern
// factory di Arturia Acid V, che smentisce l'intuizione: le linee acid vere non
// sono melodicamente affollate. Mediana di 3 sole classi di altezza (99 su 154
// ne usano <=3, e 29 sono su una nota sola), root al 69% delle note, span
// mediano 22 semitoni. La varieta' non sta nelle note: sta in ritmo, densita'
// (gate da 2 a 16), accenti (0..14), slide (0..14) e salti d'ottava.
// Dieci linee con sette note ciascuna nella stessa scala suonano tutte uguali;
// e' l'errore che questi preset correggono. Pattern e nomi sono originali —
// di Acid V si e' usato solo il profilo statistico, che e' un dato di fatto.
//
// Tutti i pattern sono scritti sulla root C3 (48) = StepSequencer::PATTERN_ROOT,
// cosi' in MIDI mode la nota del piano roll traspone la bassline in modo
// prevedibile: C3 la suona come scritta, G3 la porta una quinta sopra.
// C3 e' anche la riga piu' bassa del piano roll (che copre C3..B4): scriverli
// piu' in basso li rendeva non editabili e la UI li riavvolgeva di un'ottava,
// appiattendo i salti che danno carattere a queste linee.
// ─────────────────────────────────────────────────────────────────────────────
static const std::array<PresetData, 10> FACTORY_PRESETS = {{
    // name            cut     res   env   dec   acc   vol   dis   tun  tempo wf
    //                 dlyT  dlyF  dlyM  rvbS  rvbM  len

    // 1 — riferimento: root + quinta + ottava, il 303 come lo si aspetta
    {"Basic Acid",       700.f,.55f,.55f,.35f,.70f,.82f,.10f,.0f,130.f,0,
     .375f,.35f,.00f,.50f,.00f, 16,
     {{48,1,1,0,0},{48,1,0,0,0},{48,1,0,0,0},{60,1,0,0,0},
      {48,1,0,0,0},{48,1,0,0,0},{55,1,0,0,0},{48,1,0,0,0},
      {48,1,1,0,0},{48,1,0,0,0},{48,1,0,0,0},{60,1,0,0,0},
      {48,1,0,0,0},{55,1,0,1,0},{48,1,0,0,0},{48,1,0,0,0}}},

    // 2 — una sola nota: tutto il movimento lo fa il filtro
    {"Root Drone",       380.f,.90f,.88f,.55f,.82f,.80f,.28f,.0f,128.f,0,
     .250f,.42f,.10f,.55f,.06f, 16,
     {{48,1,1,0,0},{48,1,0,0,0},{48,1,0,0,0},{48,1,0,0,0},
      {48,1,1,0,0},{48,1,0,0,0},{48,1,0,0,0},{48,1,0,0,0},
      {48,1,1,0,0},{48,1,0,0,0},{48,1,0,0,0},{48,1,0,0,0},
      {48,1,1,0,0},{48,1,0,0,0},{48,1,0,0,0},{48,1,0,0,0}}},

    // 3 — due note e otto slide: legato continuo
    {"Glide Pair",       620.f,.84f,.62f,.50f,.74f,.80f,.22f,.0f,122.f,0,
     .333f,.45f,.15f,.50f,.08f, 16,
     {{48,1,1,1,0},{51,1,0,0,0},{48,1,0,1,0},{51,1,0,0,0},
      {48,1,1,1,0},{51,1,0,0,0},{48,1,0,1,0},{51,1,0,0,0},
      {48,1,0,1,0},{51,1,0,0,0},{48,1,0,1,0},{51,1,0,0,0},
      {48,1,0,1,0},{51,1,0,0,0},{48,1,0,1,0},{51,1,0,0,0}}},

    // 4 — una classe sola, salto d'ottava a ogni step
    {"Octave Ladder",    780.f,.68f,.58f,.22f,.78f,.82f,.18f,.0f,134.f,1,
     .375f,.32f,.00f,.45f,.00f, 16,
     {{48,1,0,0,0},{60,1,0,1,0},{48,1,0,0,0},{60,1,0,0,0},
      {48,1,1,0,0},{60,1,0,0,0},{48,1,0,0,0},{60,1,0,0,0},
      {48,1,0,0,0},{60,1,0,1,0},{48,1,0,0,0},{60,1,0,0,0},
      {48,1,1,0,0},{60,1,0,0,0},{48,1,0,0,0},{60,1,0,0,0}}},

    // 5 — sei gate su sedici: il groove sta nelle pause
    {"Sparse Groove",    900.f,.62f,.50f,.28f,.72f,.82f,.15f,.0f,124.f,0,
     .375f,.38f,.12f,.40f,.05f, 16,
     {{48,1,1,0,0},{48,0,0,0,0},{48,0,0,0,0},{60,1,0,0,0},
      {48,0,0,0,0},{48,1,0,0,0},{48,0,0,0,0},{48,0,0,0,0},
      {55,1,1,0,0},{48,0,0,0,0},{48,0,0,0,0},{48,1,0,0,0},
      {48,0,0,0,0},{60,1,0,1,0},{48,0,0,0,0},{48,0,0,0,0}}},

    // 6 — figura di otto step ripetuta due volte
    {"Half Loop",        560.f,.76f,.65f,.30f,.76f,.80f,.20f,.0f,132.f,1,
     .250f,.40f,.08f,.50f,.06f, 16,
     {{48,1,1,0,0},{48,1,0,0,0},{60,1,0,0,0},{48,1,0,0,0},
      {55,1,1,0,0},{48,1,0,0,0},{60,1,0,1,0},{51,1,0,0,0},
      {48,1,1,0,0},{48,1,0,0,0},{60,1,0,0,0},{48,1,0,0,0},
      {55,1,1,0,0},{48,1,0,0,0},{60,1,0,1,0},{51,1,0,0,0}}},

    // 7 — root, b2 e m3: la scala piu' scura dell'acid
    {"Phrygian Three",   520.f,.80f,.70f,.45f,.78f,.80f,.32f,.0f,126.f,1,
     .333f,.44f,.14f,.55f,.10f, 16,
     {{48,1,1,0,0},{49,1,0,0,0},{48,1,0,0,0},{51,1,0,0,0},
      {48,1,0,0,0},{49,1,0,1,0},{48,1,0,0,0},{51,1,0,0,0},
      {48,1,1,0,0},{49,1,0,0,0},{51,1,0,0,0},{49,1,0,0,0},
      {48,1,0,0,0},{60,1,0,1,0},{49,1,0,0,0},{48,1,0,0,0}}},

    // 8 — otto accenti: pompa a sedicesimi alterni
    {"Accent Storm",     660.f,.72f,.60f,.25f,.86f,.78f,.25f,.0f,138.f,0,
     .188f,.36f,.08f,.45f,.05f, 16,
     {{48,1,1,0,0},{48,1,0,0,0},{60,1,1,0,0},{48,1,0,0,0},
      {48,1,1,0,0},{55,1,0,0,0},{60,1,1,0,0},{48,1,0,0,0},
      {48,1,1,0,0},{48,1,0,0,0},{60,1,1,0,0},{48,1,0,0,0},
      {55,1,1,0,0},{48,1,0,0,0},{60,1,1,0,0},{48,1,0,0,0}}},

    // 9 — cinque note e decay lungo: si sovrappongono
    {"Deep Slow",        340.f,.48f,.45f,1.15f,.66f,.80f,.08f,.0f,118.f,0,
     .600f,.55f,.30f,.80f,.32f, 16,
     {{48,1,1,0,0},{48,0,0,0,0},{48,0,0,0,0},{48,0,0,0,0},
      {55,1,0,0,0},{48,0,0,0,0},{48,0,0,0,0},{60,1,0,1,0},
      {58,1,0,0,0},{48,0,0,0,0},{48,0,0,0,0},{48,0,0,0,0},
      {51,1,0,0,0},{48,0,0,0,0},{48,0,0,0,0},{48,0,0,0,0}}},

    // 10 — denso e veloce, quattro accenti sul battere
    {"Rave Sixteen",    1200.f,.78f,.66f,.18f,.84f,.84f,.30f,.0f,145.f,1,
     .188f,.34f,.06f,.40f,.04f, 16,
     {{48,1,1,0,0},{60,1,0,0,0},{48,1,0,0,0},{55,1,0,0,0},
      {48,1,1,0,0},{60,1,0,0,0},{55,1,0,0,0},{48,1,0,0,0},
      {51,1,1,0,0},{60,1,0,0,0},{48,1,0,0,0},{55,1,0,0,0},
      {48,1,1,0,0},{60,1,0,0,0},{55,1,0,1,0},{48,1,0,0,0}}}
}};

// ─────────────────────────────────────────────────────────────────────────────
// 12 Synth-Only Presets  (no pattern changes)
// ─────────────────────────────────────────────────────────────────────────────
static const std::array<SynthPresetData, 12> SYNTH_PRESETS = {{
    // name            cut    res   env   dec   acc   vol   dis   tun   wf  dlyT  dlyF  dlyM  rvbS  rvbM
    {"Acid Saw",       600.f,.75f,.70f,.20f,.75f,.80f,.00f,.0f,  0, .375f,.35f,.00f,.50f,.00f},
    {"TB Squelch",     200.f,.95f,.90f,.40f,.85f,.75f,.00f,.0f,  0, .250f,.30f,.00f,.50f,.00f},
    {"Deep Sub",       120.f,.18f,.20f,.80f,.60f,.85f,.00f,.0f,  0, .375f,.20f,.00f,.40f,.10f},
    {"Jungle Bass",    900.f,.50f,.50f,.15f,.70f,.82f,.05f,.0f,  0, .375f,.30f,.10f,.30f,.05f},
    {"Ambient Cloud",  350.f,.40f,.50f,1.0f,.55f,.72f,.00f,.0f,  0, .750f,.50f,.20f,.80f,.50f},
    {"Echo Acid",      700.f,.65f,.65f,.25f,.70f,.78f,.00f,.0f,  0, .375f,.55f,.45f,.40f,.15f},
    {"Square Stomp",   550.f,.62f,.60f,.18f,.75f,.80f,.08f,.0f,  1, .188f,.30f,.00f,.30f,.00f},
    {"Overdrive",      800.f,.40f,.45f,.20f,.70f,.78f,.75f,.0f,  0, .188f,.20f,.00f,.30f,.00f},
    {"Wobble Bass",    280.f,.82f,.85f,.90f,.75f,.78f,.00f,.0f,  0, .500f,.45f,.15f,.40f,.10f},
    {"Warm Pad",       160.f,.14f,.30f,1.5f,.50f,.70f,.00f,.0f,  0, .750f,.50f,.20f,.80f,.55f},
    {"Scream",         520.f,.98f,.65f,.15f,.85f,.80f,.15f,.0f,  0, .188f,.40f,.10f,.30f,.05f},
    {"Hybrid FX",      500.f,.60f,.55f,.35f,.70f,.75f,.20f,.0f,  0, .375f,.45f,.30f,.65f,.35f}
}};

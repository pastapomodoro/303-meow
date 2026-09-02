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
// Tarati sul profilo statistico dei 156 preset factory di Arturia Acid V:
// risonanza mediana 0.88 (non 0.68), decay piu' lunghi, distortion realmente
// in uso (drywet mediano 0.70) e ripartizione SAW/SQR quasi pari. I pattern e
// i nomi sono originali.
//
// Tutti i pattern sono scritti sulla root C3 (48) = StepSequencer::PATTERN_ROOT,
// cosi' in MIDI mode la nota del piano roll traspone la bassline in modo
// prevedibile: C3 la suona come scritta, G3 la porta una quinta sopra.
// C3 e' anche la riga piu' bassa del piano roll (che copre C3..B4): scriverli
// piu' in basso li rendeva non editabili e la UI li riavvolgeva di un'ottava,
// appiattendo i salti che danno carattere a queste linee.
// Vocabolario melodico: root, m3, 4, 5, m6, m7, maj7, ottava — piu' il b2 come
// nota di approccio, che e' l'idioma acid classico.
// ─────────────────────────────────────────────────────────────────────────────
static const std::array<PresetData, 10> FACTORY_PRESETS = {{
    // name            cut     res   env   dec   acc   vol   dis   tun  tempo wf
    //                 dlyT  dlyF  dlyM  rvbS  rvbM  len

    // 1 — discesa melodica con salto d'ottava in testa
    {"Nightshade Run",  620.f,.82f,.62f,.42f,.78f,.82f,.28f,.0f,132.f,0,
     .375f,.35f,.00f,.50f,.00f, 16,
     {{48,1,1,0,0},{60,1,0,1,0},{58,1,0,0,0},{55,1,0,0,0},
      {51,1,0,0,0},{48,1,0,0,0},{55,1,0,1,0},{58,1,0,0,0},
      {60,1,1,0,0},{58,1,0,0,0},{55,1,0,0,0},{53,1,0,1,0},
      {51,1,0,0,0},{50,1,0,0,0},{49,1,0,0,0},{48,1,0,0,0}}},

    // 2 — sinuosa, cromatismi sul m6 e sul maj7
    {"Copper Coil",     480.f,.90f,.74f,.55f,.80f,.80f,.35f,.0f,128.f,1,
     .333f,.40f,.12f,.55f,.08f, 16,
     {{48,1,1,0,0},{51,1,0,0,0},{48,1,0,0,0},{55,1,0,1,0},
      {56,1,0,0,0},{55,1,0,0,0},{53,1,0,0,0},{51,1,0,1,0},
      {48,1,1,0,0},{60,1,0,0,0},{59,1,0,1,0},{58,1,0,0,0},
      {55,1,0,0,0},{51,1,0,0,0},{49,1,0,0,0},{48,1,0,0,0}}},

    // 3 — arco ampio: sale fino al m3 sopra l'ottava e ridiscende
    {"Glass Meridian",  880.f,.68f,.55f,.30f,.75f,.82f,.22f,.0f,138.f,0,
     .375f,.32f,.00f,.45f,.00f, 16,
     {{48,1,1,0,0},{50,1,0,0,0},{51,1,0,0,0},{55,1,0,0,0},
      {58,1,0,1,0},{60,1,0,0,0},{63,1,0,0,0},{60,1,0,0,0},
      {58,1,1,0,0},{55,1,0,0,0},{51,1,0,1,0},{50,1,0,0,0},
      {48,1,0,0,0},{55,1,0,1,0},{51,1,0,0,0},{48,1,0,0,0}}},

    // 4 — profonda, pause a dare groove
    {"Ferrite Bloom",   360.f,.92f,.80f,.68f,.82f,.78f,.38f,.0f,126.f,1,
     .250f,.45f,.18f,.60f,.12f, 16,
     {{48,1,1,0,0},{48,0,0,0,0},{51,1,0,0,0},{48,1,0,0,0},
      {48,0,0,0,0},{55,1,0,1,0},{53,1,0,0,0},{48,1,0,0,0},
      {48,1,1,0,0},{48,0,0,0,0},{58,1,0,0,0},{55,1,0,0,0},
      {48,0,0,0,0},{51,1,0,1,0},{49,1,0,0,0},{48,1,0,0,0}}},

    // 5 — decay lungo, linea che respira
    {"Undertow",        540.f,.76f,.58f,.95f,.74f,.80f,.25f,.0f,122.f,0,
     .500f,.48f,.25f,.70f,.22f, 16,
     {{48,1,1,0,0},{48,1,0,0,0},{55,1,0,0,0},{58,1,0,1,0},
      {60,1,0,0,0},{58,1,0,0,0},{55,1,0,0,0},{51,1,0,0,0},
      {53,1,1,0,0},{55,1,0,0,0},{51,1,0,0,0},{48,1,0,0,0},
      {50,1,0,1,0},{51,1,0,0,0},{55,1,0,1,0},{48,1,0,0,0}}},

    // 6 — squelch: risonanza al limite, discesa dall'ottava
    {"Static Prayer",   700.f,.94f,.70f,.35f,.84f,.78f,.32f,.0f,140.f,1,
     .188f,.38f,.10f,.50f,.06f, 16,
     {{60,1,1,0,0},{58,1,0,0,0},{55,1,0,0,0},{51,1,0,0,0},
      {48,1,0,1,0},{51,1,0,0,0},{55,1,0,0,0},{58,1,0,0,0},
      {60,1,1,0,0},{63,1,0,0,0},{60,1,0,0,0},{58,1,0,0,0},
      {55,1,0,1,0},{51,1,0,0,0},{49,1,0,1,0},{48,1,0,0,0}}},

    // 7 — arpeggiata e brillante, cutoff alto e decay corto
    {"Lithium Dawn",   1150.f,.64f,.50f,.24f,.72f,.84f,.18f,.0f,145.f,0,
     .375f,.30f,.00f,.40f,.00f, 16,
     {{48,1,1,0,0},{55,1,0,0,0},{60,1,0,0,0},{55,1,0,0,0},
      {51,1,0,1,0},{58,1,0,0,0},{63,1,0,1,0},{58,1,0,0,0},
      {48,1,1,0,0},{55,1,0,0,0},{60,1,0,0,0},{67,1,0,0,0},
      {60,1,0,1,0},{55,1,0,0,0},{51,1,0,0,0},{48,1,0,0,0}}},

    // 8 — passeggiata cromatica su due registri
    {"Verdigris",       580.f,.86f,.66f,.48f,.78f,.80f,.30f,.0f,130.f,1,
     .333f,.42f,.15f,.55f,.10f, 16,
     {{48,1,1,0,0},{51,1,0,0,0},{49,1,0,0,0},{48,1,0,0,0},
      {55,1,0,1,0},{53,1,0,0,0},{51,1,0,0,0},{48,1,0,0,0},
      {58,1,1,0,0},{56,1,0,0,0},{55,1,0,0,0},{51,1,0,0,0},
      {60,1,0,1,0},{58,1,0,0,0},{55,1,0,1,0},{48,1,0,0,0}}},

    // 9 — rarefatta, coda lunga di delay e riverbero
    {"Solvent Trails",  420.f,.58f,.48f,1.20f,.70f,.80f,.15f,.0f,118.f,0,
     .600f,.55f,.32f,.80f,.35f, 16,
     {{48,1,1,0,0},{48,0,0,0,0},{55,1,0,0,0},{48,0,0,0,0},
      {58,1,0,1,0},{60,1,0,0,0},{48,0,0,0,0},{58,1,0,0,0},
      {55,1,1,0,0},{48,0,0,0,0},{51,1,0,0,0},{48,1,0,0,0},
      {48,0,0,0,0},{55,1,0,1,0},{48,0,0,0,0},{48,1,0,0,0}}},

    // 10 — scura e spinta, ottave in contrasto
    {"Obsidian Pulse",  320.f,.88f,.78f,.40f,.82f,.78f,.42f,.0f,136.f,1,
     .250f,.40f,.08f,.45f,.05f, 16,
     {{48,1,1,0,0},{48,1,0,0,0},{60,1,0,1,0},{58,1,0,0,0},
      {55,1,0,0,0},{51,1,0,0,0},{48,1,0,0,0},{55,1,0,0,0},
      {51,1,1,0,0},{48,1,0,0,0},{58,1,0,1,0},{55,1,0,0,0},
      {51,1,0,0,0},{49,1,0,1,0},{48,1,0,0,0},{60,1,0,0,0}}}
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

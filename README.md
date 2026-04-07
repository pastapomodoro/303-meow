# 303 Meow — TB-303 Bass Synthesizer Plugin

A VST3/AU emulation of the Roland TB-303 Bass Line synthesizer, built with JUCE 8. Features a 16-step sequencer, acid-house signal chain, and a custom steampunk-aesthetic UI.

---

## Overview

303 Meow reproduces the characteristic "squelch" of the TB-303 through an analog-modeled synthesis chain: a PolyBLEP oscillator → Huovilainen diode ladder filter → VCA → distortion → effects. The integrated step sequencer allows programming 8 independent 16-step patterns directly from the plugin interface, with per-step note, gate, accent, slide, and octave controls.

---

## Architecture

```
303-meow/
├── CMakeLists.txt
└── Source/
    ├── PluginProcessor.cpp/.h      # Host interface, parameter management, audio routing
    ├── PluginEditor.cpp/.h         # UI, custom LookAndFeel, preset/pattern controls
    ├── Synth/
    │   ├── TB303Engine.cpp/.h      # Master synthesis orchestrator
    │   ├── VCO.cpp/.h              # Oscillator (saw/square, PolyBLEP, portamento)
    │   ├── VCF.cpp/.h              # Ladder filter (4-pole, 2× oversampling)
    │   ├── VCA.cpp/.h              # Amplitude stage
    │   └── Envelope.cpp/.h         # Dual envelope generator (note + accent)
    ├── Sequencer/
    │   ├── StepSequencer.cpp/.h    # BPM-synced sequencer engine, 8 patterns
    │   └── Pattern.cpp/.h          # Per-step data model + serialization
    ├── UI/
    │   └── SequencerGrid.cpp/.h    # Interactive 16-step grid component
    ├── FX/
    │   └── Delay.h                 # Stereo delay line
    └── Presets.h                   # Factory presets (full + synth-only)
```

---

## Signal Flow

```
MIDI / Step Sequencer
        │
        ▼
  PluginProcessor
  (APVTS parameters)
        │
        ▼
   TB303Engine
  ┌─────┴──────────────────────────┐
  │                                │
  VCO                         Note Envelope
  (saw / square, portamento)  (decay: 0.05–2s)
  │                                │
  VCF  ◄──── Env Mod ─────────────┤
  (Huovilainen 4-pole ladder)  Accent Envelope
  │                            (fixed 200ms)
  VCA
  │
  Distortion (tanh saturation)
  │
  DC Blocker
        │
        ▼
     Delay
  (0.02–0.75s, feedback, mix)
        │
        ▼
     Reverb
  (JUCE Reverb, room size, damping)
        │
        ▼
    Audio Output (stereo)
```

---

## Synthesis Modules

### VCO — Voltage-Controlled Oscillator
- Sawtooth and square waveforms
- PolyBLEP anti-aliasing at phase discontinuities
- Portamento (70 ms slide time) for TB-303-style glide
- ±12 semitone tuning parameter

### VCF — Voltage-Controlled Filter
- Huovilainen nonlinear diode ladder model (4-pole lowpass)
- 2× oversampling via linear interpolation for numerical stability
- Cutoff range: 20 Hz – 20 kHz (log scale)
- Resonance: 0–1 (self-oscillates near 1)
- Envelope modulation: up to 3 octaves of cutoff sweep
- Accent mode: subtle resonance boost on accented steps

### VCA — Voltage-Controlled Amplifier
- Multiplies oscillator output by combined note × accent envelope signal

### Envelope
- **Note envelope:** 3 ms exponential attack, configurable RC decay (0.05–2 s); gates with each step
- **Accent envelope:** same 3 ms attack, fixed 200 ms decay; fires only on accented steps

### Distortion
- Post-VCA tanh saturation with 9× drive gain, followed by DC blocker

---

## Step Sequencer

- 8 independent pattern slots, each with 16 steps
- Step resolution: quarter / eighth / sixteenth notes
- Per-step data (stored in `Pattern`):
  - **Note** (MIDI pitch)
  - **Gate** (on/off)
  - **Accent** (triggers accent envelope)
  - **Slide** (enables portamento to next step)
  - **Octave** (±2 octave transposition)
- BPM sync to host transport or internal tempo (60–200 BPM)
- Playhead position exposed to UI for real-time step highlight
- MIDI export: current pattern can be exported as a `.mid` file

---

## Parameters (APVTS)

| ID | Range | Description |
|----|-------|-------------|
| `cutoff` | 20–20000 Hz | Filter cutoff frequency |
| `resonance` | 0–1 | Filter resonance |
| `envMod` | 0–1 | Envelope → filter modulation depth |
| `decay` | 0.05–2 s | Note envelope decay time |
| `accent` | 0–1 | Accent level |
| `volume` | 0–1 | Output volume |
| `tuning` | ±12 st | Oscillator tuning |
| `waveform` | 0/1 | 0 = sawtooth, 1 = square |
| `distortion` | 0–1 | Distortion amount |
| `delayTime` | 0.02–0.75 s | Delay time |
| `delayFeedback` | 0–0.9 | Delay feedback |
| `delayMix` | 0–1 | Delay wet/dry |
| `reverbSize` | 0–1 | Reverb room size |
| `reverbMix` | 0–1 | Reverb wet/dry |
| `tempo` | 60–200 BPM | Internal sequencer tempo |
| `play` | 0/1 | Sequencer play/stop gate |

---

## Preset System

- **8 Full Presets** — store synth parameters + pattern data (e.g. "Classic Acid", "Deep Squelch", "Dub Delay", "Space Acid")
- **12 Synth-Only Presets** — modify synthesis parameters without changing the active pattern (e.g. "Acid Saw", "TB Squelch", "Deep Sub", "Wobble Bass")
- Presets serialized via `Pattern::toValueTree()` + APVTS state; restorable across sessions

---

## UI & UX

The plugin uses a custom `TB303LookAndFeel` with a **steampunk** aesthetic:

- Copper/brass color palette with phosphor-green accents
- Rotary knobs styled as industrial gauges
- LCD-style section panels
- Decorative rivet elements

**Interactive Components:**
- `SequencerGrid` — clickable 16-step grid with per-step accent/slide indicator dots and octave mini-controls
- Preset dropdowns for full and synth-only presets
- Waveform toggle, play/stop button, tempo control

---

## Build

### Requirements
- CMake 3.22+
- C++17 compiler (Clang or GCC)
- macOS (for AU target); Windows/Linux supported for VST3
- Internet connection on first build (JUCE fetched via `FetchContent`)

### Steps

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Output artifacts:
- `build/TB303Clone_artefacts/VST3/TB303Clone.vst3`
- `build/TB303Clone_AU_artefacts/AU/TB303Clone.component` (macOS only)

### Dependencies
| Library | Version | Source |
|---------|---------|--------|
| JUCE | 8.0.6 | FetchContent (GitHub) |

JUCE modules used: `juce_audio_utils`, `juce_audio_processors`, `juce_audio_devices`, `juce_dsp`, `juce_gui_basics`, `juce_gui_extra`

---

## Real-Time Safety

- Lock-free parameter access: raw atomic pointer cache updated outside audio thread
- No heap allocation in `processBlock()`
- Thread-safe sequencer step counter via atomics (UI read, audio write)
- Pre-allocated Delay and Reverb buffers

---

## Key Touchpoints

| Touchpoint | File | Description |
|-----------|------|-------------|
| Host ↔ Plugin interface | [PluginProcessor.cpp](Source/PluginProcessor.cpp) | `processBlock()`, parameter tree, state save/restore |
| UI entry point | [PluginEditor.cpp](Source/PluginEditor.cpp) | Layout, LookAndFeel, component wiring |
| Synthesis core | [Source/Synth/TB303Engine.cpp](Source/Synth/TB303Engine.cpp) | Per-sample synthesis loop |
| Filter | [Source/Synth/VCF.cpp](Source/Synth/VCF.cpp) | Ladder filter; most CPU-critical path |
| Sequencer clock | [Source/Sequencer/StepSequencer.cpp](Source/Sequencer/StepSequencer.cpp) | BPM timing, pattern advance |
| Step data | [Source/Sequencer/Pattern.cpp](Source/Sequencer/Pattern.cpp) | Note/accent/slide serialization |
| Grid UI | [Source/UI/SequencerGrid.cpp](Source/UI/SequencerGrid.cpp) | Step interaction, playhead render |
| Factory presets | [Source/Presets.h](Source/Presets.h) | All bundled preset data |
| Build config | [CMakeLists.txt](CMakeLists.txt) | Targets, formats, JUCE setup |

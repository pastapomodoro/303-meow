# 303 MEOW — COMPLETE PROJECT CONTEXT
# For: Qwen 3.6 (or any LLM assistant)
# Author: Eugenio Bellini (@euxeney) — NABA Tesi UI/UX 2026
# Stack: JUCE 8.0.6 · C++17 · CMake · VST3/AU/Standalone

---

## 1. PROJECT OVERVIEW

303 Meow is a TB-303 clone VST/AU plugin built in JUCE 8.0.6.
Target quality: D16 Phoscyon / Arturia Acid V level.
UI aesthetic: AcidLab — skeuomorphic white panel, acid green (#88ff00) accent,
              Digitakt II-inspired layout, custom LookAndFeel with texture support.

Build system: CMake 3.22+, Ninja
Platforms: macOS (VST3 + AU + Standalone), Windows (VST3 + Standalone)
C++ Standard: 17

---

## 2. PROJECT STRUCTURE

```
303-meow/
├── CMakeLists.txt
├── Source/
│   ├── PluginProcessor.h / .cpp      — AudioProcessor, APVTS, FX chain
│   ├── PluginEditor.h / .cpp         — AudioProcessorEditor, TB303LookAndFeel
│   ├── Presets.h                     — PresetData, SynthPresetData, factory arrays
│   ├── Synth/
│   │   ├── VCO.h / .cpp             — PolyBLEP oscillator (saw/square), portamento
│   │   ├── VCF.h / .cpp             — Huovilainen diode ladder, 2x oversampling
│   │   ├── VCA.h / .cpp             — Amplifier with tanh soft clip
│   │   ├── Envelope.h / .cpp        — Attack/Decay RC-style exponential
│   │   └── TB303Engine.h / .cpp     — Main synth engine, ties VCO/VCF/VCA/ENV
│   ├── Sequencer/
│   │   ├── Pattern.h / .cpp         — Step data, serialization
│   │   ├── StepSequencer.h / .cpp   — 8 patterns, 16 steps, slide/accent
│   ├── UI/
│   │   └── SequencerGrid.h / .cpp   — 16-step grid component with acc/sld/oct
│   └── FX/
│       └── Delay.h                  — Simple delay line
```

---

## 3. AUDIO ARCHITECTURE

### Signal Flow
```
MIDI/Sequencer → TB303Engine → Delay → Reverb (juce::dsp::Reverb) → Output
```

### VCO (Source/Synth/VCO.cpp)
- Waveforms: Sawtooth, Square (user-selectable)
- Anti-aliasing: PolyBLEP correction at discontinuities
- Portamento (slide): exponential interpolation, ~70ms slide time
- Tuning: semitones → Hz via 12-TET formula

### VCF (Source/Synth/VCF.cpp)
- Model: Huovilainen diode ladder (4-pole, -24dB/oct)
- Oversampling: 2× via linear interpolation (midpoint + actual sample)
- Cutoff modulation: log domain (octaves), env + accent modulation
- Resonance: 0–0.99, accent slightly boosts resonance
- DC blocker: IIR high-pass post-filter

### VCA (Source/Synth/VCA.cpp)
- Amplitude: noteEnv + accentEnv * 1.0 (accent = +6dB)
- Output: tanh saturation for subtle harmonic content
- Volume: 0.0–1.0

### Envelope (Source/Synth/Envelope.cpp)
- Two instances: Note (attack+decay) and Accent (fixed 200ms decay)
- Attack: 3ms exponential
- Decay: user-controlled (0.05–2.0s) for Note; fixed for Accent
- Trigger: does NOT reset to 0 (retrigger from current level)

### Distortion (in TB303Engine)
- tanh(drive * input) / tanh(drive) — soft clip
- drive = 1 + distortion * 9 (parameter 0–1)
- DC blocker post-distortion

---

## 4. SEQUENCER ARCHITECTURE

### StepSequencer (Source/Sequencer/StepSequencer.cpp)
- 8 patterns (NUM_PATTERNS = 8)
- 16 steps per pattern (Pattern::MAX_STEPS = 16)
- Step resolution: 1/4, 1/8, 1/16 (stepResolution = 1, 2, 4)
- Timing: sample-accurate, based on BPM (60/bpm/resolution * sampleRate)
- Host sync: reads AudioPlayHead::CurrentPositionInfo BPM if available
- Transport: play(), stop(), reset()
- Slide: exponential portamento between steps when slide=true
- Accent: triggers accentEnv, boosts cutoff and amplitude

### Pattern / Step (Source/Sequencer/Pattern.h)
```cpp
struct Step {
    int  note   = 48;   // MIDI note (C3 default)
    bool gate   = false;
    bool accent = false;
    bool slide  = false;
    int  octave = 0;    // -1, 0, +1 transpose
};
```
- Serialized via juce::ValueTree for state persistence
- Pattern length: 1–16 steps

---

## 5. APVTS PARAMETERS

All parameters registered via AudioProcessorValueTreeState:

| ID             | Range              | Default | Description          |
|----------------|--------------------|---------|----------------------|
| cutoff         | 20–20000 Hz (0.25) | 800     | VCF cutoff           |
| resonance      | 0.0–1.0            | 0.5     | VCF resonance        |
| envMod         | 0.0–1.0            | 0.5     | Envelope modulation  |
| decay          | 0.05–2.0s          | 0.3     | Note decay time      |
| accent         | 0.0–1.0            | 0.7     | Accent level         |
| volume         | 0.0–1.0            | 0.8     | Master volume        |
| tuning         | -12–+12 semitones  | 0.0     | Global tuning        |
| waveform       | 0 or 1             | 0       | 0=SAW, 1=SQR         |
| distortion     | 0.0–1.0            | 0.0     | Distortion amount    |
| tempo          | 60–200 BPM         | 120     | Internal tempo       |
| play           | 0 or 1             | 0       | Transport on/off     |
| delayTime      | 0.02–0.75s         | 0.375   | Delay time           |
| delayFeedback  | 0.0–0.90           | 0.35    | Delay feedback       |
| delayMix       | 0.0–1.0            | 0.0     | Delay wet mix        |
| reverbSize     | 0.0–1.0            | 0.5     | Reverb room size     |
| reverbMix      | 0.0–1.0            | 0.0     | Reverb wet mix       |

---

## 6. PRESET SYSTEM

### Factory Presets (8) — PresetData struct
Full presets: synth params + FX + complete 16-step pattern.
Names: Classic Acid, Deep Squelch, Funky Groove, Dub Delay,
       Space Acid, Dark Techno, Rave Acid, Square Pulse

### Synth Presets (12) — SynthPresetData struct
Synth-only: no pattern changes.
Names: Acid Saw, TB Squelch, Deep Sub, Jungle Bass, Ambient Cloud,
       Echo Acid, Square Stomp, Overdrive, Wobble Bass, Warm Pad, Scream, Hybrid FX

### State Persistence
- APVTS state + patterns + currentPattern + presetIndex
- Stored as XML via getStateInformation / setStateInformation

---

## 7. UI ARCHITECTURE

### Current State (PluginEditor.cpp)
- Canvas size: 1200 × 660 px
- LookAndFeel: TB303LookAndFeel (extends juce::LookAndFeel_V4)
- Current color scheme: dark navy/cyan (being redesigned)

### Target Design: AcidLab
- Palette:
  - Panel: #f0f0ec (white plastic)
  - Acid accent: #88ff00 (neon green)
  - LCD background: #070e03
  - LCD text: #88ff00
  - Text: #2a2a28
  - Text dim: #888882
- Font: IBM Plex Mono (labels), IBM Plex Sans (UI)
- Aesthetic: skeuomorphic — knob shadows, bevel buttons, inset LCD bezel
- Texture plan: PNG textures (brushed aluminum, ABS plastic, rubber) applied in JUCE via Graphics::drawImageAt + OpenGL

### UI Sections (5 tabs)
1. SYNTH — VCF knobs (Cutoff/Reso/EnvMod/Decay) + main LCD display + VCA (Vol/Dist/Accent) + Transport + Waveform + Pattern numpad
2. SEQUENCER — Piano roll (16 steps × 24 notes, C3-B4) + Slide/Accent rows
3. FX — Delay / Reverb / Distortion / Chorus knobs + overview canvas
4. PRESET — Factory dropdown + Synth dropdown + Load/Play
5. SETTINGS — BPM control + Mode selector (Filter/Note/MIDI)

### LookAndFeel — drawRotarySlider (current)
- Dark knob with radial gradient
- Cyan arc fill + pointer with glow
- Target: white ABS plastic knob, #88ff00 dot indicator, real texture

### SequencerGrid (Source/UI/SequencerGrid.cpp)
- 16 columns × fixed height zones per column:
  - kLabelTop=0, kGateTop=16, kNoteUpTop=140, kNoteNmTop=162
  - kNoteDnTop=180, kOctTop=202, kAccentTop=230, kSlideTop=260, kPlayTop=290
  - kColH=300 total
- Mouse interaction: toggle gate, ▲▼ pitch, oct -1/0/+1, accent, slide
- Timer: 30Hz repaint

---

## 8. BUILD INSTRUCTIONS

```bash
# Configure
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Output locations (macOS)
build/303_Meow_artefacts/VST3/303 Meow.vst3
build/303_Meow_artefacts/AU/303 Meow.component
build/303_Meow_artefacts/Standalone/303 Meow.app
```

JUCE: fetched via FetchContent from github.com/juce-framework/JUCE, tag 8.0.6

---

## 9. MIDI MODE

- midiMode (std::atomic<bool>): when true, sequencer stops, engine responds to MIDI directly
- noteOn: engine.noteOn(midiNote, velocity>100, false, midiNote)
- noteOff: engine.noteOff()
- When midiMode=false: sequencer drives the engine via processBlock

---

## 10. KNOWN ISSUES / TODO

### DSP
- [ ] Slide interpolation can click at very short decay times
- [ ] Accent envelope fixed at 200ms — consider making it user-adjustable
- [ ] No oversampling on distortion stage (potential aliasing at high drive)
- [ ] Reverb is juce::dsp::Reverb (basic) — upgrade to Schroeder/FDN

### UI
- [ ] TB303LookAndFeel needs full rewrite for AcidLab aesthetic
- [ ] drawRotarySlider: replace with canvas-based knob (normal map + PNG texture)
- [ ] SequencerGrid: replace with piano roll (currently step grid)
- [ ] LCD display: add smiley face animation (innocent→evil as cutoff increases)
- [ ] All sections need skeuomorphic texture pass with PNG assets
- [ ] JUCE OpenGL renderer for normal map lighting on knobs

### Architecture
- [ ] Thread safety: pattern editing on UI thread, playback on audio thread (no mutex currently)
- [ ] Preset browser UI not implemented
- [ ] No undo/redo

---

## 11. DESIGN REFERENCES

- Elektron Digitakt II: dark panel, flat-top cylindrical knobs, bitmap LCD, rubber buttons
- Phoscyon2 (D16): professional knob layout, colored LED indicators
- Behringer TD-3: original TB-303 clone proportions
- TP-7: matte plastic industrial aesthetic
- AcidLab sketch: 5-section tab navigation, bento box grid layout

---

## 12. SMILEY FACE LCD COMPONENT (PLANNED)

A canvas component inside the main LCD display:
- Innocent smiley at low cutoff (acid house style, thin line art, #88ff00)
- Transforms to evil devil smiley (horns, menacing grin) as cutoff increases
- Driven by cutoff parameter value in real-time
- SVG/Canvas paths, no PNG dependency
- Reference: classic acid house rave smiley + devil smiley with horns

---

## 13. KEY FILES SUMMARY

| File | Purpose |
|------|---------|
| PluginProcessor.cpp | Audio thread: process block, FX, parameter routing |
| PluginEditor.cpp | UI thread: paint, resized, timer, LookAndFeel |
| TB303Engine.cpp | Ties VCO+VCF+VCA+ENV, distortion, DC block |
| VCF.cpp | Huovilainen ladder, 2x oversample, DC block |
| VCO.cpp | PolyBLEP SAW/SQR, slide |
| StepSequencer.cpp | Pattern playback, sample-accurate timing |
| SequencerGrid.cpp | 16-step UI grid, mouse interaction |
| Presets.h | All factory data inline |

---

## 14. HOW TO USE THIS CONTEXT

When asking Qwen to work on this project:

- Always specify which file you're editing
- Reference parameter IDs by exact string ("cutoff", "resonance", etc.)
- The APVTS is accessed via processor.getAPVTS()
- Audio thread code must NOT allocate or call non-RT-safe functions
- UI thread code (paint/resized) must NOT access audio state directly — use atomics or APVTS
- LookAndFeel methods receive Graphics& g and component bounds — no member state
- JUCE component lifecycle: constructor → addAndMakeVisible → resized → paint → destructor

---

*Generated from source code — 303 Meow v1.0.0 — April 2026*

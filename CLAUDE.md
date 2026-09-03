# 303 Meow — CLAUDE.md
# Guida tecnica completa per tesi, presentazione e documento visivo
# Autore: Eugenio Bellini (@euxeney) — NABA Tesi UI/UX 2026
# Stack: JUCE 8.0.6 · C++17 · CMake · VST3/AU/Standalone

---

## INDICE

1. [Panoramica del Progetto](#1-panoramica-del-progetto)
2. [Struttura del Repository](#2-struttura-del-repository)
3. [Build System](#3-build-system)
4. [Architettura Audio (DSP)](#4-architettura-audio-dsp)
5. [Sequencer](#5-sequencer)
6. [Parametri APVTS](#6-parametri-apvts)
7. [Sistema Preset](#7-sistema-preset)
8. [Architettura UI — WebView](#8-architettura-ui--webview)
9. [Bridge C++ ↔ JavaScript](#9-bridge-c--javascript)
10. [Frontend HTML/JS (ui.html)](#10-frontend-htmljs-uihtml)
11. [Build del Frontend (build_html.py)](#11-build-del-frontend-build_htmlpy)
12. [Flusso Dati Completo](#12-flusso-dati-completo)
13. [Funzioni e API di Riferimento](#13-funzioni-e-api-di-riferimento)
14. [Design System UI](#14-design-system-ui)
15. [Note per la Presentazione](#15-note-per-la-presentazione)

---

## 1. PANORAMICA DEL PROGETTO

**303 Meow** è un clone software del Roland TB-303 realizzato come plugin audio VST3/AU/Standalone.

| Voce | Dettaglio |
|------|-----------|
| Target qualitativo | D16 Phoscyon / Arturia Acid V |
| Estetica UI | AcidLab — pannello bianco skeuomorfico, verde acido #88ff00 |
| Ispirazione layout | Elektron Digitakt II (tab navigation, bento grid) |
| Build system | CMake 3.22+ con Ninja |
| Piattaforme | macOS (VST3 + AU + Standalone), Windows (VST3 + Standalone) |
| Linguaggio DSP | C++17 |
| Linguaggio UI | HTML5 + CSS3 + JavaScript (vanilla), embedded via JUCE WebView |
| Framework audio | JUCE 8.0.6 |

### Perché questa architettura è rilevante per la tesi (UI/UX)

La separazione netta tra **DSP** (C++) e **UI** (HTML/JS) è una scelta architetturale consapevole:
- Permette di progettare l'interfaccia con strumenti web standard (CSS, canvas, animazioni)
- Disaccoppia completamente la logica audio dalla presentazione visiva
- Dimostra come tecnologie web-based possano raggiungere qualità professionale in contesti audio

---

## 2. STRUTTURA DEL REPOSITORY

```
303-meow/
├── CMakeLists.txt                    — Build configuration (JUCE plugin target)
├── CLAUDE.md                         — Questo file
├── README.md                         — Documentazione pubblica
├── build_html.py                     — Script Python: assembla ui.html
├── sketch definitivo.html            — Prototipo UI standalone (preview browser)
├── Resources/
│   └── ui.html                       — UI produzione (generata da build_html.py)
├── _reference/
│   └── 303meow-context.md            — Contesto progetto per LLM
└── Source/
    ├── PluginProcessor.h / .cpp       — AudioProcessor: audio thread, APVTS, FX chain
    ├── PluginEditor.h / .cpp          — AudioProcessorEditor: WebView host, relay system
    ├── Presets.h                      — Dati preset inline (10 factory + 12 synth)
    ├── Synth/
    │   ├── VCO.h / .cpp              — Oscillatore PolyBLEP (SAW/SQR) + portamento
    │   ├── VCF.h / .cpp              — Filtro Huovilainen diode ladder, tap a 18 dB/ott
    │   ├── VCA.h / .cpp              — Amplificatore + tanh soft clip
    │   ├── Envelope.h / .cpp         — Inviluppo Attack/Decay esponenziale
    │   └── TB303Engine.h / .cpp      — Engine principale: VCO+VCF+VCA+ENV+Dist
    ├── Sequencer/
    │   ├── Pattern.h / .cpp          — Struttura Step, serializzazione ValueTree
    │   └── StepSequencer.h / .cpp    — 8 pattern × 16 step, timing sample-accurate
    ├── UI/
    │   └── SequencerGrid.h / .cpp    — Griglia step legacy (sostituita da WebView)
    └── FX/
        └── Delay.h                   — Delay line (header-only)
```

---

## 3. BUILD SYSTEM

### CMakeLists.txt — Struttura logica

```
juce_add_plugin(TB303Clone ...)          → target VST3 + Standalone
juce_add_plugin(TB303Clone_AU ...)       → target AU (macOS only)
juce_add_binary_data(TB303Assets ...)    → embed Resources/ui.html come BinaryData
target_compile_definitions(... JUCE_WEB_BROWSER=1 ...)
target_link_libraries(... juce_gui_extra ...)   → include WebBrowserComponent
```

### Comandi build

```bash
# Prima configurazione
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build completo (VST3 + AU + Standalone)
cmake --build build

# Solo standalone (sviluppo rapido)
cmake --build build --target TB303Clone_Standalone

# Rebuild UI senza ricompilare C++
python3 build_html.py
cmake --build build --target TB303Assets
cmake --build build --target TB303Clone_Standalone
```

### Artefatti output (macOS)

```
build/TB303Clone_artefacts/Debug/VST3/303 Meow.vst3
build/TB303Clone_artefacts/Debug/AU/303 Meow.component      (macOS)
build/TB303Clone_artefacts/Debug/Standalone/303 Meow.app
```

---

## 4. ARCHITETTURA AUDIO (DSP)

### Flusso del segnale audio

```
MIDI input / Sequencer
        ↓
   TB303Engine
   ├── VCO (genera onda grezza SAW/SQR)
   ├── ×(1 + distortion·2)  ← il drive entra QUI, prima del filtro
   ├── VCF (filtra con inviluppo + accent mod; satura se spinto)
   ├── VCA (inviluppo + accentEnv·accentLevel, soft clip ADAA)
   └── toneShaper (passa-basso a un polo, 13 kHz)
        ↓
     Delay (FX/Delay.h)
        ↓
  juce::dsp::Reverb
        ↓
  juce::dsp::Limiter (-1 dBFS) + clamp di sicurezza
        ↓
     Output stereo
```

---

### VCO — `Source/Synth/VCO.h/.cpp`

**Responsabilità:** Generazione dell'onda oscillatrice grezza.

| Metodo | Descrizione |
|--------|-------------|
| `prepare(sampleRate)` | Inizializza la frequenza campione |
| `setNote(midiNote, tuningCents)` | Converte MIDI → Hz (formula 12-TET) |
| `setWaveform(0 or 1)` | 0 = Sawtooth, 1 = Square |
| `setSlide(enable, fromNote, toNote)` | Abilita portamento esponenziale, τ = 25 ms |
| `processSample()` | Restituisce il prossimo campione float |

**Tecnica anti-aliasing:** PolyBLEP (Polynomial Band-Limited Step)
- Corregge le discontinuità nelle forme d'onda digitali
- Evita lo specchio di frequenze (aliasing) senza filtri aggiuntivi
- Usato su SAW (discontinuità a fine ciclo) e SQR (due discontinuità per ciclo)

**Portamento (Slide):**
```
currentFreq → targetFreq, approccio esponenziale per-campione
slideCoeff = exp(-1 / (0.025 * sampleRate))   // τ = 25 ms
currentFreq += (targetFreq - currentFreq) * (1 - slideCoeff)
```

---

### VCF — `Source/Synth/VCF.h/.cpp`

**Responsabilità:** Filtro passa-basso 4-polo che dà al TB-303 il suo suono caratteristico.

**Modello:** Huovilainen Diode Ladder (2006)
- Simula il filtro transistor ladder dell'originale Roland TB-303
- 4 stadi in cascata, ma **l'uscita è presa al terzo polo → -18 dB/ottava**
- È la pendenza del TB-303, non i -24 dB/ott del ladder Moog: è la ragione
  principale del suo timbro più aperto e nasale. A -24 dB/ott le armoniche
  sopra il taglio spariscono troppo e la linea non "buca" il mix.
- Il feedback resta preso dal quarto stadio, così la risonanza conserva la
  rotazione di fase dei quattro poli
- Non lineare: include `tanh()` per la saturazione dei transistor
- Compensazione risonanza: `inputGain = 1 + resonance·0.5` prima degli stadi,
  altrimenti ad alta risonanza il feedback svuota il fondamentale

| Metodo | Descrizione |
|--------|-------------|
| `setCutoff(hz)` | Frequenza di taglio (120–8000 Hz) |
| `setResonance(0.0–1.0)` | 0.99 = auto-oscillazione |
| `setEnvMod(0.0–1.0)` | Quantità di modulazione inviluppo sulla cutoff |
| `setAccentMod(0.0–1.0)` | Boost cutoff + risonanza su note accentate |
| `processSample(input, envInput, accentInput)` | Processa un campione con modulazioni |

**Oversampling 2×:** ogni campione viene processato due volte (midpoint + punto finale) per ridurre le distorsioni non-lineari ad alta frequenza.

**DC Blocker:** filtro IIR passa-alto posto in uscita per eliminare l'offset DC generato dal feedback non lineare.

---

### VCA — `Source/Synth/VCA.h/.cpp`

**Responsabilità:** Controllo dell'ampiezza finale del segnale.

```cpp
amp = min(noteEnv + accentEnv * accentLevel, 2.0)
output = adaaTanh(input * amp * volume)
```

- `noteEnv`: inviluppo principale (0.0–1.0)
- `accentEnv * accentLevel`: l'accento scala col knob Accent, non è un +6 dB fisso
- `adaaTanh()`: soft clip con antiderivative anti-aliasing (`DSP/Saturation.h`),
  perché con `amp` fino a 2.0 il tanh lavora ben fuori dalla zona lineare
  proprio sulle note accentate

---

### Envelope — `Source/Synth/Envelope.h/.cpp`

**Due istanze per ogni nota:**

| Istanza | Tipo | Attack | Decay |
|---------|------|--------|-------|
| `noteEnv` | `Type::Note` | 3ms esponenziale | user-defined (0.05–2.0s) |
| `accentEnv` | `Type::Accent` | 3ms esponenziale | fisso 200ms |

**Caratteristica chiave:** il trigger NON resetta a 0 — parte dal livello corrente (comportamento originale del TB-303, evita click su note legato).

**Coefficiente RC:**
```cpp
coeff = exp(-1.0 / (timeSec * sampleRate))
```

---

### Distortion — `TB303Engine.cpp`

```cpp
driven = vcoOut * (1.0f + distortion * 2.0f);   // 1× … 3×
vcfOut = vcf.processSample(driven, envOut, accentOut);
```

Il drive **non è un waveshaper in coda alla catena**: è guadagno d'ingresso
prima del VCF. È la differenza che conta, perché così la distorsione interagisce
con la risonanza — sono le non-linearità `tanh()` dentro il ladder a produrre
la compressione e le armoniche, come nell'hardware. Messo dopo il VCA, il drive
schiacciava un segnale già filtrato e il filtro non lo sentiva.

- `distortion = 0.0`: guadagno unitario, ingresso trasparente
- `distortion = 1.0`: 3× nel filtro, che satura in modo evidente
- Non serve più un DC blocker dedicato: quello del VCF è a valle del drive

---

### TB303Engine — `Source/Synth/TB303Engine.h/.cpp`

**Responsabilità:** Orchestratore del chain DSP.

| Metodo | Chiamato da | Thread |
|--------|-------------|--------|
| `prepare(sampleRate, blockSize)` | `PluginProcessor::prepareToPlay` | Main |
| `noteOn(midiNote, accent, slide, prevNote)` | Sequencer / MIDI | Audio |
| `noteOff()` | Sequencer / MIDI | Audio |
| `processSample()` | `PluginProcessor::processBlock` | Audio |
| `setCutoff/Resonance/EnvMod/...` | APVTS listeners | Audio |

---

## 5. SEQUENCER

### Pattern — `Source/Sequencer/Pattern.h/.cpp`

**Struttura dati di un singolo step:**

```cpp
struct Step {
    int  note   = 48;    // MIDI note (C3 = 48, B4 = 71)
    bool gate   = false; // step attivo?
    bool accent = false; // nota accentata (più forte + cutoff boost)?
    bool slide  = false; // portamento verso step successivo?
    int  octave = 0;     // trasposizione: -1, 0, +1 ottava
};
```

- 16 step per pattern (MAX_STEPS = 16)
- Serializzazione tramite `juce::ValueTree` per persistenza dello stato

---

### StepSequencer — `Source/Sequencer/StepSequencer.h/.cpp`

**Responsabilità:** Timing e playback dei pattern.

| Parametro | Valore |
|-----------|--------|
| Numero pattern | 8 |
| Step per pattern | 16 |
| Risoluzioni | 1/4 (×1), 1/8 (×2), 1/16 (×4) |
| Timing | Sample-accurate |

**Calcolo timing:**
```cpp
samplesPerStep = (60.0 / bpm / resolution) * sampleRate
```

**Host sync:** legge `AudioPlayHead::CurrentPositionInfo` — se il DAW è in play, usa il suo BPM.

**Timing:** il sequencer avanza **campione per campione** dentro lo stesso loop
che rende l'audio (`advanceSample`), non a blocchi. Sparare tutti i `noteOn` di
un blocco e poi renderizzare dal campione 0 introduceva un jitter fino a
`blockSize / sampleRate` — a 512 campioni sono 11 ms, udibili come groove
instabile. Anche gli eventi MIDI vengono processati al loro `samplePosition`.

**Swing:** lo step pari si allunga e il dispari si accorcia della stessa
frazione, quindi ogni coppia dura sempre `2 × samplesPerStep` e il tempo non
deriva. A 132 BPM: 1:1 dritto, 1.67:1 al 50%, 2.2:1 al 75%.

**MIDI mode = pattern player trasposto.** La nota in arrivo non suona da sola:
diventa la root del pattern e il sequencer parte finché la nota resta premuta —
come Acid V dentro un DAW. Premere un secondo tasto mentre il primo è giù
ritraspone *senza* ripartire da capo (legato); rilasciare tutto ferma il
sequencer. Priorità all'ultimo tasto premuto, stack di 16 note.

**API principale:**

| Metodo | Descrizione |
|--------|-------------|
| `syncBpm(playHead, fallbackBpm)` | Una volta per blocco: legge il BPM dell'host |
| `advanceSample(engine)` | Una volta per **campione**, dentro il loop audio |
| `setTranspose(semitoni)` | Trasposizione del pattern (MIDI mode) |
| `setSwing(0…0.45)` | Allunga gli step pari, accorcia i dispari |
| `play()` | Avvia playback |
| `stop()` | Ferma e resetta posizione |
| `selectPattern(index)` | Cambia pattern attivo (0–7) |
| `getCurrentStep()` | Step corrente (0–15), usato per highlight UI |
| `getCurrentPatternIndex()` | Pattern attivo |
| `isPlaying()` | Stato transport |
| `getPattern(index)` | Riferimento al Pattern per modifica step |
| `setStepResolution(1/2/4)` | Cambia risoluzione ritmica |

---

## 6. PARAMETRI APVTS

Tutti i parametri sono registrati in `PluginProcessor` via `AudioProcessorValueTreeState`.

| ID (stringa esatta) | Range | Default | Tipo JUCE | Controllo UI |
|---------------------|-------|---------|-----------|--------------|
| `cutoff` | 120–8000 Hz | 700 | Float (skew 0.4) | Knob SYNTH |
| `resonance` | 0.0–1.0 | 0.5 | Float | Knob SYNTH |
| `envMod` | 0.0–1.0 | 0.5 | Float | Knob SYNTH |
| `decay` | 0.05–2.0 s | 0.3 | Float | Knob SYNTH |
| `accent` | 0.0–1.0 | 0.7 | Float | Knob SYNTH |
| `volume` | 0.0–1.0 | 0.8 | Float | Knob SYNTH |
| `tuning` | -12–+12 semitoni | 0.0 | Float | Knob SYNTH |
| `waveform` | 0 o 1 | 0 | Float (binary) | Toggle SAW/SQR |
| `distortion` | 0.0–1.0 | 0.0 | Float | Knob SYNTH/FX |
| `tempo` | 60–200 BPM | 160 | Float | BPM display |
| `swing` | 0–75 % | 0 | Float | Slider SHUFFLE (SETTINGS) |
| `play` | 0 o 1 | 0 | Float (binary) | Play/Stop button |
| `delayTime` | 0.02–0.75 s | 0.375 | Float | Knob FX |
| `delayFeedback` | 0.0–0.90 | 0.35 | Float | Knob FX |
| `delayMix` | 0.0–1.0 | 0.0 | Float | Knob FX |
| `reverbSize` | 0.0–1.0 | 0.5 | Float | Knob FX |
| `reverbMix` | 0.0–1.0 | 0.0 | Float | Knob FX |

**Come leggere/scrivere un parametro da C++:**
```cpp
// Lettura
float v = *apvts.getRawParameterValue("cutoff");

// Scrittura (thread-safe)
apvts.getParameter("cutoff")->setValueNotifyingHost(normalizedValue);
```

---

## 7. SISTEMA PRESET

### Factory Presets — 10 preset completi

Ogni preset include: tutti i parametri synth + FX + pattern completo 16-step.

Uno per **archetipo**, non uno per "atmosfera". Gli archetipi vengono
dall'analisi dei 154 pattern factory di Arturia Acid V, che smentisce
l'intuizione: le linee acid vere non sono melodicamente affollate. Mediana di
**3 sole classi di altezza** (99 su 154 ne usano ≤3, 29 stanno su una nota
sola), root al **69%** delle note, span mediano 22 semitoni. La varietà non sta
nelle note: sta in ritmo, densità dei gate (da 2 a 16), accenti (0–14), slide
(0–14) e salti d'ottava. Dieci linee con sette note ciascuna nella stessa scala
suonano tutte uguali — errore commesso e poi corretto.

| # | Nome | Classi | Gate | Acc | Slide | Carattere |
|---|------|--------|------|-----|-------|-----------|
| 0 | Basic Acid | 2 | 16 | 2 | 1 | Riferimento: root + quinta + ottava, res 0.55 |
| 1 | Root Drone | 1 | 16 | 4 | 0 | Una nota sola: muove tutto il filtro |
| 2 | Glide Pair | 2 | 16 | 2 | 8 | Due note, legato continuo |
| 3 | Octave Ladder | 1 | 16 | 2 | 2 | Salto d'ottava a ogni step |
| 4 | Sparse Groove | 2 | 6 | 2 | 1 | Il groove sta nelle pause |
| 5 | Half Loop | 3 | 16 | 4 | 2 | Figura di 8 step ripetuta |
| 6 | Phrygian Three | 3 | 16 | 2 | 2 | Root, b2, m3 — la scala più scura |
| 7 | Accent Storm | 2 | 16 | 8 | 0 | Pompa a sedicesimi alterni |
| 8 | Deep Slow | 4 | 5 | 1 | 1 | Decay 1.15 s, note sovrapposte |
| 9 | Rave Sixteen | 3 | 16 | 4 | 1 | Denso e veloce, 145 BPM |

**Tutti scritti sulla root C3 = 48** (`StepSequencer::PATTERN_ROOT`), che è anche
la riga più bassa del piano roll. Scriverli più in basso li rendeva non
editabili e la UI li riavvolgeva di un'ottava, appiattendo i salti.

I pattern e i nomi sono originali: di Acid V si è usato solo il profilo
statistico, che è un dato di fatto e non un'opera creativa.

### Synth Presets — 12 preset solo timbrici

Solo parametri synth, senza modificare il pattern.

| # | Nome |
|---|------|
| 0 | Acid Saw |
| 1 | TB Squelch |
| 2 | Deep Sub |
| 3 | Jungle Bass |
| 4 | Ambient Cloud |
| 5 | Echo Acid |
| 6 | Square Stomp |
| 7 | Overdrive |
| 8 | Wobble Bass |
| 9 | Warm Pad |
| 10 | Scream |
| 11 | Hybrid FX |

### Persistenza dello stato

```cpp
// Salvataggio (PluginProcessor::getStateInformation)
auto state = apvts.copyState();
// + pattern data come XML child nodes
// → MemoryBlock serializzato

// Ripristino (PluginProcessor::setStateInformation)
// → parsing XML → ripristino APVTS + pattern
```

---

## 8. ARCHITETTURA UI — WEBVIEW

### Perché WebView invece di JUCE nativo?

In JUCE 8, `WebBrowserComponent` con native integration permette di costruire UI con HTML/CSS/JS e comunicare con il backend C++ in modo bidirezionale. Questo approccio:

- Libera il designer dall'API grafica JUCE (Graphics::fillRect, drawText, ecc.)
- Permette di usare CSS per animazioni, transizioni, layout responsive
- Consente di prototipare l'UI nel browser e poi deployarla nel plugin senza modifiche

### Come funziona

```
Resources/ui.html (file HTML embedded come BinaryData)
        ↕
juce::WebBrowserComponent (PluginEditor)
        ↕
WebSliderRelay / WebToggleButtonRelay (bridge parameter)
        ↕
AudioProcessorValueTreeState (APVTS)
        ↕
DSP chain (audio thread)
```

### PluginEditor.h — Struttura

```cpp
class TB303Editor : public juce::AudioProcessorEditor,
                    public juce::Timer
{
public:
    TB303Processor& processor;
    std::optional<Resource> getResource(const juce::String& url);

private:
    // Un relay per ogni parametro APVTS
    juce::WebSliderRelay cutoffRelay { "cutoff" };
    // ... (14 slider relay totali)
    juce::WebToggleButtonRelay playRelay { "play" };
    juce::WebToggleButtonRelay waveformRelay { "waveform" };

    juce::WebBrowserComponent browser;   // deve essere DOPO i relay

    // Attachments: collegano relay ↔ APVTS
    std::unique_ptr<WebSliderParameterAttachment> cutoffAtt;
    // ...

    int lastStep = -1, lastPattern = -1;
    juce::var buildStateVar();  // serializza stato sequencer → JSON
};
```

---

## 9. BRIDGE C++ ↔ JAVASCRIPT

### Direzione C++ → JS (emitting events)

Il timer a 50ms (20fps) controlla se lo step o il pattern è cambiato e invia un aggiornamento:

```cpp
// PluginEditor.cpp — timerCallback()
void TB303Editor::timerCallback()
{
    int curStep = seq.getCurrentStep();
    int curPat  = seq.getCurrentPatternIndex();

    if(curStep != lastStep || curPat != lastPattern) {
        lastStep    = curStep;
        lastPattern = curPat;
        browser.emitEventIfBrowserIsVisible("stateUpdate", buildStateVar());
    }
}
```

**Struttura del payload `stateUpdate`:**
```json
{
  "step":    3,
  "playing": true,
  "pattern": 0,
  "steps": [
    { "gate": true, "note": 48, "accent": false, "slide": false },
    ...  // 16 oggetti
  ]
}
```

### Direzione JS → C++ (native functions)

Registrate in `makeOptions()` via `.withNativeFunction(name, lambda)`:

| Funzione JS | Parametri | Azione C++ |
|-------------|-----------|------------|
| `selectPattern(i)` | index 0–7 | `sequencer.selectPattern(i)` |
| `toggleStep(i)` | step 0–15 | inverte `step.gate` |
| `setStepNote(i, note)` | step, MIDI | imposta nota + gate; se uguale → toggle off |
| `setStepAccent(i, on)` | step, bool | imposta `step.accent` |
| `setStepSlide(i, on)` | step, bool | imposta `step.slide` |
| `setResolution(r)` | 1/2/4 | `processor.setStepResolution(r)` |
| `loadPreset(i)` | index 0–7 | carica factory preset completo |
| `loadSynthPreset(i)` | index 0–11 | carica solo parametri synth |
| `setMidiMode(on)` | bool | abilita/disabilita MIDI mode |

**Esempio chiamata da JS:**
```javascript
const setNote = window.__JUCE__.getNativeFunction("setStepNote");
setNote(stepIndex, midiNote);
```

### WebSliderRelay — Come funziona

```
JS SliderState → relay.setValue() → attachment → APVTS.setValueNotifyingHost()
                                                         ↓
                                              audio thread legge il valore
```

Il relay è una via bidirezionale: JUCE invia anche gli aggiornamenti JS quando il parametro cambia (es. da automazione DAW o da preset load).

---

## 10. FRONTEND HTML/JS (ui.html)

### File sorgente: `sketch definitivo.html`

Il file originale è un prototipo standalone completamente funzionale nel browser, senza alcuna dipendenza JUCE. Contiene:

- **HTML strutturale:** 5 sezioni tab (SYNTH, SEQUENCER, FX, PRESET, SETTINGS)
- **CSS completo:** palette AcidLab, knob canvas, layout a bento box
- **JavaScript autonomo:** stato interno simulato, sequencer JS via setTimeout, tap tempo

### 5 Sezioni UI

#### SYNTH
- Knob: Cutoff, Resonance, Env Mod, Decay, Volume, Distort, Accent, Tuning
- LCD display centrale con mascotte gatto animata
- Transport: Play/Stop
- Waveform selector: SAW / SQR
- Pattern selector: 8 pulsanti (PAT 1–8)
- LED meter verticale

#### SEQUENCER
- Piano roll: 16 colonne × 24 righe (C3–B4, MIDI 48–71)
- Ogni cella: click → attiva nota nello step
- Toggle: ACC (accent) e SLD (slide) per step
- La riga attiva viene evidenziata durante la riproduzione

#### FX
- Delay: Time + Feedback + Mix (canvas knob)
- Reverb: Size + Mix (canvas knob)
- Distortion: Amount (condiviso con SYNTH)
- Chorus (decorativo)
- Canvas SVG per il pannello visivo

#### PRESET
- Dropdown factory preset (10 voci)
- Dropdown synth preset (12 voci)
- Pulsante Load

#### SETTINGS
- BPM display: +/− e tap tempo
- Mode selector: Filter / Note / MIDI
- Risoluzione step: 1/4, 1/8, 1/16

---

### Piano Roll — Mappatura MIDI

```
Riga 0  = B4  (MIDI 71)
Riga 1  = A#4 (MIDI 70)
Riga 2  = A4  (MIDI 69)
...
Riga 23 = C3  (MIDI 48)
```

Array `ALL_NOTES` in JS (da indice 0 = top):
```javascript
const ALL_NOTES = ["B4","A#4","A4","G#4","G4","F#4","F4","E4","D#4","D4",
                   "C#4","C4","B3","A#3","A3","G#3","G3","F#3","F3","E3",
                   "D#3","D3","C#3","C3"];
```

Conversione nota nome → MIDI:
```javascript
const NOTE_TO_MIDI = { "C3":48, "C#3":49, ..., "B4":71 }
```

---

## 11. BUILD DEL FRONTEND (build_html.py)

### Strategia: Two-Script Overlay

Il file `ui.html` produzione è generato da `build_html.py` combinando:

1. **`sketch definitivo.html` (body completo + script originale):** fornisce HTML, CSS, e tutta la logica JS originale (canvas knob, piano roll, LED meter, nav)
2. **Script bridge JUCE (secondo `<script>`):** si sovrappone dopo, patcha le funzioni necessarie se `window.__JUCE__` è presente

Questo approccio garantisce che `sketch definitivo.html` continui a funzionare nel browser per preview, mentre `ui.html` funziona nel plugin.

### Struttura del bridge overlay (aggiunto da build_html.py)

```javascript
// Attivato solo se JUCE WebView è presente
if (window.__JUCE__) {

  // Funzioni native JUCE
  const selectPattern = window.__JUCE__.getNativeFunction("selectPattern");
  const toggleStep    = window.__JUCE__.getNativeFunction("toggleStep");
  // ...

  // Relay per i parametri knob
  const cutoffState = Juce.getSliderState("cutoff");
  // ...

  // Patch knob → JUCE relay
  const KNOB_MAP = {
    "Cutoff":     cutoffState,
    "Resonance":  resonanceState,
    // ...
  };

  // Override transport
  window.togglePlay = function() { /* playRelay toggle */ };

  // Override pattern buttons
  window.selectPatternUI = function(i) { selectPattern(i); };

  // Listener aggiornamento stato dal C++
  Juce.addEventListener("stateUpdate", function(data) {
    highlightStep(data.step);
    syncNotesToCanvas(data.steps);
    // ...
  });
}
```

### ⚠️ Due avvertenze prima di toccare il frontend

**1. `build_html.py` non va lanciato così com'è.** `sketch definitivo.html` è
rimasto fermo ad agosto 2025 (~134 KB) mentre `index.html` e `Resources/ui.html`
sono andati avanti (~195 KB). Rigenerare dal sketch **sovrascriverebbe** mesi di
lavoro: DSP JS, piano roll, swing, pattern transpose, preset. Le modifiche al
frontend si fanno **direttamente su `index.html` e `Resources/ui.html`, tenendoli
in sync** (sono lo stesso file: uno per il deploy statico, uno embedded nel
plugin). Poi:

```bash
cmake --build build --target TB303Assets          # reimpacchetta ui.html
cmake --build build --target TB303Clone_Standalone
```

**2. Il bridge JUCE è disattivato.** `INCLUDE_JUCE_BRIDGE = False`, quindi in
`Resources/ui.html` non c'è nessun riferimento a `window.__JUCE__`.
`PluginEditor.cpp` espone comunque 16 relay e 11 funzioni native
(`selectPattern`, `loadPreset`, `setMidiMode`, …) ma **nessuno le chiama**: nel
plugin VST/AU l'interfaccia non è collegata al DSP C++, e i knob muovono il
motore WebAudio in JS. È uno stato deliberato — il progetto lavora sulla versione
standalone — ma va saputo, perché è la ragione per cui alcune funzioni C++
(MIDI mode, swing) non sono raggiungibili dal plugin.

---

## 12. FLUSSO DATI COMPLETO

### Esempio: utente muove il knob Cutoff nell'UI

```
1. JS: utente trascina il knob canvas "Cutoff"
2. JS: knob aggiorna il valore interno e chiama cutoffState.setValue(v)
3. JUCE WebSliderRelay riceve il valore
4. WebSliderParameterAttachment propaga a APVTS parameter "cutoff"
5. APVTS notifica il processor (thread-safe tramite ValueTree listener)
6. Audio thread: PluginProcessor::processBlock() legge il nuovo valore
7. TB303Engine::setCutoff(hz) aggiorna VCF::setCutoff(hz)
8. Il filtro cambia frequenza dal prossimo campione audio
```

### Esempio: sequencer raggiunge il prossimo step

```
1. Audio thread: StepSequencer avanza samplesPerStep
2. Step attivo: engine.noteOn(note, accent, slide, prevNote)
3. VCO: setNote() (e setSlide() se slide=true)
4. VCF, VCA, Envelope: aggiornati con nuovi valori
5. TB303Editor::timerCallback() (50ms): curStep != lastStep
6. buildStateVar() serializza lo stato corrente
7. browser.emitEventIfBrowserIsVisible("stateUpdate", payload)
8. JS riceve l'evento "stateUpdate"
9. JS evidenzia la colonna attiva nel piano roll/sequencer
```

### Esempio: preset caricato da UI

```
1. JS: utente clicca Load nel dropdown factory preset
2. JS: chiama getNativeFunction("loadPreset")(index)
3. C++ lambda: processor.loadPreset(index)
4. PluginProcessor::loadPreset(): scrive su APVTS + aggiorna pattern
5. APVTS notifica tutti i relay → JS riceve aggiornamenti
6. Parametri knob si aggiornano visivamente
```

---

## 13. FUNZIONI E API DI RIFERIMENTO

### PluginProcessor (TB303Processor)

```cpp
// Accessori principali
juce::AudioProcessorValueTreeState& getAPVTS();
StepSequencer& getSequencer();

// Preset
void loadPreset(int index);       // factory preset (synth + FX + pattern)
void loadSynthPreset(int index);  // solo parametri synth

// Modalità
void setMidiMode(bool enable);
void setStepResolution(int r);    // 1=1/4, 2=1/8, 4=1/16

// State persistence (JUCE standard)
void getStateInformation(juce::MemoryBlock& destData) override;
void setStateInformation(const void* data, int sizeInBytes) override;
```

### StepSequencer

```cpp
void play();
void stop();
void reset();
void selectPattern(int index);           // 0–7
Pattern& getPattern(int index);          // accesso diretto agli step
int  getCurrentStep() const;             // step corrente durante play
int  getCurrentPatternIndex() const;
bool isPlaying() const;
void setStepResolution(int r);
```

### Pattern

```cpp
Step  getStep(int index) const;          // 0–15
void  setStep(int index, const Step& s);
int   getLength() const;
void  setLength(int n);                  // 1–16
```

### TB303Editor (WebView host)

```cpp
// Resource provider: serve ui.html dal BinaryData
std::optional<Resource> getResource(const juce::String& url);

// Timer: invia stateUpdate ogni 50ms se step/pattern cambia
void timerCallback() override;

// Serializzazione stato sequencer
juce::var buildStateVar();
```

---

## 14. DESIGN SYSTEM UI

### Palette colori (AcidLab)

| Token | Valore | Uso |
|-------|--------|-----|
| `--panel` | `#f0f0ec` | Sfondo pannello principale (bianco plastica) |
| `--accent` | `#88ff00` | Verde acido: indicatori attivi, LED, testo LCD |
| `--lcd-bg` | `#070e03` | Sfondo display LCD |
| `--lcd-text` | `#88ff00` | Testo LCD |
| `--text` | `#2a2a28` | Testo principale |
| `--text-dim` | `#888882` | Etichette secondarie |
| `--knob-body` | `#d8d8d4` | Corpo knob plastica ABS |
| `--knob-shadow` | `#b0b0a8` | Ombra knob |
| `--button-active` | `#88ff00` | Pulsante attivo |
| `--button-idle` | `#c8c8c4` | Pulsante idle |

### Tipografia

| Uso | Font | Peso | Dimensione |
|-----|------|------|------------|
| Etichette knob | IBM Plex Mono | 400 | 9–10px |
| Display LCD | IBM Plex Mono | 700 | 14–16px |
| Tab navigation | IBM Plex Sans | 600 | 11px |
| Valori numerici | IBM Plex Mono | 400 | 12px |

### Canvas Knob — Come funziona

Ogni knob nell'UI è un `<canvas>` element disegnato via JavaScript:

```javascript
function drawKnob(canvas, value, label) {
    const ctx = canvas.getContext("2d");
    // 1. Corpo: cerchio con gradiente radiale (bianco plastica)
    // 2. Track: arco grigio da -135° a +135°
    // 3. Fill: arco verde #88ff00 da -135° fino al valore
    // 4. Pointer: linea dal centro al bordo (dot verde)
    // 5. Drag: mousedown → mousemove → calcola angolo → setValue()
}
```

Range angolare: da -135° (min) a +135° (max), totale 270° di rotazione.

### Drag interaction knob

```javascript
canvas.addEventListener("mousedown", (e) => {
    startY = e.clientY;
    startVal = currentValue;
    isDragging = true;
});
document.addEventListener("mousemove", (e) => {
    if(!isDragging) return;
    const delta = (startY - e.clientY) / 150;  // 150px = range completo
    const newVal = Math.max(0, Math.min(1, startVal + delta));
    setValue(newVal);
    redraw();
});
```

---

## 15. NOTE PER LA PRESENTAZIONE

### Punti chiave da enfatizzare

1. **Separazione totale DSP / UI:** Il plugin audio può funzionare senza interfaccia grafica. L'UI è un layer separato che comunica via messaggi — non tocca mai direttamente l'audio.

2. **Fedeltà all'originale TB-303:**
   - VCF Huovilainen con tap a **18 dB/ott**, la pendenza del 303 (non i 24 del Moog)
   - PolyBLEP: stesso anti-aliasing usato nei plugin commerciali
   - Drive **prima** del filtro, così interagisce con la risonanza
   - Slide: durante il portamento l'inviluppo **non** riparte, come nell'hardware
   - Range del cutoff limitato a 120–8000 Hz: con 20–20000 un quarto della corsa
     del knob stava sopra i 6 kHz, dove su un basso non c'è più niente da filtrare

3. **Web-first UI in un plugin audio:**
   - Prima volta che questa tecnica viene usata a livello di tesi (JUCE 8 è uscito nel 2024)
   - L'intera UI è navigabile nel browser come stand-alone prototype
   - Workflow design-to-plugin senza ricompilazioni C++

4. **Preset system completo:**
   - 10 factory preset (synth + sequencer), uno per archetipo di pattern
   - 12 synth preset
   - Persistenza dello stato via XML / ValueTree

### Possibili domande e risposte

**D: Il MIDI mode funziona nel plugin?**
R: La logica C++ c'è ed è completa, ma il bridge JUCE **non è iniettato** in
`Resources/ui.html` (vedi l'avvertenza in §11), quindi il toggle MIDI non si può
accendere dall'interfaccia del plugin e `patternMode` resta false: il MIDI in
arrivo dal DAW suona note singole. Nella pagina web il pattern transpose
funziona. Per abilitarlo nel plugin va riattivato il bridge.

**D: Come viene garantita la sicurezza thread?**
R: I relay JUCE (`WebSliderRelay`) usano meccanismi thread-safe interni. I parametri APVTS possono essere scritti da qualsiasi thread. Il sequencer usa accesso diretto agli step sul thread UI (potenziale race condition nota, documentata nei TODO).

**D: Perché JUCE e non un framework audio puro come iPlug2?**
R: JUCE è lo standard industriale per plugin audio professionali. VST3/AU/AAX support, APVTS, WebView integration, DSP module — tutto in un ecosistema unico.

**D: Come si gestisce la latenza UI?**
R: Il timer a 50ms (20fps) è intenzionalmente basso — l'audio processa a 44100+ Hz. La UI non ha bisogno di latenza sub-millisecondo; 50ms è impercettibile per l'occhio umano.

**D: Il plugin funziona con qualsiasi DAW?**
R: Sì — VST3 (Ableton, Logic, Cubase, Reaper), AU (Logic, GarageBand), Standalone (senza DAW).

### Diagramma visivo suggerito per la presentazione

```
┌─────────────────────────────────────────┐
│              UI (HTML/JS)               │
│  Knob → SliderState → WebSliderRelay    │
│  Button → ToggleState → ToggleRelay     │
│  NativeFunction("setStepNote", ...)     │
└──────────────────┬──────────────────────┘
                   │ JUCE WebBrowserComponent
                   │ (bidirezionale, thread-safe)
┌──────────────────▼──────────────────────┐
│           APVTS (Parameter Store)       │
│  "cutoff" / "resonance" / "tempo" / ... │
└──────────────────┬──────────────────────┘
                   │ Audio thread
┌──────────────────▼──────────────────────┐
│            TB303Engine                  │
│  VCO → VCF → VCA → Distortion          │
│  + Delay + Reverb                       │
└──────────────────┬──────────────────────┘
                   │
┌──────────────────▼──────────────────────┐
│         StepSequencer                   │
│  8 pattern × 16 step                    │
│  sample-accurate timing                 │
└─────────────────────────────────────────┘
```

---

*303 Meow v1.0.0 — CLAUDE.md — allineato al codice il 3 settembre 2026*
*Eugenio Bellini — NABA UI/UX Design Thesis*

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

juce::AudioProcessorValueTreeState::ParameterLayout TB303Processor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "cutoff", "Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.01f, 0.25f), 800.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "resonance", "Resonance",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "envMod", "Env Mod",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "decay", "Decay",
        juce::NormalisableRange<float>(0.05f, 2.0f, 0.001f, 0.4f), 0.3f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "accent", "Accent",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "volume", "Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "tuning", "Tuning",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "waveform", "Waveform",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "distortion", "Distortion",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "tempo", "Tempo",
        juce::NormalisableRange<float>(60.0f, 200.0f, 0.1f), 160.0f));

    // Swing 0..75%: 0 = griglia dritta. Diviso 200 diventa lo scostamento
    // frazionario applicato agli step pari (75% -> 0.375 di step).
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "swing", "Shuffle",
        juce::NormalisableRange<float>(0.0f, 75.0f, 0.1f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "play", "Play",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f));

    // ── FX ────────────────────────────────────────────────────────────────
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "delayTime", "Delay Time",
        juce::NormalisableRange<float>(0.02f, 0.75f, 0.001f), 0.375f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "delayFeedback", "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 0.90f), 0.35f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "delayMix", "Delay Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "reverbSize", "Reverb Size",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "reverbMix", "Reverb Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    return layout;
}

TB303Processor::TB303Processor()
    : AudioProcessor(BusesProperties()
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    pCutoff        = apvts.getRawParameterValue("cutoff");
    pResonance     = apvts.getRawParameterValue("resonance");
    pEnvMod        = apvts.getRawParameterValue("envMod");
    pDecay         = apvts.getRawParameterValue("decay");
    pAccent        = apvts.getRawParameterValue("accent");
    pVolume        = apvts.getRawParameterValue("volume");
    pTuning        = apvts.getRawParameterValue("tuning");
    pWaveform      = apvts.getRawParameterValue("waveform");
    pDistortion    = apvts.getRawParameterValue("distortion");
    pTempo         = apvts.getRawParameterValue("tempo");
    pPlay          = apvts.getRawParameterValue("play");
    pSwing         = apvts.getRawParameterValue("swing");
    pDelayTime     = apvts.getRawParameterValue("delayTime");
    pDelayFeedback = apvts.getRawParameterValue("delayFeedback");
    pDelayMix      = apvts.getRawParameterValue("delayMix");
    pReverbSize    = apvts.getRawParameterValue("reverbSize");
    pReverbMix     = apvts.getRawParameterValue("reverbMix");

    // Default 303 preset così il plugin apre già su un punto di partenza
    // musicale ("Nightshade Run"). Il tempo del preset viene sovrascritto a
    // 160 BPM come richiesto.
    loadPreset(0);
    if (auto* t = apvts.getParameter("tempo"))
        t->setValueNotifyingHost(t->convertTo0to1(160.0f));
}

TB303Processor::~TB303Processor() {}

const juce::String TB303Processor::getName() const { return JucePlugin_Name; }
bool  TB303Processor::acceptsMidi()  const { return true;  }
bool  TB303Processor::producesMidi() const { return false; }
bool  TB303Processor::isMidiEffect() const { return false; }
double TB303Processor::getTailLengthSeconds() const { return 2.0; }

int  TB303Processor::getNumPrograms()             { return 1; }
int  TB303Processor::getCurrentProgram()          { return 0; }
void TB303Processor::setCurrentProgram(int)       {}
const juce::String TB303Processor::getProgramName(int) { return {}; }
void TB303Processor::changeProgramName(int, const juce::String&) {}

bool TB303Processor::hasEditor() const { return true; }

void TB303Processor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock);
    sequencer.prepare(sampleRate);

    delay.prepare(sampleRate, 1500);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<uint32_t>(samplesPerBlock);
    spec.numChannels      = 2;
    reverb.prepare(spec);
    reverb.reset();

    limiter.prepare(spec);
    limiter.setThreshold(-1.0f);   // dBFS: un dB di margine sotto il fondo scala
    // 60 ms: a 160 BPM i sedicesimi distano 94 ms, quindi un rilascio piu' lungo
    // trascinerebbe la riduzione di guadagno di un accento sulla nota dopo e si
    // sentirebbe come pompaggio. Piu' corto invece inizierebbe a seguire la
    // forma d'onda del basso e distorcerebbe.
    limiter.setRelease(60.0f);
    limiter.reset();

    // 20 ms su tutto tranne il cutoff, che ne prende 30: e' quello che si
    // sente di piu' quando salta, ed e' anche quello che si muove di piu'.
    smCutoff    .reset(sampleRate, 0.030);
    smResonance .reset(sampleRate, 0.020);
    smEnvMod    .reset(sampleRate, 0.020);
    smAccent    .reset(sampleRate, 0.020);
    smVolume    .reset(sampleRate, 0.020);
    smDistortion.reset(sampleRate, 0.020);
    smTuning    .reset(sampleRate, 0.020);

    // Partenza sui valori correnti, altrimenti la prima nota dopo il load
    // arriva mentre i parametri stanno ancora rampando dal default.
    smCutoff    .setCurrentAndTargetValue(juce::jmax(20.0f, pCutoff->load()));
    smResonance .setCurrentAndTargetValue(pResonance->load());
    smEnvMod    .setCurrentAndTargetValue(pEnvMod->load());
    smAccent    .setCurrentAndTargetValue(pAccent->load());
    smVolume    .setCurrentAndTargetValue(pVolume->load());
    smDistortion.setCurrentAndTargetValue(pDistortion->load());
    smTuning    .setCurrentAndTargetValue(pTuning->load());
}

void TB303Processor::releaseResources() {}

bool TB303Processor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Reverb requires stereo output
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void TB303Processor::processBlock(juce::AudioBuffer<float>& buffer,
                                   juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // ── Update synth parameters ───────────────────────────────────────────
    // I continui passano dallo smoothing e vengono letti campione per campione
    // nel loop di render; qui si fissa solo il bersaglio.
    smCutoff    .setTargetValue(juce::jmax(20.0f, pCutoff->load()));
    smResonance .setTargetValue(pResonance->load());
    smEnvMod    .setTargetValue(pEnvMod->load());
    smAccent    .setTargetValue(pAccent->load());
    smVolume    .setTargetValue(pVolume->load());
    smDistortion.setTargetValue(pDistortion->load());
    smTuning    .setTargetValue(pTuning->load());

    // Decay e waveform non moltiplicano il segnale: il primo cambia solo la
    // costante di tempo dell'inviluppo, il secondo agisce al prossimo ciclo
    // dell'oscillatore. Nessuno dei due produce uno scalino.
    engine.setDecay    (pDecay->load());
    engine.setWaveform (pWaveform->load() > 0.5f ? 1 : 0);

    // ── Update FX parameters ─────────────────────────────────────────────
    delay.setTime    (pDelayTime->load());
    delay.setFeedback(pDelayFeedback->load());
    delay.setMix     (pDelayMix->load());

    // dryLevel restava a 1.0 mentre il wet saliva: il riverbero non miscelava,
    // sommava, e a mix alto la somma usciva sopra fondo scala prima ancora di
    // arrivare all'uscita. Qui e' un crossfade, quindi il livello resta stabile
    // su tutta la corsa del knob.
    const float revMix = pReverbMix->load();
    juce::dsp::Reverb::Parameters reverbParams;
    reverbParams.roomSize   = pReverbSize->load();
    reverbParams.damping    = 0.55f;   // code un filo meno brillanti
    reverbParams.wetLevel   = revMix;
    reverbParams.dryLevel   = 1.0f - revMix;
    reverbParams.width      = 0.85f;
    reverbParams.freezeMode = 0.0f;
    reverb.setParameters(reverbParams);

    // ── Transport ─────────────────────────────────────────────────────────
    bool shouldPlay  = pPlay->load() > 0.5f;
    bool isMidiMode  = midiMode.load();

    // Cambio di modalita': azzera le note tenute, altrimenti una nota rimasta
    // giu' al momento dello switch lascerebbe il sequencer in trasposizione.
    if (isMidiMode != lastPatternMode)
    {
        releaseAllHeld();
        sequencer.setTranspose(0);
        sequencer.stop();
        lastPatternMode = isMidiMode;
    }

    if (!isMidiMode)
    {
        // Sequencer mode: transport dal pulsante Play, nessuna trasposizione
        if (shouldPlay && !sequencer.isPlaying())  sequencer.play();
        else if (!shouldPlay && sequencer.isPlaying()) sequencer.stop();
    }
    // In MIDI mode il transport lo guidano i note on/off, gestiti nel loop
    // per-campione qui sotto.

    double bpm = static_cast<double>(pTempo->load());

    // ── Sync sequencer BPM once per block ────────────────────────────────
    sequencer.setSwing(pSwing->load() / 200.0f);
    sequencer.syncBpm(getPlayHead(), bpm);

    // ── Per-sample loop: MIDI + sequencer interleaved with audio rendering ─
    auto* left  = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);

    auto midiIt = midiMessages.cbegin();

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        // Gli eventi MIDI scattano al loro samplePosition, non a inizio blocco:
        // stesso motivo per cui il sequencer ora avanza campione per campione.
        while (midiIt != midiMessages.cend() && (*midiIt).samplePosition <= i)
        {
            handleMidiEvent((*midiIt).getMessage(), isMidiMode);
            ++midiIt;
        }

        sequencer.advanceSample(engine);

        engine.setCutoff     (smCutoff.getNextValue());
        engine.setResonance  (smResonance.getNextValue());
        engine.setEnvMod     (smEnvMod.getNextValue());
        engine.setAccentLevel(smAccent.getNextValue());
        engine.setVolume     (smVolume.getNextValue());
        engine.setDistortion (smDistortion.getNextValue());
        engine.setTuning     (smTuning.getNextValue() * 100.0f);  // semitoni → cent

        float mono  = engine.processSample();
        float mixed = delay.processSample(mono);
        left[i]  = mixed;
        right[i] = mixed;
    }

    // ── Reverb (stereo) ───────────────────────────────────────────────────
    juce::dsp::AudioBlock<float>           block(buffer);
    juce::dsp::ProcessContextReplacing<float> ctx(block);
    reverb.process(ctx);

    // ── Stadio di uscita ──────────────────────────────────────────────────
    // Il limiter prende i picchi che restano — autooscillazione del filtro,
    // accenti, code di delay che si sommano — e li tiene sotto -1 dBFS.
    limiter.process(ctx);

    // Rete di sicurezza. Il limiter ha un attacco finito, quindi un transiente
    // abbastanza ripido puo' passare prima che il gain reduction lo prenda;
    // meglio un clamp che un campione fuori scala nel buffer dell'host.
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* d = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            d[i] = juce::jlimit(-1.0f, 1.0f, d[i]);
    }
}

// ── MIDI ──────────────────────────────────────────────────────────────────────
void TB303Processor::releaseAllHeld()
{
    heldCount = 0;
}

void TB303Processor::handleMidiEvent(const juce::MidiMessage& msg, bool patternMode)
{
    if (msg.isNoteOn())
    {
        const int note = msg.getNoteNumber();

        if (!patternMode)
        {
            engine.noteOn(note, msg.getVelocity() > 100, false, note);
            return;
        }

        const bool wasSilent = (heldCount == 0);

        if (heldCount < kMaxHeld)
            heldNotes[static_cast<size_t>(heldCount++)] = note;

        sequencer.setTranspose(note - StepSequencer::PATTERN_ROOT);

        // Un attacco da fermo riparte da step 0; premere una seconda nota
        // mentre la prima e' giu' ritraspone senza resettare la posizione,
        // cosi' il legato nel piano roll scivola di tonalita' a tempo.
        if (wasSilent)
            sequencer.play();
    }
    else if (msg.isNoteOff())
    {
        if (!patternMode)
        {
            engine.noteOff();
            return;
        }

        const int note = msg.getNoteNumber();

        for (int i = 0; i < heldCount; ++i)
        {
            if (heldNotes[static_cast<size_t>(i)] == note)
            {
                for (int j = i; j < heldCount - 1; ++j)
                    heldNotes[static_cast<size_t>(j)] = heldNotes[static_cast<size_t>(j + 1)];
                --heldCount;
                break;
            }
        }

        if (heldCount == 0)
            sequencer.stop();
        else
            sequencer.setTranspose(heldNotes[static_cast<size_t>(heldCount - 1)]
                                   - StepSequencer::PATTERN_ROOT);
    }
    else if (msg.isAllNotesOff() || msg.isAllSoundOff())
    {
        releaseAllHeld();
        if (patternMode) sequencer.stop();
        else             engine.noteOff();
    }
}

juce::AudioProcessorEditor* TB303Processor::createEditor()
{
    return new TB303Editor(*this);
}

// ── Preset loading ────────────────────────────────────────────────────────────
void TB303Processor::loadPreset(int index)
{
    if (index < 0 || index >= static_cast<int>(FACTORY_PRESETS.size())) return;

    const PresetData& p = FACTORY_PRESETS[static_cast<size_t>(index)];

    auto setParam = [&](const char* id, float v)
    {
        if (auto* param = apvts.getParameter(id))
            param->setValueNotifyingHost(
                param->convertTo0to1(v));
    };

    setParam("cutoff",        p.cutoff);
    setParam("resonance",     p.resonance);
    setParam("envMod",        p.envMod);
    setParam("decay",         p.decay);
    setParam("accent",        p.accent);
    setParam("volume",        p.volume);
    setParam("distortion",    p.distortion);
    setParam("tuning",        p.tuning);
    setParam("tempo",         p.tempo);
    setParam("waveform",      static_cast<float>(p.waveform));
    setParam("delayTime",     p.delayTime);
    setParam("delayFeedback", p.delayFeedback);
    setParam("delayMix",      p.delayMix);
    setParam("reverbSize",    p.reverbSize);
    setParam("reverbMix",     p.reverbMix);

    // Load pattern into currently selected pattern slot
    int patIdx = sequencer.getCurrentPatternIndex();
    Pattern& pat = sequencer.getPattern(patIdx);
    pat.setLength(p.patternLength);

    for (int s = 0; s < Pattern::MAX_STEPS; ++s)
    {
        Step step;
        step.note   = p.steps[s].note;
        step.gate   = p.steps[s].gate;
        step.accent = p.steps[s].accent;
        step.slide  = p.steps[s].slide;
        step.octave = p.steps[s].octave;
        pat.setStep(s, step);
    }

    currentPresetIndex      = index;
    currentSynthPresetIndex = -1;  // full preset overrides synth-only
}

void TB303Processor::loadSynthPreset(int index)
{
    if (index < 0 || index >= static_cast<int>(SYNTH_PRESETS.size())) return;

    const SynthPresetData& p = SYNTH_PRESETS[static_cast<size_t>(index)];

    auto setParam = [&](const char* id, float v)
    {
        if (auto* param = apvts.getParameter(id))
            param->setValueNotifyingHost(param->convertTo0to1(v));
    };

    setParam("cutoff",        p.cutoff);
    setParam("resonance",     p.resonance);
    setParam("envMod",        p.envMod);
    setParam("decay",         p.decay);
    setParam("accent",        p.accent);
    setParam("volume",        p.volume);
    setParam("distortion",    p.distortion);
    setParam("tuning",        p.tuning);
    setParam("waveform",      static_cast<float>(p.waveform));
    setParam("delayTime",     p.delayTime);
    setParam("delayFeedback", p.delayFeedback);
    setParam("delayMix",      p.delayMix);
    setParam("reverbSize",    p.reverbSize);
    setParam("reverbMix",     p.reverbMix);

    currentSynthPresetIndex = index;
    currentPresetIndex      = -1;  // synth-only preset clears full preset
}

juce::String TB303Processor::getCurrentPresetName() const
{
    if (currentPresetIndex >= 0 &&
        currentPresetIndex < static_cast<int>(FACTORY_PRESETS.size()))
        return juce::String(FACTORY_PRESETS[static_cast<size_t>(currentPresetIndex)].name);
    return "Custom";
}

juce::String TB303Processor::getDisplayPresetName() const
{
    if (currentPresetIndex >= 0 &&
        currentPresetIndex < static_cast<int>(FACTORY_PRESETS.size()))
        return juce::String(FACTORY_PRESETS[static_cast<size_t>(currentPresetIndex)].name);
    if (currentSynthPresetIndex >= 0 &&
        currentSynthPresetIndex < static_cast<int>(SYNTH_PRESETS.size()))
        return juce::String(SYNTH_PRESETS[static_cast<size_t>(currentSynthPresetIndex)].name);
    return "Custom";
}

// ── State persistence ─────────────────────────────────────────────────────────
void TB303Processor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();

    juce::ValueTree patternsTree("Patterns");
    for (int i = 0; i < StepSequencer::NUM_PATTERNS; ++i)
    {
        auto pt = sequencer.getPattern(i).toValueTree();
        pt.setProperty("index", i, nullptr);
        patternsTree.addChild(pt, -1, nullptr);
    }
    state.addChild(patternsTree, -1, nullptr);
    state.setProperty("currentPattern",    sequencer.getCurrentPatternIndex(), nullptr);
    state.setProperty("currentPreset",     currentPresetIndex, nullptr);
    state.setProperty("currentSynthPreset", currentSynthPresetIndex, nullptr);
    state.setProperty("stepResolution",    sequencer.getStepResolution(), nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TB303Processor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (!xml) return;

    auto state = juce::ValueTree::fromXml(*xml);
    apvts.replaceState(state);

    auto patternsTree = state.getChildWithName("Patterns");
    if (patternsTree.isValid())
    {
        for (auto child : patternsTree)
        {
            int idx = child.getProperty("index", -1);
            if (idx >= 0 && idx < StepSequencer::NUM_PATTERNS)
                sequencer.getPattern(idx).fromValueTree(child);
        }
    }

    int patIdx = state.getProperty("currentPattern", 0);
    sequencer.selectPattern(patIdx);

    currentPresetIndex      = state.getProperty("currentPreset",      -1);
    currentSynthPresetIndex = state.getProperty("currentSynthPreset", -1);
    int stepRes = state.getProperty("stepResolution", 4);
    sequencer.setStepResolution(stepRes);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TB303Processor();
}

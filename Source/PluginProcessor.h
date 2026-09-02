#pragma once
#include <JuceHeader.h>
#include <array>
#include "Synth/TB303Engine.h"
#include "Sequencer/StepSequencer.h"
#include "FX/Delay.h"

class TB303Processor : public juce::AudioProcessor
{
public:
    TB303Processor();
    ~TB303Processor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool  acceptsMidi() const override;
    bool  producesMidi() const override;
    bool  isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int  getNumPrograms() override;
    int  getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Accessors used by the editor
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    StepSequencer& getSequencer()                  { return sequencer; }

    // Preset system
    void loadPreset(int index);
    void loadSynthPreset(int index);
    int  getCurrentPresetIndex()      const { return currentPresetIndex; }
    int  getCurrentSynthPresetIndex() const { return currentSynthPresetIndex; }
    juce::String getCurrentPresetName()    const;
    juce::String getDisplayPresetName()    const;

    // Step resolution
    void setStepResolution(int r)  { sequencer.setStepResolution(r); }
    int  getStepResolution() const { return sequencer.getStepResolution(); }

    // MIDI mode: la nota in arrivo diventa la root del pattern e il sequencer
    // parte finche' la nota resta premuta — comportamento "pattern player" di
    // Acid V/Phoscyon dentro un DAW. Fuori da MIDI mode il MIDI suona note
    // singole sopra il sequencer, come prima.
    void setMidiMode(bool on) { midiMode.store(on); }
    bool getMidiMode() const  { return midiMode.load(); }

private:
    std::atomic<bool> midiMode { false };

    // ── Note tenute in MIDI mode (priorita' all'ultima premuta) ───────────
    static constexpr int kMaxHeld = 16;
    std::array<int, kMaxHeld> heldNotes {};
    int  heldCount = 0;
    bool lastPatternMode = false;

    void handleMidiEvent(const juce::MidiMessage& msg, bool patternMode);
    void releaseAllHeld();
    juce::AudioProcessorValueTreeState apvts;
    TB303Engine   engine;
    StepSequencer sequencer;
    Delay         delay;
    juce::dsp::Reverb reverb;

    // ── Stadio di uscita ─────────────────────────────────────────────────
    // Il VCF in autooscillazione piu' l'accent che somma +6 dB arrivano
    // tranquillamente sopra 0 dBFS: senza niente in fondo alla catena e' il
    // convertitore del DAW a troncare, ed e' la distorsione peggiore che ci
    // sia. Il limiter sta a -1 dBFS e lavora solo sui picchi.
    juce::dsp::Limiter<float> limiter;

    // Smoothing dei parametri che, se saltano a inizio blocco, si sentono:
    // il cutoff perche' rifa' i coefficienti del ladder, gli altri perche'
    // moltiplicano direttamente il segnale.
    // Il cutoff e' moltiplicativo: una rampa lineare da 20 Hz a 20 kHz
    // passerebbe quasi tutto il tempo sopra i 10 kHz, mentre in frequenza
    // l'orecchio ragiona in ottave.
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> smCutoff;
    juce::SmoothedValue<float> smResonance, smEnvMod, smAccent, smVolume,
                               smDistortion, smTuning;

    int           currentPresetIndex      = -1;
    int           currentSynthPresetIndex = -1;

    // Cached parameter pointers (audio thread)
    std::atomic<float>* pCutoff        = nullptr;
    std::atomic<float>* pResonance     = nullptr;
    std::atomic<float>* pEnvMod        = nullptr;
    std::atomic<float>* pDecay         = nullptr;
    std::atomic<float>* pAccent        = nullptr;
    std::atomic<float>* pVolume        = nullptr;
    std::atomic<float>* pTuning        = nullptr;
    std::atomic<float>* pWaveform      = nullptr;
    std::atomic<float>* pDistortion    = nullptr;
    std::atomic<float>* pTempo         = nullptr;
    std::atomic<float>* pPlay          = nullptr;
    std::atomic<float>* pSwing         = nullptr;
    std::atomic<float>* pDelayTime     = nullptr;
    std::atomic<float>* pDelayFeedback = nullptr;
    std::atomic<float>* pDelayMix      = nullptr;
    std::atomic<float>* pReverbSize    = nullptr;
    std::atomic<float>* pReverbMix     = nullptr;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TB303Processor)
};

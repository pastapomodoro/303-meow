#pragma once
#include "Pattern.h"
#include "../Synth/TB303Engine.h"
#include <JuceHeader.h>
#include <array>
#include <atomic>

class StepSequencer
{
public:
    static constexpr int NUM_PATTERNS = 8;

    // Root su cui sono scritti i pattern factory (C3 = riga piu' bassa
    // del piano roll, che copre C3..B4). In MIDI mode la nota
    // in arrivo diventa la nuova root e il pattern viene trasposto di
    // (nota - PATTERN_ROOT) semitoni, come fa Acid V dentro Ableton.
    static constexpr int PATTERN_ROOT = 48;

    StepSequencer();

    void prepare(double sampleRate);

    // Call once per audio block to sync BPM from host playhead
    void syncBpm(juce::AudioPlayHead* playHead, double fallbackBpm);
    // Call once per sample inside the audio loop — fires noteOn when step changes
    void advanceSample(TB303Engine& engine);

    // Transport
    void play();
    void stop();
    void reset();
    bool isPlaying() const;

    // Pattern management (UI thread safe via atomic / copy-on-write)
    void        selectPattern(int index);
    int         getCurrentPatternIndex() const;
    Pattern&    getPattern(int index);
    const Pattern& getPattern(int index) const;

    // Current step for UI highlight
    int getCurrentStep() const;

    // Step resolution: 1=quarter, 2=eighth, 4=sixteenth
    void setStepResolution(int r);
    int  getStepResolution() const { return stepResolution; }

    // Trasposizione del pattern in semitoni (audio thread only)
    void setTranspose(int semitones) { transpose = semitones; }
    int  getTranspose() const        { return transpose; }

    // Swing: allunga gli step pari e accorcia i dispari della stessa quantita',
    // cosi' la durata di ogni coppia resta invariata e il tempo non deriva.
    // 0.0 = griglia dritta. Limitato a 0.45 per non azzerare lo step dispari.
    void setSwing(float amount)
    {
        swing = juce::jlimit(0.0, 0.45, static_cast<double>(amount));
    }

private:
    std::array<Pattern, NUM_PATTERNS> patterns;

    double sampleRate      = 44100.0;
    double samplesPerStep  = 0.0;
    double sampleCounter   = 0.0;

    int currentPatternIdx  = 0;
    int currentStep        = 0;
    int previousNote       = 36;
    bool previousSlide     = false;
    int  stepResolution    = 4;        // 1=quarter, 2=eighth, 4=sixteenth
    int    transpose       = 0;        // semitoni, impostato dalla nota MIDI
    double swing           = 0.0;      // 0..0.45
    bool playing           = false;
    bool firstBeat         = false;   // fires step 0 on first processBlock call

    std::atomic<int> currentStepAtomic { 0 };

    void advanceStep(TB303Engine& engine, double bpm);
    void sendStepEvents(TB303Engine& engine, int step);
};

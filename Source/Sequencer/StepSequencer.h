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

    StepSequencer();

    void prepare(double sampleRate);

    // Called from processBlock — advances sequencer for each sample
    // Returns nothing; fires events directly into the engine
    void processBlock(TB303Engine& engine,
                      juce::AudioPlayHead* playHead,
                      int numSamples,
                      double bpm);

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
    bool playing           = false;
    bool firstBeat         = false;   // fires step 0 on first processBlock call

    std::atomic<int> currentStepAtomic { 0 };

    void advanceStep(TB303Engine& engine, double bpm);
    void sendStepEvents(TB303Engine& engine, int step);
};

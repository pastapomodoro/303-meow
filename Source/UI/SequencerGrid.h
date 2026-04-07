#pragma once
#include <JuceHeader.h>
#include "../Sequencer/Pattern.h"
#include "../Sequencer/StepSequencer.h"

// Compact horizontal step sequencer row used in the main LCD section.
// Layout (within the component bounds, typically 800×220 and positioned on the right):
//   - Top strip:  "STEP SEQUENCER" label and octave mini-row
//   - Middle row: 16 tactile step buttons in a single horizontal line
//   - Bottom row: per-step accent + slide indicator dots

class SequencerGrid : public juce::Component,
                      public juce::Timer
{
public:
    explicit SequencerGrid(StepSequencer& seq);
    ~SequencerGrid() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void timerCallback() override;

private:
    StepSequencer& sequencer;

    // Compact layout metrics (relative to component bounds height, ~220px)
    static constexpr float kTopStripH   = 36.0f;   // label + octave mini-row
    static constexpr float kStepRowH    = 46.0f;   // main step buttons
    static constexpr float kDotRowH     = 20.0f;   // accent/slide dots
    static constexpr float kVerticalGap = 6.0f;

    enum ZoneId {
        Z_NONE   = -1,
        Z_STEP,          // main gate / step on/off
        Z_OCT_M1,
        Z_OCT_0,
        Z_OCT_P1,
        Z_ACCENT,
        Z_SLIDE
    };

    struct HitResult { int step = -1; ZoneId zone = Z_NONE; };

    float       colW()  const { return getWidth() / float(Pattern::MAX_STEPS); }
    HitResult   hitTest(juce::Point<float> p) const;

    void drawStep(juce::Graphics& g, int idx, float x, float w,
                  const Step& s, bool active,
                  bool isPlayhead, bool playing) const;

    static juce::String noteNameStr(int midiNote);
};

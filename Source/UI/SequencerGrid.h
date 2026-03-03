#pragma once
#include <JuceHeader.h>
#include "../Sequencer/Pattern.h"
#include "../Sequencer/StepSequencer.h"

// Per-step column sequencer editor with light warm-cream theme.
// Each of the 16 step columns contains:
//   - Step number label
//   - Gate button (large, shows note name when on)
//   - Note pitch ▲ / name display / ▼
//   - Octave buttons  [-1] [0] [+1]
//   - Accent toggle
//   - Slide toggle
//   - Playhead bar at bottom

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

    // ── Zone y-offsets within each column (total height = kColH = 300) ──────
    static constexpr float kLabelTop  =   0.0f;   // step number label   (h=16)
    static constexpr float kGateTop   =  16.0f;   // gate button         (h=124)
    static constexpr float kNoteUpTop = 140.0f;   // ▲ pitch-up          (h=22)
    static constexpr float kNoteNmTop = 162.0f;   // note name (display) (h=18)
    static constexpr float kNoteDnTop = 180.0f;   // ▼ pitch-down        (h=22)
    static constexpr float kOctTop    = 202.0f;   // octave row          (h=28)
    static constexpr float kAccentTop = 230.0f;   // accent button       (h=30)
    static constexpr float kSlideTop  = 260.0f;   // slide button        (h=30)
    static constexpr float kPlayTop   = 290.0f;   // playhead bar        (h=10)
    static constexpr float kColH      = 300.0f;

    enum ZoneId { Z_NONE=-1, Z_LABEL, Z_GATE,
                  Z_NOTE_UP, Z_NOTE_NM, Z_NOTE_DN,
                  Z_OCT_M1, Z_OCT_0, Z_OCT_P1,
                  Z_ACCENT, Z_SLIDE };

    struct HitResult { int step = -1; ZoneId zone = Z_NONE; };

    float       colW()  const { return getWidth() / float(Pattern::MAX_STEPS); }
    HitResult   hitTest(juce::Point<float> p) const;

    void drawColumn(juce::Graphics& g, int idx, float x, float w,
                    const Step& s, bool active,
                    bool isPlayhead, bool playing) const;

    static juce::String noteNameStr(int midiNote);
};

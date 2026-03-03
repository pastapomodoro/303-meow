#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/SequencerGrid.h"
#include "Presets.h"

//==============================================================================
// Light warm-cream LookAndFeel — orange accents
//==============================================================================
class TB303LookAndFeel : public juce::LookAndFeel_V4
{
public:
    TB303LookAndFeel();

    void drawRotarySlider(juce::Graphics&,
                          int x, int y, int w, int h,
                          float sliderPos,
                          float startAngle, float endAngle,
                          juce::Slider&) override;

    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour&,
                              bool highlighted, bool down) override;

    void drawButtonText(juce::Graphics&, juce::TextButton&,
                        bool highlighted, bool down) override;

    juce::Font getLabelFont(juce::Label&) override;
    juce::Font getTextButtonFont(juce::TextButton&, int) override;
};

//==============================================================================
class TB303Editor : public juce::AudioProcessorEditor,
                    public juce::Timer
{
public:
    explicit TB303Editor(TB303Processor& p);
    ~TB303Editor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    TB303Processor& processor;
    TB303LookAndFeel laf;

    //── Sequencer grid ──────────────────────────────────────────────────────
    SequencerGrid sequencerGrid;

    //── Header: preset dropdowns ─────────────────────────────────────────────
    juce::Label    fullPresetLbl, synthPresetLbl;
    juce::ComboBox fullPresetBox;    // 8 factory full presets
    juce::ComboBox synthPresetBox;   // 12 synth-only presets

    //── Pattern select buttons (8) + step resolution (3) + MIDI mode ───────
    std::array<juce::TextButton, StepSequencer::NUM_PATTERNS> patternButtons;
    std::array<juce::TextButton, 3>                           resButtons;   // 1/16, 1/8, 1/4
    juce::TextButton                                          midiModeButton;

    //── Main knobs (row 1) ───────────────────────────────────────────────────
    juce::Slider cutoffKnob, resonanceKnob, envModKnob, decayKnob;
    juce::Slider accentKnob, volumeKnob, distortionKnob, tuningKnob;
    juce::Label  cutoffLbl, resonanceLbl, envModLbl, decayLbl;
    juce::Label  accentLbl, volumeLbl, distortionLbl, tuningLbl;

    //── FX knobs (row 2) ────────────────────────────────────────────────────
    juce::Slider delayTimeKnob, delayFbKnob, delayMixKnob;
    juce::Slider reverbSizeKnob, reverbMixKnob, tempoKnob;
    juce::Label  delayTimeLbl, delayFbLbl, delayMixLbl;
    juce::Label  reverbSizeLbl, reverbMixLbl, tempoLbl;

    //── Waveform toggle & play button ───────────────────────────────────────
    juce::TextButton waveformButton;
    juce::TextButton playButton;

    //── APVTS attachments ────────────────────────────────────────────────────
    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttach> cutoffAtt, resonanceAtt, envModAtt, decayAtt;
    std::unique_ptr<SliderAttach> accentAtt, volumeAtt, distortionAtt, tuningAtt;
    std::unique_ptr<SliderAttach> delayTimeAtt, delayFbAtt, delayMixAtt;
    std::unique_ptr<SliderAttach> reverbSizeAtt, reverbMixAtt, tempoAtt;

    //── Helpers ──────────────────────────────────────────────────────────────
    void setupKnob(juce::Slider& k, juce::Label& l, const juce::String& name);
    void drawLCDScreen(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawSectionPanel(juce::Graphics& g, juce::Rectangle<int> bounds,
                          const juce::String& title);
    void drawLogo(juce::Graphics& g, juce::Rectangle<int> bounds);

    void updatePatternButtons();
    void updatePlayButton();
    void updateWaveformButton();
    void updateFullPresetBox();
    void updateSynthPresetBox();
    void updateResButtons();
    void updateMidiModeButton();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TB303Editor)
};

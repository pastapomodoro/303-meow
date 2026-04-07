#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/SequencerGrid.h"
#include "Presets.h"

//==============================================================================
// MeowLookAndFeel — dark-metal skeuomorphic + orange/green accents
//==============================================================================
class MeowLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MeowLookAndFeel();

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
    void mouseDown(const juce::MouseEvent& e) override;

private:
    TB303Processor& processor;
    MeowLookAndFeel laf;

    //── Tab ──────────────────────────────────────────────────────────────────
    int currentTab = 0;   // 0=SYNTH  1=SEQ  2=FX  3=PRESET  4=SETTINGS
    void showCurrentTab();

    //── Sequencer grid ───────────────────────────────────────────────────────
    SequencerGrid sequencerGrid;

    //── Presets ──────────────────────────────────────────────────────────────
    juce::Label    fullPresetLbl, synthPresetLbl;
    juce::ComboBox fullPresetBox, synthPresetBox;

    //── Buttons ──────────────────────────────────────────────────────────────
    std::array<juce::TextButton, StepSequencer::NUM_PATTERNS> patternButtons;
    std::array<juce::TextButton, 3>                           resButtons;
    juce::TextButton midiModeButton, playButton, waveformButton, midiExportButton;

    //── Synth knobs ──────────────────────────────────────────────────────────
    juce::Slider cutoffKnob, resonanceKnob, envModKnob, decayKnob;
    juce::Slider accentKnob, volumeKnob, tuningKnob;
    juce::Label  cutoffLbl,  resonanceLbl,  envModLbl,  decayLbl;
    juce::Label  accentLbl,  volumeLbl,     tuningLbl;

    //── FX knobs ─────────────────────────────────────────────────────────────
    juce::Slider distortionKnob, tempoKnob;
    juce::Slider delayTimeKnob,  delayFbKnob,   delayMixKnob;
    juce::Slider reverbSizeKnob, reverbMixKnob;
    juce::Label  distortionLbl,  tempoLbl;
    juce::Label  delayTimeLbl,   delayFbLbl,    delayMixLbl;
    juce::Label  reverbSizeLbl,  reverbMixLbl;

    //── APVTS ────────────────────────────────────────────────────────────────
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SA> cutoffAtt, resonanceAtt, envModAtt, decayAtt;
    std::unique_ptr<SA> accentAtt, volumeAtt, distortionAtt, tuningAtt;
    std::unique_ptr<SA> delayTimeAtt, delayFbAtt, delayMixAtt;
    std::unique_ptr<SA> reverbSizeAtt, reverbMixAtt, tempoAtt;

    //── File chooser ─────────────────────────────────────────────────────────
    std::unique_ptr<juce::FileChooser> fileChooser;

    //── Animation ────────────────────────────────────────────────────────────
    float oscPhase = 0.0f;

    //── Helpers ──────────────────────────────────────────────────────────────
    void setupKnob(juce::Slider& k, juce::Label& l, const juce::String& name);

    void drawTabBar       (juce::Graphics& g);
    void drawSynthTab     (juce::Graphics& g);
    void drawSequencerTab (juce::Graphics& g);
    void drawFxTab        (juce::Graphics& g);
    void drawPresetTab    (juce::Graphics& g);
    void drawSettingsTab  (juce::Graphics& g);

    void drawPanel   (juce::Graphics& g, juce::Rectangle<int> r,
                      const juce::String& title,
                      juce::Colour accent = juce::Colour(0xffff6600));
    void drawLCDPanel(juce::Graphics& g, juce::Rectangle<int> r);
    void drawLogo    (juce::Graphics& g);
    void drawRivet   (juce::Graphics& g, float x, float y, float r = 4.5f);

    void exportMidiPattern();
    void updatePatternButtons();
    void updatePlayButton();
    void updateWaveformButton();
    void updateFullPresetBox();
    void updateSynthPresetBox();
    void updateResButtons();
    void updateMidiModeButton();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TB303Editor)
};

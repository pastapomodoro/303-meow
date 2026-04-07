#include "PluginEditor.h"

// =============================================================================
// COLOR PALETTE  —  matches sketch: dark-metal + orange + green
// =============================================================================
namespace Col
{
    const juce::Colour bg          { 0xff111111 };
    const juce::Colour panel       { 0xff181818 };
    const juce::Colour surface     { 0xff1f1f1f };
    const juce::Colour surface2    { 0xff282828 };
    const juce::Colour border      { 0xff2e2e2e };
    const juce::Colour border2     { 0xff3a3a3a };

    const juce::Colour orange      { 0xffff6600 };
    const juce::Colour orangeDim   { 0xff3a1800 };

    const juce::Colour green       { 0xff00dd55 };
    const juce::Colour greenDim    { 0xff003318 };
    const juce::Colour red         { 0xffff2222 };
    const juce::Colour blue        { 0xff0088ff };
    const juce::Colour blueDim     { 0xff001a33 };

    const juce::Colour lcdBg       { 0xff050d02 };
    const juce::Colour lcdGreen    { 0xffb8e830 };
    const juce::Colour lcdDim      { 0xff2a4008 };

    const juce::Colour text        { 0xffcccccc };
    const juce::Colour textDim     { 0xff666666 };
    const juce::Colour textXs      { 0xff444444 };
}

// =============================================================================
// LAYOUT
// =============================================================================
static constexpr int kW       = 1160;
static constexpr int kH       = 680;
static constexpr int kTabBarH = 48;
static constexpr int kPad     = 14;

static juce::String mono() { return "Menlo"; }

// =============================================================================
// LOOK AND FEEL — constructor
// =============================================================================
MeowLookAndFeel::MeowLookAndFeel()
{
    setColour(juce::Slider::rotarySliderFillColourId,  Col::orange);
    setColour(juce::Slider::thumbColourId,             Col::orange);
    setColour(juce::Slider::trackColourId,             Col::surface2);
    setColour(juce::Slider::textBoxTextColourId,       Col::textXs);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    setColour(juce::Label::textColourId,               Col::textDim);
    setColour(juce::TextButton::buttonColourId,        Col::surface);
    setColour(juce::TextButton::buttonOnColourId,      Col::orangeDim);
    setColour(juce::TextButton::textColourOffId,       Col::textDim);
    setColour(juce::TextButton::textColourOnId,        Col::orange);
    setColour(juce::ComboBox::backgroundColourId,      Col::surface);
    setColour(juce::ComboBox::textColourId,            Col::textDim);
    setColour(juce::ComboBox::arrowColourId,           Col::orange);
    setColour(juce::ComboBox::outlineColourId,         Col::border2);
    setColour(juce::PopupMenu::backgroundColourId,             Col::panel);
    setColour(juce::PopupMenu::textColourId,                   Col::textDim);
    setColour(juce::PopupMenu::highlightedBackgroundColourId,  Col::orangeDim);
    setColour(juce::PopupMenu::highlightedTextColourId,        Col::orange);
}

// =============================================================================
// ROTARY KNOB  —  11-pass skeuomorphic dark-metal knob
// =============================================================================
void MeowLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                       int x, int y, int w, int h,
                                       float pos, float /*startA*/, float /*endA*/,
                                       juce::Slider&)
{
    const float radius = float(juce::jmin(w, h)) * 0.5f - 6.0f;
    const float cx = float(x) + float(w) * 0.5f;
    const float cy = float(y) + float(h) * 0.5f;

    const float startArc = juce::degreesToRadians(-140.0f);
    const float endArc   = juce::degreesToRadians( 140.0f);
    const float valArc   = startArc + pos * (endArc - startArc);

    // 1 — Drop shadow
    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.fillEllipse(cx - radius - 1.0f, cy - radius + 2.0f,
                  (radius + 1.0f) * 2.0f, (radius + 1.0f) * 2.0f);

    // 2 — Ambient glow from arc value
    {
        const float gr = radius + 10.0f;
        juce::ColourGradient halo(Col::orange.withAlpha(0.03f + pos * 0.09f), cx, cy,
                                  juce::Colours::transparentBlack, cx, cy + gr, true);
        g.setGradientFill(halo);
        g.fillEllipse(cx - gr, cy - gr, gr * 2.0f, gr * 2.0f);
    }

    // 3 — Outer metallic bezel ring
    {
        juce::ColourGradient bezel(juce::Colour(0xff525252), cx - radius * 0.6f, cy - radius * 0.7f,
                                   juce::Colour(0xff1a1a1a), cx + radius * 0.6f, cy + radius * 0.7f, false);
        g.setGradientFill(bezel);
        g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
    }

    // 4 — Knob body (dark metal)
    const float bodyR = radius - 3.0f;
    {
        juce::ColourGradient body(juce::Colour(0xff3c3c3c), cx - bodyR * 0.35f, cy - bodyR * 0.45f,
                                  juce::Colour(0xff191919), cx + bodyR * 0.35f, cy + bodyR * 0.55f, false);
        g.setGradientFill(body);
        g.fillEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    }

    // 5 — Engraved inner ring
    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.drawEllipse(cx - bodyR + 1.5f, cy - bodyR + 1.5f,
                  (bodyR - 1.5f) * 2.0f, (bodyR - 1.5f) * 2.0f, 0.7f);

    // 6 — Concentric machined grooves
    g.setColour(juce::Colours::white.withAlpha(0.015f));
    for (float r = bodyR * 0.35f; r < bodyR - 3.0f; r += 2.0f)
        g.drawEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f, 0.4f);

    // 7 — Arc track (engraved groove)
    const float arcR = bodyR - 6.0f;
    {
        juce::Path track;
        track.addArc(cx - arcR, cy - arcR, arcR * 2.0f, arcR * 2.0f,
                     startArc, endArc, true);
        g.setColour(juce::Colour(0xff0a0a0a));
        g.strokePath(track, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    // 8 — Arc fill  (3-pass orange glow)
    if (pos > 0.002f)
    {
        juce::Path fill;
        fill.addArc(cx - arcR, cy - arcR, arcR * 2.0f, arcR * 2.0f,
                    startArc, valArc, true);
        g.setColour(Col::orange.withAlpha(0.14f));
        g.strokePath(fill, juce::PathStrokeType(7.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
        g.setColour(Col::orange.withAlpha(0.40f));
        g.strokePath(fill, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
        g.setColour(Col::orange);
        g.strokePath(fill, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
    }

    // 9 — Pointer line (metallic with shadow)
    {
        const float lineStart = bodyR * 0.20f;
        const float lineEnd   = bodyR * 0.70f;
        float sx = cx + std::sin(valArc) * lineStart;
        float sy = cy - std::cos(valArc) * lineStart;
        float ex = cx + std::sin(valArc) * lineEnd;
        float ey = cy - std::cos(valArc) * lineEnd;

        g.setColour(juce::Colours::black.withAlpha(0.50f));
        g.drawLine(sx + 0.8f, sy + 0.8f, ex + 0.8f, ey + 0.8f, 2.6f);
        g.setColour(juce::Colour(0xffbbbbbb));
        g.drawLine(sx, sy, ex, ey, 1.8f);
        g.setColour(juce::Colours::white.withAlpha(0.35f));
        g.drawLine(sx, sy, ex, ey, 0.5f);
    }

    // 10 — Glowing tip dot
    {
        float ex = cx + std::sin(valArc) * (bodyR * 0.70f);
        float ey = cy - std::cos(valArc) * (bodyR * 0.70f);
        g.setColour(Col::orange.withAlpha(0.30f));
        g.fillEllipse(ex - 4.5f, ey - 4.5f, 9.0f, 9.0f);
        g.setColour(Col::orange);
        g.fillEllipse(ex - 2.2f, ey - 2.2f, 4.4f, 4.4f);
    }

    // 11 — Center cap
    {
        const float capR = bodyR * 0.24f;
        g.setColour(juce::Colours::black.withAlpha(0.50f));
        g.fillEllipse(cx - capR + 0.5f, cy - capR + 0.5f, capR * 2.0f, capR * 2.0f);
        juce::ColourGradient cap(juce::Colour(0xff2a2a2a), cx - capR * 0.3f, cy - capR * 0.4f,
                                 juce::Colour(0xff0e0e0e), cx + capR * 0.3f, cy + capR * 0.4f, false);
        g.setGradientFill(cap);
        g.fillEllipse(cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);
        g.setColour(juce::Colour(0xff444444).withAlpha(0.60f));
        g.drawEllipse(cx - capR, cy - capR, capR * 2.0f, capR * 2.0f, 0.8f);
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.fillEllipse(cx - capR * 0.3f, cy - capR * 0.45f, capR * 0.5f, capR * 0.35f);
    }
}

// =============================================================================
// BUTTON — 3D beveled
// =============================================================================
void MeowLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& btn,
                                            const juce::Colour&,
                                            bool highlighted, bool down)
{
    auto b  = btn.getLocalBounds().toFloat().reduced(0.5f);
    bool on = btn.getToggleState();

    if (on)
    {
        g.setColour(Col::orangeDim);
        g.fillRoundedRectangle(b, 3.0f);
        juce::ColourGradient inner(juce::Colours::black.withAlpha(0.18f), b.getX(), b.getY(),
                                   juce::Colours::transparentBlack,        b.getX(), b.getY() + 6.0f, false);
        g.setGradientFill(inner);
        g.fillRoundedRectangle(b, 3.0f);
        g.setColour(Col::orange);
        g.drawRoundedRectangle(b, 3.0f, 1.0f);
    }
    else
    {
        g.setColour(juce::Colour(0xff0a0a0a));
        g.fillRoundedRectangle(b.translated(0.0f, 1.0f), 3.0f);
        juce::ColourGradient body(juce::Colour(0xff333333), b.getX(), b.getY(),
                                  juce::Colour(0xff1c1c1c), b.getX(), b.getBottom(), false);
        g.setGradientFill(body);
        g.fillRoundedRectangle(b, 3.0f);
        g.setColour(juce::Colours::white.withAlpha(0.04f));
        g.drawLine(b.getX() + 4.0f, b.getY() + 1.0f,
                   b.getRight() - 4.0f, b.getY() + 1.0f, 0.5f);
        g.setColour(Col::border2);
        g.drawRoundedRectangle(b, 3.0f, 0.8f);
    }
    if (highlighted && !on)
    {
        g.setColour(juce::Colours::white.withAlpha(0.03f));
        g.fillRoundedRectangle(b, 3.0f);
    }
    if (down)
    {
        g.setColour(juce::Colours::black.withAlpha(0.16f));
        g.fillRoundedRectangle(b, 3.0f);
    }
}

void MeowLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& btn, bool, bool)
{
    g.setColour(btn.getToggleState() ? Col::orange : Col::textDim);
    g.setFont(getTextButtonFont(btn, btn.getHeight()));
    g.drawText(btn.getButtonText(), btn.getLocalBounds(), juce::Justification::centred, false);
}

juce::Font MeowLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(juce::FontOptions().withName(mono()).withHeight(9.0f));
}
juce::Font MeowLookAndFeel::getTextButtonFont(juce::TextButton&, int)
{
    return juce::Font(juce::FontOptions().withName(mono()).withHeight(9.0f));
}

// =============================================================================
// EDITOR — constructor
// =============================================================================
TB303Editor::TB303Editor(TB303Processor& p)
    : AudioProcessorEditor(p),
      processor(p),
      sequencerGrid(p.getSequencer())
{
    setLookAndFeel(&laf);

    // Presets
    fullPresetLbl.setText("PATTERN PRESET", juce::dontSendNotification);
    addAndMakeVisible(fullPresetLbl);
    fullPresetBox.setTextWhenNothingSelected("-- Select --");
    for (int i = 0; i < (int)FACTORY_PRESETS.size(); ++i)
        fullPresetBox.addItem(FACTORY_PRESETS[(size_t)i].name, i + 1);
    fullPresetBox.onChange = [this]() {
        int sel = fullPresetBox.getSelectedId() - 1;
        if (sel >= 0) { processor.loadPreset(sel); updateSynthPresetBox(); }
    };
    addAndMakeVisible(fullPresetBox);

    synthPresetLbl.setText("SYNTH PRESET", juce::dontSendNotification);
    addAndMakeVisible(synthPresetLbl);
    synthPresetBox.setTextWhenNothingSelected("-- Select --");
    for (int i = 0; i < (int)SYNTH_PRESETS.size(); ++i)
        synthPresetBox.addItem(SYNTH_PRESETS[(size_t)i].name, i + 1);
    synthPresetBox.onChange = [this]() {
        int sel = synthPresetBox.getSelectedId() - 1;
        if (sel >= 0) { processor.loadSynthPreset(sel); updateFullPresetBox(); }
    };
    addAndMakeVisible(synthPresetBox);

    // Pattern buttons
    for (int i = 0; i < StepSequencer::NUM_PATTERNS; ++i)
    {
        patternButtons[(size_t)i].setButtonText("P" + juce::String(i + 1));
        patternButtons[(size_t)i].setClickingTogglesState(false);
        patternButtons[(size_t)i].onClick = [this, i]() {
            processor.getSequencer().selectPattern(i);
            updatePatternButtons();
        };
        addAndMakeVisible(patternButtons[(size_t)i]);
    }

    // Resolution
    const char* resLbls[3] = { "1/16", "1/8", "1/4" };
    const int   resVals[3] = { 4, 2, 1 };
    for (int i = 0; i < 3; ++i)
    {
        resButtons[(size_t)i].setButtonText(resLbls[i]);
        resButtons[(size_t)i].setClickingTogglesState(false);
        resButtons[(size_t)i].onClick = [this, rv = resVals[i]]() {
            processor.setStepResolution(rv);
            updateResButtons();
        };
        addAndMakeVisible(resButtons[(size_t)i]);
    }

    // MIDI mode
    midiModeButton.setButtonText("MIDI IN");
    midiModeButton.setClickingTogglesState(false);
    midiModeButton.onClick = [this]() {
        processor.setMidiMode(!processor.getMidiMode());
        updateMidiModeButton(); updatePlayButton();
    };
    addAndMakeVisible(midiModeButton);

    // Play
    playButton.setButtonText("PLAY");
    playButton.setClickingTogglesState(false);
    playButton.onClick = [this]() {
        auto* param = processor.getAPVTS().getParameter("play");
        if (param) param->setValueNotifyingHost(param->getValue() > 0.5f ? 0.0f : 1.0f);
        updatePlayButton();
    };
    addAndMakeVisible(playButton);

    // Waveform
    waveformButton.setButtonText("SAW");
    waveformButton.setClickingTogglesState(false);
    waveformButton.onClick = [this]() {
        auto* param = processor.getAPVTS().getParameter("waveform");
        if (param) param->setValueNotifyingHost(param->getValue() > 0.5f ? 0.0f : 1.0f);
        updateWaveformButton();
    };
    addAndMakeVisible(waveformButton);

    // MIDI export
    midiExportButton.setButtonText("EXPORT MIDI");
    midiExportButton.setClickingTogglesState(false);
    midiExportButton.onClick = [this]() { exportMidiPattern(); };
    addAndMakeVisible(midiExportButton);

    // Synth knobs
    setupKnob(cutoffKnob,    cutoffLbl,    "CUTOFF");
    setupKnob(resonanceKnob, resonanceLbl, "RESO");
    setupKnob(envModKnob,    envModLbl,    "ENV MOD");
    setupKnob(decayKnob,     decayLbl,     "DECAY");
    setupKnob(accentKnob,    accentLbl,    "ACCENT");
    setupKnob(volumeKnob,    volumeLbl,    "VOLUME");
    setupKnob(tuningKnob,    tuningLbl,    "TUNING");

    // FX knobs
    setupKnob(distortionKnob, distortionLbl, "DRIVE");
    setupKnob(tempoKnob,      tempoLbl,      "TEMPO");
    setupKnob(delayTimeKnob,  delayTimeLbl,  "TIME");
    setupKnob(delayFbKnob,    delayFbLbl,    "FEEDBACK");
    setupKnob(delayMixKnob,   delayMixLbl,   "MIX");
    setupKnob(reverbSizeKnob, reverbSizeLbl, "SIZE");
    setupKnob(reverbMixKnob,  reverbMixLbl,  "MIX");

    // APVTS
    auto& apvts = processor.getAPVTS();
    cutoffAtt     = std::make_unique<SA>(apvts, "cutoff",        cutoffKnob);
    resonanceAtt  = std::make_unique<SA>(apvts, "resonance",     resonanceKnob);
    envModAtt     = std::make_unique<SA>(apvts, "envMod",        envModKnob);
    decayAtt      = std::make_unique<SA>(apvts, "decay",         decayKnob);
    accentAtt     = std::make_unique<SA>(apvts, "accent",        accentKnob);
    volumeAtt     = std::make_unique<SA>(apvts, "volume",        volumeKnob);
    distortionAtt = std::make_unique<SA>(apvts, "distortion",    distortionKnob);
    tuningAtt     = std::make_unique<SA>(apvts, "tuning",        tuningKnob);
    delayTimeAtt  = std::make_unique<SA>(apvts, "delayTime",     delayTimeKnob);
    delayFbAtt    = std::make_unique<SA>(apvts, "delayFeedback", delayFbKnob);
    delayMixAtt   = std::make_unique<SA>(apvts, "delayMix",      delayMixKnob);
    reverbSizeAtt = std::make_unique<SA>(apvts, "reverbSize",    reverbSizeKnob);
    reverbMixAtt  = std::make_unique<SA>(apvts, "reverbMix",     reverbMixKnob);
    tempoAtt      = std::make_unique<SA>(apvts, "tempo",         tempoKnob);

    addAndMakeVisible(sequencerGrid);

    updatePatternButtons(); updatePlayButton(); updateWaveformButton();
    updateFullPresetBox();  updateSynthPresetBox();
    updateResButtons();     updateMidiModeButton();
    showCurrentTab();

    startTimerHz(20);
    setSize(kW, kH);
}

TB303Editor::~TB303Editor() { stopTimer(); setLookAndFeel(nullptr); }

// =============================================================================
// TAB SWITCHING
// =============================================================================
void TB303Editor::showCurrentTab()
{
    auto vis = [](bool v, juce::Component& c) { c.setVisible(v); };
    bool s = (currentTab == 0), q = (currentTab == 1), f = (currentTab == 2);
    bool pr = (currentTab == 3);

    // Synth tab
    vis(s, cutoffKnob); vis(s, cutoffLbl); vis(s, resonanceKnob); vis(s, resonanceLbl);
    vis(s, envModKnob);  vis(s, envModLbl);  vis(s, decayKnob);     vis(s, decayLbl);
    vis(s, accentKnob);  vis(s, accentLbl);  vis(s, volumeKnob);    vis(s, volumeLbl);
    vis(s, tuningKnob);  vis(s, tuningLbl);
    vis(s, playButton);  vis(s, waveformButton); vis(s, midiModeButton); vis(s, midiExportButton);
    for (auto& b : patternButtons) vis(s, b);
    for (auto& b : resButtons)     vis(s, b);

    // Sequencer
    vis(q, sequencerGrid);

    // FX
    vis(f, distortionKnob); vis(f, distortionLbl); vis(f, tempoKnob); vis(f, tempoLbl);
    vis(f, delayTimeKnob);  vis(f, delayTimeLbl);  vis(f, delayFbKnob); vis(f, delayFbLbl);
    vis(f, delayMixKnob);   vis(f, delayMixLbl);
    vis(f, reverbSizeKnob); vis(f, reverbSizeLbl); vis(f, reverbMixKnob); vis(f, reverbMixLbl);

    // Preset
    vis(pr, fullPresetLbl); vis(pr, fullPresetBox);
    vis(pr, synthPresetLbl); vis(pr, synthPresetBox);
}

void TB303Editor::mouseDown(const juce::MouseEvent& e)
{
    if (e.y < 0 || e.y >= kTabBarH) return;
    const int tabsStart = 220, statusW = 130;
    const int tabsW = kW - tabsStart - statusW;
    if (e.x < tabsStart || e.x >= tabsStart + tabsW) return;
    int idx = juce::jlimit(0, 4, (e.x - tabsStart) * 5 / tabsW);
    if (idx != currentTab) { currentTab = idx; showCurrentTab(); resized(); repaint(); }
}

// =============================================================================
// SETUP HELPERS
// =============================================================================
void TB303Editor::setupKnob(juce::Slider& k, juce::Label& l, const juce::String& name)
{
    k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 14);
    k.setLookAndFeel(&laf);
    addAndMakeVisible(k);
    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(8.5f)));
    l.setColour(juce::Label::textColourId, Col::textDim);
    addAndMakeVisible(l);
}

// =============================================================================
// DRAWING — Rivet
// =============================================================================
void TB303Editor::drawRivet(juce::Graphics& g, float x, float y, float r)
{
    g.setColour(juce::Colours::black.withAlpha(0.50f));
    g.fillEllipse(x - r, y - r + 1.0f, r * 2.0f, r * 2.0f);
    juce::ColourGradient body(juce::Colour(0xff484848), x - r * 0.4f, y - r * 0.4f,
                               juce::Colour(0xff1e1e1e), x + r * 0.4f, y + r * 0.4f, false);
    g.setGradientFill(body);
    g.fillEllipse(x - r, y - r, r * 2.0f, r * 2.0f);
    g.setColour(juce::Colours::white.withAlpha(0.13f));
    g.fillEllipse(x - r * 0.35f, y - r * 0.45f, r * 0.50f, r * 0.40f);
    g.setColour(juce::Colour(0xff555555).withAlpha(0.45f));
    g.drawEllipse(x - r, y - r, r * 2.0f, r * 2.0f, 0.6f);
}

// =============================================================================
// DRAWING — Panel (with brushed texture, bevel, header)
// =============================================================================
void TB303Editor::drawPanel(juce::Graphics& g, juce::Rectangle<int> rect,
                             const juce::String& title, juce::Colour accent)
{
    auto r = rect.toFloat();

    // Drop shadow
    g.setColour(juce::Colours::black.withAlpha(0.30f));
    g.fillRoundedRectangle(r.translated(1.5f, 1.5f), 4.0f);

    // Body fill
    {
        juce::ColourGradient bg(Col::panel.brighter(0.03f), r.getX(), r.getY(),
                                Col::panel.darker(0.02f),   r.getX(), r.getBottom(), false);
        g.setGradientFill(bg);
        g.fillRoundedRectangle(r, 4.0f);
    }

    // Inner shadow (top darker)
    {
        juce::ColourGradient top(juce::Colours::black.withAlpha(0.10f), r.getX(), r.getY(),
                                 juce::Colours::transparentBlack,        r.getX(), r.getY() + 10.0f, false);
        g.setGradientFill(top);
        g.fillRoundedRectangle(r, 4.0f);
    }

    // Brushed metal texture (subtle diagonal lines)
    g.saveState();
    g.reduceClipRegion(rect);
    g.setColour(juce::Colours::white.withAlpha(0.012f));
    for (float xi = r.getX() - r.getHeight(); xi < r.getRight() + r.getHeight(); xi += 3.5f)
        g.drawLine(xi, r.getBottom(), xi + r.getHeight(), r.getY(), 0.4f);
    g.restoreState();

    // Bevel highlights
    g.setColour(juce::Colour(0xff383838));
    g.drawLine(r.getX() + 5.0f, r.getY() + 0.5f, r.getRight() - 5.0f, r.getY() + 0.5f, 0.5f);
    g.setColour(juce::Colour(0xff0e0e0e));
    g.drawLine(r.getX() + 5.0f, r.getBottom() - 0.5f,
               r.getRight() - 5.0f, r.getBottom() - 0.5f, 0.5f);

    // Outer border
    g.setColour(Col::border.withAlpha(0.80f));
    g.drawRoundedRectangle(r, 4.0f, 1.0f);

    // Header
    if (title.isNotEmpty())
    {
        auto hdr = rect.withHeight(24).toFloat();
        juce::ColourGradient hBg(Col::surface.brighter(0.02f), hdr.getX(), hdr.getY(),
                                 Col::surface.darker(0.02f),   hdr.getX(), hdr.getBottom(), false);
        g.setGradientFill(hBg);
        g.fillRoundedRectangle(hdr, 4.0f);
        g.fillRect(hdr.getX(), hdr.getBottom() - 4.0f, hdr.getWidth(), 4.0f);

        g.setColour(Col::border.withAlpha(0.50f));
        g.drawLine(hdr.getX() + 8.0f, hdr.getBottom(),
                   hdr.getRight() - 8.0f, hdr.getBottom(), 0.5f);

        // Accent bar
        g.setColour(accent);
        g.fillRoundedRectangle(hdr.getX() + 10.0f, hdr.getY() + 6.0f, 3.0f, 12.0f, 1.5f);

        // Title
        g.setColour(Col::textDim);
        g.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(8.5f)));
        g.drawText(title, juce::Rectangle<float>(hdr.getX() + 20.0f, hdr.getY(),
                   hdr.getWidth() - 28.0f, hdr.getHeight()),
                   juce::Justification::centredLeft, false);
    }
}

// =============================================================================
// DRAWING — Logo (cat-gear + brand)
// =============================================================================
void TB303Editor::drawLogo(juce::Graphics& g)
{
    const float gx = 30.0f, gy = 24.0f, gr = 10.0f;

    // Gear teeth
    g.setColour(Col::border2);
    for (int i = 0; i < 8; ++i)
    {
        float a = float(i) * juce::MathConstants<float>::pi * 0.25f;
        juce::Path tooth;
        tooth.addRectangle(-2.0f, -(gr + 3.5f), 4.0f, 4.5f);
        tooth.applyTransform(juce::AffineTransform::rotation(a).translated(gx, gy));
        g.fillPath(tooth);
    }

    // Gear body
    juce::ColourGradient gearBody(juce::Colour(0xff404040), gx - gr * 0.4f, gy - gr * 0.5f,
                                  juce::Colour(0xff1a1a1a), gx + gr * 0.4f, gy + gr * 0.5f, false);
    g.setGradientFill(gearBody);
    g.fillEllipse(gx - gr, gy - gr, gr * 2.0f, gr * 2.0f);
    g.setColour(Col::bg);
    g.fillEllipse(gx - gr * 0.42f, gy - gr * 0.42f, gr * 0.84f, gr * 0.84f);
    g.setColour(juce::Colour(0xff404040).withAlpha(0.60f));
    g.drawEllipse(gx - gr * 0.42f, gy - gr * 0.42f, gr * 0.84f, gr * 0.84f, 0.8f);

    // Cat ears (green)
    g.setColour(Col::green.withAlpha(0.90f));
    { juce::Path e; e.startNewSubPath(gx-9,gy-gr-1); e.lineTo(gx-5,gy-gr-9); e.lineTo(gx-1,gy-gr-1); e.closeSubPath(); g.fillPath(e); }
    { juce::Path e; e.startNewSubPath(gx+1,gy-gr-1); e.lineTo(gx+5,gy-gr-9); e.lineTo(gx+9,gy-gr-1); e.closeSubPath(); g.fillPath(e); }

    // Eyes
    g.setColour(Col::orange);
    g.fillEllipse(gx - 5.0f, gy - 2.5f, 3.0f, 3.0f);
    g.fillEllipse(gx + 2.0f, gy - 2.5f, 3.0f, 3.0f);

    // Brand
    g.setColour(Col::orange);
    g.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(16.0f).withStyle("Bold")));
    g.drawText("303 MEOW", 54, 8, 160, 26, juce::Justification::centredLeft, false);
    g.setColour(Col::textDim);
    g.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(8.0f)));
    g.drawText("BASS SYNTHESIZER", 56, 30, 160, 12, juce::Justification::centredLeft, false);
}

// =============================================================================
// DRAWING — LCD Panel (CRT phosphor oscilloscope)
// =============================================================================
void TB303Editor::drawLCDPanel(juce::Graphics& g, juce::Rectangle<int> b)
{
    // Bezel
    g.setColour(Col::surface2);
    g.fillRoundedRectangle(b.toFloat().expanded(3.0f), 6.0f);
    g.setColour(Col::border2);
    g.drawRoundedRectangle(b.toFloat().expanded(3.0f), 6.0f, 1.0f);

    // Screen
    auto scr = b.reduced(2);
    {
        juce::ColourGradient bg(Col::lcdBg, float(scr.getX()), float(scr.getY()),
                                juce::Colour(0xff020801), float(scr.getX()), float(scr.getBottom()), false);
        g.setGradientFill(bg);
        g.fillRoundedRectangle(scr.toFloat(), 3.0f);
    }

    g.saveState();
    g.reduceClipRegion(scr);

    // Grid
    g.setColour(Col::lcdDim.withAlpha(0.50f));
    for (int c = 1; c < 5; ++c)
    {
        float x = float(scr.getX()) + float(c) / 5.0f * float(scr.getWidth());
        g.drawLine(x, float(scr.getY()), x, float(scr.getBottom()), 0.4f);
    }
    for (int r = 1; r < 4; ++r)
    {
        float y = float(scr.getY()) + float(r) / 4.0f * float(scr.getHeight());
        g.drawLine(float(scr.getX()), y, float(scr.getRight()), y, 0.4f);
    }

    // Crosshair
    g.setColour(Col::lcdDim.withAlpha(0.80f));
    g.drawLine(float(scr.getCentreX()), float(scr.getY()),
               float(scr.getCentreX()), float(scr.getBottom()), 0.7f);
    g.drawLine(float(scr.getX()), float(scr.getCentreY()),
               float(scr.getRight()), float(scr.getCentreY()), 0.7f);

    // Scanlines
    for (int ly = scr.getY(); ly < scr.getBottom(); ly += 2)
    {
        g.setColour(juce::Colours::black.withAlpha(0.12f));
        g.drawHorizontalLine(ly, float(scr.getX()), float(scr.getRight()));
    }

    // Waveform
    auto& apvts = processor.getAPVTS();
    float cutoff = apvts.getRawParameterValue("cutoff")->load();
    float reso   = apvts.getRawParameterValue("resonance")->load();
    float env    = apvts.getRawParameterValue("envMod")->load();
    float dist   = apvts.getRawParameterValue("distortion")->load();
    float wfP    = apvts.getRawParameterValue("waveform")->load();
    bool  isSqr  = wfP > 0.5f;

    // Normalize cutoff from 20-20000 range to 0-1
    float cutNorm = (cutoff - 20.0f) / 19980.0f;

    const float W    = float(scr.getWidth());
    const float H    = float(scr.getHeight());
    const float midY = float(scr.getCentreY());
    const float twoPi = juce::MathConstants<float>::twoPi;
    const float freq  = 2.0f + cutNorm * 4.5f;
    const float amp   = 0.28f + cutNorm * 0.12f + reso * 0.14f;
    const float resoQ = reso * 0.90f;

    juce::Path wave;
    for (int i = 0; i < 220; ++i)
    {
        float t  = float(i) / 219.0f;
        float ph = t * freq * twoPi + oscPhase;
        float yv = 0.0f;
        if (!isSqr) {
            float pn = std::fmod(ph / twoPi, 1.0f);
            if (pn < 0.0f) pn += 1.0f;
            yv = 2.0f * pn - 1.0f;
        } else {
            float pn = std::fmod(ph / twoPi, 1.0f);
            if (pn < 0.0f) pn += 1.0f;
            yv = pn < 0.5f ? 1.0f : -1.0f;
        }
        float ring = resoQ * std::sin(ph * 2.8f) * std::exp(-t * (1.4f - resoQ * 1.2f));
        yv += ring * 0.45f;
        float envEnv = 0.55f + 0.45f * std::sin(t * juce::MathConstants<float>::pi + env * 0.6f);
        if (dist > 0.02f)
        {
            float thr = juce::jmax(0.08f, 1.0f - dist * 0.62f);
            yv = std::tanh(yv / thr) * thr;
        }
        float px = float(scr.getX()) + t * W;
        float py = midY - yv * amp * H * 0.46f * envEnv;
        if (i == 0) wave.startNewSubPath(px, py); else wave.lineTo(px, py);
    }

    // 4-pass phosphor glow
    g.setColour(Col::lcdGreen.withAlpha(0.06f));
    g.strokePath(wave, juce::PathStrokeType(10.0f));
    g.setColour(Col::lcdGreen.withAlpha(0.14f));
    g.strokePath(wave, juce::PathStrokeType(5.0f));
    g.setColour(Col::lcdGreen.withAlpha(0.40f));
    g.strokePath(wave, juce::PathStrokeType(2.4f));
    g.setColour(Col::lcdGreen);
    g.strokePath(wave, juce::PathStrokeType(1.0f));

    // Status text
    auto inner = scr.reduced(8, 4);
    // Top
    g.setColour(Col::lcdGreen.withAlpha(0.60f));
    g.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(9.0f)));
    g.drawText(juce::String("303 MEOW") + juce::String(isSqr ? "  SQR" : "  SAW"),
               inner.removeFromTop(12), juce::Justification::centredLeft, false);
    // Bottom
    g.setColour(Col::lcdGreen.withAlpha(0.75f));
    g.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(8.0f)));
    juce::String info = "CUT:" + juce::String(int(cutoff)) + "Hz "
        "RES:" + juce::String(reso, 2) + " ENV:" + juce::String(env, 2);
    g.drawText(info, inner.removeFromBottom(12), juce::Justification::centredLeft, false);

    // Scan cursor
    {
        float scanT = std::fmod(oscPhase * 0.12f, 1.0f);
        if (scanT < 0.0f) scanT += 1.0f;
        float scanX = float(scr.getX()) + scanT * W;
        g.setColour(Col::lcdGreen.withAlpha(0.15f));
        g.drawLine(scanX, float(scr.getY()), scanX, float(scr.getBottom()), 1.0f);
    }

    // Vignette
    {
        juce::ColourGradient vig(juce::Colours::transparentBlack,
                                 float(scr.getCentreX()), float(scr.getCentreY()),
                                 juce::Colours::black.withAlpha(0.42f),
                                 float(scr.getX()), float(scr.getY()), true);
        g.setGradientFill(vig);
        g.fillRoundedRectangle(scr.toFloat(), 3.0f);
    }

    g.restoreState();

    // Phosphor border
    g.setColour(Col::lcdGreen.withAlpha(0.08f));
    g.drawRoundedRectangle(scr.toFloat(), 3.0f, 1.2f);
}

// =============================================================================
// DRAWING — Tab Bar
// =============================================================================
void TB303Editor::drawTabBar(juce::Graphics& g)
{
    // Background
    g.setColour(Col::panel);
    g.fillRect(0, 0, kW, kTabBarH);
    g.setColour(Col::border);
    g.drawHorizontalLine(kTabBarH - 1, 0.0f, float(kW));

    drawLogo(g);

    // Tabs
    const char* nums[]   = { "01", "02", "03", "04", "05" };
    const char* labels[] = { "SYNTH", "SEQUENCER", "FX", "PRESET", "SETTINGS" };
    const int tabsStart = 220, statusW = 130;
    const int tabsW     = kW - tabsStart - statusW;
    const int tabW      = tabsW / 5;

    for (int i = 0; i < 5; ++i)
    {
        int tx = tabsStart + i * tabW;
        bool active = (i == currentTab);

        if (active)
        {
            g.setColour(Col::surface);
            g.fillRect(tx, 0, tabW, kTabBarH);
        }

        // Number
        g.setColour(Col::textXs);
        g.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(7.5f)));
        g.drawText(nums[i], tx + 8, 0, 20, kTabBarH, juce::Justification::centredLeft, false);

        // Label
        g.setColour(active ? Col::orange : Col::textDim);
        g.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(9.5f)));
        g.drawText(labels[i], tx + 24, 0, tabW - 30, kTabBarH,
                   juce::Justification::centredLeft, false);

        if (active)
        {
            g.setColour(Col::orange);
            g.fillRect(tx, kTabBarH - 2, tabW, 2);
        }
    }

    // Status
    {
        bool playing = processor.getSequencer().isPlaying();
        float bpm = processor.getAPVTS().getRawParameterValue("tempo")->load();
        float dx = float(kW) - 110.0f, dy = float(kTabBarH) * 0.5f;

        if (playing)
        {
            g.setColour(Col::green.withAlpha(0.20f));
            g.fillEllipse(dx - 7.0f, dy - 7.0f, 14.0f, 14.0f);
        }
        g.setColour(playing ? Col::green : Col::textXs);
        g.fillEllipse(dx - 3.0f, dy - 3.0f, 6.0f, 6.0f);

        g.setColour(Col::textDim);
        g.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(8.0f)));
        g.drawText(juce::String(playing ? "ACTIVE" : "IDLE") + " \xc2\xb7 " + juce::String(int(bpm)) + " BPM",
                   kW - 120, 0, 110, kTabBarH, juce::Justification::centred, false);
    }

    // Corner rivets
    drawRivet(g, 8.0f, 8.0f, 3.5f);
    drawRivet(g, float(kW) - 8.0f, 8.0f, 3.5f);
}

// =============================================================================
// SYNTH TAB
// =============================================================================
void TB303Editor::drawSynthTab(juce::Graphics& g)
{
    const int y0 = kTabBarH + kPad;
    const int col1x = kPad, col1w = 340;
    const int col3w = 220, col3x = kW - kPad - col3w;
    const int col2x = col1x + col1w + 8, col2w = col3x - col2x - 8;

    // Col 1
    drawPanel(g, { col1x, y0,       col1w, 210 }, "DISPLAY");
    drawLCDPanel(g, { col1x + 8, y0 + 28, col1w - 16, 172 });

    drawPanel(g, { col1x, y0 + 218, col1w, 92 },  "TRANSPORT", Col::green);
    drawPanel(g, { col1x, y0 + 318, col1w, 64 },  "PATTERN");
    drawPanel(g, { col1x, y0 + 390, col1w, 210 }, "PRESETS");

    // Col 2
    drawPanel(g, { col2x, y0,       col2w, 130 }, "VCO \xe2\x80\x94 OSCILLATOR");
    drawPanel(g, { col2x, y0 + 138, col2w, 220 }, "VCF \xe2\x80\x94 FILTER");
    drawPanel(g, { col2x, y0 + 366, col2w, 234 }, "VCA \xe2\x80\x94 AMPLIFIER");

    // Col 3
    drawPanel(g, { col3x, y0,       col3w, 150 }, "STEP RES", Col::blue);
    drawPanel(g, { col3x, y0 + 158, col3w, 240 }, "SYNTH INFO");

    // Info rows
    struct Row { const char* l; const char* v; };
    Row rows[] = { {"ENGINE","TB-303"}, {"FILTER","LADDER"}, {"OVERSAMP","2\xc3\x97"}, {"VOICE","MONO"}, {"WAVE", ""} };
    float wfP = processor.getAPVTS().getRawParameterValue("waveform")->load();
    const char* wfStr = wfP > 0.5f ? "SQR" : "SAW";

    g.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(8.5f)));
    for (int i = 0; i < 5; ++i)
    {
        int ry = y0 + 158 + 28 + i * 38;
        g.setColour(Col::textDim);
        g.drawText(rows[i].l, col3x + 14, ry, 100, 14, juce::Justification::centredLeft);
        g.setColour(Col::orange);
        g.drawText(i == 4 ? wfStr : rows[i].v, col3x + 14, ry, col3w - 28, 14,
                   juce::Justification::centredRight);
        if (i < 4) {
            g.setColour(Col::border);
            g.drawHorizontalLine(ry + 26, float(col3x + 10), float(col3x + col3w - 10));
        }
    }
}

// =============================================================================
// SEQUENCER TAB
// =============================================================================
void TB303Editor::drawSequencerTab(juce::Graphics& g)
{
    const int y0 = kTabBarH + kPad;
    drawPanel(g, { kPad, y0, 340, 130 }, "DISPLAY");
    drawLCDPanel(g, { kPad + 8, y0 + 28, 324, 94 });

    drawPanel(g, { kPad, y0 + 138, kW - kPad * 2, kH - y0 - 138 - kPad }, "STEP SEQUENCER");
}

// =============================================================================
// FX TAB
// =============================================================================
void TB303Editor::drawFxTab(juce::Graphics& g)
{
    const int y0 = kTabBarH + kPad;
    const int gap = 10;
    const int panW = (kW - kPad * 2 - gap * 2) / 3;

    drawPanel(g, { kPad,                    y0, panW, 340 }, "DELAY");
    drawPanel(g, { kPad + panW + gap,       y0, panW, 340 }, "REVERB");
    drawPanel(g, { kPad + (panW + gap) * 2, y0, panW, 340 }, "DISTORTION", Col::red);

    drawPanel(g, { kPad, y0 + 350, kW - kPad * 2, 55 }, "FX CHAIN", Col::blue);

    // Chain chips
    const char* chips[] = { "DELAY", "REVERB", "DISTORT", "TEMPO" };
    g.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(8.5f)));
    for (int i = 0; i < 4; ++i)
    {
        int cx = kPad + 12 + i * 100;
        int cy = y0 + 350 + 26;
        g.setColour(Col::surface2);
        g.fillRoundedRectangle(float(cx), float(cy), 88.0f, 20.0f, 3.0f);
        g.setColour(Col::border2);
        g.drawRoundedRectangle(float(cx), float(cy), 88.0f, 20.0f, 3.0f, 0.8f);
        g.setColour(Col::textDim);
        g.drawText(chips[i], cx, cy, 88, 20, juce::Justification::centred, false);
    }
}

// =============================================================================
// PRESET TAB
// =============================================================================
void TB303Editor::drawPresetTab(juce::Graphics& g)
{
    const int y0 = kTabBarH + kPad;
    const int listW = 360;
    const int halfH = (kH - y0 - kPad) / 2 - 4;

    drawPanel(g, { kPad, y0,           listW, halfH }, "FACTORY PRESETS");
    drawPanel(g, { kPad, y0 + halfH + 8, listW, halfH }, "SYNTH PRESETS");

    const int detX = kPad + listW + 10;
    const int detW = kW - detX - kPad;
    drawPanel(g, { detX, y0,       detW, 200 }, "SELECTED PRESET");
    drawLCDPanel(g, { detX + 10, y0 + 30, detW - 20, 130 });
    drawPanel(g, { detX, y0 + 210, detW, halfH * 2 + 8 - 210 }, "USER PRESETS");

    // Factory list
    g.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(9.0f)));
    int curPre = processor.getCurrentPresetIndex();
    for (int i = 0; i < juce::jmin((int)FACTORY_PRESETS.size(), 8); ++i)
    {
        int py = y0 + 28 + i * 34;
        bool sel = (i == curPre);
        if (sel)
        {
            g.setColour(Col::orangeDim);
            g.fillRoundedRectangle(float(kPad + 6), float(py), float(listW - 12), 28.0f, 3.0f);
            g.setColour(Col::orange);
            g.drawRoundedRectangle(float(kPad + 6), float(py), float(listW - 12), 28.0f, 3.0f, 1.0f);
        }
        g.setColour(Col::textXs);
        g.drawText(juce::String(i + 1).paddedLeft('0', 2), kPad + 12, py + 7, 20, 14,
                   juce::Justification::centredLeft);
        g.setColour(sel ? Col::orange : Col::textDim);
        g.drawText(FACTORY_PRESETS[(size_t)i].name, kPad + 36, py + 7, listW - 54, 14,
                   juce::Justification::centredLeft);
    }
}

// =============================================================================
// SETTINGS TAB
// =============================================================================
void TB303Editor::drawSettingsTab(juce::Graphics& g)
{
    const int y0 = kTabBarH + kPad;
    const int panW = (kW - kPad * 2 - 20) / 3;
    const int gap = 10;

    drawPanel(g, { kPad,                    y0, panW, 320 }, "MIDI");
    drawPanel(g, { kPad + panW + gap,       y0, panW, 320 }, "AUDIO", Col::blue);
    drawPanel(g, { kPad + (panW + gap) * 2, y0, panW, 320 }, "ABOUT");

    g.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(8.5f)));

    // MIDI rows
    struct Row { const char* l; const char* v; };
    Row midi[] = { {"CHANNEL","1"}, {"MODE","INTERNAL"}, {"CLOCK","INTERNAL"}, {"THRU","OFF"}, {"NOTE CHASE","ON"} };
    for (int i = 0; i < 5; ++i)
    {
        int ry = y0 + 32 + i * 40;
        g.setColour(Col::textDim);
        g.drawText(midi[i].l, kPad + 14, ry, 120, 14, juce::Justification::centredLeft);
        g.setColour(Col::orange);
        g.drawText(midi[i].v, kPad + 14, ry, panW - 28, 14, juce::Justification::centredRight);
        if (i < 4) { g.setColour(Col::border); g.drawHorizontalLine(ry + 28, float(kPad + 10), float(kPad + panW - 10)); }
    }

    // Audio rows
    Row audio[] = { {"SAMPLE RATE","44100 Hz"}, {"BUFFER","256"}, {"TUNING REF","A = 440 Hz"}, {"OVERSAMPLE","2\xc3\x97"}, {"DC BLOCK","ON"} };
    int ax = kPad + panW + gap;
    for (int i = 0; i < 5; ++i)
    {
        int ry = y0 + 32 + i * 40;
        g.setColour(Col::textDim);
        g.drawText(audio[i].l, ax + 14, ry, 120, 14, juce::Justification::centredLeft);
        g.setColour(Col::orange);
        g.drawText(audio[i].v, ax + 14, ry, panW - 28, 14, juce::Justification::centredRight);
        if (i < 4) { g.setColour(Col::border); g.drawHorizontalLine(ry + 28, float(ax + 10), float(ax + panW - 10)); }
    }

    // About card
    int abx = kPad + (panW + gap) * 2;
    g.setColour(Col::orangeDim);
    g.fillRoundedRectangle(float(abx + 10), float(y0 + 30), float(panW - 20), 270.0f, 4.0f);
    g.setColour(Col::orange);
    g.drawRoundedRectangle(float(abx + 10), float(y0 + 30), float(panW - 20), 270.0f, 4.0f, 1.0f);

    g.setColour(Col::orange);
    g.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(14.0f).withStyle("Bold")));
    g.drawText("303 MEOW", abx + 22, y0 + 46, panW - 44, 18, juce::Justification::topLeft);

    g.setFont(juce::Font(juce::FontOptions().withName(mono()).withHeight(8.5f)));
    g.setColour(Col::textDim);
    const char* lines[] = {
        "Bass Synthesizer \xc2\xb7 v1.1",
        "by @euxeney \xc2\xb7 Eugenio Bellini",
        "NABA \xe2\x80\x94 Tesi UI/UX 2026",
        "",
        "TB-303 Clone \xc2\xb7 JUCE Framework",
        "VST3 \xc2\xb7 AU \xc2\xb7 Standalone",
        "",
        "16-step pattern sequencer",
        "Ladder filter \xc2\xb7 Delay \xc2\xb7 Reverb",
        "MIDI input \xc2\xb7 MIDI export"
    };
    for (int i = 0; i < 10; ++i)
        g.drawText(lines[i], abx + 22, y0 + 72 + i * 18, panW - 44, 14, juce::Justification::topLeft);
}

// =============================================================================
// PAINT
// =============================================================================
void TB303Editor::paint(juce::Graphics& g)
{
    g.fillAll(Col::bg);
    drawTabBar(g);

    switch (currentTab)
    {
        case 0: drawSynthTab(g);     break;
        case 1: drawSequencerTab(g); break;
        case 2: drawFxTab(g);        break;
        case 3: drawPresetTab(g);    break;
        case 4: drawSettingsTab(g);  break;
    }

    // Outer frame + bottom rivets
    g.setColour(Col::border.withAlpha(0.60f));
    g.drawRect(0, 0, kW, kH, 1);
    drawRivet(g, 8.0f, float(kH) - 8.0f, 3.5f);
    drawRivet(g, float(kW) - 8.0f, float(kH) - 8.0f, 3.5f);
}

// =============================================================================
// RESIZED — pixel-perfect component placement
// =============================================================================
void TB303Editor::resized()
{
    const int y0 = kTabBarH + kPad;
    const int col1x = kPad, col1w = 340;
    const int col3w = 220, col3x = kW - kPad - col3w;
    const int col2x = col1x + col1w + 8, col2w = col3x - col2x - 8;

    if (currentTab == 0) // SYNTH
    {
        // Transport
        playButton.setBounds      (col1x + 10, y0 + 226, 78, 26);
        waveformButton.setBounds  (col1x + 96, y0 + 226, 72, 26);
        midiModeButton.setBounds  (col1x + 176, y0 + 226, 72, 26);
        midiExportButton.setBounds(col1x + 10, y0 + 258, 150, 26);

        // Waveform row (reused waveform button above)

        // Pattern buttons
        for (int i = 0; i < StepSequencer::NUM_PATTERNS; ++i)
            patternButtons[(size_t)i].setBounds(col1x + 8 + i * 40, y0 + 326, 34, 22);

        // Resolution (col3)
        for (int i = 0; i < 3; ++i)
            resButtons[(size_t)i].setBounds(col3x + 14, y0 + 28 + i * 36, col3w - 28, 26);

        // Preset boxes
        fullPresetLbl.setBounds (col1x + 10, y0 + 394, col1w - 20, 12);
        fullPresetBox.setBounds (col1x + 10, y0 + 408, col1w - 20, 30);
        synthPresetLbl.setBounds(col1x + 10, y0 + 446, col1w - 20, 12);
        synthPresetBox.setBounds(col1x + 10, y0 + 460, col1w - 20, 30);

        // VCO — tuning knob (centered in 130px panel)
        {
            const int kw = 80, kh = 94; // 80 knob + 14 textbox
            tuningKnob.setBounds(col2x + (col2w - kw) / 2, y0 + 28, kw, kh);
            tuningLbl.setBounds (col2x, y0 + 28 + kh + 2, col2w, 14);
        }

        // VCF — cutoff, reso, envmod, decay, accent (5 knobs)
        {
            const int vcfY  = y0 + 138;
            const int slotW = col2w / 5;
            const int kw = 80, kh = 94;
            const int knobY = vcfY + 28 + (220 - 28 - kh - 14 - 4) / 2;
            juce::Slider* knobs[] = { &cutoffKnob, &resonanceKnob, &envModKnob, &decayKnob, &accentKnob };
            juce::Label*  lbls[]  = { &cutoffLbl,  &resonanceLbl,  &envModLbl,  &decayLbl,  &accentLbl  };
            for (int i = 0; i < 5; ++i)
            {
                knobs[i]->setBounds(col2x + i * slotW + (slotW - kw) / 2, knobY, kw, kh);
                lbls[i]->setBounds (col2x + i * slotW, knobY + kh + 2, slotW, 14);
            }
        }

        // VCA — volume (centered)
        {
            const int vcaY  = y0 + 366;
            const int kw = 80, kh = 94;
            const int knobY = vcaY + 28 + (234 - 28 - kh - 14 - 4) / 2;
            volumeKnob.setBounds(col2x + (col2w / 2 - kw) / 2, knobY, kw, kh);
            volumeLbl.setBounds (col2x, knobY + kh + 2, col2w / 2, 14);
        }
    }
    else if (currentTab == 1) // SEQUENCER
    {
        sequencerGrid.setBounds(kPad + 4, y0 + 138 + 28, kW - kPad * 2 - 8,
                                kH - y0 - 138 - 28 - kPad - 8);
    }
    else if (currentTab == 2) // FX
    {
        const int gap = 10;
        const int panW = (kW - kPad * 2 - gap * 2) / 3;
        const int kw = 72, kh = 86; // 72 knob + 14 textbox
        const int knobY = y0 + 50;
        const int lblH = 14;

        // Delay: time, fb, mix
        {
            const int bx = kPad;
            const int slotW = panW / 3;
            juce::Slider* dk[] = { &delayTimeKnob, &delayFbKnob, &delayMixKnob };
            juce::Label*  dl[] = { &delayTimeLbl,  &delayFbLbl,  &delayMixLbl  };
            for (int i = 0; i < 3; ++i)
            {
                dk[i]->setBounds(bx + i * slotW + (slotW - kw) / 2, knobY, kw, kh);
                dl[i]->setBounds(bx + i * slotW, knobY + kh + 2, slotW, lblH);
            }
        }

        // Reverb: size, mix
        {
            const int bx = kPad + panW + gap;
            const int slotW = panW / 2;
            juce::Slider* rk[] = { &reverbSizeKnob, &reverbMixKnob };
            juce::Label*  rl[] = { &reverbSizeLbl,  &reverbMixLbl  };
            for (int i = 0; i < 2; ++i)
            {
                rk[i]->setBounds(bx + i * slotW + (slotW - kw) / 2, knobY, kw, kh);
                rl[i]->setBounds(bx + i * slotW, knobY + kh + 2, slotW, lblH);
            }
        }

        // Distortion: drive
        {
            const int bx = kPad + (panW + gap) * 2;
            distortionKnob.setBounds(bx + (panW - kw) / 2, knobY, kw, kh);
            distortionLbl.setBounds (bx, knobY + kh + 2, panW, lblH);
        }

        // Tempo knob — in FX chain strip area
        {
            tempoKnob.setBounds(kPad + 440, y0 + 352, 50, 50);
            tempoLbl.setBounds (kPad + 420, y0 + 352 + 40, 90, lblH);
        }
    }
    else if (currentTab == 3) // PRESET
    {
        const int listW = 360;
        const int detX = kPad + listW + 10;
        const int detW = kW - detX - kPad;
        fullPresetLbl.setBounds (detX + 10, y0 + 170, detW - 20, 12);
        fullPresetBox.setBounds (detX + 10, y0 + 184, (detW - 30) / 2, 26);
        synthPresetLbl.setBounds(detX + (detW - 30) / 2 + 20, y0 + 170, detW / 2 - 20, 12);
        synthPresetBox.setBounds(detX + (detW - 30) / 2 + 20, y0 + 184, (detW - 30) / 2, 26);
    }
}

// =============================================================================
// TIMER
// =============================================================================
void TB303Editor::timerCallback()
{
    oscPhase += 0.22f;
    repaint();
    updatePlayButton();
    updateWaveformButton();
    updatePatternButtons();
    updateResButtons();
    updateMidiModeButton();
}

// =============================================================================
// MIDI EXPORT
// =============================================================================
void TB303Editor::exportMidiPattern()
{
    auto&          seq = processor.getSequencer();
    const Pattern& pat = seq.getPattern(seq.getCurrentPatternIndex());
    float bpm          = processor.getAPVTS().getRawParameterValue("tempo")->load();
    int   resolution   = processor.getStepResolution();

    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(480);
    juce::MidiMessageSequence track;
    track.addEvent(juce::MidiMessage::tempoMetaEvent(int(60000000.0 / bpm)), 0.0);
    track.addEvent(juce::MidiMessage::timeSignatureMetaEvent(4, 4), 0.0);
    int tps = 480 / resolution;

    for (int i = 0; i < pat.getLength(); ++i)
    {
        const Step& st = pat.getStep(i);
        if (!st.gate) continue;
        int note = juce::jlimit(0, 127, st.note + st.octave * 12);
        int vel  = st.accent ? 100 : 64;
        double on  = double(i * tps);
        double off = on + double(tps) * 0.85;
        track.addEvent(juce::MidiMessage::noteOn (1, note, (juce::uint8)vel), on);
        track.addEvent(juce::MidiMessage::noteOff(1, note, (juce::uint8)0),   off);
    }
    track.addEvent(juce::MidiMessage::endOfTrack(), double(pat.getLength() * tps));
    track.updateMatchedPairs();
    midiFile.addTrack(track);

    fileChooser = std::make_unique<juce::FileChooser>(
        "Export MIDI Pattern",
        juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
              .getChildFile("303meow_pattern.mid"), "*.mid");
    fileChooser->launchAsync(
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [mf = std::move(midiFile)](const juce::FileChooser& fc) mutable {
            auto res = fc.getResult();
            if (res != juce::File{})
            {
                auto stream = res.createOutputStream();
                if (stream) mf.writeTo(*stream, 0);
            }
        });
}

// =============================================================================
// UPDATE HELPERS
// =============================================================================
void TB303Editor::updatePatternButtons()
{
    int cur = processor.getSequencer().getCurrentPatternIndex();
    for (int i = 0; i < StepSequencer::NUM_PATTERNS; ++i)
        patternButtons[(size_t)i].setToggleState(i == cur, juce::dontSendNotification);
}

void TB303Editor::updatePlayButton()
{
    bool midi = processor.getMidiMode();
    bool playing = processor.getSequencer().isPlaying();
    if (midi)
    {
        playButton.setButtonText("PLAY");
        playButton.setToggleState(false, juce::dontSendNotification);
        playButton.setEnabled(false);
    }
    else
    {
        playButton.setEnabled(true);
        playButton.setButtonText(playing ? "STOP" : "PLAY");
        playButton.setToggleState(playing, juce::dontSendNotification);
    }
}

void TB303Editor::updateWaveformButton()
{
    float wf = processor.getAPVTS().getRawParameterValue("waveform")->load();
    bool sq = wf > 0.5f;
    waveformButton.setButtonText(sq ? "SQR" : "SAW");
    waveformButton.setToggleState(sq, juce::dontSendNotification);
}

void TB303Editor::updateFullPresetBox()
{
    int idx = processor.getCurrentPresetIndex();
    fullPresetBox.setSelectedId(idx >= 0 ? idx + 1 : 0, juce::dontSendNotification);
}

void TB303Editor::updateSynthPresetBox()
{
    int idx = processor.getCurrentSynthPresetIndex();
    synthPresetBox.setSelectedId(idx >= 0 ? idx + 1 : 0, juce::dontSendNotification);
}

void TB303Editor::updateResButtons()
{
    int res = processor.getStepResolution();
    resButtons[0].setToggleState(res == 4, juce::dontSendNotification);
    resButtons[1].setToggleState(res == 2, juce::dontSendNotification);
    resButtons[2].setToggleState(res == 1, juce::dontSendNotification);
}

void TB303Editor::updateMidiModeButton()
{
    bool on = processor.getMidiMode();
    midiModeButton.setToggleState(on, juce::dontSendNotification);
    midiModeButton.setButtonText(on ? "MIDI ON" : "MIDI IN");
}

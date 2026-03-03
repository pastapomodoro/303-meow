#include "PluginEditor.h"

//──────────────────────────────────────────────────────────────────────────────
// Color palette  —  dark futuristic / cyber-minimal
//──────────────────────────────────────────────────────────────────────────────
namespace Col
{
    const juce::Colour bg      { 0xff0b0b18 };   // deep navy bg
    const juce::Colour panel   { 0xff101020 };   // panel bg
    const juce::Colour panelHi { 0xff161628 };   // raised panel surface
    const juce::Colour border  { 0xff1c1c34 };   // dark border
    const juce::Colour cyan    { 0xff00dde8 };   // electric teal-cyan (main accent)
    const juce::Colour cyanD   { 0xff005566 };   // dim cyan
    const juce::Colour cyanL   { 0xff88f0ff };   // bright cyan for glow
    const juce::Colour amber   { 0xffffcc00 };   // amber/gold
    const juce::Colour amberD  { 0xff664400 };   // dim amber
    const juce::Colour text    { 0xffaab8d0 };   // main text (cool blue-grey)
    const juce::Colour textDim { 0xff404860 };   // dim text
    const juce::Colour lcdBg   { 0xff030c05 };   // very dark LCD bg (dark green)
    const juce::Colour lcdTxt  { 0xffffcc44 };   // amber LCD text
    const juce::Colour lcdDim  { 0xff553300 };   // dim amber (LCD inactive)
    const juce::Colour secHdr  { 0xff00aabb };   // section header (dimmer cyan)
    const juce::Colour midiCol { 0xff0077cc };   // blue for MIDI mode
}

// ── Layout constants ─────────────────────────────────────────────────────────
static constexpr int kW         = 1200;
static constexpr int kH         = 660;
static constexpr int kHeaderH   = 70;    // y=0..70
static constexpr int kPatBarH   = 37;    // y=70..107
static constexpr int kSeqH      = 300;   // y=107..407
static constexpr int kSynthSecY = 407;
static constexpr int kSynthSecH = 132;
static constexpr int kFxSecY    = 539;
static constexpr int kFxSecH    = 121;
static constexpr int kKnobH1    = 90;
static constexpr int kKnobH2    = 86;
static constexpr int kLblH1     = 14;
static constexpr int kLblH2     = 12;
static constexpr int kKnobY1    = kSynthSecY + 18;   // 425
static constexpr int kKnobY2    = kFxSecY    + 18;   // 557
static constexpr int kSlotW     = 150;               // 8 × 150 = 1200

//==============================================================================
// TB303LookAndFeel
//==============================================================================
TB303LookAndFeel::TB303LookAndFeel()
{
    setColour(juce::Slider::rotarySliderFillColourId, Col::cyan);
    setColour(juce::Slider::thumbColourId,            Col::cyan);
    setColour(juce::Slider::trackColourId,            Col::border);
    setColour(juce::Label::textColourId,              Col::text);
    setColour(juce::TextButton::buttonColourId,       Col::panel);
    setColour(juce::TextButton::buttonOnColourId,     Col::cyan);
    setColour(juce::TextButton::textColourOffId,      Col::text);
    setColour(juce::TextButton::textColourOnId,       juce::Colour(0xff001a1e));
    setColour(juce::ComboBox::backgroundColourId,     Col::panelHi);
    setColour(juce::ComboBox::textColourId,           Col::text);
    setColour(juce::ComboBox::arrowColourId,          Col::cyan);
    setColour(juce::ComboBox::buttonColourId,         Col::panel);
    setColour(juce::ComboBox::outlineColourId,        Col::border);
    setColour(juce::PopupMenu::backgroundColourId,            Col::panelHi);
    setColour(juce::PopupMenu::textColourId,                  Col::text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, Col::cyanD);
    setColour(juce::PopupMenu::highlightedTextColourId,       Col::cyanL);
}

void TB303LookAndFeel::drawRotarySlider(juce::Graphics& g,
                                         int x, int y, int w, int h,
                                         float pos, float startA, float endA,
                                         juce::Slider&)
{
    const float r  = float(juce::jmin(w, h)) * 0.5f - 3.0f;
    const float cx = float(x) + w * 0.5f;
    const float cy = float(y) + h * 0.5f;
    const float a  = startA + pos * (endA - startA);

    // Dark knob body with subtle radial gradient
    juce::ColourGradient body(juce::Colour(0xff252538), cx - r*0.35f, cy - r*0.45f,
                               juce::Colour(0xff0d0d1c), cx + r*0.40f, cy + r*0.50f, false);
    g.setGradientFill(body);
    g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);

    // Outer rim  (subtle teal ring)
    g.setColour(juce::Colour(0xff2a2a44));
    g.drawEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f, 1.5f);

    // Inner ring glow  (very faint cyan)
    g.setColour(Col::cyan.withAlpha(0.12f));
    g.drawEllipse(cx - r + 2.0f, cy - r + 2.0f, (r-2.0f)*2.0f, (r-2.0f)*2.0f, 1.0f);

    // Arc track
    const float ir = r - 4.5f;
    juce::Path track, fill;
    track.addArc(cx-ir, cy-ir, ir*2, ir*2, startA, endA, true);
    g.setColour(juce::Colour(0xff151528));
    g.strokePath(track, juce::PathStrokeType(3.0f));

    // Arc fill (cyan)
    fill.addArc(cx-ir, cy-ir, ir*2, ir*2, startA, a, true);
    g.setColour(Col::cyan);
    g.strokePath(fill, juce::PathStrokeType(3.0f,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Pointer
    juce::Path ptr;
    ptr.addRectangle(-1.2f, -(r - 4.0f), 2.4f, r * 0.55f);
    ptr.applyTransform(juce::AffineTransform::rotation(a).translated(cx, cy));
    // Glow
    g.setColour(Col::cyan.withAlpha(0.25f));
    g.strokePath(ptr, juce::PathStrokeType(5.0f));
    // Pointer body
    g.setColour(Col::cyanL);
    g.fillPath(ptr);
    // Center dot
    g.setColour(Col::cyan);
    g.fillEllipse(cx - 3.0f, cy - 3.0f, 6.0f, 6.0f);
}

void TB303LookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& btn,
                                             const juce::Colour&,
                                             bool highlighted, bool down)
{
    auto  b  = btn.getLocalBounds().toFloat().reduced(0.5f);
    bool  on = btn.getToggleState();
    bool isMidi = btn.getButtonText().containsIgnoreCase("MIDI");

    juce::Colour topC, botC, borderC;
    if (on && isMidi)
    {
        topC    = Col::midiCol.brighter(0.15f);
        botC    = Col::midiCol.darker(0.35f);
        borderC = Col::midiCol.darker(0.5f);
    }
    else if (on)
    {
        topC    = highlighted ? Col::cyanL : Col::cyan;
        botC    = Col::cyan.darker(0.35f);
        borderC = Col::cyanD;
    }
    else
    {
        topC    = highlighted ? Col::panelHi.brighter(0.08f) : Col::panelHi;
        botC    = Col::panel;
        borderC = Col::border.brighter(0.1f);
    }
    if (down) std::swap(topC, botC);

    juce::ColourGradient gr(topC, b.getX(), b.getY(), botC, b.getX(), b.getBottom(), false);
    g.setGradientFill(gr);
    g.fillRoundedRectangle(b, 3.0f);

    g.setColour(borderC);
    g.drawRoundedRectangle(b, 3.0f, 1.0f);
}

void TB303LookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& btn, bool, bool)
{
    bool on = btn.getToggleState();
    bool isMidi = btn.getButtonText().containsIgnoreCase("MIDI");
    juce::Colour tc;
    if (on && isMidi)   tc = juce::Colours::white;
    else if (on)        tc = juce::Colour(0xff001a1e);  // dark text on cyan
    else                tc = Col::text;
    g.setColour(tc);
    g.setFont(getTextButtonFont(btn, btn.getHeight()));
    g.drawText(btn.getButtonText(), btn.getLocalBounds(), juce::Justification::centred, false);
}

juce::Font TB303LookAndFeel::getLabelFont(juce::Label&)              { return juce::Font(juce::FontOptions(9.5f)); }
juce::Font TB303LookAndFeel::getTextButtonFont(juce::TextButton&, int) { return juce::Font(juce::FontOptions(10.0f, juce::Font::bold)); }

//==============================================================================
// TB303Editor
//==============================================================================
TB303Editor::TB303Editor(TB303Processor& p)
    : AudioProcessorEditor(p),
      processor(p),
      sequencerGrid(p.getSequencer())
{
    setLookAndFeel(&laf);

    // ── Preset dropdowns ─────────────────────────────────────────────────────
    fullPresetLbl.setText("PATTERN PRESET", juce::dontSendNotification);
    fullPresetLbl.setColour(juce::Label::textColourId, Col::secHdr);
    fullPresetLbl.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
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
    synthPresetLbl.setColour(juce::Label::textColourId, Col::secHdr);
    synthPresetLbl.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    addAndMakeVisible(synthPresetLbl);

    synthPresetBox.setTextWhenNothingSelected("-- Select --");
    for (int i = 0; i < (int)SYNTH_PRESETS.size(); ++i)
        synthPresetBox.addItem(SYNTH_PRESETS[(size_t)i].name, i + 1);
    synthPresetBox.onChange = [this]() {
        int sel = synthPresetBox.getSelectedId() - 1;
        if (sel >= 0) { processor.loadSynthPreset(sel); updateFullPresetBox(); }
    };
    addAndMakeVisible(synthPresetBox);

    // ── Pattern buttons ───────────────────────────────────────────────────────
    for (int i = 0; i < StepSequencer::NUM_PATTERNS; ++i)
    {
        patternButtons[i].setButtonText("P" + juce::String(i + 1));
        patternButtons[i].setClickingTogglesState(false);
        patternButtons[i].onClick = [this, i]() {
            processor.getSequencer().selectPattern(i);
            updatePatternButtons();
        };
        addAndMakeVisible(patternButtons[i]);
    }

    // ── Resolution buttons ────────────────────────────────────────────────────
    const char* resLbls[3] = { "1/16", "1/8", "1/4" };
    const int   resVals[3] = { 4, 2, 1 };
    for (int i = 0; i < 3; ++i)
    {
        resButtons[i].setButtonText(resLbls[i]);
        resButtons[i].setClickingTogglesState(false);
        resButtons[i].onClick = [this, rv = resVals[i]]() {
            processor.setStepResolution(rv);
            updateResButtons();
        };
        addAndMakeVisible(resButtons[i]);
    }

    // ── MIDI mode button ──────────────────────────────────────────────────────
    midiModeButton.setButtonText("MIDI IN");
    midiModeButton.setClickingTogglesState(false);
    midiModeButton.onClick = [this]() {
        processor.setMidiMode(!processor.getMidiMode());
        updateMidiModeButton();
        updatePlayButton();
    };
    addAndMakeVisible(midiModeButton);

    // ── Play / Waveform ───────────────────────────────────────────────────────
    playButton.setButtonText("PLAY");
    playButton.setClickingTogglesState(false);
    playButton.onClick = [this]() {
        auto* param = processor.getAPVTS().getParameter("play");
        if (param) param->setValueNotifyingHost(param->getValue() > 0.5f ? 0.0f : 1.0f);
        updatePlayButton();
    };
    addAndMakeVisible(playButton);

    waveformButton.setButtonText("SAW");
    waveformButton.setClickingTogglesState(false);
    waveformButton.onClick = [this]() {
        auto* param = processor.getAPVTS().getParameter("waveform");
        if (param) param->setValueNotifyingHost(param->getValue() > 0.5f ? 0.0f : 1.0f);
        updateWaveformButton();
    };
    addAndMakeVisible(waveformButton);

    // ── Synth knobs ───────────────────────────────────────────────────────────
    setupKnob(cutoffKnob,     cutoffLbl,     "CUTOFF");
    setupKnob(resonanceKnob,  resonanceLbl,  "RESO");
    setupKnob(envModKnob,     envModLbl,     "ENV MOD");
    setupKnob(decayKnob,      decayLbl,      "DECAY");
    setupKnob(accentKnob,     accentLbl,     "ACCENT");
    setupKnob(volumeKnob,     volumeLbl,     "VOLUME");
    setupKnob(distortionKnob, distortionLbl, "DIST");
    setupKnob(tuningKnob,     tuningLbl,     "TUNING");

    // ── FX knobs ──────────────────────────────────────────────────────────────
    setupKnob(delayTimeKnob,  delayTimeLbl,  "DLY TIME");
    setupKnob(delayFbKnob,    delayFbLbl,    "DLY FB");
    setupKnob(delayMixKnob,   delayMixLbl,   "DLY MIX");
    setupKnob(reverbSizeKnob, reverbSizeLbl, "RVB SIZE");
    setupKnob(reverbMixKnob,  reverbMixLbl,  "RVB MIX");
    setupKnob(tempoKnob,      tempoLbl,      "TEMPO");

    // ── APVTS attachments ─────────────────────────────────────────────────────
    auto& apvts = processor.getAPVTS();
    cutoffAtt     = std::make_unique<SliderAttach>(apvts, "cutoff",        cutoffKnob);
    resonanceAtt  = std::make_unique<SliderAttach>(apvts, "resonance",     resonanceKnob);
    envModAtt     = std::make_unique<SliderAttach>(apvts, "envMod",        envModKnob);
    decayAtt      = std::make_unique<SliderAttach>(apvts, "decay",         decayKnob);
    accentAtt     = std::make_unique<SliderAttach>(apvts, "accent",        accentKnob);
    volumeAtt     = std::make_unique<SliderAttach>(apvts, "volume",        volumeKnob);
    distortionAtt = std::make_unique<SliderAttach>(apvts, "distortion",    distortionKnob);
    tuningAtt     = std::make_unique<SliderAttach>(apvts, "tuning",        tuningKnob);
    delayTimeAtt  = std::make_unique<SliderAttach>(apvts, "delayTime",     delayTimeKnob);
    delayFbAtt    = std::make_unique<SliderAttach>(apvts, "delayFeedback", delayFbKnob);
    delayMixAtt   = std::make_unique<SliderAttach>(apvts, "delayMix",      delayMixKnob);
    reverbSizeAtt = std::make_unique<SliderAttach>(apvts, "reverbSize",    reverbSizeKnob);
    reverbMixAtt  = std::make_unique<SliderAttach>(apvts, "reverbMix",     reverbMixKnob);
    tempoAtt      = std::make_unique<SliderAttach>(apvts, "tempo",         tempoKnob);

    addAndMakeVisible(sequencerGrid);

    updatePatternButtons();
    updatePlayButton();
    updateWaveformButton();
    updateFullPresetBox();
    updateSynthPresetBox();
    updateResButtons();
    updateMidiModeButton();

    startTimerHz(15);
    setSize(kW, kH);
}

TB303Editor::~TB303Editor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

//──────────────────────────────────────────────────────────────────────────────
// paint
//──────────────────────────────────────────────────────────────────────────────
void TB303Editor::paint(juce::Graphics& g)
{
    // Window background (flat dark navy)
    g.fillAll(Col::bg);

    // Subtle horizontal scan gradient on background
    juce::ColourGradient bgGrad(Col::bg.brighter(0.04f), 0.f, 0.f,
                                 Col::bg,                 0.f, float(kH), false);
    g.setGradientFill(bgGrad);
    g.fillAll();

    // ── Header panel ──────────────────────────────────────────────────────────
    g.setColour(Col::panelHi);
    g.fillRect(0, 0, kW, kHeaderH);

    // Header bottom border (thin cyan line)
    g.setColour(Col::cyan.withAlpha(0.4f));
    g.fillRect(0.f, float(kHeaderH) - 1.0f, float(kW), 1.5f);

    drawLogo(g, { 0, 0, 190, kHeaderH });
    drawLCDScreen(g, { 635, 5, 460, 60 });

    // ── Pattern bar ───────────────────────────────────────────────────────────
    g.setColour(Col::panel);
    g.fillRect(0, kHeaderH, kW, kPatBarH);
    g.setColour(Col::border.brighter(0.15f));
    g.fillRect(0.f, float(kHeaderH + kPatBarH) - 1.0f, float(kW), 1.0f);

    g.setColour(Col::textDim);
    g.setFont(juce::Font(juce::FontOptions(8.0f, juce::Font::bold)));
    g.drawText("PATTERN",  juce::Rectangle<int>{ 5,   kHeaderH, 72, kPatBarH }, juce::Justification::centredLeft,  false);
    g.drawText("STEP RES", juce::Rectangle<int>{ 724, kHeaderH, 78, kPatBarH }, juce::Justification::centredLeft,  false);
    g.drawText("INPUT",    juce::Rectangle<int>{ 1058, kHeaderH, 50, kPatBarH }, juce::Justification::centredLeft, false);

    // Thin vertical separator in pattern bar
    g.setColour(Col::border.brighter(0.2f));
    g.fillRect(720, kHeaderH + 4, 1, kPatBarH - 8);
    g.fillRect(1054, kHeaderH + 4, 1, kPatBarH - 8);

    // ── Sequencer area ────────────────────────────────────────────────────────
    // (drawn by SequencerGrid component)

    // ── Synth section panel ───────────────────────────────────────────────────
    drawSectionPanel(g, { 0, kSynthSecY, kW, kSynthSecH }, "SYNTHESIZER");

    // ── FX section panel ──────────────────────────────────────────────────────
    drawSectionPanel(g, { 0, kFxSecY, kW, kFxSecH }, "EFFECTS  +  TEMPO");

    // WAVE label
    g.setColour(Col::textDim);
    g.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    g.drawText("WAVE", juce::Rectangle<int>{ 0, kKnobY2 + kKnobH2, kSlotW, kLblH2 },
               juce::Justification::centred, false);

    // Outer window border (thin cyan)
    g.setColour(Col::cyan.withAlpha(0.20f));
    g.drawRect(0.0f, 0.0f, float(kW), float(kH), 1.0f);

    // Corner screws
    auto drawScrew = [&](float sx, float sy) {
        g.setColour(Col::border.brighter(0.3f));
        g.drawEllipse(sx - 5.0f, sy - 5.0f, 10.0f, 10.0f, 1.0f);
        g.setColour(Col::textDim.withAlpha(0.35f));
        g.drawLine(sx - 2.5f, sy,       sx + 2.5f, sy,       0.7f);
        g.drawLine(sx,        sy - 2.5f, sx,        sy + 2.5f, 0.7f);
    };
    drawScrew(10.0f,          10.0f);
    drawScrew(float(kW)-10.0f, 10.0f);
    drawScrew(10.0f,          float(kH)-10.0f);
    drawScrew(float(kW)-10.0f, float(kH)-10.0f);
}

//──────────────────────────────────────────────────────────────────────────────
// resized  —  single setBounds per component, aligned to kSlotW grid
//──────────────────────────────────────────────────────────────────────────────
void TB303Editor::resized()
{
    // ── Header (y=0..70) ─────────────────────────────────────────────────────
    fullPresetLbl.setBounds(190,  7, 112, 14);
    fullPresetBox.setBounds(190, 24, 215, 32);
    synthPresetLbl.setBounds(415,  7, 112, 14);
    synthPresetBox.setBounds(415, 24, 215, 32);
    playButton.setBounds(1110, 13, 80, 44);

    // ── Pattern bar (y=70..107) ───────────────────────────────────────────────
    const int py = kHeaderH + 5, ph = kPatBarH - 10;
    for (int i = 0; i < StepSequencer::NUM_PATTERNS; ++i)
        patternButtons[i].setBounds(80 + i * 78, py, 74, ph);

    resButtons[0].setBounds(806, py, 72, ph);
    resButtons[1].setBounds(882, py, 72, ph);
    resButtons[2].setBounds(958, py, 72, ph);

    midiModeButton.setBounds(1110, py, 82, ph);

    // ── Sequencer (y=107..407) ───────────────────────────────────────────────
    sequencerGrid.setBounds(0, kHeaderH + kPatBarH, kW, kSeqH);

    // ── Synth knobs (y=425..531, 8 slots × 150px) ────────────────────────────
    struct KL { juce::Slider* k; juce::Label* l; };
    KL row1[8] = {
        { &cutoffKnob,     &cutoffLbl     }, { &resonanceKnob, &resonanceLbl },
        { &envModKnob,     &envModLbl     }, { &decayKnob,     &decayLbl     },
        { &accentKnob,     &accentLbl     }, { &volumeKnob,    &volumeLbl    },
        { &distortionKnob, &distortionLbl }, { &tuningKnob,    &tuningLbl    }
    };
    for (int i = 0; i < 8; ++i)
    {
        int sx = i * kSlotW;
        row1[i].k->setBounds(sx + 30, kKnobY1, 90, kKnobH1);
        row1[i].l->setBounds(sx,      kKnobY1 + kKnobH1, kSlotW, kLblH1);
    }

    // ── FX row (y=557..643, 8 slots × 150px) ─────────────────────────────────
    waveformButton.setBounds(20, kKnobY2 + 20, 110, 46);

    KL row2[6] = {
        { &delayTimeKnob,  &delayTimeLbl  }, { &delayFbKnob,    &delayFbLbl    },
        { &delayMixKnob,   &delayMixLbl   }, { &reverbSizeKnob, &reverbSizeLbl },
        { &reverbMixKnob,  &reverbMixLbl  }, { &tempoKnob,      &tempoLbl      }
    };
    for (int i = 0; i < 6; ++i)
    {
        int sx = (i + 1) * kSlotW;
        row2[i].k->setBounds(sx + 30, kKnobY2, 90, kKnobH2);
        row2[i].l->setBounds(sx,      kKnobY2 + kKnobH2, kSlotW, kLblH2);
    }
}

//──────────────────────────────────────────────────────────────────────────────
// timerCallback
//──────────────────────────────────────────────────────────────────────────────
void TB303Editor::timerCallback()
{
    repaint(635, 5, 460, 60);   // LCD region only
    updatePlayButton();
    updateWaveformButton();
    updatePatternButtons();
    updateResButtons();
    updateMidiModeButton();
}

//──────────────────────────────────────────────────────────────────────────────
// setupKnob
//──────────────────────────────────────────────────────────────────────────────
void TB303Editor::setupKnob(juce::Slider& k, juce::Label& l, const juce::String& name)
{
    k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    k.setLookAndFeel(&laf);
    addAndMakeVisible(k);

    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    l.setColour(juce::Label::textColourId, Col::textDim);
    addAndMakeVisible(l);
}

//──────────────────────────────────────────────────────────────────────────────
// drawLCDScreen
//──────────────────────────────────────────────────────────────────────────────
void TB303Editor::drawLCDScreen(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // LCD bezel (dark outline)
    g.setColour(juce::Colour(0xff050504));
    g.fillRoundedRectangle(bounds.toFloat().expanded(3.0f), 5.0f);
    g.setColour(Col::cyan.withAlpha(0.35f));
    g.drawRoundedRectangle(bounds.toFloat().expanded(3.0f), 5.0f, 1.0f);

    // LCD screen bg
    g.setColour(Col::lcdBg);
    g.fillRoundedRectangle(bounds.toFloat(), 3.0f);

    // Scanlines
    for (int ly = bounds.getY(); ly < bounds.getBottom(); ly += 2)
    {
        g.setColour(juce::Colours::black.withAlpha(0.12f));
        g.drawHorizontalLine(ly, float(bounds.getX()), float(bounds.getRight()));
    }

    auto& seq    = processor.getSequencer();
    float bpm    = processor.getAPVTS().getRawParameterValue("tempo")->load();
    float wfP    = processor.getAPVTS().getRawParameterValue("waveform")->load();
    int   step   = seq.getCurrentStep();
    int   patIdx = seq.getCurrentPatternIndex();
    int   patLen = seq.getPattern(patIdx).getLength();
    bool  play   = seq.isPlaying();
    bool  midi   = processor.getMidiMode();

    juce::String preset = processor.getDisplayPresetName();
    const char*  wave   = wfP > 0.5f ? "SQR" : "SAW";

    auto inner = bounds.reduced(8, 5);

    // Line 1 — play indicator, mode, preset, BPM
    juce::String playIco = play ? juce::CharPointer_UTF8("\xe2\x96\xb6") : juce::String("||");
    juce::String modeStr = midi ? juce::String("  \xc2\xbb MIDI") : juce::String("");
    juce::String line1   = playIco + modeStr + "  " + preset + "   BPM: " + juce::String(bpm, 1);

    // Glow pass
    g.setColour(Col::lcdTxt.withAlpha(0.18f));
    g.setFont(juce::Font(juce::FontOptions(11.5f, juce::Font::bold)));
    g.drawText(line1, inner.translated(1, 1), juce::Justification::topLeft, false);
    // Main text
    g.setColour(Col::lcdTxt);
    g.drawText(line1, inner, juce::Justification::topLeft, false);

    // Line 2 — step, pattern, wave, resolution
    juce::String resStr;
    switch (processor.getStepResolution()) { case 1: resStr="1/4"; break; case 2: resStr="1/8"; break; default: resStr="1/16"; }
    juce::String line2 = "STEP: " + juce::String(step+1).paddedLeft('0',2) + "/" + juce::String(patLen)
                       + "   PAT: P" + juce::String(patIdx+1)
                       + "   WAVE: " + wave + "   RES: " + resStr;
    g.setColour(Col::lcdTxt.withAlpha(0.75f));
    g.setFont(juce::Font(juce::FontOptions(9.5f)));
    g.drawText(line2, inner.withY(inner.getY()+17).withHeight(14), juce::Justification::topLeft, false);

    // Step indicator bar (16 cells)
    const int   barH  = 7;
    const int   barY  = bounds.getBottom() - barH - 4;
    const float stepW = (bounds.getWidth() - 16.0f) / Pattern::MAX_STEPS;
    for (int i = 0; i < Pattern::MAX_STEPS; ++i)
    {
        float rx = bounds.getX() + 8.0f + i * stepW;
        bool  lit = (i == step && play);
        // Glow under active step
        if (lit) {
            g.setColour(Col::amber.withAlpha(0.18f));
            g.fillRoundedRectangle(rx - 1.0f, float(barY) - 1.0f, stepW + 2.0f, float(barH) + 2.0f, 2.0f);
        }
        g.setColour(lit ? Col::amber : Col::lcdDim);
        g.fillRoundedRectangle(rx + 0.5f, float(barY), stepW - 1.0f, float(barH), 1.5f);
    }
}

//──────────────────────────────────────────────────────────────────────────────
// drawSectionPanel
//──────────────────────────────────────────────────────────────────────────────
void TB303Editor::drawSectionPanel(juce::Graphics& g, juce::Rectangle<int> bounds,
                                    const juce::String& title)
{
    g.setColour(Col::panel);
    g.fillRect(bounds);

    // Thin cyan top accent line
    g.setColour(Col::cyan.withAlpha(0.55f));
    g.fillRect(bounds.getX(), bounds.getY(), bounds.getWidth(), 1);

    // Section title
    g.setColour(Col::secHdr);
    g.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    g.drawText(title, bounds.withHeight(18), juce::Justification::centred, false);

    // Vertical slot dividers (subtle)
    g.setColour(Col::border.brighter(0.15f));
    for (int i = 1; i < 8; ++i)
        g.fillRect(i * kSlotW, bounds.getY() + 14, 1, bounds.getHeight() - 14);
}

//──────────────────────────────────────────────────────────────────────────────
// drawLogo  — futuristic cyber cat + "303 Meow"
//──────────────────────────────────────────────────────────────────────────────
void TB303Editor::drawLogo(juce::Graphics& g, juce::Rectangle<int> /*bounds*/)
{
    // ── Brand text ────────────────────────────────────────────────────────────
    // Glow layer
    g.setColour(Col::cyan.withAlpha(0.20f));
    g.setFont(juce::Font(juce::FontOptions(26.0f, juce::Font::bold)));
    g.drawText("303 Meow", juce::Rectangle<int>{ 4, 11, 132, 32 }, juce::Justification::centredLeft, false);
    // Main text
    g.setColour(Col::cyan);
    g.drawText("303 Meow", juce::Rectangle<int>{ 5, 10, 132, 32 }, juce::Justification::centredLeft, false);

    // Subtitle
    g.setColour(Col::textDim);
    g.setFont(juce::Font(juce::FontOptions(8.0f)));
    g.drawText("by @benzodiazepics", juce::Rectangle<int>{ 5, 47, 150, 12 }, juce::Justification::centredLeft, false);

    // Thin cyan underline
    g.setColour(Col::cyan.withAlpha(0.28f));
    g.fillRect(5, 44, 145, 1);

    // ── Futuristic cat face ───────────────────────────────────────────────────
    const float cx = 168.0f, cy = 36.0f, r = 27.0f;

    // Outer aura / glow rings
    for (float dr = 8.0f; dr >= 1.0f; dr -= 2.5f) {
        float alpha = (8.0f - dr) / 8.0f * 0.06f;
        g.setColour(Col::cyan.withAlpha(alpha));
        g.drawEllipse(cx - r - dr, cy - r - dr, (r + dr)*2.0f, (r + dr)*2.0f, 1.5f);
    }

    // Ears — sharp geometric triangles
    juce::Path ears;
    // Left ear
    ears.addTriangle(cx - r*0.58f, cy - r*0.74f,
                     cx - r*1.08f, cy - r*1.68f,
                     cx - r*0.04f, cy - r*0.84f);
    // Right ear
    ears.addTriangle(cx + r*0.04f, cy - r*0.84f,
                     cx + r*1.08f, cy - r*1.68f,
                     cx + r*0.58f, cy - r*0.74f);
    // Ear fill (dark)
    g.setColour(Col::bg);
    g.fillPath(ears);
    // Ear outline (cyan glow)
    g.setColour(Col::cyan.withAlpha(0.85f));
    g.strokePath(ears, juce::PathStrokeType(1.2f));
    // Inner ear glow fill
    juce::Path iEars;
    iEars.addTriangle(cx - r*0.50f, cy - r*0.78f,
                      cx - r*0.88f, cy - r*1.48f,
                      cx - r*0.12f, cy - r*0.88f);
    iEars.addTriangle(cx + r*0.12f, cy - r*0.88f,
                      cx + r*0.88f, cy - r*1.48f,
                      cx + r*0.50f, cy - r*0.78f);
    g.setColour(Col::cyan.withAlpha(0.10f));
    g.fillPath(iEars);

    // Head fill (dark body)
    juce::ColourGradient headGrad(juce::Colour(0xff131322), cx - r*0.3f, cy - r*0.4f,
                                   juce::Colour(0xff070714), cx + r*0.3f, cy + r*0.5f, false);
    g.setGradientFill(headGrad);
    g.fillEllipse(cx - r, cy - r, r*2.0f, r*2.0f);
    // Head outline (cyan)
    g.setColour(Col::cyan.withAlpha(0.80f));
    g.drawEllipse(cx - r, cy - r, r*2.0f, r*2.0f, 1.3f);

    // Eyes — almond / slightly angular
    const float eyeX1 = cx - r*0.39f;
    const float eyeX2 = cx + r*0.39f;
    const float eyeY  = cy - r*0.06f;
    const float ew = r*0.40f, eh = r*0.28f;

    // Eye glow (outer halo)
    g.setColour(Col::cyan.withAlpha(0.18f));
    g.fillEllipse(eyeX1 - ew*0.75f, eyeY - eh*0.75f, ew*1.5f, eh*1.5f);
    g.fillEllipse(eyeX2 - ew*0.75f, eyeY - eh*0.75f, ew*1.5f, eh*1.5f);

    // Eye iris (bright cyan)
    g.setColour(Col::cyanL.withAlpha(0.92f));
    g.fillEllipse(eyeX1 - ew*0.5f, eyeY - eh*0.5f, ew, eh);
    g.fillEllipse(eyeX2 - ew*0.5f, eyeY - eh*0.5f, ew, eh);

    // Pupil — thin vertical slit
    const float pw = ew * 0.14f;
    const float ph = eh * 0.88f;
    g.setColour(juce::Colour(0xff02040a));
    g.fillEllipse(eyeX1 - pw*0.5f, eyeY - ph*0.5f, pw, ph);
    g.fillEllipse(eyeX2 - pw*0.5f, eyeY - ph*0.5f, pw, ph);

    // Catchlight (small white spark)
    const float sw = ew * 0.18f, sh = eh * 0.20f;
    g.setColour(juce::Colours::white.withAlpha(0.75f));
    g.fillEllipse(eyeX1 + ew*0.14f, eyeY - eh*0.26f, sw, sh);
    g.fillEllipse(eyeX2 + ew*0.14f, eyeY - eh*0.26f, sw, sh);

    // Nose — small upward triangle, pinkish
    juce::Path nose;
    nose.addTriangle(cx - r*0.085f, cy + r*0.22f,
                     cx + r*0.085f, cy + r*0.22f,
                     cx,            cy + r*0.36f);
    g.setColour(juce::Colour(0xffcc5577));
    g.fillPath(nose);

    // Mouth — twin bezier curves
    juce::Path mouth;
    mouth.startNewSubPath(cx, cy + r*0.36f);
    mouth.cubicTo(cx - r*0.09f, cy + r*0.49f, cx - r*0.25f, cy + r*0.48f, cx - r*0.23f, cy + r*0.42f);
    mouth.startNewSubPath(cx, cy + r*0.36f);
    mouth.cubicTo(cx + r*0.09f, cy + r*0.49f, cx + r*0.25f, cy + r*0.48f, cx + r*0.23f, cy + r*0.42f);
    g.setColour(Col::cyan.withAlpha(0.55f));
    g.strokePath(mouth, juce::PathStrokeType(0.9f));

    // Whiskers — thin, slightly glowing cyan
    g.setColour(Col::cyan.withAlpha(0.40f));
    const float wy = cy + r*0.25f;
    // Left side
    g.drawLine(cx - r*0.94f, wy - r*0.09f, cx - r*0.30f, wy + r*0.02f, 0.85f);
    g.drawLine(cx - r*0.97f, wy + r*0.10f, cx - r*0.30f, wy + r*0.12f, 0.85f);
    g.drawLine(cx - r*0.91f, wy + r*0.28f, cx - r*0.30f, wy + r*0.20f, 0.85f);
    // Right side
    g.drawLine(cx + r*0.30f, wy + r*0.02f, cx + r*0.94f, wy - r*0.09f, 0.85f);
    g.drawLine(cx + r*0.30f, wy + r*0.12f, cx + r*0.97f, wy + r*0.10f, 0.85f);
    g.drawLine(cx + r*0.30f, wy + r*0.20f, cx + r*0.91f, wy + r*0.28f, 0.85f);
}

//──────────────────────────────────────────────────────────────────────────────
// Update helpers
//──────────────────────────────────────────────────────────────────────────────
void TB303Editor::updatePatternButtons()
{
    int cur = processor.getSequencer().getCurrentPatternIndex();
    for (int i = 0; i < StepSequencer::NUM_PATTERNS; ++i)
        patternButtons[i].setToggleState(i == cur, juce::dontSendNotification);
}

void TB303Editor::updatePlayButton()
{
    bool midi    = processor.getMidiMode();
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
    bool  sq = wf > 0.5f;
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

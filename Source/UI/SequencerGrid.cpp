#include "SequencerGrid.h"

// ── Dark-metal + orange palette (matches new sketch) ──────────────────────────
namespace GCol
{
    // Backgrounds
    const juce::Colour panelDark    { 0xff181818 };
    const juce::Colour panelDarker  { 0xff111111 };
    const juce::Colour sectionBg    { 0xff1f1f1f };

    // Borders (unused legacy names kept for compat)
    const juce::Colour copper       { 0xff3a3a3a };
    const juce::Colour copperHi     { 0xff3a3a3a };
    const juce::Colour borderCopper { 0xff2e2e2e };

    // Green — step ON (acid signature)
    const juce::Colour neon         { 0xff00dd55 };
    const juce::Colour neonGlow     { 0xff44ff88 };
    const juce::Colour neonDim      { 0xff003318 };

    // Orange — playhead + accent (primary accent from sketch)
    const juce::Colour amber        { 0xffff6600 };
    const juce::Colour amberGlow    { 0xffff8833 };

    // Labels
    const juce::Colour labelActive  { 0xffff6600 };   // orange header
    const juce::Colour labelDim     { 0xff666666 };   // dim grey

    // Step states
    const juce::Colour stepOn       { 0xff00dd55 };   // green ON
    const juce::Colour stepOff      { 0xff1f1f1f };   // dark OFF
    const juce::Colour stepBorder   { 0xff2e2e2e };   // dark border
    const juce::Colour stepPlaying  { 0xffffffff };   // white playhead

    // Dots
    const juce::Colour accentDotOn  { 0xffff6600 };   // orange accent dot
    const juce::Colour slideDotOn   { 0xff0088ff };   // blue slide dot
    const juce::Colour dotOff       { 0xff282828 };   // dark off

    // Octave buttons
    const juce::Colour octBg        { 0xff181818 };
    const juce::Colour octText      { 0xff666666 };
}

SequencerGrid::SequencerGrid(StepSequencer& seq)
    : sequencer(seq)
{
    startTimerHz(30);
}

SequencerGrid::~SequencerGrid()
{
    stopTimer();
}

void SequencerGrid::timerCallback()
{
    repaint();
}

void SequencerGrid::resized() {}

// ── Hit testing ─────────────────────────────────────────────────────────────
SequencerGrid::HitResult SequencerGrid::hitTest(juce::Point<float> p) const
{
    if (!getLocalBounds().toFloat().contains(p))
        return {};

    const float cw   = colW();
    const int   step = juce::jlimit(0, Pattern::MAX_STEPS - 1, int(p.x / cw));
    const float relX = p.x - float(step) * cw;
    const float relY = p.y;

    const float stepRowTop    = kTopStripH + kVerticalGap;
    const float stepRowBottom = stepRowTop + kStepRowH;
    const float dotRowTop     = stepRowBottom + kVerticalGap;
    const float dotRowBottom  = dotRowTop + kDotRowH;

    if (relY >= stepRowTop && relY <= stepRowBottom)
        return { step, Z_STEP };

    if (relY >= dotRowTop && relY <= dotRowBottom)
    {
        if (relX < cw * 0.5f) return { step, Z_ACCENT };
        return { step, Z_SLIDE };
    }

    if (relY >= kTopStripH - 18.0f && relY <= kTopStripH)
    {
        float third = cw / 3.0f;
        if (relX < third)        return { step, Z_OCT_M1 };
        if (relX < third * 2.0f) return { step, Z_OCT_0  };
        return { step, Z_OCT_P1 };
    }

    return { step, Z_NONE };
}

// ── Note name helper ─────────────────────────────────────────────────────────
juce::String SequencerGrid::noteNameStr(int midiNote)
{
    static const char* names[] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
    int n   = ((midiNote % 12) + 12) % 12;
    int oct = midiNote / 12 - 1;
    return juce::String(names[n]) + juce::String(oct);
}

// ── Draw a single step ───────────────────────────────────────────────────────
void SequencerGrid::drawStep(juce::Graphics& g,
                             int   idx,
                             float x,
                             float w,
                             const Step& s,
                             bool  active,
                             bool  isPlayhead,
                             bool  playing) const
{
    const float stepRowTop    = kTopStripH + kVerticalGap;
    const float stepRowBottom = stepRowTop + kStepRowH;
    const float dotRowTop     = stepRowBottom + kVerticalGap;

    const float bx = x + 2.5f;
    const float bw = w - 5.0f;

    // Step index number (dim, above button)
    g.setColour(GCol::labelDim);
    g.setFont(juce::Font(juce::FontOptions().withName("Menlo").withHeight(9.0f)));
    g.drawText(juce::String(idx + 1),
               juce::Rectangle<float>(bx, 4.0f, bw, 10.0f),
               juce::Justification::centred, false);

    if (!active)
    {
        // Inactive: very dim, dark mahogany
        juce::Rectangle<float> r(bx, stepRowTop, bw, kStepRowH);
        g.setColour(GCol::stepOff.withAlpha(0.7f));
        g.fillRoundedRectangle(r, 4.0f);
        g.setColour(GCol::stepBorder.withAlpha(0.5f));
        g.drawRoundedRectangle(r, 4.0f, 0.8f);
        return;
    }

    // === Main step button ===
    {
        juce::Rectangle<float> r(bx, stepRowTop, bw, kStepRowH);

        if (s.gate)
        {
            // Acid green fill with glow
            juce::ColourGradient gateGrad(
                GCol::neonGlow,  bx,        stepRowTop,
                GCol::neon,      bx,        stepRowBottom, false);
            g.setGradientFill(gateGrad);
            g.fillRoundedRectangle(r, 4.0f);

            // Inner glow border
            g.setColour(GCol::neonGlow.withAlpha(0.30f));
            g.drawRoundedRectangle(r.expanded(1.2f), 5.0f, 1.4f);
        }
        else
        {
            // Off: dark with subtle copper tint
            juce::ColourGradient offGrad(
                GCol::panelDark.brighter(0.04f), bx, stepRowTop,
                GCol::panelDarker,               bx, stepRowBottom, false);
            g.setGradientFill(offGrad);
            g.fillRoundedRectangle(r, 4.0f);
            g.setColour(GCol::stepBorder);
            g.drawRoundedRectangle(r, 4.0f, 0.8f);
        }

        // Playhead — amber outline (SteamPunk playhead — very distinctive)
        if (isPlayhead && playing)
        {
            // Outer amber glow
            g.setColour(GCol::amber.withAlpha(0.22f));
            g.drawRoundedRectangle(r.expanded(3.0f), 6.0f, 3.0f);
            // Main border
            g.setColour(GCol::stepPlaying);
            g.drawRoundedRectangle(r.expanded(1.5f), 5.0f, 2.0f);
        }

        // Note name / gate hint
        int midiNote = juce::jlimit(0, 127, s.note + s.octave * 12);
        juce::String label = s.gate ? noteNameStr(midiNote) : "+";
        g.setFont(juce::Font(juce::FontOptions().withName("Menlo").withHeight(10.0f).withStyle("Bold")));
        g.setColour(s.gate ? juce::Colour(0xff040d05) : GCol::labelDim);
        g.drawText(label, r, juce::Justification::centred, false);
    }

    // === Accent + slide dots ===
    {
        const float dotR   = 4.2f;
        const float dotCxA = bx + bw * 0.25f;
        const float dotCxS = bx + bw * 0.75f;
        const float dotCy  = dotRowTop + kDotRowH * 0.5f;

        auto drawDot = [&](float cx, bool on, juce::Colour onCol)
        {
            juce::Rectangle<float> dr(cx - dotR, dotCy - dotR, dotR * 2.0f, dotR * 2.0f);
            g.setColour(on ? onCol : GCol::dotOff);
            g.fillEllipse(dr);
            if (on)
            {
                g.setColour(onCol.withAlpha(0.65f));
                g.drawEllipse(dr.expanded(1.2f), 1.0f);
            }
        };

        drawDot(dotCxA, s.accent, GCol::accentDotOn);
        drawDot(dotCxS, s.slide,  GCol::slideDotOn);
    }

    // === Octave mini-buttons (in top strip, bottom portion) ===
    {
        const float rowBottom = kTopStripH;
        const float rowTop    = rowBottom - 16.0f;
        const float y         = rowTop + 2.0f;
        const float h         = (rowBottom - 4.0f) - y;
        const float third     = bw / 3.0f;

        struct OctBtn { int val; const char* label; };
        const OctBtn octs[3] = { { -1, "-" }, { 0, "0" }, { +1, "+" } };

        for (int i = 0; i < 3; ++i)
        {
            float ox = bx + float(i) * third;
            juce::Rectangle<float> r(ox + 1.0f, y, third - 2.0f, h);
            bool sel = (s.octave == octs[i].val);

            if (sel)
            {
                juce::ColourGradient selGr(GCol::neonDim.brighter(0.1f), r.getX(), r.getY(),
                                           GCol::neonDim,                 r.getX(), r.getBottom(), false);
                g.setGradientFill(selGr);
                g.fillRoundedRectangle(r, 2.0f);
                g.setColour(GCol::neon.withAlpha(0.6f));
                g.drawRoundedRectangle(r, 2.0f, 0.7f);
            }
            else
            {
                g.setColour(GCol::octBg);
                g.fillRoundedRectangle(r, 2.0f);
                g.setColour(GCol::borderCopper.withAlpha(0.4f));
                g.drawRoundedRectangle(r, 2.0f, 0.5f);
            }

            g.setFont(juce::Font(juce::FontOptions().withName("Menlo").withHeight(8.0f).withStyle("Bold")));
            g.setColour(sel ? GCol::neon : GCol::octText);
            g.drawText(octs[i].label, r, juce::Justification::centred, false);
        }
    }
}

// ── paint ────────────────────────────────────────────────────────────────────
void SequencerGrid::paint(juce::Graphics& g)
{
    // Dark mahogany background
    juce::ColourGradient bg(GCol::panelDarker, 0.0f, 0.0f,
                            GCol::panelDark,   0.0f, float(getHeight()), false);
    g.setGradientFill(bg);
    g.fillAll();

    auto bounds = getLocalBounds().toFloat();

    // === Section header strip ===
    {
        juce::Rectangle<float> hdr = bounds.removeFromTop(kTopStripH);
        juce::Rectangle<float> titleR = hdr.removeFromTop(18.0f);

        // Gradient title bar
        juce::ColourGradient titleBg(GCol::sectionBg,   titleR.getX(), titleR.getY(),
                                     GCol::panelDarker,  titleR.getX(), titleR.getBottom(), false);
        g.setGradientFill(titleBg);
        g.fillRect(titleR);

        // Amber accent glow on title
        g.setColour(GCol::amberGlow.withAlpha(0.05f));
        g.fillRect(titleR);

        // Title text
        g.setColour(GCol::labelActive);
        {
            auto f = juce::Font(juce::FontOptions().withName("Menlo")
                                                    .withHeight(9.0f).withStyle("Bold"));
            g.setFont(f);
        }
        g.drawText("STEP  SEQUENCER",
                   titleR.reduced(8.0f, 0.0f),
                   juce::Justification::centredLeft, false);

        // Copper accent bottom line
        g.setColour(GCol::borderCopper.withAlpha(0.5f));
        g.fillRect(titleR.getX(), titleR.getBottom() - 1.0f, titleR.getWidth(), 1.0f);
    }

    // === Draw all steps ===
    const Pattern& pat     = sequencer.getPattern(sequencer.getCurrentPatternIndex());
    const int      patLen  = pat.getLength();
    const int      curStep = sequencer.getCurrentStep();
    const bool     playing = sequencer.isPlaying();
    const float    cw      = colW();

    for (int i = 0; i < Pattern::MAX_STEPS; ++i)
    {
        const Step& s = pat.getStep(i);
        drawStep(g, i, float(i) * cw, cw, s,
                 /*active*/   i < patLen,
                 /*playhead*/ i == curStep,
                 playing);
    }

    // === Outer frame — copper border ===
    g.setColour(GCol::borderCopper.withAlpha(0.55f));
    g.drawRect(getLocalBounds().toFloat(), 1.0f);
}

// ── Mouse down ───────────────────────────────────────────────────────────────
void SequencerGrid::mouseDown(const juce::MouseEvent& e)
{
    HitResult hr = hitTest(e.position);
    if (hr.step < 0) return;

    Pattern& pat    = sequencer.getPattern(sequencer.getCurrentPatternIndex());
    const int patLen = pat.getLength();

    if (hr.step >= patLen) return;

    Step& s = pat.getStep(hr.step);

    switch (hr.zone)
    {
        case Z_STEP:    s.gate   = !s.gate; break;
        case Z_OCT_M1:  s.octave = -1; break;
        case Z_OCT_0:   s.octave =  0; break;
        case Z_OCT_P1:  s.octave = +1; break;
        case Z_ACCENT:  s.accent = !s.accent; break;
        case Z_SLIDE:   s.slide  = !s.slide; break;
        default: break;
    }

    repaint();
}

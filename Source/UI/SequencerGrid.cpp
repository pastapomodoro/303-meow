#include "SequencerGrid.h"

// ── Dark futuristic / cyber-minimal palette ──────────────────────────────────
namespace GCol {
    const juce::Colour bg       { 0xff0b0b18 };   // deep navy bg
    const juce::Colour colBg    { 0xff101020 };   // step column bg (even)
    const juce::Colour colBgAlt { 0xff0d0d1c };   // step column bg (odd)
    const juce::Colour inactive { 0xff080812 };   // inactive step (darker)
    const juce::Colour gateOn   { 0xff00cce0 };   // electric cyan gate ON
    const juce::Colour gateOff  { 0xff181830 };   // dark gate OFF
    const juce::Colour playGlow { 0xffffcc00 };   // amber playhead glow
    const juce::Colour accent   { 0xffff9900 };   // amber-orange accent
    const juce::Colour slide    { 0xff0088ff };   // blue slide indicator
    const juce::Colour text     { 0xffaab8d0 };   // cool blue-grey text
    const juce::Colour textDim  { 0xff404868 };   // dim text
    const juce::Colour border   { 0xff1c1c34 };   // column border
    const juce::Colour btnBg    { 0xff161628 };   // control button bg
    const juce::Colour btnAct   { 0xff00aabb };   // active control button (dim cyan)
    const juce::Colour octNeg   { 0xff1a5090 };   // -1 octave (blue)
    const juce::Colour octZero  { 0xff1a5050 };   //  0 octave (teal)
    const juce::Colour octPos   { 0xff705020 };   // +1 octave (amber-brown)
    const juce::Colour noteBtn  { 0xff2a3448 };   // ▲▼ button tint
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

void SequencerGrid::resized() {}   // layout computed on-the-fly in paint/hitTest

// ── Hit testing ─────────────────────────────────────────────────────────────
SequencerGrid::HitResult SequencerGrid::hitTest(juce::Point<float> p) const
{
    if (!getLocalBounds().toFloat().contains(p))
        return {};

    const float cw   = colW();
    const int   step = juce::jlimit(0, Pattern::MAX_STEPS - 1, int(p.x / cw));
    const float relX = p.x - step * cw;
    const float relY = p.y;

    if (relY < kGateTop)   return { step, Z_LABEL };
    if (relY < kNoteUpTop) return { step, Z_GATE };
    if (relY < kNoteNmTop) return { step, Z_NOTE_UP };
    if (relY < kNoteDnTop) return { step, Z_NOTE_NM };  // display only
    if (relY < kOctTop)    return { step, Z_NOTE_DN };
    if (relY < kAccentTop) {
        // Octave zone: split horizontally into 3
        float third = cw / 3.0f;
        if (relX < third)         return { step, Z_OCT_M1 };
        if (relX < third * 2.0f)  return { step, Z_OCT_0  };
        return { step, Z_OCT_P1 };
    }
    if (relY < kSlideTop)  return { step, Z_ACCENT };
    if (relY < kPlayTop)   return { step, Z_SLIDE  };
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

// ── Draw a single step column ────────────────────────────────────────────────
void SequencerGrid::drawColumn(juce::Graphics& g,
                                int   idx,
                                float x,
                                float w,
                                const Step& s,
                                bool  active,
                                bool  isPlayhead,
                                bool  playing) const
{
    const float gap  = 1.5f;          // gap between columns
    const float bx   = x + gap;
    const float bw   = w - gap * 2.0f;
    const juce::Colour colBg = (idx % 2 == 0) ? GCol::colBg : GCol::colBgAlt;

    // ── Column background ─────────────────────────────────────────────────
    g.setColour(active ? colBg : GCol::inactive);
    g.fillRect(bx, 0.0f, bw, kColH);

    // Playhead glow border
    if (isPlayhead && playing)
    {
        g.setColour(GCol::playGlow.withAlpha(0.35f));
        g.fillRect(bx, 0.0f, bw, kColH);
        g.setColour(GCol::playGlow);
        g.drawRect(bx, 0.0f, bw, kColH, 2.0f);
    }

    if (!active)
    {
        // Inactive: just show dim step number and return
        g.setColour(GCol::textDim);
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText(juce::String(idx + 1),
                   juce::Rectangle<float>(bx, kLabelTop + 1.0f, bw, kGateTop - 2.0f),
                   juce::Justification::centred, false);
        return;
    }

    // ── Step number label ─────────────────────────────────────────────────
    {
        g.setColour(isPlayhead ? GCol::playGlow : GCol::textDim);
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText(juce::String(idx + 1),
                   juce::Rectangle<float>(bx, kLabelTop + 1.0f, bw, kGateTop - 2.0f),
                   juce::Justification::centred, false);
    }

    // ── Gate button ───────────────────────────────────────────────────────
    {
        const float gy = kGateTop + 2.0f;
        const float gh = kNoteUpTop - kGateTop - 4.0f;
        juce::Rectangle<float> gateR(bx + 2.0f, gy, bw - 4.0f, gh);

        if (s.gate)
        {
            juce::ColourGradient grad(GCol::gateOn.brighter(0.15f),
                                      gateR.getX(), gateR.getY(),
                                      GCol::gateOn.darker(0.2f),
                                      gateR.getX(), gateR.getBottom(), false);
            g.setGradientFill(grad);
        }
        else
        {
            juce::ColourGradient grad(GCol::gateOff.brighter(0.05f),
                                      gateR.getX(), gateR.getY(),
                                      GCol::gateOff.darker(0.1f),
                                      gateR.getX(), gateR.getBottom(), false);
            g.setGradientFill(grad);
        }
        g.fillRoundedRectangle(gateR, 4.0f);
        g.setColour(s.gate ? GCol::gateOn.darker(0.4f) : GCol::border);
        g.drawRoundedRectangle(gateR, 4.0f, 1.0f);

        // Note name inside gate (when gate ON)
        if (s.gate)
        {
            int midiNote = juce::jlimit(0, 127, s.note + s.octave * 12);
            g.setColour(juce::Colours::white);
            g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
            g.drawText(noteNameStr(midiNote), gateR, juce::Justification::centred, false);
        }
        else
        {
            // "+" hint when gate off
            g.setColour(GCol::textDim.withAlpha(0.5f));
            g.setFont(juce::Font(juce::FontOptions(20.0f)));
            g.drawText("+", gateR, juce::Justification::centred, false);
        }
    }

    // ── Note ▲ button ────────────────────────────────────────────────────
    {
        juce::Rectangle<float> r(bx + 2.0f, kNoteUpTop + 1.0f, bw - 4.0f, kNoteNmTop - kNoteUpTop - 2.0f);
        g.setColour(GCol::btnBg);
        g.fillRoundedRectangle(r, 2.0f);
        g.setColour(GCol::noteBtn);
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawText(juce::CharPointer_UTF8("\xe2\x96\xb2"), r, juce::Justification::centred, false); // ▲
    }

    // ── Note name display ─────────────────────────────────────────────────
    {
        juce::Rectangle<float> r(bx, kNoteNmTop, bw, kNoteDnTop - kNoteNmTop);
        int midiNote = juce::jlimit(0, 127, s.note + s.octave * 12);
        g.setColour(s.gate ? GCol::text : GCol::textDim);
        g.setFont(juce::Font(juce::FontOptions(9.5f, juce::Font::bold)));
        g.drawText(noteNameStr(midiNote), r, juce::Justification::centred, false);
    }

    // ── Note ▼ button ────────────────────────────────────────────────────
    {
        juce::Rectangle<float> r(bx + 2.0f, kNoteDnTop + 1.0f, bw - 4.0f, kOctTop - kNoteDnTop - 2.0f);
        g.setColour(GCol::btnBg);
        g.fillRoundedRectangle(r, 2.0f);
        g.setColour(GCol::noteBtn);
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawText(juce::CharPointer_UTF8("\xe2\x96\xbc"), r, juce::Justification::centred, false); // ▼
    }

    // ── Octave buttons [-1] [0] [+1] ─────────────────────────────────────
    {
        const float oy  = kOctTop + 1.0f;
        const float oh  = kAccentTop - kOctTop - 2.0f;
        const float ow  = (bw - 4.0f) / 3.0f;
        struct OctBtn { int val; juce::Colour col; const char* label; };
        OctBtn octs[3] = { {-1, GCol::octNeg, "-1"}, {0, GCol::octZero, "0"}, {1, GCol::octPos, "+1"} };

        for (int i = 0; i < 3; ++i)
        {
            juce::Rectangle<float> r(bx + 2.0f + i * ow, oy, ow - 1.0f, oh);
            bool sel = (s.octave == octs[i].val);
            g.setColour(sel ? octs[i].col : GCol::btnBg);
            g.fillRoundedRectangle(r, 2.0f);
            g.setColour(sel ? juce::Colours::white : GCol::textDim);
            g.setFont(juce::Font(juce::FontOptions(8.0f, sel ? juce::Font::bold : juce::Font::plain)));
            g.drawText(octs[i].label, r, juce::Justification::centred, false);
        }
    }

    // ── Accent button ────────────────────────────────────────────────────
    {
        juce::Rectangle<float> r(bx + 2.0f, kAccentTop + 2.0f, bw - 4.0f, kSlideTop - kAccentTop - 4.0f);
        g.setColour(s.accent ? GCol::accent : GCol::btnBg);
        g.fillRoundedRectangle(r, 2.0f);
        g.setColour(s.accent ? juce::Colours::white : GCol::textDim);
        g.setFont(juce::Font(juce::FontOptions(8.0f, s.accent ? juce::Font::bold : juce::Font::plain)));
        g.drawText("ACC", r, juce::Justification::centred, false);
    }

    // ── Slide button ─────────────────────────────────────────────────────
    {
        juce::Rectangle<float> r(bx + 2.0f, kSlideTop + 2.0f, bw - 4.0f, kPlayTop - kSlideTop - 4.0f);
        g.setColour(s.slide ? GCol::slide : GCol::btnBg);
        g.fillRoundedRectangle(r, 2.0f);
        g.setColour(s.slide ? juce::Colours::white : GCol::textDim);
        g.setFont(juce::Font(juce::FontOptions(8.0f, s.slide ? juce::Font::bold : juce::Font::plain)));
        g.drawText("SLD", r, juce::Justification::centred, false);
    }

    // ── Playhead bar ──────────────────────────────────────────────────────
    {
        juce::Rectangle<float> r(bx, kPlayTop, bw, kColH - kPlayTop);
        g.setColour(isPlayhead && playing ? GCol::playGlow : GCol::btnBg.darker(0.05f));
        g.fillRect(r);
    }
}

// ── paint ────────────────────────────────────────────────────────────────────
void SequencerGrid::paint(juce::Graphics& g)
{
    g.fillAll(GCol::bg);

    const Pattern& pat      = sequencer.getPattern(sequencer.getCurrentPatternIndex());
    const int      patLen   = pat.getLength();
    const int      curStep  = sequencer.getCurrentStep();
    const bool     playing  = sequencer.isPlaying();
    const float    cw       = colW();

    for (int i = 0; i < Pattern::MAX_STEPS; ++i)
    {
        const Step& s = pat.getStep(i);
        drawColumn(g, i, i * cw, cw, s,
                   /*active*/   i < patLen,
                   /*playhead*/ i == curStep,
                   playing);
    }

    // Outer border
    g.setColour(GCol::border);
    g.drawRect(getLocalBounds().toFloat(), 1.0f);
}

// ── Mouse down ───────────────────────────────────────────────────────────────
void SequencerGrid::mouseDown(const juce::MouseEvent& e)
{
    HitResult hr = hitTest(e.position);
    if (hr.step < 0) return;

    Pattern& pat    = sequencer.getPattern(sequencer.getCurrentPatternIndex());
    const int patLen = pat.getLength();

    // Only allow interaction on active steps
    if (hr.step >= patLen) return;

    Step& s = pat.getStep(hr.step);

    switch (hr.zone)
    {
        case Z_GATE:    s.gate   = !s.gate; break;
        case Z_NOTE_UP: s.note   = juce::jmin(s.note + 1, 72); break;
        case Z_NOTE_DN: s.note   = juce::jmax(s.note - 1, 24); break;
        case Z_OCT_M1:  s.octave = -1; break;
        case Z_OCT_0:   s.octave =  0; break;
        case Z_OCT_P1:  s.octave = +1; break;
        case Z_ACCENT:  s.accent = !s.accent; break;
        case Z_SLIDE:   s.slide  = !s.slide; break;
        default: break;
    }

    repaint();
}

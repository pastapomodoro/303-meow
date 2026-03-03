#pragma once
#include <JuceHeader.h>
#include <array>

struct Step
{
    int  note   = 48;  // MIDI note (C3 = 48 by default)
    bool gate   = false;
    bool accent = false;
    bool slide  = false;
    int  octave = 0;   // -1, 0, +1 transposition in octaves
};

class Pattern
{
public:
    Pattern();

    static constexpr int MAX_STEPS = 16;

    void  setLength(int len);          // 1-16
    int   getLength() const;

    Step& getStep(int index);
    const Step& getStep(int index) const;
    void  setStep(int index, const Step& step);

    // Serialization for APVTS state
    juce::ValueTree toValueTree() const;
    void fromValueTree(const juce::ValueTree& tree);

private:
    std::array<Step, MAX_STEPS> steps;
    int length = 16;
};

#include "Pattern.h"

Pattern::Pattern()
{
    // Initialize with a simple default pattern
    for (int i = 0; i < MAX_STEPS; ++i)
    {
        steps[i].note   = 36 + (i % 12);
        steps[i].gate   = (i % 2 == 0);
        steps[i].accent = (i == 0 || i == 8);
        steps[i].slide  = false;
        steps[i].octave = 0;
    }
}

void Pattern::setLength(int len)
{
    length = juce::jlimit(1, MAX_STEPS, len);
}

int Pattern::getLength() const { return length; }

Step& Pattern::getStep(int index)
{
    jassert(index >= 0 && index < MAX_STEPS);
    return steps[static_cast<size_t>(index)];
}

const Step& Pattern::getStep(int index) const
{
    jassert(index >= 0 && index < MAX_STEPS);
    return steps[static_cast<size_t>(index)];
}

void Pattern::setStep(int index, const Step& step)
{
    if (index >= 0 && index < MAX_STEPS)
        steps[static_cast<size_t>(index)] = step;
}

juce::ValueTree Pattern::toValueTree() const
{
    juce::ValueTree tree("Pattern");
    tree.setProperty("length", length, nullptr);

    for (int i = 0; i < MAX_STEPS; ++i)
    {
        juce::ValueTree stepTree("Step");
        stepTree.setProperty("note",   steps[i].note,          nullptr);
        stepTree.setProperty("gate",   steps[i].gate   ? 1 : 0, nullptr);
        stepTree.setProperty("accent", steps[i].accent ? 1 : 0, nullptr);
        stepTree.setProperty("slide",  steps[i].slide  ? 1 : 0, nullptr);
        stepTree.setProperty("octave", steps[i].octave,         nullptr);
        tree.addChild(stepTree, -1, nullptr);
    }
    return tree;
}

void Pattern::fromValueTree(const juce::ValueTree& tree)
{
    if (!tree.isValid()) return;

    length = tree.getProperty("length", 16);

    int idx = 0;
    for (auto child : tree)
    {
        if (child.hasType("Step") && idx < MAX_STEPS)
        {
            steps[idx].note   = child.getProperty("note",   48);
            steps[idx].gate   = static_cast<int>(child.getProperty("gate",   0)) != 0;
            steps[idx].accent = static_cast<int>(child.getProperty("accent", 0)) != 0;
            steps[idx].slide  = static_cast<int>(child.getProperty("slide",  0)) != 0;
            steps[idx].octave = child.getProperty("octave", 0);
            ++idx;
        }
    }
}

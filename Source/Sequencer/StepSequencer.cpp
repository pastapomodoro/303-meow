#include "StepSequencer.h"
#include <cmath>

StepSequencer::StepSequencer()
{
    currentPatternIdx = 0;
    currentStep       = 0;
}

void StepSequencer::prepare(double sr)
{
    sampleRate    = sr;
    sampleCounter = 0.0;
}

void StepSequencer::play()
{
    playing       = true;
    currentStep   = 0;
    sampleCounter = 0.0;
    previousSlide = false;
    firstBeat     = true;   // step 0 fires on the first processBlock call
    currentStepAtomic.store(0, std::memory_order_relaxed);
}

void StepSequencer::stop()
{
    playing       = false;
    currentStep   = 0;
    sampleCounter = 0.0;
    currentStepAtomic.store(0, std::memory_order_relaxed);
}

void StepSequencer::reset()
{
    currentStep   = 0;
    sampleCounter = 0.0;
    previousSlide = false;
    firstBeat     = false;
    currentStepAtomic.store(0, std::memory_order_relaxed);
}

bool StepSequencer::isPlaying() const { return playing; }

void StepSequencer::selectPattern(int index)
{
    if (index >= 0 && index < NUM_PATTERNS)
        currentPatternIdx = index;
}

int StepSequencer::getCurrentPatternIndex() const { return currentPatternIdx; }

Pattern& StepSequencer::getPattern(int index)
{
    jassert(index >= 0 && index < NUM_PATTERNS);
    return patterns[static_cast<size_t>(index)];
}

const Pattern& StepSequencer::getPattern(int index) const
{
    jassert(index >= 0 && index < NUM_PATTERNS);
    return patterns[static_cast<size_t>(index)];
}

void StepSequencer::setStepResolution(int r)
{
    if (r == 1 || r == 2 || r == 4)
        stepResolution = r;
}

int StepSequencer::getCurrentStep() const
{
    return currentStepAtomic.load(std::memory_order_relaxed);
}

void StepSequencer::sendStepEvents(TB303Engine& engine, int step)
{
    const Pattern& pat = patterns[static_cast<size_t>(currentPatternIdx)];
    const Step&    s   = pat.getStep(step);

    if (!s.gate)
    {
        previousSlide = false;
        return;
    }

    int midiNote  = juce::jlimit(0, 127, s.note + s.octave * 12 + transpose);
    bool doSlide  = previousSlide;

    engine.noteOn(midiNote, s.accent, doSlide, previousNote);

    previousNote  = midiNote;
    previousSlide = s.slide;
}

void StepSequencer::advanceStep(TB303Engine& engine, double /*bpm*/)
{
    const Pattern& pat = patterns[static_cast<size_t>(currentPatternIdx)];
    int len = pat.getLength();
    currentStep = (currentStep + 1) % len;
    currentStepAtomic.store(currentStep, std::memory_order_relaxed);

    sendStepEvents(engine, currentStep);
}

void StepSequencer::syncBpm(juce::AudioPlayHead* playHead, double fallbackBpm)
{
    double bpm = fallbackBpm;

    if (playHead != nullptr)
    {
        juce::AudioPlayHead::CurrentPositionInfo pos;
        if (playHead->getCurrentPosition(pos))
        {
            if (pos.isPlaying && pos.bpm > 0.0)
                bpm = pos.bpm;
        }
    }

    double stepSec = 60.0 / bpm / static_cast<double>(stepResolution);
    samplesPerStep = stepSec * sampleRate;
}

void StepSequencer::advanceSample(TB303Engine& engine)
{
    if (!playing) return;

    if (sampleCounter <= 0.0)
    {
        if (firstBeat)
        {
            sendStepEvents(engine, currentStep);
            firstBeat = false;
        }
        sampleCounter += samplesPerStep;
    }

    sampleCounter -= 1.0;

    if (sampleCounter <= 0.0)
        advanceStep(engine, 0.0);
}

#include "StepSequencer.h"
#include <cmath>

StepSequencer::StepSequencer()
{
    currentPatternIdx = 0;
    currentStep       = 0;
    currentPatternAtomic.store(0, std::memory_order_relaxed);
}

void StepSequencer::prepare(double sr)
{
    sampleRate    = sr;
    sampleCounter = 0.0;
}

void StepSequencer::play()
{
    playing          = true;
    currentStep      = 0;
    sampleCounter    = 0.0;
    previousSlide    = false;
    firstBeat        = true;   // step 0 fires on the first processBlock call
    needsHostResync  = true;   // align to DAW grid on first block
    currentStepAtomic.store(0, std::memory_order_relaxed);
}

void StepSequencer::stop()
{
    playing       = false;
    currentStep   = 0;
    sampleCounter = 0.0;
    pendingPattern.store(-1, std::memory_order_relaxed);
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
    if (index < 0 || index >= NUM_PATTERNS)
        return;

    if (!playing)
    {
        // Da fermo non c'e' nessun giro da finire: il cambio e' immediato.
        currentPatternIdx = index;
        currentPatternAtomic.store(index, std::memory_order_relaxed);
        pendingPattern.store(-1, std::memory_order_relaxed);
        return;
    }

    // In riproduzione il cambio si accoda e scatta quando il pattern corrente
    // arriva in fondo, come sul TB-303: premere un tasto a meta' battuta non
    // la interrompe. Ripremere il pattern che sta suonando annulla la coda.
    pendingPattern.store(index == currentPatternIdx ? -1 : index,
                         std::memory_order_relaxed);
}

int StepSequencer::getCurrentPatternIndex() const
{
    return currentPatternAtomic.load(std::memory_order_relaxed);
}

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
    // La lunghezza la decide il pattern che sta suonando ORA: e' lui che deve
    // finire il giro prima dell'eventuale cambio.
    const int len = patterns[static_cast<size_t>(currentPatternIdx)].getLength();
    currentStep = (currentStep + 1) % len;

    // Rientrando sullo step 0 il giro e' chiuso: qui, e solo qui, entra il
    // pattern accodato. exchange() lo consuma in un colpo, quindi non serve
    // altra sincronizzazione con il thread che ha premuto il tasto.
    if (currentStep == 0)
    {
        const int queued = pendingPattern.exchange(-1, std::memory_order_relaxed);
        if (queued >= 0 && queued < NUM_PATTERNS)
            currentPatternIdx = queued;
    }

    currentStepAtomic.store(currentStep, std::memory_order_relaxed);
    currentPatternAtomic.store(currentPatternIdx, std::memory_order_relaxed);

    sendStepEvents(engine, currentStep);
}

void StepSequencer::syncBpm(juce::AudioPlayHead* playHead, double fallbackBpm)
{
    double bpm = fallbackBpm;
    bool ppqValid = false;
    double ppqPos = 0.0;

    if (playHead != nullptr)
    {
        juce::AudioPlayHead::CurrentPositionInfo pos;
        if (playHead->getCurrentPosition(pos))
        {
            if (pos.bpm > 0.0)
                bpm = pos.bpm;
            ppqPos   = pos.ppqPosition;
            ppqValid = (ppqPos >= 0.0);
        }
    }

    double stepSec = 60.0 / bpm / static_cast<double>(stepResolution);
    samplesPerStep = stepSec * sampleRate;

    if (!playing || !ppqValid || samplesPerStep <= 0.0)
    {
        lastPpqPos = ppqPos;
        return;
    }

    // Detect DAW loop/jump: if ppq jumped backward, re-arm phase lock
    if (lastPpqPos >= 0.0 && ppqPos < lastPpqPos - 0.1)
        needsHostResync = true;
    lastPpqPos = ppqPos;

    // Phase-lock: snap step + sampleCounter to host grid on first block after play()
    // (and after any DAW loop/jump)
    if (needsHostResync)
    {
        double stepsPerQN = static_cast<double>(stepResolution);
        const Pattern& pat = patterns[static_cast<size_t>(currentPatternIdx)];
        int len    = pat.getLength();
        int newStep = static_cast<int>(std::floor(ppqPos * stepsPerQN)) % len;

        currentStep = newStep;
        currentStepAtomic.store(currentStep, std::memory_order_relaxed);
        sampleCounter   = 0.0;  // firstBeat (set by play()) will fire the step note
        needsHostResync = false;
    }
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

        // Il refill avviene dopo advanceStep(), quindi currentStep e' già lo
        // step che sta partendo: la sua parita' decide se allungarlo o
        // accorciarlo.
        const double sw = (currentStep % 2 == 0) ? swing : -swing;
        sampleCounter += samplesPerStep * (1.0 + sw);
    }

    sampleCounter -= 1.0;

    if (sampleCounter <= 0.0)
        advanceStep(engine, 0.0);
}

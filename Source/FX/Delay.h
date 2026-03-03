#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class Delay
{
public:
    void prepare(double sampleRate, int maxDelayMs = 1000)
    {
        sr = sampleRate;
        size_t bufSize = static_cast<size_t>(sr * maxDelayMs / 1000.0) + 2;
        buffer.assign(bufSize, 0.0f);
        writePos = 0;
    }

    void reset()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writePos = 0;
    }

    void setTime(float seconds)    { timeSeconds = seconds; }
    void setFeedback(float fb)     { feedback = std::min(0.95f, std::max(0.0f, fb)); }
    void setMix(float m)           { mix = std::max(0.0f, std::min(1.0f, m)); }

    float processSample(float input)
    {
        if (buffer.empty()) return input;

        size_t delaySamples = static_cast<size_t>(timeSeconds * sr);
        if (delaySamples >= buffer.size()) delaySamples = buffer.size() - 1;

        size_t readPos = (writePos + buffer.size() - delaySamples) % buffer.size();
        float delayed  = buffer[readPos];

        buffer[writePos] = input + delayed * feedback;
        writePos = (writePos + 1) % buffer.size();

        // Parallel wet/dry mix
        return input * (1.0f - mix) + delayed * mix;
    }

private:
    std::vector<float> buffer;
    size_t   writePos    = 0;
    double   sr          = 44100.0;
    float    timeSeconds = 0.375f;
    float    feedback    = 0.4f;
    float    mix         = 0.0f;
};

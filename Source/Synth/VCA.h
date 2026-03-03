#pragma once

class VCA
{
public:
    VCA();
    void setVolume(float v);   // 0.0 - 1.0
    float processSample(float input, float noteEnv, float accentEnv);

private:
    float volume = 0.8f;
};

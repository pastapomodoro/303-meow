#pragma once

class Envelope
{
public:
    enum class Type { Note, Accent };

    explicit Envelope(Type type);

    void prepare(double sampleRate);
    void setDecay(float decaySeconds);  // only used for Note envelope
    void trigger();
    void reset();

    float processSample();
    float getValue() const { return static_cast<float>(value); }

private:
    Type   type;
    double sampleRate  = 44100.0;
    double value       = 0.0;

    // Attack
    double attackCoeff = 0.0;

    // Decay
    double decayCoeff  = 0.0;

    enum class State { Idle, Attack, Decay };
    State state = State::Idle;

    static double computeCoeff(double timeSec, double sampleRate);
};

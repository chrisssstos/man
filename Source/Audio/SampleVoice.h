#pragma once
#include <JuceHeader.h>

class SampleVoice : public juce::SynthesiserVoice
{
public:
    SampleVoice();

    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound* sound, int pitchWheel) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                          int startSample, int numSamples) override;
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

private:
    double samplePosition = 0.0;
    float gain = 0.0f;
    bool isPlaying = false;
    juce::ADSR adsr;
};

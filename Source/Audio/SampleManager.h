#pragma once
#include <JuceHeader.h>
#include <map>
#include <memory>

class SampleManager
{
public:
    SampleManager();

    struct LoadedSample
    {
        juce::AudioBuffer<float> buffer;
        double sampleRate = 44100.0;
    };

    const LoadedSample* loadSample (const juce::String& path);
    void loadDirectory (const juce::File& directory);
    void setBaseSampleDirectory (const juce::File& dir) { baseSampleDir = dir; }
    const juce::File& getBaseSampleDirectory() const { return baseSampleDir; }

private:
    juce::AudioFormatManager formatManager;
    std::map<juce::String, std::unique_ptr<LoadedSample>> cache;
    juce::CriticalSection lock;
    juce::File baseSampleDir;
};

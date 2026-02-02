#pragma once
#include <JuceHeader.h>
#include "Model/Element.h"

class SampleSound : public juce::SynthesiserSound
{
public:
    SampleSound (const ElementID& elementId,
                 juce::AudioBuffer<float> buffer,
                 double sourceSampleRate,
                 int midiNote);

    bool appliesToNote (int midiNoteNumber) override;
    bool appliesToChannel (int midiChannel) override;

    const juce::AudioBuffer<float>& getAudioData() const { return audioData; }
    double getSourceSampleRate() const                    { return sourceSampleRate; }
    const ElementID& getElementId() const                 { return elementId; }
    int getAssignedNote() const                           { return assignedNote; }

private:
    ElementID elementId;
    juce::AudioBuffer<float> audioData;
    double sourceSampleRate;
    int assignedNote;
};

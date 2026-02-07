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

    int getStartSample() const  { return startSample; }
    int getEndSample() const    { return endSample < 0 ? audioData.getNumSamples() : endSample; }
    void setTrimSamples (int start, int end) { startSample = start; endSample = end; }

private:
    ElementID elementId;
    juce::AudioBuffer<float> audioData;
    double sourceSampleRate;
    int assignedNote;
    int startSample = 0;
    int endSample = -1;
};

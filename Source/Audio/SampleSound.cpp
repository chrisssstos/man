#include "SampleSound.h"

SampleSound::SampleSound (const ElementID& elementId,
                           juce::AudioBuffer<float> buffer,
                           double sourceSampleRate,
                           int midiNote)
    : elementId (elementId),
      audioData (std::move (buffer)),
      sourceSampleRate (sourceSampleRate),
      assignedNote (midiNote)
{
}

bool SampleSound::appliesToNote (int midiNoteNumber)
{
    return midiNoteNumber == assignedNote;
}

bool SampleSound::appliesToChannel (int)
{
    return true;
}

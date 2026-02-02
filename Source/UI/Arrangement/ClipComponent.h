#pragma once
#include <JuceHeader.h>

// ClipComponent — visual rectangle representing a clip on the arrangement timeline
class ClipComponent : public juce::Component
{
public:
    ClipComponent (int clipIndex, const juce::String& name, juce::Colour colour,
                   double startBeat, double durationBeats);

    void paint (juce::Graphics& g) override;

    int getClipIndex() const { return clipIndex; }
    void setClipIndex (int idx) { clipIndex = idx; }

    double getStartBeat() const     { return startBeat; }
    double getDurationBeats() const { return durationBeats; }

private:
    int clipIndex;
    juce::String name;
    juce::Colour colour;
    double startBeat;
    double durationBeats;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipComponent)
};

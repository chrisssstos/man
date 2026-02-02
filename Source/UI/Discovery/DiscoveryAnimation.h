#pragma once
#include <JuceHeader.h>

class DiscoveryAnimation : public juce::Component, public juce::Timer
{
public:
    DiscoveryAnimation();

    void show (const juce::String& elementName, juce::Colour colour);
    void timerCallback() override;
    void paint (juce::Graphics& g) override;

private:
    juce::String displayName;
    juce::Colour displayColour;
    float progress = 0.0f;
    bool animating = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiscoveryAnimation)
};

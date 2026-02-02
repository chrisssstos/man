#pragma once
#include "VisualRenderer.h"

class CompositeVisual : public VisualRenderer
{
public:
    CompositeVisual (std::unique_ptr<VisualRenderer> base, juce::Colour glowColour);

    void paint (juce::Graphics& g, juce::Rectangle<float> bounds,
                float phase, juce::Colour colour,
                const juce::StringPairArray& params) override;

private:
    std::unique_ptr<VisualRenderer> baseRenderer;
    juce::Colour glowColour;
};

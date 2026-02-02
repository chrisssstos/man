#pragma once
#include "VisualRenderer.h"

class ImageVisual : public VisualRenderer
{
public:
    void paint (juce::Graphics& g, juce::Rectangle<float> bounds,
                float phase, juce::Colour colour,
                const juce::StringPairArray& params) override;

private:
    juce::Image cachedImage;
    juce::String cachedPath;
};

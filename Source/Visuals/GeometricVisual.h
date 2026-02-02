#pragma once
#include "VisualRenderer.h"

class GeometricVisual : public VisualRenderer
{
public:
    void paint (juce::Graphics& g, juce::Rectangle<float> bounds,
                float phase, juce::Colour colour,
                const juce::StringPairArray& params) override;
};

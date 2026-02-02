#pragma once
#include "VisualRenderer.h"

class ParticleVisual : public VisualRenderer
{
public:
    void paint (juce::Graphics& g, juce::Rectangle<float> bounds,
                float phase, juce::Colour colour,
                const juce::StringPairArray& params) override;
};

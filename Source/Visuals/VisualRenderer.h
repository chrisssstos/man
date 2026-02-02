#pragma once
#include <JuceHeader.h>
#include "Model/Element.h"
#include <memory>

class VisualRenderer
{
public:
    virtual ~VisualRenderer() = default;

    virtual void paint (juce::Graphics& g, juce::Rectangle<float> bounds,
                        float phase, juce::Colour colour,
                        const juce::StringPairArray& params) = 0;

    static std::unique_ptr<VisualRenderer> create (VisualElement::VisualKind kind);
};

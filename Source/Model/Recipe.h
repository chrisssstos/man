#pragma once
#include "Element.h"

struct Recipe
{
    ElementID ingredientA;
    ElementID ingredientB;
    ElementID resultId;

    // Result element properties (used to create the AudioVisualElement)
    juce::String resultName;
    juce::String samplePath;
    VisualElement::VisualKind visualKind = VisualElement::VisualKind::Waveform;
    juce::Colour colour;
    juce::StringPairArray visualParams;

    bool matches (const ElementID& a, const ElementID& b) const
    {
        return (ingredientA == a && ingredientB == b)
            || (ingredientA == b && ingredientB == a);
    }
};

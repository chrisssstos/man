#pragma once
#include <JuceHeader.h>
#include "UI/Discovery/ElementTile.h"
#include "Model/ElementLibrary.h"

class ElementPalette : public juce::Component
{
public:
    ElementPalette (ElementLibrary& library);

    void rebuild();
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    ElementLibrary& library;
    juce::OwnedArray<ElementTile> tiles;
    juce::Viewport viewport;
    juce::Component tileContainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ElementPalette)
};

#pragma once
#include <JuceHeader.h>
#include "UI/Discovery/ElementTile.h"
#include "Model/ElementLibrary.h"
#include "UI/Common/TouchConstants.h"

class ElementPalette : public juce::Component
{
public:
    ElementPalette (ElementLibrary& library);

    void rebuild();
    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;

    bool isDrawerExpanded() const { return expanded; }
    void setDrawerExpanded (bool expand);
    int getCollapsedHeight() const { return kHandleHeight; }
    int getExpandedHeight() const { return kHandleHeight + kDrawerContentHeight; }

private:
    ElementLibrary& library;
    juce::OwnedArray<ElementTile> tiles;
    juce::Viewport viewport;
    juce::Component tileContainer;

    bool expanded = false;
    static constexpr int kHandleHeight = 48;
    static constexpr int kDrawerContentHeight = 120;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ElementPalette)
};

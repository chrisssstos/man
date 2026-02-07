#pragma once
#include <JuceHeader.h>
#include "ElementTile.h"
#include "Model/ElementLibrary.h"
#include "UI/Common/TouchConstants.h"

class DiscoveryPanel : public juce::Component
{
public:
    DiscoveryPanel (ElementLibrary& library);

    void rebuild();
    void paint (juce::Graphics& g) override;
    void resized() override;

    void setTileListener (ElementTile::Listener* l) { tileListener = l; }
    int getDiscoveryCount() const { return tiles.size(); }

private:
    ElementLibrary& library;
    juce::OwnedArray<ElementTile> tiles;
    juce::Viewport viewport;
    juce::Component tileContainer;
    ElementTile::Listener* tileListener = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiscoveryPanel)
};

#include "DiscoveryPanel.h"

DiscoveryPanel::DiscoveryPanel (ElementLibrary& lib)
    : library (lib)
{
    viewport.setViewedComponent (&tileContainer, false);
    viewport.setScrollBarsShown (true, true);
    addAndMakeVisible (viewport);
}

void DiscoveryPanel::rebuild()
{
    tiles.clear();
    tileContainer.removeAllChildren();

    for (auto* av : library.getAllDiscoveredAV())
    {
        auto* tile = tiles.add (new ElementTile (av));
        tileContainer.addAndMakeVisible (tile);
        if (tileListener)
            tile->addListener (tileListener);
    }

    resized();
}

void DiscoveryPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (TouchUI::kBgPanel));

    // Header area
    auto headerArea = getLocalBounds().removeFromTop (32);
    g.setColour (juce::Colour (TouchUI::kBgCard));
    g.fillRect (headerArea);

    // Accent line
    g.setColour (juce::Colour (0xff8844cc));
    g.fillRect (headerArea.getX(), headerArea.getBottom() - 2, headerArea.getWidth(), 2);

    g.setColour (juce::Colours::white.withAlpha (0.8f));
    g.setFont (juce::FontOptions (TouchUI::kFontSmall));

    auto totalAV = (int) library.getAllAudioVisuals().size();
    auto discovered = (int) library.getAllDiscoveredAV().size();
    g.drawText ("Discoveries (" + juce::String (discovered) + "/" + juce::String (totalAV) + ")",
                headerArea, juce::Justification::centred);
}

void DiscoveryPanel::resized()
{
    auto area = getLocalBounds().withTrimmedTop (34);
    viewport.setBounds (area);

    int tileSize = TouchUI::kTileSize;
    int gap = TouchUI::kTileGap;
    int cols = juce::jmax (1, (area.getWidth() - gap) / (tileSize + gap));
    int rows = ((int) tiles.size() + cols - 1) / juce::jmax (1, cols);
    int containerHeight = rows * (tileSize + gap) + gap;
    tileContainer.setSize (juce::jmax (area.getWidth(), cols * (tileSize + gap) + gap),
                           juce::jmax (containerHeight, area.getHeight()));

    for (int i = 0; i < tiles.size(); ++i)
    {
        int col = i % cols;
        int row = i / cols;
        tiles[i]->setBounds (gap + col * (tileSize + gap),
                             gap + row * (tileSize + gap),
                             tileSize, tileSize);
    }
}

#include "DiscoveryPanel.h"

DiscoveryPanel::DiscoveryPanel (ElementLibrary& lib)
    : library (lib)
{
    viewport.setViewedComponent (&tileContainer, false);
    viewport.setScrollBarsShown (false, true);
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
    g.fillAll (juce::Colour (0xff0e0e24));

    // Header area
    auto headerArea = getLocalBounds().removeFromTop (24);
    g.setColour (juce::Colour (0xff1a1a2a));
    g.fillRect (headerArea);

    // Accent line
    g.setColour (juce::Colour (0xff8844cc));
    g.fillRect (headerArea.getX(), headerArea.getBottom() - 2, headerArea.getWidth(), 2);

    g.setColour (juce::Colours::white.withAlpha (0.8f));
    g.setFont (juce::FontOptions (13.0f));

    auto totalAV = (int) library.getAllAudioVisuals().size();
    auto discovered = (int) library.getAllDiscoveredAV().size();
    g.drawText ("Discoveries (" + juce::String (discovered) + "/" + juce::String (totalAV) + ")",
                headerArea, juce::Justification::centred);
}

void DiscoveryPanel::resized()
{
    auto area = getLocalBounds().withTrimmedTop (26);
    viewport.setBounds (area);

    int rows = juce::jmax (1, (area.getHeight() - kGap) / (kTileSize + kGap));
    int cols = ((int) tiles.size() + rows - 1) / juce::jmax (1, rows);
    int containerWidth = cols * (kTileSize + kGap) + kGap;
    tileContainer.setSize (juce::jmax (containerWidth, area.getWidth()), area.getHeight());

    for (int i = 0; i < tiles.size(); ++i)
    {
        int row = i % rows;
        int col = i / rows;
        tiles[i]->setBounds (kGap + col * (kTileSize + kGap),
                             kGap + row * (kTileSize + kGap),
                             kTileSize, kTileSize);
    }
}

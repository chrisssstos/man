#include "ElementPalette.h"

ElementPalette::ElementPalette (ElementLibrary& lib)
    : library (lib)
{
    viewport.setViewedComponent (&tileContainer, false);
    viewport.setScrollBarsShown (true, false);
    addAndMakeVisible (viewport);
    rebuild();
}

void ElementPalette::rebuild()
{
    tiles.clear();
    tileContainer.removeAllChildren();

    for (auto* av : library.getAllDiscoveredAV())
    {
        auto* tile = tiles.add (new ElementTile (av));
        tileContainer.addAndMakeVisible (tile);
    }

    resized();
}

void ElementPalette::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0f0f28));
    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.setFont (12.0f);
    g.drawText ("Elements", getLocalBounds().removeFromTop (20), juce::Justification::centred);
}

void ElementPalette::resized()
{
    auto area = getLocalBounds().withTrimmedTop (22);
    viewport.setBounds (area);

    int tileSize = 80;
    int gap = 4;
    int cols = juce::jmax (1, (area.getWidth() - gap) / (tileSize + gap));
    int rows = ((int) tiles.size() + cols - 1) / cols;
    tileContainer.setSize (area.getWidth(), rows * (tileSize + gap) + gap);

    for (int i = 0; i < tiles.size(); ++i)
    {
        int col = i % cols;
        int row = i / cols;
        tiles[i]->setBounds (gap + col * (tileSize + gap),
                             gap + row * (tileSize + gap),
                             tileSize, tileSize);
    }
}

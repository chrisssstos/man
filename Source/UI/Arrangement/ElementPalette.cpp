#include "ElementPalette.h"

ElementPalette::ElementPalette (ElementLibrary& lib)
    : library (lib)
{
    viewport.setViewedComponent (&tileContainer, false);
    viewport.setScrollBarsShown (false, true);
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
    auto bounds = getLocalBounds();

    // Handle bar
    auto handleArea = bounds.removeFromTop (kHandleHeight);
    g.setColour (juce::Colour (TouchUI::kBgCard));
    g.fillRect (handleArea);

    // Top border accent
    g.setColour (juce::Colour (TouchUI::kAccentPink).withAlpha (0.4f));
    g.fillRect (handleArea.getX(), handleArea.getY(), handleArea.getWidth(), 2);

    // Handle label
    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.setFont (juce::FontOptions (TouchUI::kFontSmall));
    juce::String label = expanded ? "Elements \u25BC" : "Elements \u25B2";
    g.drawText (label, handleArea, juce::Justification::centred);

    // Content area background
    if (expanded)
    {
        g.setColour (juce::Colour (TouchUI::kBgPanel));
        g.fillRect (bounds);
    }
}

void ElementPalette::resized()
{
    auto area = getLocalBounds().withTrimmedTop (kHandleHeight);

    if (! expanded)
    {
        viewport.setVisible (false);
        return;
    }

    viewport.setVisible (true);
    viewport.setBounds (area);

    int tileSize = TouchUI::kTileSize;
    int gap = TouchUI::kTileGap;

    // Horizontal scrolling row
    int containerWidth = (int) tiles.size() * (tileSize + gap) + gap;
    tileContainer.setSize (juce::jmax (containerWidth, area.getWidth()), area.getHeight());

    for (int i = 0; i < tiles.size(); ++i)
    {
        tiles[i]->setBounds (gap + i * (tileSize + gap),
                             gap,
                             tileSize, tileSize);
    }
}

void ElementPalette::mouseDown (const juce::MouseEvent& e)
{
    auto handleArea = getLocalBounds().removeFromTop (kHandleHeight);
    if (handleArea.contains (e.getPosition()))
    {
        setDrawerExpanded (! expanded);
    }
}

void ElementPalette::setDrawerExpanded (bool expand)
{
    if (expanded != expand)
    {
        expanded = expand;
        resized();
        repaint();

        // Notify parent to relayout
        if (auto* parent = getParentComponent())
            parent->resized();
    }
}

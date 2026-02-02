#include "ClipComponent.h"

ClipComponent::ClipComponent (int idx, const juce::String& n, juce::Colour c,
                               double start, double dur)
    : clipIndex (idx), name (n), colour (c), startBeat (start), durationBeats (dur)
{
}

void ClipComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);

    // Filled rounded rectangle — smooth gradient feel
    g.setColour (colour.withAlpha (0.65f));
    g.fillRoundedRectangle (bounds, 6.0f);

    // Top highlight for depth
    auto topSlice = bounds.removeFromTop (bounds.getHeight() * 0.35f);
    g.setColour (colour.brighter (0.3f).withAlpha (0.25f));
    g.fillRoundedRectangle (topSlice.withBottom (topSlice.getBottom() + 6.0f), 6.0f);

    // Border
    bounds = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (colour.brighter (0.5f));
    g.drawRoundedRectangle (bounds, 6.0f, 1.5f);

    // Clip name text
    if (bounds.getWidth() > 24.0f)
    {
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (11.0f));
        g.drawText (name, bounds.reduced (5.0f, 0.0f),
                    juce::Justification::centredLeft, true);
    }
}

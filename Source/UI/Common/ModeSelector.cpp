#include "ModeSelector.h"

ModeSelector::ModeSelector()
{
}

void ModeSelector::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    g.fillAll (juce::Colour (TouchUI::kBgDeep));

    // Bottom divider line
    g.setColour (juce::Colour (TouchUI::kBgCard));
    g.fillRect (bounds.getX(), bounds.getBottom() - 1, bounds.getWidth(), 1);

    int zoneW = bounds.getWidth() / 3;
    juce::StringArray labels { "Discovery", "Arrange", "Live" };

    for (int i = 0; i < 3; ++i)
    {
        auto zone = juce::Rectangle<int> (bounds.getX() + i * zoneW, bounds.getY(),
                                           zoneW, bounds.getHeight());
        bool isActive = (i == (int) currentMode);

        // Label
        g.setColour (isActive ? juce::Colours::white : juce::Colours::white.withAlpha (0.4f));
        g.setFont (juce::FontOptions (TouchUI::kFontLarge));
        g.drawText (labels[i], zone, juce::Justification::centred);

        // Active underline glow bar
        if (isActive)
        {
            auto barRect = juce::Rectangle<float> ((float) zone.getX() + (float) zoneW * 0.2f,
                                                    (float) zone.getBottom() - 4.0f,
                                                    (float) zoneW * 0.6f, 4.0f);
            // Glow
            g.setColour (juce::Colour (TouchUI::kAccentPink).withAlpha (0.25f));
            g.fillRoundedRectangle (barRect.expanded (2.0f, 2.0f), 3.0f);

            g.setColour (juce::Colour (TouchUI::kAccentPink));
            g.fillRoundedRectangle (barRect, 2.0f);
        }
    }
}

void ModeSelector::mouseDown (const juce::MouseEvent& e)
{
    int zoneW = getWidth() / 3;
    int zoneIndex = e.x / juce::jmax (1, zoneW);
    zoneIndex = juce::jlimit (0, 2, zoneIndex);

    setMode (static_cast<Mode> (zoneIndex));
}

void ModeSelector::setModeExternal (Mode mode)
{
    if (currentMode != mode)
    {
        currentMode = mode;
        repaint();
    }
}

void ModeSelector::setMode (Mode mode)
{
    if (currentMode != mode)
    {
        currentMode = mode;
        repaint();
        listeners.call ([mode] (Listener& l) { l.modeChanged (mode); });
    }
}

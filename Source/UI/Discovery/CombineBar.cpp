#include "CombineBar.h"

CombineBar::CombineBar()
{
    startTimerHz (30);
}

void CombineBar::setSoundSlot (SoundElement* snd)
{
    soundSlot = snd;
    repaint();
}

void CombineBar::setVisualSlot (VisualElement* vis)
{
    visualSlot = vis;
    repaint();
}

void CombineBar::clearSoundSlot()
{
    soundSlot = nullptr;
    repaint();
}

void CombineBar::clearVisualSlot()
{
    visualSlot = nullptr;
    repaint();
}

juce::Rectangle<float> CombineBar::getSoundSlotBounds() const
{
    float slotW = 56.0f, slotH = 56.0f;
    float plusW = 24.0f;
    float btnW = 80.0f;
    float spacing = 12.0f;
    float totalW = slotW + plusW + slotW + spacing + btnW;
    float startX = ((float) getWidth() - totalW) * 0.5f;
    float y = ((float) getHeight() - slotH) * 0.5f;
    return { startX, y, slotW, slotH };
}

juce::Rectangle<float> CombineBar::getVisualSlotBounds() const
{
    auto sb = getSoundSlotBounds();
    float plusW = 24.0f;
    float y = sb.getY();
    return { sb.getRight() + plusW, y, 56.0f, 56.0f };
}

juce::Rectangle<float> CombineBar::getCombineButtonBounds() const
{
    auto vb = getVisualSlotBounds();
    float spacing = 12.0f;
    float y = ((float) getHeight() - 56.0f) * 0.5f;
    return { vb.getRight() + spacing, y, 80.0f, 56.0f };
}

void CombineBar::timerCallback()
{
    if (soundSlot != nullptr && visualSlot != nullptr)
    {
        pulsePhase += 1.0f / 30.0f;
        if (pulsePhase >= 1.0f)
            pulsePhase -= 1.0f;
        repaint();
    }
}

void CombineBar::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Gradient background (subtle purple)
    auto bgTop = juce::Colour (0xff12123a);
    auto bgBot = juce::Colour (0xff0a0a28);
    g.setGradientFill (juce::ColourGradient (bgTop, 0.0f, bounds.getY(),
                                              bgBot, 0.0f, bounds.getBottom(), false));
    g.fillRoundedRectangle (bounds, 6.0f);

    // Top glow border line
    g.setColour (juce::Colour (TouchUI::kAccentPink).withAlpha (0.45f));
    g.drawLine (bounds.getX() + 8.0f, bounds.getY() + 0.5f,
                bounds.getRight() - 8.0f, bounds.getY() + 0.5f, 1.5f);

    // Bottom glow border line
    g.setColour (juce::Colour (TouchUI::kAccentPink).withAlpha (0.25f));
    g.drawLine (bounds.getX() + 8.0f, bounds.getBottom() - 0.5f,
                bounds.getRight() - 8.0f, bounds.getBottom() - 0.5f, 1.0f);

    bool canCombine = (soundSlot != nullptr && visualSlot != nullptr);

    // Sound slot
    auto soundBounds = getSoundSlotBounds();
    g.setColour (juce::Colour (TouchUI::kBgCard));
    g.fillRoundedRectangle (soundBounds, 8.0f);
    if (soundSlot != nullptr)
    {
        g.setColour (juce::Colour (0xff44dd88));
        g.drawRoundedRectangle (soundBounds.reduced (1.0f), 8.0f, 2.0f);
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (TouchUI::kFontTiny));
        g.drawText (soundSlot->getName(), soundBounds.reduced (4.0f),
                    juce::Justification::centred, true);
    }
    else
    {
        g.setColour (juce::Colours::white.withAlpha (0.3f));
        g.setFont (juce::FontOptions (TouchUI::kFontTiny));
        g.drawText ("Sound", soundBounds, juce::Justification::centred);
    }

    // Plus sign (centered between the two slots)
    float plusX = (soundBounds.getRight() + getVisualSlotBounds().getX()) * 0.5f;
    float plusY = bounds.getCentreY();
    g.setColour (juce::Colours::white.withAlpha (0.5f));
    g.setFont (juce::FontOptions (TouchUI::kFontLarge));
    g.drawText ("+", (int) (plusX - 8.0f), (int) (plusY - 10.0f), 16, 20,
                juce::Justification::centred);

    // Visual slot
    auto visualBounds = getVisualSlotBounds();
    g.setColour (juce::Colour (TouchUI::kBgCard));
    g.fillRoundedRectangle (visualBounds, 8.0f);
    if (visualSlot != nullptr)
    {
        g.setColour (juce::Colour (TouchUI::kAccentCyan));
        g.drawRoundedRectangle (visualBounds.reduced (1.0f), 8.0f, 2.0f);
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (TouchUI::kFontTiny));
        g.drawText (visualSlot->getName(), visualBounds.reduced (4.0f),
                    juce::Justification::centred, true);
    }
    else
    {
        g.setColour (juce::Colours::white.withAlpha (0.3f));
        g.setFont (juce::FontOptions (TouchUI::kFontTiny));
        g.drawText ("Visual", visualBounds, juce::Justification::centred);
    }

    // Combine button (wider 80px)
    auto combineBounds = getCombineButtonBounds();
    if (canCombine)
    {
        float pulse = 0.6f + 0.4f * std::sin (pulsePhase * juce::MathConstants<float>::twoPi);
        auto glowColour = juce::Colour (TouchUI::kAccentPink).withAlpha (pulse * 0.3f);
        g.setColour (glowColour);
        g.fillRoundedRectangle (combineBounds.expanded (4.0f), 12.0f);

        g.setColour (juce::Colour (TouchUI::kAccentPink));
        g.fillRoundedRectangle (combineBounds, 8.0f);
    }
    else
    {
        g.setColour (juce::Colour (TouchUI::kBgCard));
        g.fillRoundedRectangle (combineBounds, 8.0f);
    }

    g.setColour (canCombine ? juce::Colours::white : juce::Colours::white.withAlpha (0.3f));
    g.setFont (juce::FontOptions (TouchUI::kFontSmall).withStyle ("Bold"));
    g.drawText ("COMBINE", combineBounds, juce::Justification::centred);
}

void CombineBar::mouseDown (const juce::MouseEvent& e)
{
    auto pos = e.position;

    if (getSoundSlotBounds().contains (pos) && soundSlot != nullptr)
    {
        listeners.call ([] (Listener& l) { l.slotCleared (true); });
        return;
    }

    if (getVisualSlotBounds().contains (pos) && visualSlot != nullptr)
    {
        listeners.call ([] (Listener& l) { l.slotCleared (false); });
        return;
    }

    if (getCombineButtonBounds().contains (pos) && soundSlot != nullptr && visualSlot != nullptr)
    {
        listeners.call ([] (Listener& l) { l.combineRequested(); });
    }
}

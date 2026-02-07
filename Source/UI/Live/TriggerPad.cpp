#include "TriggerPad.h"

TriggerPad::TriggerPad (AudioVisualElement* elem, const juce::String& key)
    : element (elem), keyLabel (key)
{
    if (elem != nullptr)
        renderer = VisualRenderer::create (elem->getVisualKind());
    startTimerHz (30);
}

TriggerPad::~TriggerPad()
{
    stopTimer();
}

void TriggerPad::timerCallback()
{
    animPhase += 1.0f / (30.0f * 3.0f);
    if (animPhase >= 1.0f)
        animPhase -= 1.0f;
    if (active || renderer)
        repaint();
}

void TriggerPad::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Active state: outer glow rings
    if (active && element != nullptr)
    {
        auto colour = element->getColour();
        // 3 glow layers, decreasing alpha
        g.setColour (colour.withAlpha (0.08f));
        g.fillRoundedRectangle (bounds.expanded (6.0f), TouchUI::kCornerRadiusLg + 6.0f);
        g.setColour (colour.withAlpha (0.12f));
        g.fillRoundedRectangle (bounds.expanded (3.0f), TouchUI::kCornerRadiusLg + 3.0f);
        g.setColour (colour.withAlpha (0.18f));
        g.fillRoundedRectangle (bounds, TouchUI::kCornerRadiusLg);
    }

    auto bgColour = active ? juce::Colour (0xff3a3a5c) : juce::Colour (TouchUI::kBgCard);
    g.setColour (bgColour);
    g.fillRoundedRectangle (bounds, TouchUI::kCornerRadiusLg);

    if (element != nullptr)
    {
        auto colour = element->getColour();
        g.setColour (active ? colour : colour.withAlpha (0.4f));
        g.drawRoundedRectangle (bounds.reduced (1), TouchUI::kCornerRadiusLg, active ? 3.0f : 1.0f);

        if (renderer)
        {
            auto visArea = bounds.reduced (10).withTrimmedBottom (34);
            renderer->paint (g, visArea, animPhase, colour, element->getVisualParams());
        }

        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (TouchUI::kFontSmall));
        g.drawText (element->getName(), bounds.removeFromBottom (22),
                    juce::Justification::centred);
    }

    // Key label
    g.setColour (juce::Colours::white.withAlpha (0.5f));
    g.setFont (juce::FontOptions (13.0f));
    g.drawText (keyLabel, bounds.removeFromTop (18).removeFromLeft (24).translated (6, 4),
                juce::Justification::centredLeft);
}

void TriggerPad::mouseDown (const juce::MouseEvent&)
{
    setActive (true);
    listeners.call ([this] (Listener& l) { l.padPressed (this); });
}

void TriggerPad::mouseUp (const juce::MouseEvent&)
{
    setActive (false);
    listeners.call ([this] (Listener& l) { l.padReleased (this); });
}

void TriggerPad::setActive (bool a)
{
    active = a;
    repaint();
}

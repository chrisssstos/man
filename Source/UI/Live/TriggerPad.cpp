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
    auto bgColour = active ? juce::Colour (0xff3a3a5c) : juce::Colour (0xff1a1a3a);
    g.setColour (bgColour);
    g.fillRoundedRectangle (bounds, 8.0f);

    if (element != nullptr)
    {
        auto colour = element->getColour();
        g.setColour (active ? colour : colour.withAlpha (0.4f));
        g.drawRoundedRectangle (bounds.reduced (1), 8.0f, active ? 3.0f : 1.0f);

        if (renderer)
        {
            auto visArea = bounds.reduced (8).withTrimmedBottom (30);
            renderer->paint (g, visArea, animPhase, colour, element->getVisualParams());
        }

        g.setColour (juce::Colours::white);
        g.setFont (11.0f);
        g.drawText (element->getName(), bounds.removeFromBottom (18),
                    juce::Justification::centred);
    }

    // Key label
    g.setColour (juce::Colours::white.withAlpha (0.5f));
    g.setFont (10.0f);
    g.drawText (keyLabel, bounds.removeFromTop (14).removeFromLeft (20).translated (4, 2),
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

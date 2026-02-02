#include "CombineZone.h"

CombineZone::CombineZone()
{
    addAndMakeVisible (combineButton);
    combineButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffe94560));
    combineButton.setEnabled (false);
    combineButton.onClick = [this]
    {
        if (slotA.isValid() && slotB.isValid())
            listeners.call ([&] (Listener& l) { l.onCombineRequest (slotA, slotB); });
    };
}

void CombineZone::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour (hovering ? juce::Colour (0xff2a2a4a) : juce::Colour (0xff1a1a3a));
    g.fillRoundedRectangle (bounds, 8.0f);

    g.setColour (juce::Colour (0xff533483));
    g.drawRoundedRectangle (bounds.reduced (2), 8.0f, hovering ? 2.0f : 1.0f);

    g.setColour (juce::Colours::white.withAlpha (0.8f));
    g.setFont (13.0f);

    // Slot A
    auto slotAArea = bounds.removeFromTop (bounds.getHeight() * 0.35f).reduced (10);
    g.drawRoundedRectangle (slotAArea, 4.0f, 1.0f);
    g.drawText (slotA.isValid() ? slotAName : "Drop element here...",
                slotAArea, juce::Justification::centred);

    // Slot B
    auto slotBArea = bounds.withTrimmedBottom (50).reduced (10);
    g.drawRoundedRectangle (slotBArea, 4.0f, 1.0f);
    g.drawText (slotB.isValid() ? slotBName : "Drop element here...",
                slotBArea, juce::Justification::centred);

    // Plus sign between slots
    g.setFont (24.0f);
    g.setColour (juce::Colour (0xffe94560));
    g.drawText ("+", getLocalBounds().withHeight (getHeight()).reduced (0, getHeight() / 3),
                juce::Justification::centred);
}

bool CombineZone::isInterestedInDragSource (const SourceDetails&)
{
    return true;
}

void CombineZone::itemDragEnter (const SourceDetails&)
{
    hovering = true;
    repaint();
}

void CombineZone::itemDragExit (const SourceDetails&)
{
    hovering = false;
    repaint();
}

void CombineZone::itemDropped (const SourceDetails& details)
{
    hovering = false;
    auto id = details.description.toString();

    if (! slotA.isValid())
    {
        slotA = { id };
        slotAName = id; // Will be replaced with proper name by the view
    }
    else if (! slotB.isValid())
    {
        slotB = { id };
        slotBName = id;
        combineButton.setEnabled (true);
    }

    repaint();
    checkAndCombine();
}

void CombineZone::setSlotA (const ElementID& id, const juce::String& name)
{
    slotA = id;
    slotAName = name;
    combineButton.setEnabled (slotA.isValid() && slotB.isValid());
    repaint();
}

void CombineZone::setSlotB (const ElementID& id, const juce::String& name)
{
    slotB = id;
    slotBName = name;
    combineButton.setEnabled (slotA.isValid() && slotB.isValid());
    repaint();
}

void CombineZone::clearSlots()
{
    slotA = {};
    slotB = {};
    slotAName.clear();
    slotBName.clear();
    combineButton.setEnabled (false);
    repaint();
}

void CombineZone::checkAndCombine()
{
    if (slotA.isValid() && slotB.isValid())
        listeners.call ([&] (Listener& l) { l.onCombineRequest (slotA, slotB); });
}

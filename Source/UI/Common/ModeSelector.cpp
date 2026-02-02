#include "ModeSelector.h"

ModeSelector::ModeSelector()
{
    addAndMakeVisible (discoveryButton);
    addAndMakeVisible (arrangementButton);
    addAndMakeVisible (liveButton);

    discoveryButton.onClick   = [this] { setMode (Mode::Discovery); };
    arrangementButton.onClick = [this] { setMode (Mode::Arrangement); };
    liveButton.onClick        = [this] { setMode (Mode::Live); };

    updateButtonStates();
}

void ModeSelector::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff16213e));
}

void ModeSelector::resized()
{
    auto area = getLocalBounds().reduced (4);
    int buttonWidth = area.getWidth() / 3;
    discoveryButton.setBounds (area.removeFromLeft (buttonWidth));
    arrangementButton.setBounds (area.removeFromLeft (buttonWidth));
    liveButton.setBounds (area);
}

void ModeSelector::setMode (Mode mode)
{
    if (currentMode != mode)
    {
        currentMode = mode;
        updateButtonStates();
        listeners.call ([mode] (Listener& l) { l.modeChanged (mode); });
    }
}

void ModeSelector::updateButtonStates()
{
    auto activeColour = juce::Colour (0xffe94560);
    auto normalColour = juce::Colour (0xff533483);

    discoveryButton.setColour (juce::TextButton::buttonColourId,
        currentMode == Mode::Discovery ? activeColour : normalColour);
    arrangementButton.setColour (juce::TextButton::buttonColourId,
        currentMode == Mode::Arrangement ? activeColour : normalColour);
    liveButton.setColour (juce::TextButton::buttonColourId,
        currentMode == Mode::Live ? activeColour : normalColour);
}

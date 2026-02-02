#include "TransportBar.h"

TransportBar::TransportBar()
{
    addAndMakeVisible (playButton);
    addAndMakeVisible (stopButton);
    addAndMakeVisible (recordButton);
    addAndMakeVisible (exportButton);
    addAndMakeVisible (bpmSlider);
    addAndMakeVisible (beatLabel);

    playButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2ecc71));
    stopButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffe74c3c));
    recordButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffe94560));
    exportButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3498db));

    bpmSlider.setRange (40.0, 300.0, 1.0);
    bpmSlider.setValue (120.0);
    bpmSlider.setTextValueSuffix (" BPM");
    bpmSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 80, 24);
    bpmSlider.onValueChange = [this]
    {
        listeners.call ([bpm = bpmSlider.getValue()] (Listener& l) { l.bpmChanged (bpm); });
    };

    beatLabel.setText ("Beat: 0.0", juce::dontSendNotification);
    beatLabel.setColour (juce::Label::textColourId, juce::Colours::white);

    playButton.onClick   = [this] { listeners.call ([] (Listener& l) { l.playClicked(); }); };
    stopButton.onClick   = [this] { listeners.call ([] (Listener& l) { l.stopClicked(); }); };
    recordButton.onClick = [this] { listeners.call ([] (Listener& l) { l.recordClicked(); }); };
    exportButton.onClick = [this] { listeners.call ([] (Listener& l) { l.exportClicked(); }); };
}

void TransportBar::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff16213e));
}

void TransportBar::resized()
{
    auto area = getLocalBounds().reduced (4);
    playButton.setBounds (area.removeFromLeft (60));
    area.removeFromLeft (4);
    stopButton.setBounds (area.removeFromLeft (60));
    area.removeFromLeft (4);
    recordButton.setBounds (area.removeFromLeft (70));
    area.removeFromLeft (8);
    bpmSlider.setBounds (area.removeFromLeft (200));
    area.removeFromLeft (8);
    beatLabel.setBounds (area.removeFromLeft (120));
    exportButton.setBounds (area.removeFromRight (70));
}

void TransportBar::setPlaying (bool playing)
{
    playButton.setEnabled (! playing);
    stopButton.setEnabled (playing);
}

void TransportBar::setBPM (double bpm)
{
    bpmSlider.setValue (bpm, juce::dontSendNotification);
}

void TransportBar::setCurrentBeat (double beat)
{
    beatLabel.setText ("Beat: " + juce::String (beat, 1), juce::dontSendNotification);
}

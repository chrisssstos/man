#include "TransportBar.h"

TransportBar::TransportBar()
{
    addAndMakeVisible (playButton);
    addAndMakeVisible (pauseButton);
    addAndMakeVisible (stopButton);
    addAndMakeVisible (recordButton);
    addAndMakeVisible (exportButton);
    addAndMakeVisible (bpmSlider);
    addAndMakeVisible (beatLabel);
    addAndMakeVisible (snapCombo);
    addAndMakeVisible (snapLabel);

    playButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2ecc71));
    pauseButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xfff39c12));
    stopButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffe74c3c));
    recordButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffe94560));
    exportButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3498db));

    pauseButton.setEnabled (false);
    stopButton.setEnabled (false);

    bpmSlider.setRange (40.0, 300.0, 1.0);
    bpmSlider.setValue (120.0);
    bpmSlider.setTextValueSuffix (" BPM");
    bpmSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 80, 24);
    bpmSlider.onValueChange = [this]
    {
        currentBPM = bpmSlider.getValue();
        listeners.call ([bpm = currentBPM] (Listener& l) { l.bpmChanged (bpm); });
    };

    beatLabel.setText ("Beat: 0.0 | 0:00.0", juce::dontSendNotification);
    beatLabel.setColour (juce::Label::textColourId, juce::Colours::white);

    snapLabel.setText ("Snap:", juce::dontSendNotification);
    snapLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.7f));
    snapLabel.setFont (juce::FontOptions (12.0f));

    snapCombo.addItem ("Off", 1);
    snapCombo.addItem ("1 Bar", 2);
    snapCombo.addItem ("1 Beat", 3);
    snapCombo.addItem ("1/2 Beat", 4);
    snapCombo.addItem ("1/4 Beat", 5);
    snapCombo.setSelectedId (3);
    snapCombo.onChange = [this]
    {
        double snap = 0.0;
        switch (snapCombo.getSelectedId())
        {
            case 1: snap = 0.0; break;
            case 2: snap = 4.0; break;
            case 3: snap = 1.0; break;
            case 4: snap = 0.5; break;
            case 5: snap = 0.25; break;
            default: snap = 1.0; break;
        }
        listeners.call ([snap] (Listener& l) { l.snapChanged (snap); });
    };

    playButton.onClick   = [this] { listeners.call ([] (Listener& l) { l.playClicked(); }); };
    pauseButton.onClick  = [this] { listeners.call ([] (Listener& l) { l.pauseClicked(); }); };
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
    playButton.setBounds (area.removeFromLeft (50));
    area.removeFromLeft (3);
    pauseButton.setBounds (area.removeFromLeft (50));
    area.removeFromLeft (3);
    stopButton.setBounds (area.removeFromLeft (50));
    area.removeFromLeft (3);
    recordButton.setBounds (area.removeFromLeft (60));
    area.removeFromLeft (8);
    bpmSlider.setBounds (area.removeFromLeft (200));
    area.removeFromLeft (8);
    beatLabel.setBounds (area.removeFromLeft (150));
    area.removeFromLeft (8);
    snapLabel.setBounds (area.removeFromLeft (36));
    snapCombo.setBounds (area.removeFromLeft (90));
    exportButton.setBounds (area.removeFromRight (60));
}

void TransportBar::setPlaying (bool playing)
{
    playButton.setEnabled (! playing);
    pauseButton.setEnabled (playing);
    stopButton.setEnabled (playing);
}

void TransportBar::setBPM (double bpm)
{
    currentBPM = bpm;
    bpmSlider.setValue (bpm, juce::dontSendNotification);
}

void TransportBar::setCurrentBeat (double beat)
{
    double seconds = (beat / currentBPM) * 60.0;
    int mins = (int) (seconds / 60.0);
    double secs = seconds - mins * 60.0;

    auto text = "Beat: " + juce::String (beat, 1)
              + " | " + juce::String (mins) + ":" + juce::String (secs, 1).paddedLeft ('0', 4);
    beatLabel.setText (text, juce::dontSendNotification);
}

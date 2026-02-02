#include "ArrangementView.h"

ArrangementView::ArrangementView (Sketch& sk, ElementLibrary& lib, AudioEngine& eng)
    : sketch (sk), elementLibrary (lib), audioEngine (eng),
      timeline (sk, lib, eng.getSampleManager()), visualCanvas (lib),
      elementPalette (lib)
{
    addAndMakeVisible (transportBar);
    addAndMakeVisible (timeline);
    addAndMakeVisible (visualCanvas);
    addAndMakeVisible (elementPalette);

    paletteToggle.setButtonText ("<");
    paletteToggle.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a1a3a));
    paletteToggle.setColour (juce::TextButton::textColourOffId, juce::Colours::white.withAlpha (0.7f));
    paletteToggle.onClick = [this]
    {
        paletteVisible = ! paletteVisible;
        paletteToggle.setButtonText (paletteVisible ? "<" : ">");
        elementPalette.setVisible (paletteVisible);
        resized();
    };
    addAndMakeVisible (paletteToggle);

    transportBar.addListener (this);
    timeline.addListener (this);

    visualCanvas.setActiveElementsProvider ([this]() { return audioEngine.getActiveElementIds(); });

    audioEngine.setSketch (&sketch);
    startTimerHz (30);
}

ArrangementView::~ArrangementView()
{
    stopTimer();
}

void ArrangementView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0a1e));
}

void ArrangementView::resized()
{
    auto area = getLocalBounds();
    transportBar.setBounds (area.removeFromTop (40));

    auto canvasH = juce::jmax (120, area.getHeight() * 20 / 100);
    visualCanvas.setBounds (area.removeFromBottom (canvasH));

    int toggleW = 16;

    if (paletteVisible)
    {
        elementPalette.setBounds (area.removeFromLeft (kPaletteWidth));
        paletteToggle.setBounds (area.removeFromLeft (toggleW));
    }
    else
    {
        paletteToggle.setBounds (area.removeFromLeft (toggleW));
    }

    timeline.setBounds (area);
}

void ArrangementView::timerCallback()
{
    bool playing = audioEngine.isPlaying();

    if (playing)
    {
        playheadBeat = audioEngine.getCurrentBeat();
        transportBar.setCurrentBeat (playheadBeat);
        timeline.setPlayheadBeat (playheadBeat);
    }
    else
    {
        // Detect auto-stop from audio engine (reached end of arrangement)
        if (wasPlaying)
            transportBar.setPlaying (false);

        // Keep showing playhead at last known position
        timeline.setPlayheadBeat (playheadBeat);
    }

    wasPlaying = playing;
}

void ArrangementView::playClicked()
{
    audioEngine.setPosition (0.0);
    playheadBeat = 0.0;
    audioEngine.play();
    transportBar.setPlaying (true);
}

void ArrangementView::pauseClicked()
{
    playheadBeat = audioEngine.getCurrentBeat();
    audioEngine.stop();
    transportBar.setPlaying (false);
    timeline.setPlayheadBeat (playheadBeat);
    transportBar.setCurrentBeat (playheadBeat);
}

void ArrangementView::stopClicked()
{
    audioEngine.stop();
    playheadBeat = 0.0;
    audioEngine.setPosition (0.0);
    transportBar.setPlaying (false);
    transportBar.setCurrentBeat (0.0);
    timeline.setPlayheadBeat (playheadBeat);
}

void ArrangementView::recordClicked()
{
}

void ArrangementView::exportClicked()
{
}

void ArrangementView::bpmChanged (double newBPM)
{
    audioEngine.setBPM (newBPM);
    timeline.setBPM (newBPM);
}

void ArrangementView::snapChanged (double snapBeats)
{
    timeline.setSnapBeats (snapBeats);
}

void ArrangementView::timelinePlayheadClicked (double beat)
{
    playheadBeat = beat;
    audioEngine.setPosition (beat);
    transportBar.setCurrentBeat (beat);
    timeline.setPlayheadBeat (beat);
}

void ArrangementView::rebuild()
{
    elementPalette.rebuild();
    timeline.rebuild();
}

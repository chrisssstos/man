#include "ArrangementView.h"

ArrangementView::ArrangementView (Sketch& sk, ElementLibrary& lib, AudioEngine& eng)
    : sketch (sk), elementLibrary (lib), audioEngine (eng),
      timeline (sk, lib), visualCanvas (lib)
{
    addAndMakeVisible (transportBar);
    addAndMakeVisible (timeline);
    addAndMakeVisible (visualCanvas);

    transportBar.addListener (this);

    visualCanvas.setActiveElementsProvider ([this]() { return audioEngine.getActiveElementIds(); });

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

    timeline.setBounds (area);
}

void ArrangementView::timerCallback()
{
    if (audioEngine.isPlaying())
    {
        double beat = audioEngine.getCurrentBeat();
        transportBar.setCurrentBeat (beat);
        timeline.setPlayheadBeat (beat);
    }
    else
    {
        timeline.setPlayheadBeat (-1.0);
    }
}

void ArrangementView::playClicked()
{
    audioEngine.setPosition (0.0);
    audioEngine.play();
    transportBar.setPlaying (true);
}

void ArrangementView::stopClicked()
{
    audioEngine.stop();
    transportBar.setPlaying (false);
    timeline.setPlayheadBeat (-1.0);
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
}

void ArrangementView::rebuild()
{
    timeline.rebuild();
}

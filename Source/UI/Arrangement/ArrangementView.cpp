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
    g.fillAll (juce::Colour (TouchUI::kBgDeep));
}

void ArrangementView::resized()
{
    auto area = getLocalBounds();

    // Transport bar: 64px top
    transportBar.setBounds (area.removeFromTop (TouchUI::kTransportHeight));

    // Visual canvas: 15% of remaining, min 120px
    auto canvasH = juce::jmax (120, area.getHeight() * 15 / 100);
    visualCanvas.setBounds (area.removeFromBottom (canvasH));

    // Element palette drawer at bottom (above canvas)
    int paletteH = elementPalette.isDrawerExpanded()
                     ? elementPalette.getExpandedHeight()
                     : elementPalette.getCollapsedHeight();
    elementPalette.setBounds (area.removeFromBottom (paletteH));

    // Timeline fills the rest
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
        if (wasPlaying)
            transportBar.setPlaying (false);

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

#pragma once
#include <JuceHeader.h>
#include "TransportBar.h"
#include "Timeline.h"
#include "Visuals/VisualCanvas.h"
#include "Model/Sketch.h"
#include "Model/ElementLibrary.h"
#include "Audio/AudioEngine.h"
#include "ElementPalette.h"

class ArrangementView : public juce::Component,
                        public juce::Timer,
                        public TransportBar::Listener,
                        public ArrangementTimeline::Listener
{
public:
    ArrangementView (Sketch& sketch, ElementLibrary& library, AudioEngine& engine);
    ~ArrangementView() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void playClicked() override;
    void pauseClicked() override;
    void stopClicked() override;
    void recordClicked() override;
    void exportClicked() override;
    void bpmChanged (double newBPM) override;
    void snapChanged (double snapBeats) override;

    // ArrangementTimeline::Listener
    void timelinePlayheadClicked (double beat) override;

    void rebuild();

private:
    Sketch& sketch;
    ElementLibrary& elementLibrary;
    AudioEngine& audioEngine;

    TransportBar transportBar;
    ArrangementTimeline timeline;
    VisualCanvas visualCanvas;
    ElementPalette elementPalette;

    juce::TextButton paletteToggle;
    bool paletteVisible = true;
    bool wasPlaying = false;
    double playheadBeat = 0.0;
    static constexpr int kPaletteWidth = 140;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArrangementView)
};

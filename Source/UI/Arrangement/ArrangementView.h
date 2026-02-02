#pragma once
#include <JuceHeader.h>
#include "TransportBar.h"
#include "Timeline.h"
#include "Visuals/VisualCanvas.h"
#include "Model/Sketch.h"
#include "Model/ElementLibrary.h"
#include "Audio/AudioEngine.h"

class ArrangementView : public juce::Component,
                        public juce::Timer,
                        public TransportBar::Listener
{
public:
    ArrangementView (Sketch& sketch, ElementLibrary& library, AudioEngine& engine);
    ~ArrangementView() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void playClicked() override;
    void stopClicked() override;
    void recordClicked() override;
    void exportClicked() override;
    void bpmChanged (double newBPM) override;

    void rebuild();

private:
    Sketch& sketch;
    ElementLibrary& elementLibrary;
    AudioEngine& audioEngine;

    TransportBar transportBar;
    ArrangementTimeline timeline;
    VisualCanvas visualCanvas;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArrangementView)
};

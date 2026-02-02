#pragma once
#include <JuceHeader.h>
#include "TrackLane.h"
#include "Model/Sketch.h"
#include "Model/ElementLibrary.h"

// ArrangementTimeline — FL Studio playlist-style timeline with track lanes and clips
class ArrangementTimeline : public juce::Component,
                            public TrackLane::Listener
{
public:
    ArrangementTimeline (Sketch& sketch, ElementLibrary& library);

    void paint (juce::Graphics& g) override;
    void resized() override;

    void rebuild();
    void setPlayheadBeat (double beat);
    void rebuildClips();

    // TrackLane::Listener
    void trackClipAdded (const juce::String& elementId, double beat) override;
    void trackClipRemoved (int clipIndex) override;

private:
    Sketch& sketch;
    ElementLibrary& elementLibrary;

    juce::OwnedArray<TrackLane> trackLanes;
    juce::Viewport gridViewport;
    juce::Component gridContainer;

    double playheadBeat = -1.0;

    static constexpr int kRulerHeight = 24;

    void paintRuler (juce::Graphics& g, juce::Rectangle<int> area);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArrangementTimeline)
};

#pragma once
#include <JuceHeader.h>
#include "TrackLane.h"
#include "Model/Sketch.h"
#include "Model/ElementLibrary.h"
#include "Audio/SampleManager.h"
#include "UI/Common/TouchConstants.h"

class ArrangementTimeline : public juce::Component,
                            public TrackLane::Listener
{
public:
    ArrangementTimeline (Sketch& sketch, ElementLibrary& library, SampleManager& sampleManager);

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void mouseMagnify (const juce::MouseEvent& e, float scaleFactor) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

    void rebuild();
    void setPlayheadBeat (double beat);
    void rebuildClips();

    void setPixelsPerBeat (double ppb);
    void setSnapBeats (double snap);
    void setBPM (double bpm);
    double getPixelsPerBeat() const { return pixelsPerBeat; }

    // TrackLane::Listener
    void trackClipAdded (const juce::String& elementId, int track, double beat) override;
    void trackClipRemoved (int clipIndex) override;
    void trackClipMoved (int clipIndex, double newBeat) override;
    void trackClipResized (int clipIndex, double newDuration) override;
    void trackClipTrackChanged (int clipIndex, int deltaTrack) override;
    void trackClipDragEnded (int clipIndex) override;

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void timelinePlayheadClicked (double beat) = 0;
    };

    void addListener (Listener* l)    { timelineListeners.add (l); }
    void removeListener (Listener* l) { timelineListeners.remove (l); }

private:
    Sketch& sketch;
    ElementLibrary& elementLibrary;
    SampleManager& sampleManager;

    juce::OwnedArray<TrackLane> trackLanes;

    class GridDropTarget : public juce::Component,
                           public juce::DragAndDropTarget
    {
    public:
        GridDropTarget (ArrangementTimeline& o) : owner (o) {}

        bool isInterestedInDragSource (const SourceDetails& details) override;
        void itemDropped (const SourceDetails& details) override;
        void itemDragEnter (const SourceDetails&) override {}
        void itemDragExit (const SourceDetails&) override {}

    private:
        ArrangementTimeline& owner;
    };

    GridDropTarget gridContainer;
    juce::Viewport gridViewport;

    double playheadBeat = -1.0;
    double pixelsPerBeat = 60.0;
    double snapBeats = 1.0;
    double bpm = 120.0;

    bool draggingPlayhead = false;
    bool needsRebuildAfterDrag = false;
    static constexpr int kNumTracks = 12;
    static constexpr int kRulerHeight = 40;
    static constexpr double kMinPixelsPerBeat = 15.0;
    static constexpr double kMaxPixelsPerBeat = 240.0;

    juce::ListenerList<Listener> timelineListeners;

    void paintRuler (juce::Graphics& g, juce::Rectangle<int> area);
    void updateTrackLaneZoom();
    TrackLane* findLaneAtY (int y);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArrangementTimeline)
};

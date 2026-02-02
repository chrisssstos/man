#pragma once
#include <JuceHeader.h>
#include "ClipComponent.h"
#include "Model/Sketch.h"
#include "Model/ElementLibrary.h"
#include "Audio/SampleManager.h"

class TrackLane : public juce::Component,
                  public juce::DragAndDropTarget,
                  public ClipComponent::Listener
{
public:
    static constexpr int kHeaderWidth = 50;
    static constexpr int kRowHeight   = 40;

    TrackLane (int trackIndex, Sketch& sketch, ElementLibrary& library,
              SampleManager& sampleManager);

    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;

    void rebuildClips();
    void setPlayheadBeat (double beat);

    void setPixelsPerBeat (double ppb);
    void setSnapBeats (double snap);
    double getPixelsPerBeat() const { return pixelsPerBeat; }
    double getSnapBeats() const { return snapBeats; }

    int getTrackIndex() const { return trackIndex; }

    // DragAndDropTarget
    bool isInterestedInDragSource (const SourceDetails& details) override;
    void itemDragEnter (const SourceDetails& details) override;
    void itemDragMove (const SourceDetails& details) override;
    void itemDragExit (const SourceDetails& details) override;
    void itemDropped (const SourceDetails& details) override;

    // ClipComponent::Listener
    void clipMoved (int clipIndex, double newBeat) override;
    void clipResized (int clipIndex, double newDuration) override;
    void clipDeleted (int clipIndex) override;
    void clipTrackChanged (int clipIndex, int deltaTrack) override;

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void trackClipAdded (const juce::String& elementId, int track, double beat) = 0;
        virtual void trackClipRemoved (int clipIndex) = 0;
        virtual void trackClipMoved (int clipIndex, double newBeat) = 0;
        virtual void trackClipResized (int clipIndex, double newDuration) = 0;
        virtual void trackClipTrackChanged (int clipIndex, int deltaTrack) = 0;
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    int trackIndex;
    Sketch& sketch;
    ElementLibrary& elementLibrary;
    SampleManager& sampleManager;
    double playheadBeat = -1.0;
    double pixelsPerBeat = 60.0;
    double snapBeats = 1.0;

    bool dragHovering = false;
    int dragHoverX = 0;

    juce::OwnedArray<ClipComponent> clipComponents;
    juce::ListenerList<Listener> listeners;

    void layoutClips();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackLane)
};

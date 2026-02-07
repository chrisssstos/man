#pragma once
#include <JuceHeader.h>
#include "UI/Common/TouchConstants.h"

class ClipComponent : public juce::Component, public juce::Timer
{
public:
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void clipMoved (int clipIndex, double newBeat) = 0;
        virtual void clipResized (int clipIndex, double newDuration) = 0;
        virtual void clipDeleted (int clipIndex) = 0;
        virtual void clipTrackChanged (int clipIndex, int deltaTrack) = 0;
        virtual void clipDragEnded (int clipIndex) = 0;
    };

    ClipComponent (int clipIndex, const juce::String& name, juce::Colour colour,
                   double startBeat, double durationBeats);

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;
    void timerCallback() override;

    int getClipIndex() const { return clipIndex; }
    void setClipIndex (int idx) { clipIndex = idx; }

    double getStartBeat() const     { return startBeat; }
    double getDurationBeats() const { return durationBeats; }

    void setPixelsPerBeat (double ppb)   { pixelsPerBeat = ppb; }
    void setSnapBeats (double snap)      { snapValue = snap; }
    void setHeaderWidth (int w)          { headerWidth = w; }
    void setRowHeight (int h)            { rowHeight = h; }
    int getVisualTrackOffset() const     { return visualTrackOffset; }

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

    void setWaveform (const juce::AudioBuffer<float>* buffer, int numSamples);

private:
    static constexpr int kResizeHandleWidth = TouchUI::kClipResizeHandle;
    static constexpr int kLongPressMs = 500;

    int clipIndex;
    juce::String name;
    juce::Colour colour;
    double startBeat;
    double durationBeats;

    double pixelsPerBeat = 60.0;
    double snapValue = 1.0;
    int headerWidth = TouchUI::kTrackHeaderWidth;

    bool hovering = false;
    bool dragging = false;
    bool resizing = false;
    bool longPressTriggered = false;
    double dragOffsetBeats = 0.0;
    double dragStartBeat = 0.0;
    double dragStartDuration = 0.0;
    int rowHeight = TouchUI::kTrackRowHeight;
    int dragAccumulatedY = 0;
    int visualTrackOffset = 0;

    juce::ListenerList<Listener> listeners;
    juce::Path waveformPath;

    bool isInResizeZone (const juce::Point<int>& pos) const;
    double snapBeat (double beat) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipComponent)
};

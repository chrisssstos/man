#pragma once
#include <JuceHeader.h>
#include "ClipComponent.h"
#include "Model/Sketch.h"

// TrackLane — one horizontal lane for an AV element in the arrangement
class TrackLane : public juce::Component
{
public:
    static constexpr int kHeaderWidth = 160;
    static constexpr int kRowHeight   = 40;
    static constexpr int kPixelsPerBeat = 60;

    TrackLane (const juce::String& elementId, const juce::String& name,
              juce::Colour colour, Sketch& sketch);

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;

    void rebuildClips();
    void setPlayheadBeat (double beat);

    const juce::String& getElementId() const { return elementId; }

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void trackClipAdded (const juce::String& elementId, double beat) = 0;
        virtual void trackClipRemoved (int clipIndex) = 0;
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    juce::String elementId;
    juce::String name;
    juce::Colour colour;
    Sketch& sketch;
    double playheadBeat = -1.0;

    juce::OwnedArray<ClipComponent> clipComponents;
    juce::ListenerList<Listener> listeners;

    void layoutClips();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackLane)
};

#pragma once
#include <JuceHeader.h>
#include "Model/Element.h"
#include "Audio/SampleManager.h"
#include "Audio/AudioEngine.h"
#include "Visuals/VideoDecoder.h"
#include "UI/Common/TouchConstants.h"

class ElementEditor : public juce::Component
{
public:
    ElementEditor (AudioEngine& audioEngine, SampleManager& sampleManager);

    void openForSound (SoundElement* element);
    void openForVisual (VisualElement* element);
    void close();
    bool isOpen() const { return visible; }

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void editorClosed (bool trimApplied) = 0;
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    AudioEngine& audioEngine;
    SampleManager& sampleManager;

    bool visible = false;
    bool isAudioMode = true;

    SoundElement* soundElement = nullptr;
    VisualElement* visualElement = nullptr;

    float trimStart = 0.0f;
    float trimEnd = 1.0f;

    // Drag state
    enum class DragTarget { None, Start, End };
    DragTarget dragTarget = DragTarget::None;

    // Waveform cache
    juce::Path waveformPath;
    bool waveformBuilt = false;

    // Video filmstrip cache
    std::vector<juce::Image> filmstripFrames;
    bool filmstripBuilt = false;
    VideoDecoder filmstripDecoder;

    // Layout helpers
    juce::Rectangle<int> getHeaderBounds() const;
    juce::Rectangle<int> getContentBounds() const;
    juce::Rectangle<int> getFooterBounds() const;
    juce::Rectangle<int> getDoneButtonBounds() const;
    juce::Rectangle<int> getPlayButtonBounds() const;
    float xForTrimValue (float val) const;
    float trimValueForX (float x) const;

    void buildWaveform();
    void buildFilmstrip();
    void drawWaveform (juce::Graphics& g, juce::Rectangle<int> area);
    void drawFilmstrip (juce::Graphics& g, juce::Rectangle<int> area);
    void drawTrimHandles (juce::Graphics& g, juce::Rectangle<int> area);
    void drawOverlays (juce::Graphics& g, juce::Rectangle<int> area);

    juce::String formatTime (float fraction) const;
    juce::File resolveSampleFile (const juce::String& path) const;

    juce::ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ElementEditor)
};

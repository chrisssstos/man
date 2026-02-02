#pragma once
#include <JuceHeader.h>
#include "Model/Element.h"
#include "Visuals/VisualRenderer.h"
#include "Audio/SampleManager.h"

class ElementTile : public juce::Component, public juce::Timer
{
public:
    ElementTile (Element* element, SampleManager* sampleManager = nullptr);
    ~ElementTile() override;

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;
    void timerCallback() override;

    Element* getElement() const { return element; }

    bool isSelected() const { return selected; }
    void setSelected (bool s);
    bool isActive() const { return active; }
    void setActive (bool a);

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void tileClicked (ElementTile*) = 0;
        virtual void tileDoubleClicked (ElementTile*) {}
        virtual void tilePlayClicked (ElementTile*) {}
        virtual void tilePressed (ElementTile*) {}
        virtual void tileReleased (ElementTile*) {}
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

    enum class ElementType { Sound, Visual, AudioVisual };
    ElementType getElementType() const { return elemType; }

private:
    Element* element;
    ElementType elemType;
    std::unique_ptr<VisualRenderer> renderer;
    bool selected = false;
    bool active = false;
    float animPhase = 0.0f;
    float dashOffset = 0.0f;
    juce::Colour tileColour;
    juce::StringPairArray visualParams;
    bool playButtonHovered = false;
    bool needsAnimation = false;

    // Cached paint data
    juce::Path waveformPath;
    bool waveformBuilt = false;
    SampleManager* sampleMgr = nullptr;

    juce::ListenerList<Listener> listeners;

    juce::Rectangle<float> getPlayButtonBounds() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ElementTile)
};

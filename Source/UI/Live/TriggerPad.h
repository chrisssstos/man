#pragma once
#include <JuceHeader.h>
#include "Model/Element.h"
#include "Visuals/VisualRenderer.h"
#include "UI/Common/TouchConstants.h"

class TriggerPad : public juce::Component, public juce::Timer
{
public:
    TriggerPad (AudioVisualElement* element, const juce::String& keyLabel);
    ~TriggerPad() override;

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void timerCallback() override;

    AudioVisualElement* getElement() const { return element; }
    bool isActive() const { return active; }
    void setActive (bool a);

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void padPressed (TriggerPad*) = 0;
        virtual void padReleased (TriggerPad*) = 0;
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    AudioVisualElement* element;
    juce::String keyLabel;
    bool active = false;
    float animPhase = 0.0f;
    std::unique_ptr<VisualRenderer> renderer;

    juce::ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriggerPad)
};

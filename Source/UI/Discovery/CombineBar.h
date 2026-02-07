#pragma once
#include <JuceHeader.h>
#include "Model/Element.h"
#include "UI/Common/TouchConstants.h"

class CombineBar : public juce::Component, public juce::Timer
{
public:
    CombineBar();

    void setSoundSlot (SoundElement* snd);
    void setVisualSlot (VisualElement* vis);
    void clearSoundSlot();
    void clearVisualSlot();

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void timerCallback() override;

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void combineRequested() = 0;
        virtual void slotCleared (bool isSound) = 0;
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    SoundElement* soundSlot = nullptr;
    VisualElement* visualSlot = nullptr;
    float pulsePhase = 0.0f;

    juce::ListenerList<Listener> listeners;

    juce::Rectangle<float> getSoundSlotBounds() const;
    juce::Rectangle<float> getVisualSlotBounds() const;
    juce::Rectangle<float> getCombineButtonBounds() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CombineBar)
};

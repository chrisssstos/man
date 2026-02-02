#pragma once
#include <JuceHeader.h>
#include "TriggerPad.h"
#include "Model/ElementLibrary.h"

class TriggerGrid : public juce::Component, public TriggerPad::Listener
{
public:
    TriggerGrid (ElementLibrary& library);

    void rebuild();
    void paint (juce::Graphics& g) override;
    void resized() override;

    void padPressed (TriggerPad* pad) override;
    void padReleased (TriggerPad* pad) override;

    // Keyboard mapping
    TriggerPad* getPadForKey (int keyCode) const;

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void padTriggered (const ElementID& id) = 0;
        virtual void padReleased (const ElementID& id) = 0;
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    ElementLibrary& library;
    juce::OwnedArray<TriggerPad> pads;
    juce::ListenerList<Listener> listeners;

    static const juce::StringArray& getKeyLabels();
    static const std::vector<int>& getKeyCodes();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriggerGrid)
};

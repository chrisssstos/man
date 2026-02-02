#pragma once
#include <JuceHeader.h>
#include "Model/Element.h"

class CombineZone : public juce::Component, public juce::DragAndDropTarget
{
public:
    CombineZone();

    void paint (juce::Graphics& g) override;

    bool isInterestedInDragSource (const SourceDetails& details) override;
    void itemDragEnter (const SourceDetails&) override;
    void itemDragExit (const SourceDetails&) override;
    void itemDropped (const SourceDetails& details) override;

    void setSlotA (const ElementID& id, const juce::String& name);
    void setSlotB (const ElementID& id, const juce::String& name);
    void clearSlots();

    const ElementID& getSlotA() const { return slotA; }
    const ElementID& getSlotB() const { return slotB; }

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void onCombineRequest (const ElementID& a, const ElementID& b) = 0;
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    ElementID slotA, slotB;
    juce::String slotAName, slotBName;
    bool hovering = false;

    juce::TextButton combineButton { "Combine!" };
    juce::ListenerList<Listener> listeners;

    void checkAndCombine();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CombineZone)
};

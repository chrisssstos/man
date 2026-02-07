#pragma once
#include <JuceHeader.h>
#include "TouchConstants.h"

class ModeSelector : public juce::Component
{
public:
    enum class Mode { Discovery, Arrangement, Live };

    ModeSelector();

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;

    Mode getCurrentMode() const { return currentMode; }
    void setModeExternal (Mode mode);

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void modeChanged (Mode newMode) = 0;
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    Mode currentMode = Mode::Discovery;
    juce::ListenerList<Listener> listeners;

    void setMode (Mode mode);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModeSelector)
};

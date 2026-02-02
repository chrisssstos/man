#pragma once
#include <JuceHeader.h>

class ModeSelector : public juce::Component
{
public:
    enum class Mode { Discovery, Arrangement, Live };

    ModeSelector();

    void paint (juce::Graphics& g) override;
    void resized() override;

    Mode getCurrentMode() const { return currentMode; }

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void modeChanged (Mode newMode) = 0;
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    Mode currentMode = Mode::Discovery;
    juce::TextButton discoveryButton { "Discovery" };
    juce::TextButton arrangementButton { "Arrange" };
    juce::TextButton liveButton { "Live" };

    juce::ListenerList<Listener> listeners;

    void setMode (Mode mode);
    void updateButtonStates();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModeSelector)
};

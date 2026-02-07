#pragma once
#include <JuceHeader.h>
#include "UI/Common/TouchConstants.h"

class TransportBar : public juce::Component
{
public:
    TransportBar();

    void paint (juce::Graphics& g) override;
    void resized() override;

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void playClicked() = 0;
        virtual void pauseClicked() = 0;
        virtual void stopClicked() = 0;
        virtual void recordClicked() = 0;
        virtual void exportClicked() = 0;
        virtual void bpmChanged (double newBPM) = 0;
        virtual void snapChanged (double snapBeats) = 0;
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

    void setPlaying (bool playing);
    void setBPM (double bpm);
    void setCurrentBeat (double beat);

private:
    juce::TextButton playButton { "Play" };
    juce::TextButton pauseButton { "Pause" };
    juce::TextButton stopButton { "Stop" };
    juce::TextButton recordButton { "Record" };
    juce::TextButton exportButton { "Export" };
    juce::Slider bpmSlider;
    juce::Label beatLabel;
    juce::ComboBox snapCombo;
    juce::Label snapLabel;

    double currentBPM = 120.0;

    juce::ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransportBar)
};

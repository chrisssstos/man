#pragma once
#include <JuceHeader.h>
#include "TriggerGrid.h"
#include "Visuals/VisualCanvas.h"
#include "Model/ElementLibrary.h"
#include "Model/Sketch.h"
#include "Audio/AudioEngine.h"

class LiveView : public juce::Component,
                 public TriggerGrid::Listener,
                 public juce::KeyListener
{
public:
    LiveView (ElementLibrary& library, AudioEngine& engine, Sketch& sketch);
    ~LiveView() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void padTriggered (const ElementID& id) override;
    void padReleased (const ElementID& id) override;

    bool keyPressed (const juce::KeyPress& key, juce::Component*) override;
    bool keyStateChanged (bool isKeyDown, juce::Component*) override;

    void rebuild();

private:
    ElementLibrary& elementLibrary;
    AudioEngine& audioEngine;
    Sketch& sketch;

    VisualCanvas visualCanvas;
    TriggerGrid triggerGrid;
    juce::TextButton recordButton { "Record" };
    juce::TextButton stopButton { "Stop" };

    std::set<int> heldKeys;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LiveView)
};

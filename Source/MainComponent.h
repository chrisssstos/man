#pragma once
#include <JuceHeader.h>
#include "Model/ElementLibrary.h"
#include "Model/RecipeBook.h"
#include "Model/DiscoveryLog.h"
#include "Model/Sketch.h"
#include "Audio/AudioEngine.h"
#include "Audio/SampleManager.h"
#include "UI/Common/ModeSelector.h"
#include "UI/Discovery/DiscoveryView.h"
#include "UI/Arrangement/ArrangementView.h"
#include "UI/Live/LiveView.h"

class MainComponent : public juce::Component,
                      public juce::DragAndDropContainer,
                      public ModeSelector::Listener
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void modeChanged (ModeSelector::Mode newMode) override;

    ElementLibrary& getElementLibrary()  { return elementLibrary; }
    RecipeBook& getRecipeBook()          { return recipeBook; }
    DiscoveryLog& getDiscoveryLog()      { return discoveryLog; }
    Sketch& getSketch()                  { return sketch; }
    AudioEngine& getAudioEngine()        { return audioEngine; }

private:
    ElementLibrary elementLibrary;
    RecipeBook recipeBook;
    DiscoveryLog discoveryLog;
    Sketch sketch;
    SampleManager sampleManager;
    AudioEngine audioEngine;

    ModeSelector modeSelector;
    std::unique_ptr<DiscoveryView> discoveryView;
    std::unique_ptr<ArrangementView> arrangementView;
    std::unique_ptr<LiveView> liveView;

    ModeSelector::Mode currentMode = ModeSelector::Mode::Discovery;

    void initializeBaseElements();
    void showCurrentMode();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

#pragma once
#include <JuceHeader.h>
#include "VisualPanel.h"
#include "SoundPanel.h"
#include "DiscoveryPanel.h"
#include "DiscoveryAnimation.h"
#include "Model/ElementLibrary.h"
#include "Model/RecipeBook.h"
#include "Model/DiscoveryLog.h"
#include "Audio/AudioEngine.h"
#include "Visuals/VisualCanvas.h"

class DiscoveryView : public juce::Component,
                      public ElementTile::Listener,
                      public DiscoveryLog::Listener
{
public:
    DiscoveryView (ElementLibrary& library, RecipeBook& recipeBook,
                   DiscoveryLog& discoveryLog, AudioEngine& audioEngine);
    ~DiscoveryView() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void tileClicked (ElementTile* tile) override;
    void tileDoubleClicked (ElementTile* tile) override;
    void tilePlayClicked (ElementTile* tile) override;
    void tilePressed (ElementTile* tile) override;
    void tileReleased (ElementTile* tile) override;
    void onNewDiscovery (const ElementID& id) override;

    void rebuild();

private:
    ElementLibrary& elementLibrary;
    RecipeBook& recipeBook;
    DiscoveryLog& discoveryLog;
    AudioEngine& audioEngine;

    // Panel 1 (left) - Sounds
    SoundPanel soundPanel;

    // Panel 2 (right) - Visuals
    VisualPanel visualPanel;

    // Combine button bar
    juce::TextButton combineButton { "COMBINE" };

    // Panel 3 (bottom 16:9) - Combined AV library + preview
    DiscoveryPanel discoveryPanel;
    VisualCanvas visualCanvas;

    DiscoveryAnimation discoveryAnimation;

    // Selection tracking
    ElementTile* selectedSound = nullptr;
    ElementTile* selectedVisual = nullptr;

    bool tryCombine (const ElementID& a, const ElementID& b);
    void updateCombineButton();
    void previewSoundTile (ElementTile* tile);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiscoveryView)
};

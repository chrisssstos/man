#pragma once
#include <JuceHeader.h>
#include "VisualPanel.h"
#include "SoundPanel.h"
#include "DiscoveryPanel.h"
#include "CombineBar.h"
#include "ElementEditor.h"
#include "DiscoveryAnimation.h"
#include "Model/ElementLibrary.h"
#include "Model/RecipeBook.h"
#include "Model/DiscoveryLog.h"
#include "Audio/AudioEngine.h"
#include "Visuals/VisualCanvas.h"
#include "UI/Common/TouchConstants.h"

class DiscoveryView : public juce::Component,
                      public ElementTile::Listener,
                      public DiscoveryLog::Listener,
                      public CombineBar::Listener,
                      public ElementEditor::Listener
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

    // CombineBar::Listener
    void combineRequested() override;
    void slotCleared (bool isSound) override;

    // ElementEditor::Listener
    void editorClosed (bool trimApplied) override;

    void rebuild();

private:
    ElementLibrary& elementLibrary;
    RecipeBook& recipeBook;
    DiscoveryLog& discoveryLog;
    AudioEngine& audioEngine;

    // Panels (split-screen layout)
    SoundPanel soundPanel;
    VisualPanel visualPanel;
    DiscoveryPanel discoveryPanel;

    // Floating visual canvas (corner preview)
    VisualCanvas visualCanvas;

    // Combine bar (center divider)
    CombineBar combineBar;

    // Trim editor overlay
    ElementEditor elementEditor;

    DiscoveryAnimation discoveryAnimation;

    // Selection tracking
    ElementTile* selectedSound = nullptr;
    ElementTile* selectedVisual = nullptr;

    bool tryCombine (const ElementID& a, const ElementID& b);
    void updateCombineBar();
    void previewSoundTile (ElementTile* tile);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiscoveryView)
};

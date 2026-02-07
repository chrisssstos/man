#pragma once
#include <JuceHeader.h>
#include "ElementTile.h"
#include "Model/ElementLibrary.h"
#include "Audio/AudioEngine.h"
#include "UI/Common/TouchConstants.h"

class SoundPanel : public juce::Component,
                   public juce::DragAndDropTarget,
                   public juce::FileDragAndDropTarget
{
public:
    SoundPanel (ElementLibrary& library, AudioEngine& audioEngine);

    void rebuild();
    void paint (juce::Graphics& g) override;
    void resized() override;

    void setTileListener (ElementTile::Listener* l) { tileListener = l; }

    // Selection
    ElementTile* getSelectedTile() const { return selectedTile; }
    void clearSelection();

    // DragAndDropTarget
    bool isInterestedInDragSource (const SourceDetails& details) override;
    void itemDragEnter (const SourceDetails&) override;
    void itemDragExit (const SourceDetails&) override;
    void itemDropped (const SourceDetails& details) override;

    // FileDragAndDropTarget (OS file drops)
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

private:
    ElementLibrary& library;
    AudioEngine& audioEngine;
    juce::OwnedArray<ElementTile> tiles;
    juce::Viewport viewport;
    juce::Component tileContainer;
    ElementTile::Listener* tileListener = nullptr;
    ElementTile* selectedTile = nullptr;
    bool hovering = false;

    juce::TextButton importButton { "+" };
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundPanel)
};

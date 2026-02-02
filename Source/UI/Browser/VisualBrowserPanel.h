#pragma once
#include <JuceHeader.h>
#include "VisualParamEditor.h"
#include "UI/Common/VisualPreview.h"
#include "Model/ElementLibrary.h"

class VisualBrowserPanel : public juce::Component,
                           public VisualParamEditor::Listener
{
public:
    VisualBrowserPanel (ElementLibrary& library);
    ~VisualBrowserPanel() override;

    void resized() override;
    void paint (juce::Graphics& g) override;
    void paramChanged() override;

private:
    ElementLibrary& elementLibrary;

    struct VisualPresetData
    {
        juce::String name;
        VisualElement::VisualKind kind;
        juce::Colour colour;
        juce::StringPairArray params;
    };

    std::vector<VisualPresetData> presets;
    int selectedPreset = 0;

    // Current editing state
    VisualPresetData editingPreset;

    juce::ListBox presetList;
    std::unique_ptr<juce::ListBoxModel> presetModel;
    VisualPreview preview;
    VisualParamEditor paramEditor;

    juce::TextButton addToLibraryButton { "Add to Library" };
    juce::TextButton loadImageButton { "Load Image..." };

    // Colour button
    juce::TextButton colourButton { "Colour" };

    void initPresets();
    void selectPreset (int index);
    void updatePreview();

    class PresetListModel;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VisualBrowserPanel)
};

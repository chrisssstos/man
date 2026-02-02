#include "VisualBrowserPanel.h"

class VisualBrowserPanel::PresetListModel : public juce::ListBoxModel
{
public:
    PresetListModel (VisualBrowserPanel& o) : owner (o) {}
    int getNumRows() override { return (int) owner.presets.size(); }

    void paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected) override
    {
        if (row < 0 || row >= (int) owner.presets.size())
            return;

        if (selected)
            g.fillAll (juce::Colour (0xff2a2a5a));
        else
            g.fillAll (row % 2 == 0 ? juce::Colour (0xff141430) : juce::Colour (0xff181840));

        auto& preset = owner.presets[(size_t) row];
        g.setColour (preset.colour);
        g.fillRect (4, 4, height - 8, height - 8);

        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.setFont (13.0f);
        g.drawText (preset.name, height + 4, 0, width - height - 8, height,
                    juce::Justification::centredLeft);
    }

    void selectedRowsChanged (int lastRowSelected) override
    {
        if (lastRowSelected >= 0)
            owner.selectPreset (lastRowSelected);
    }

private:
    VisualBrowserPanel& owner;
};

VisualBrowserPanel::VisualBrowserPanel (ElementLibrary& lib)
    : elementLibrary (lib)
{
    initPresets();

    presetModel = std::make_unique<PresetListModel> (*this);
    presetList.setModel (presetModel.get());
    presetList.setRowHeight (28);
    presetList.setColour (juce::ListBox::backgroundColourId, juce::Colour (0xff12122a));
    addAndMakeVisible (presetList);

    addAndMakeVisible (preview);
    paramEditor.addListener (this);
    addAndMakeVisible (paramEditor);

    addToLibraryButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a6a2a));
    addAndMakeVisible (addToLibraryButton);

    loadImageButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2a5a));
    addAndMakeVisible (loadImageButton);

    colourButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2a5a));
    addAndMakeVisible (colourButton);

    addToLibraryButton.onClick = [this]
    {
        elementLibrary.addUserVisual (editingPreset.name, editingPreset.kind,
                                      editingPreset.colour, editingPreset.params);
    };

    loadImageButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser> (
            "Select Image", juce::File(), "*.png;*.jpg;*.jpeg;*.gif;*.bmp");

        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc)
            {
                auto result = fc.getResult();
                if (result.existsAsFile())
                {
                    juce::StringPairArray imgParams;
                    imgParams.set ("imagePath", result.getFullPathName());
                    elementLibrary.addUserVisual (result.getFileNameWithoutExtension(),
                                                  VisualElement::VisualKind::Image,
                                                  juce::Colours::white, imgParams);
                }
            });
    };

    colourButton.onClick = [this]
    {
        // Cycle through a palette of colours
        static const juce::Colour palette[] = {
            juce::Colour (0xff00ddff), juce::Colour (0xffff8800),
            juce::Colour (0xff88ff00), juce::Colour (0xffff4444),
            juce::Colour (0xff4488ff), juce::Colour (0xffaa44ff),
            juce::Colour (0xffffdd44), juce::Colour (0xffff66cc),
            juce::Colour (0xff44ffaa), juce::Colour (0xffffffdd)
        };
        static int colourIndex = 0;
        colourIndex = (colourIndex + 1) % 10;
        editingPreset.colour = palette[colourIndex];
        colourButton.setColour (juce::TextButton::buttonColourId, editingPreset.colour);
        updatePreview();
    };

    if (! presets.empty())
    {
        presetList.selectRow (0);
        selectPreset (0);
    }
}

VisualBrowserPanel::~VisualBrowserPanel()
{
    paramEditor.removeListener (this);
}

void VisualBrowserPanel::initPresets()
{
    // Waveform presets
    auto addWaveform = [this] (const juce::String& name, const juce::String& type,
                                double freq, double amp, juce::Colour col)
    {
        VisualPresetData p;
        p.name = name; p.kind = VisualElement::VisualKind::Waveform; p.colour = col;
        p.params.set ("frequency", juce::String (freq));
        p.params.set ("amplitude", juce::String (amp));
        p.params.set ("type", type);
        presets.push_back (p);
    };

    addWaveform ("Sine Wave",       "sine",   2.0, 0.5, juce::Colour (0xff00ddff));
    addWaveform ("Fast Sine",       "sine",   6.0, 0.8, juce::Colour (0xff00ffaa));
    addWaveform ("Saw Wave",        "saw",    3.0, 0.6, juce::Colour (0xffff8800));
    addWaveform ("Sharp Saw",       "saw",    8.0, 0.9, juce::Colour (0xffff4400));
    addWaveform ("Square Wave",     "square", 2.0, 0.7, juce::Colour (0xff88ff00));
    addWaveform ("Pulse Square",    "square", 5.0, 0.5, juce::Colour (0xffaaff44));
    addWaveform ("Gentle Sine",     "sine",   1.0, 0.3, juce::Colour (0xff8888ff));

    // Geometric presets
    auto addGeometric = [this] (const juce::String& name, int sides, double rot,
                                 bool fill, juce::Colour col)
    {
        VisualPresetData p;
        p.name = name; p.kind = VisualElement::VisualKind::Geometric; p.colour = col;
        p.params.set ("sides", juce::String (sides));
        p.params.set ("rotation_speed", juce::String (rot));
        p.params.set ("fill", fill ? "true" : "false");
        presets.push_back (p);
    };

    addGeometric ("Triangle",       3, 1.0, true,  juce::Colour (0xffff4444));
    addGeometric ("Spinning Tri",   3, 4.0, false, juce::Colour (0xffff6666));
    addGeometric ("Square",         4, 0.5, true,  juce::Colour (0xff44ff44));
    addGeometric ("Pentagon",       5, 0.8, true,  juce::Colour (0xffff88ff));
    addGeometric ("Hexagon",        6, 0.5, true,  juce::Colour (0xff4488ff));
    addGeometric ("Octagon",        8, 0.3, false, juce::Colour (0xffaa44ff));
    addGeometric ("Dodecagon",     12, 0.2, true,  juce::Colour (0xffddaaff));

    // Particle presets
    auto addParticle = [this] (const juce::String& name, int count, double speed,
                                double size, juce::Colour col)
    {
        VisualPresetData p;
        p.name = name; p.kind = VisualElement::VisualKind::Particle; p.colour = col;
        p.params.set ("count", juce::String (count));
        p.params.set ("speed", juce::String (speed));
        p.params.set ("size", juce::String (size));
        presets.push_back (p);
    };

    addParticle ("Sparks",         20, 2.0, 3.0, juce::Colour (0xffffdd44));
    addParticle ("Fireflies",      15, 0.8, 5.0, juce::Colour (0xff88ff44));
    addParticle ("Storm",          60, 4.0, 2.0, juce::Colour (0xffaaaaff));
    addParticle ("Dust",           40, 0.5, 2.0, juce::Colour (0xff888888));
    addParticle ("Stars",          30, 0.3, 4.0, juce::Colour (0xffffffdd));
    addParticle ("Confetti",       50, 3.0, 3.5, juce::Colour (0xffff66cc));
}

void VisualBrowserPanel::selectPreset (int index)
{
    if (index < 0 || index >= (int) presets.size())
        return;

    selectedPreset = index;
    editingPreset = presets[(size_t) index];
    paramEditor.setKind (editingPreset.kind, editingPreset.params, editingPreset.colour);
    updatePreview();
}

void VisualBrowserPanel::paramChanged()
{
    updatePreview();
}

void VisualBrowserPanel::updatePreview()
{
    preview.setElement (editingPreset.kind, editingPreset.colour, editingPreset.params);
}

void VisualBrowserPanel::resized()
{
    auto area = getLocalBounds();

    // Preview at top
    preview.setBounds (area.removeFromTop (120));

    // Param editor
    int paramH = std::min (150, area.getHeight() / 3);
    paramEditor.setBounds (area.removeFromTop (paramH));

    // Buttons
    auto buttonRow = area.removeFromBottom (28);
    addToLibraryButton.setBounds (buttonRow.removeFromLeft (buttonRow.getWidth() / 3));
    colourButton.setBounds (buttonRow.removeFromLeft (buttonRow.getWidth() / 2));
    loadImageButton.setBounds (buttonRow);

    // Preset list takes the rest
    presetList.setBounds (area);
}

void VisualBrowserPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0e0e24));
}

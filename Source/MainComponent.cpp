#include "MainComponent.h"
#include "BinaryData.h"
#include "UI/Common/TouchConstants.h"

MainComponent::MainComponent()
    : audioEngine (elementLibrary, sampleManager),
      modeSelector()
{
    // Set the sample directory: look inside the app bundle's Resources first
    auto appBundle = juce::File::getSpecialLocation (juce::File::currentApplicationFile);
    auto samplesDir = appBundle.getChildFile ("Contents/Resources/Samples");
    if (! samplesDir.isDirectory())
    {
        // Fallback for development: look relative to the source tree
        auto exeDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
        samplesDir = exeDir.getChildFile ("Samples");
    }
    sampleManager.setBaseSampleDirectory (samplesDir);

    initializeBaseElements();

    discoveryView = std::make_unique<DiscoveryView> (elementLibrary, recipeBook, discoveryLog, audioEngine);
    arrangementView = std::make_unique<ArrangementView> (sketch, elementLibrary, audioEngine);
    liveView = std::make_unique<LiveView> (elementLibrary, audioEngine, sketch);

    addAndMakeVisible (modeSelector);
    addAndMakeVisible (swipeContainer);

    swipeContainer.addPage (discoveryView.get());
    swipeContainer.addPage (arrangementView.get());
    swipeContainer.addPage (liveView.get());

    modeSelector.addListener (this);
    swipeContainer.addListener (this);

    setSize (1280, 800);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (TouchUI::kBgDeep));
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    modeSelector.setBounds (area.removeFromTop (TouchUI::kModeBarHeight));
    swipeContainer.setBounds (area);
}

void MainComponent::modeChanged (ModeSelector::Mode newMode)
{
    int pageIndex = (int) newMode;

    if (newMode == ModeSelector::Mode::Arrangement)
        arrangementView->rebuild();
    else if (newMode == ModeSelector::Mode::Live)
        liveView->rebuild();

    swipeContainer.setCurrentPage (pageIndex);
}

void MainComponent::swipePageChanged (int newPage)
{
    auto mode = static_cast<ModeSelector::Mode> (newPage);
    modeSelector.setModeExternal (mode);

    if (mode == ModeSelector::Mode::Arrangement)
        arrangementView->rebuild();
    else if (mode == ModeSelector::Mode::Live)
        liveView->rebuild();
}

void MainComponent::initializeBaseElements()
{
    // Visual elements
    {
        juce::StringPairArray p;
        p.set ("frequency", "2.0"); p.set ("amplitude", "0.5"); p.set ("type", "sine");
        elementLibrary.addElement (std::make_unique<VisualElement> (
            ElementID { "vis_waveform_sine" }, "Sine Wave",
            VisualElement::VisualKind::Waveform, juce::Colour (0xff00ddff), p));
    }
    {
        juce::StringPairArray p;
        p.set ("frequency", "3.0"); p.set ("amplitude", "0.6"); p.set ("type", "saw");
        elementLibrary.addElement (std::make_unique<VisualElement> (
            ElementID { "vis_waveform_saw" }, "Saw Wave",
            VisualElement::VisualKind::Waveform, juce::Colour (0xffff8800), p));
    }
    {
        juce::StringPairArray p;
        p.set ("frequency", "2.0"); p.set ("amplitude", "0.7"); p.set ("type", "square");
        elementLibrary.addElement (std::make_unique<VisualElement> (
            ElementID { "vis_waveform_square" }, "Square Wave",
            VisualElement::VisualKind::Waveform, juce::Colour (0xff88ff00), p));
    }
    {
        juce::StringPairArray p;
        p.set ("sides", "3"); p.set ("rotation_speed", "1.0"); p.set ("fill", "true");
        elementLibrary.addElement (std::make_unique<VisualElement> (
            ElementID { "vis_geometric_tri" }, "Triangle",
            VisualElement::VisualKind::Geometric, juce::Colour (0xffff4444), p));
    }
    {
        juce::StringPairArray p;
        p.set ("sides", "6"); p.set ("rotation_speed", "0.5"); p.set ("fill", "true");
        elementLibrary.addElement (std::make_unique<VisualElement> (
            ElementID { "vis_geometric_hex" }, "Hexagon",
            VisualElement::VisualKind::Geometric, juce::Colour (0xff4488ff), p));
    }
    {
        juce::StringPairArray p;
        p.set ("sides", "8"); p.set ("rotation_speed", "0.3"); p.set ("fill", "false");
        elementLibrary.addElement (std::make_unique<VisualElement> (
            ElementID { "vis_geometric_oct" }, "Octagon",
            VisualElement::VisualKind::Geometric, juce::Colour (0xffaa44ff), p));
    }
    {
        juce::StringPairArray p;
        p.set ("count", "20"); p.set ("speed", "2.0"); p.set ("size", "3.0");
        elementLibrary.addElement (std::make_unique<VisualElement> (
            ElementID { "vis_particle_spark" }, "Sparks",
            VisualElement::VisualKind::Particle, juce::Colour (0xffffdd44), p));
    }
    {
        juce::StringPairArray p;
        p.set ("count", "40"); p.set ("speed", "0.5"); p.set ("size", "2.0");
        elementLibrary.addElement (std::make_unique<VisualElement> (
            ElementID { "vis_particle_dust" }, "Dust",
            VisualElement::VisualKind::Particle, juce::Colour (0xff888888), p));
    }

    // Sound elements
    elementLibrary.addElement (std::make_unique<SoundElement> (ElementID { "snd_kick" },      "Kick",       "kick.wav"));
    elementLibrary.addElement (std::make_unique<SoundElement> (ElementID { "snd_snare" },     "Snare",      "snare.wav"));
    elementLibrary.addElement (std::make_unique<SoundElement> (ElementID { "snd_hihat" },     "Hi-Hat",     "hihat.wav"));
    elementLibrary.addElement (std::make_unique<SoundElement> (ElementID { "snd_pad_warm" },  "Warm Pad",   "pad_warm.wav"));
    elementLibrary.addElement (std::make_unique<SoundElement> (ElementID { "snd_bass_pluck" },"Bass Pluck", "bass_pluck.wav"));
    elementLibrary.addElement (std::make_unique<SoundElement> (ElementID { "snd_fx_riser" },  "FX Riser",   "fx_riser.wav"));
    elementLibrary.addElement (std::make_unique<SoundElement> (ElementID { "snd_vocal_chop" },"Vocal Chop", "vocal_chop.wav"));
    elementLibrary.addElement (std::make_unique<SoundElement> (ElementID { "snd_bell_tone" }, "Bell Tone",  "bell_tone.wav"));

    // Load recipes from binary data
    auto jsonStr = juce::String::fromUTF8 (BinaryData::default_recipes_json,
                                            BinaryData::default_recipes_jsonSize);
    recipeBook.loadFromJSON (jsonStr);

    // Pre-create all AV elements from recipes (undiscovered)
    for (const auto& recipe : recipeBook.getAllRecipes())
    {
        elementLibrary.addElement (std::make_unique<AudioVisualElement> (
            recipe.resultId, recipe.resultName,
            recipe.ingredientA, recipe.ingredientB,
            recipe.samplePath, recipe.visualKind,
            recipe.colour, recipe.visualParams));
    }

    audioEngine.rebuildSounds();
}

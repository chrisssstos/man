#include "DiscoveryView.h"

DiscoveryView::DiscoveryView (ElementLibrary& lib, RecipeBook& rb,
                               DiscoveryLog& dl, AudioEngine& ae)
    : elementLibrary (lib), recipeBook (rb), discoveryLog (dl), audioEngine (ae),
      soundPanel (lib, ae), visualPanel (lib), discoveryPanel (lib), visualCanvas (lib)
{
    addAndMakeVisible (soundPanel);
    addAndMakeVisible (visualPanel);
    addAndMakeVisible (combineButton);
    addAndMakeVisible (discoveryPanel);
    addAndMakeVisible (visualCanvas);
    addChildComponent (discoveryAnimation);

    // Set listeners BEFORE rebuild so tiles get them
    soundPanel.setTileListener (this);
    visualPanel.setTileListener (this);
    discoveryPanel.setTileListener (this);
    discoveryLog.addListener (this);

    // Now rebuild panels — tiles will have listeners attached
    soundPanel.rebuild();
    visualPanel.rebuild();
    discoveryPanel.rebuild();

    // Wire the visual canvas to show active AV elements from the audio engine
    visualCanvas.setActiveElementsProvider ([&ae] () { return ae.getActiveElementIds(); });

    combineButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a1a1a));
    combineButton.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    combineButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white.withAlpha (0.5f));
    combineButton.setEnabled (false);

    combineButton.onClick = [this]
    {
        if (selectedSound != nullptr && selectedVisual != nullptr)
        {
            tryCombine (selectedSound->getElement()->getId(),
                        selectedVisual->getElement()->getId());

            // Deselect directly on tiles, then clear pointers
            selectedSound->setSelected (false);
            selectedVisual->setSelected (false);
            selectedSound = nullptr;
            selectedVisual = nullptr;
            updateCombineButton();
        }
    };

    updateCombineButton();
}

DiscoveryView::~DiscoveryView()
{
    discoveryLog.removeListener (this);
}

void DiscoveryView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0a1e));
}

void DiscoveryView::resized()
{
    auto area = getLocalBounds();
    int gap = 4;

    // Combine button bar
    int combineBarH = 44;

    // Panel 3 height: 16:9 aspect ratio based on width
    int panel3H = juce::jmax (200, (area.getWidth() * 9) / 16);
    // Cap it so panels above still have space
    int topPanelMinH = 250;
    int availableForBottom = area.getHeight() - combineBarH - gap * 3;
    panel3H = juce::jmin (panel3H, availableForBottom - topPanelMinH);
    panel3H = juce::jmax (150, panel3H);

    int topPanelH = area.getHeight() - combineBarH - panel3H - gap * 3;

    // Top row: Sounds (left) | Visuals (right)
    auto topArea = area.removeFromTop (topPanelH);
    int halfW = (topArea.getWidth() - gap) / 2;
    soundPanel.setBounds (topArea.removeFromLeft (halfW));
    topArea.removeFromLeft (gap);
    visualPanel.setBounds (topArea);

    area.removeFromTop (gap);

    // Combine button bar
    combineButton.setBounds (area.removeFromTop (combineBarH));

    area.removeFromTop (gap);

    // Panel 3: split between discovery tiles and visual canvas preview
    auto bottomArea = area;
    int canvasW = (bottomArea.getWidth() * 40) / 100;
    auto canvasArea = bottomArea.removeFromRight (canvasW);
    visualCanvas.setBounds (canvasArea);
    discoveryPanel.setBounds (bottomArea);

    discoveryAnimation.setBounds (getLocalBounds());
}

void DiscoveryView::tileClicked (ElementTile* tile)
{
    auto type = tile->getElementType();

    if (type == ElementTile::ElementType::Sound)
    {
        if (selectedSound == tile)
        {
            // Toggle off
            tile->setSelected (false);
            selectedSound = nullptr;
        }
        else
        {
            // Deselect previous sound
            if (selectedSound != nullptr)
                selectedSound->setSelected (false);
            selectedSound = tile;
            tile->setSelected (true);

            // Auto-preview the sound
            previewSoundTile (tile);
        }
    }
    else if (type == ElementTile::ElementType::Visual)
    {
        if (selectedVisual == tile)
        {
            // Toggle off
            tile->setSelected (false);
            selectedVisual = nullptr;
        }
        else
        {
            // Deselect previous visual
            if (selectedVisual != nullptr)
                selectedVisual->setSelected (false);
            selectedVisual = tile;
            tile->setSelected (true);
        }
    }
    updateCombineButton();
}

void DiscoveryView::tileDoubleClicked (ElementTile*) {}

void DiscoveryView::tilePressed (ElementTile* tile)
{
    audioEngine.triggerElement (tile->getElement()->getId());
}

void DiscoveryView::tileReleased (ElementTile* tile)
{
    audioEngine.stopElement (tile->getElement()->getId());
}

void DiscoveryView::tilePlayClicked (ElementTile* tile)
{
    auto type = tile->getElementType();

    if (type == ElementTile::ElementType::Sound)
        previewSoundTile (tile);
    else if (type == ElementTile::ElementType::AudioVisual)
        audioEngine.triggerElement (tile->getElement()->getId());
}

void DiscoveryView::previewSoundTile (ElementTile* tile)
{
    auto* snd = static_cast<SoundElement*> (tile->getElement());
    auto path = snd->getSamplePath();
    juce::File file (path);
    if (! file.existsAsFile())
    {
        auto appBundle = juce::File::getSpecialLocation (juce::File::currentApplicationFile);
        auto samplesDir = appBundle.getChildFile ("Contents/Resources/Samples");
        if (! samplesDir.isDirectory())
        {
            auto exeDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
            samplesDir = exeDir.getChildFile ("Samples");
        }
        file = samplesDir.getChildFile (path);
    }
    if (file.existsAsFile())
        audioEngine.previewFile (file);
}

void DiscoveryView::onNewDiscovery (const ElementID& id)
{
    auto* elem = elementLibrary.getElementById (id);
    if (auto* av = dynamic_cast<AudioVisualElement*> (elem))
    {
        discoveryAnimation.show (av->getName(), av->getColour());
        discoveryPanel.rebuild();
    }
}

bool DiscoveryView::tryCombine (const ElementID& a, const ElementID& b)
{
    // Try recipe book first
    const auto* recipe = recipeBook.findRecipe (a, b);
    if (recipe != nullptr)
    {
        if (discoveryLog.isDiscovered (recipe->resultId))
            return false;

        auto* existing = elementLibrary.getElementById (recipe->resultId);
        if (existing != nullptr)
            existing->setDiscovered (true);

        discoveryLog.markDiscovered (recipe->resultId);
        audioEngine.rebuildSounds();
        return true;
    }

    // Dynamic combination: any Visual + any Sound = new AV element
    auto* elemA = elementLibrary.getElementById (a);
    auto* elemB = elementLibrary.getElementById (b);
    if (elemA == nullptr || elemB == nullptr)
        return false;

    VisualElement* vis = dynamic_cast<VisualElement*> (elemA);
    SoundElement* snd = dynamic_cast<SoundElement*> (elemB);
    if (vis == nullptr || snd == nullptr)
    {
        vis = dynamic_cast<VisualElement*> (elemB);
        snd = dynamic_cast<SoundElement*> (elemA);
    }
    if (vis == nullptr || snd == nullptr)
        return false;

    juce::String avIdStr = "av_dynamic_" + juce::String ((a.id + b.id).hashCode64());
    ElementID avId { avIdStr };

    if (discoveryLog.isDiscovered (avId))
        return false;

    if (elementLibrary.getElementById (avId) == nullptr)
    {
        auto avName = vis->getName() + " " + snd->getName();
        elementLibrary.addElement (std::make_unique<AudioVisualElement> (
            avId, avName, vis->getId(), snd->getId(),
            snd->getSamplePath(), vis->getVisualKind(),
            vis->getBaseColour(), vis->getParams()));
    }

    auto* avElem = elementLibrary.getElementById (avId);
    if (avElem != nullptr)
        avElem->setDiscovered (true);

    discoveryLog.markDiscovered (avId);
    audioEngine.rebuildSounds();
    discoveryPanel.rebuild();
    return true;
}

void DiscoveryView::updateCombineButton()
{
    bool canCombine = (selectedSound != nullptr && selectedVisual != nullptr);
    combineButton.setEnabled (canCombine);

    if (canCombine)
    {
        combineButton.setButtonText ("COMBINE: " + selectedSound->getElement()->getName()
                                      + " + " + selectedVisual->getElement()->getName());
        combineButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a8a2a));
    }
    else if (selectedSound != nullptr)
    {
        combineButton.setButtonText ("Select a visual to combine with " + selectedSound->getElement()->getName());
        combineButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a1a1a));
    }
    else if (selectedVisual != nullptr)
    {
        combineButton.setButtonText ("Select a sound to combine with " + selectedVisual->getElement()->getName());
        combineButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a1a1a));
    }
    else
    {
        combineButton.setButtonText ("COMBINE - Select a sound and a visual");
        combineButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a1a1a));
    }
}

void DiscoveryView::rebuild()
{
    selectedSound = nullptr;
    selectedVisual = nullptr;
    soundPanel.rebuild();
    visualPanel.rebuild();
    discoveryPanel.rebuild();
    updateCombineButton();
}

#include "DiscoveryView.h"

DiscoveryView::DiscoveryView (ElementLibrary& lib, RecipeBook& rb,
                               DiscoveryLog& dl, AudioEngine& ae)
    : elementLibrary (lib), recipeBook (rb), discoveryLog (dl), audioEngine (ae),
      soundPanel (lib, ae), visualPanel (lib),
      discoveryPanel (lib), visualCanvas (lib),
      elementEditor (ae, ae.getSampleManager())
{
    addAndMakeVisible (soundPanel);
    addAndMakeVisible (visualPanel);
    addAndMakeVisible (discoveryPanel);
    addAndMakeVisible (combineBar);
    addAndMakeVisible (visualCanvas);
    addChildComponent (discoveryAnimation);
    addChildComponent (elementEditor);

    // Set listeners BEFORE rebuild so tiles get them
    soundPanel.setTileListener (this);
    visualPanel.setTileListener (this);
    discoveryPanel.setTileListener (this);
    discoveryLog.addListener (this);
    combineBar.addListener (this);
    elementEditor.addListener (this);

    // Now rebuild panels
    soundPanel.rebuild();
    visualPanel.rebuild();
    discoveryPanel.rebuild();

    // Wire the visual canvas to show active AV elements
    visualCanvas.setActiveElementsProvider ([&ae] () { return ae.getActiveElementIds(); });

    updateCombineBar();
}

DiscoveryView::~DiscoveryView()
{
    discoveryLog.removeListener (this);
}

void DiscoveryView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (TouchUI::kBgDeep));

    // Section labels
    auto area = getLocalBounds();
    int labelH = 28;
    int halfW = area.getWidth() / 2;

    g.setFont (juce::FontOptions (TouchUI::kFontSmall));

    // "Sounds" label (left)
    g.setColour (juce::Colours::white.withAlpha (0.6f));
    g.drawText ("Sounds", 10, 4, halfW - 20, labelH,
                juce::Justification::centredLeft);

    // "Visuals" label (right)
    g.drawText ("Visuals", halfW + 10, 4, halfW - 20, labelH,
                juce::Justification::centredLeft);
}

void DiscoveryView::resized()
{
    auto area = getLocalBounds();
    int gap = TouchUI::kStandardGap;
    int labelH = 28;

    // Section labels at very top
    area.removeFromTop (labelH);

    // Top section: SoundPanel + VisualPanel (~55% of remaining)
    int topH = area.getHeight() * 55 / 100;
    auto topArea = area.removeFromTop (topH);
    int halfW = topArea.getWidth() / 2;
    soundPanel.setBounds (topArea.removeFromLeft (halfW - gap / 2));
    topArea.removeFromLeft (gap);
    visualPanel.setBounds (topArea);

    // CombineBar in center (80px, prominent divider)
    auto combineArea = area.removeFromTop (80);
    combineBar.setBounds (combineArea);

    // Bottom section: DiscoveryPanel + VisualCanvas (remaining)
    area.removeFromTop (gap);
    int canvasW = juce::jmax (120, area.getWidth() * 35 / 100);
    visualCanvas.setBounds (area.removeFromRight (canvasW));
    discoveryPanel.setBounds (area);

    discoveryAnimation.setBounds (getLocalBounds());

    if (elementEditor.isOpen())
        elementEditor.setBounds (getLocalBounds());
}

void DiscoveryView::tileClicked (ElementTile* tile)
{
    auto type = tile->getElementType();

    if (type == ElementTile::ElementType::Sound)
    {
        if (selectedSound == tile)
        {
            tile->setSelected (false);
            selectedSound = nullptr;
        }
        else
        {
            if (selectedSound != nullptr)
                selectedSound->setSelected (false);
            selectedSound = tile;
            tile->setSelected (true);
            previewSoundTile (tile);
        }
    }
    else if (type == ElementTile::ElementType::Visual)
    {
        if (selectedVisual == tile)
        {
            tile->setSelected (false);
            selectedVisual = nullptr;
        }
        else
        {
            if (selectedVisual != nullptr)
                selectedVisual->setSelected (false);
            selectedVisual = tile;
            tile->setSelected (true);
        }
    }
    updateCombineBar();
}

void DiscoveryView::tileDoubleClicked (ElementTile* tile)
{
    if (tile == nullptr || tile->getElement() == nullptr)
        return;

    auto type = tile->getElementType();

    if (type == ElementTile::ElementType::Sound)
    {
        elementEditor.setBounds (getLocalBounds());
        elementEditor.openForSound (static_cast<SoundElement*> (tile->getElement()));
    }
    else if (type == ElementTile::ElementType::Visual)
    {
        auto* vis = static_cast<VisualElement*> (tile->getElement());
        if (vis->getVisualKind() == VisualElement::VisualKind::Video)
        {
            elementEditor.setBounds (getLocalBounds());
            elementEditor.openForVisual (vis);
        }
    }
}

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

void DiscoveryView::combineRequested()
{
    if (selectedSound != nullptr && selectedVisual != nullptr)
    {
        tryCombine (selectedSound->getElement()->getId(),
                    selectedVisual->getElement()->getId());

        selectedSound->setSelected (false);
        selectedVisual->setSelected (false);
        selectedSound = nullptr;
        selectedVisual = nullptr;
        updateCombineBar();
    }
}

void DiscoveryView::slotCleared (bool isSound)
{
    if (isSound && selectedSound != nullptr)
    {
        selectedSound->setSelected (false);
        selectedSound = nullptr;
    }
    else if (! isSound && selectedVisual != nullptr)
    {
        selectedVisual->setSelected (false);
        selectedVisual = nullptr;
    }
    updateCombineBar();
}

void DiscoveryView::editorClosed (bool trimApplied)
{
    if (trimApplied)
    {
        soundPanel.rebuild();
        visualPanel.rebuild();
        discoveryPanel.rebuild();
    }
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

void DiscoveryView::updateCombineBar()
{
    if (selectedSound != nullptr)
        combineBar.setSoundSlot (static_cast<SoundElement*> (selectedSound->getElement()));
    else
        combineBar.clearSoundSlot();

    if (selectedVisual != nullptr)
        combineBar.setVisualSlot (static_cast<VisualElement*> (selectedVisual->getElement()));
    else
        combineBar.clearVisualSlot();
}

void DiscoveryView::rebuild()
{
    selectedSound = nullptr;
    selectedVisual = nullptr;
    soundPanel.rebuild();
    visualPanel.rebuild();
    discoveryPanel.rebuild();
    updateCombineBar();
}

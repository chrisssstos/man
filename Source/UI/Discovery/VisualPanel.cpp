#include "VisualPanel.h"
#include "UI/Browser/BrowserDragSource.h"

VisualPanel::VisualPanel (ElementLibrary& lib)
    : library (lib)
{
    viewport.setViewedComponent (&tileContainer, false);
    viewport.setScrollBarsShown (true, false);
    addAndMakeVisible (viewport);

    importButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff4488ff));
    importButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    importButton.setTooltip ("Import image files...");
    importButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser> (
            "Import Images",
            juce::File::getSpecialLocation (juce::File::userPicturesDirectory),
            "*.png;*.jpg;*.jpeg;*.gif;*.bmp");

        fileChooser->launchAsync (juce::FileBrowserComponent::openMode
                                    | juce::FileBrowserComponent::canSelectFiles
                                    | juce::FileBrowserComponent::canSelectMultipleItems,
            [this] (const juce::FileChooser& fc)
            {
                for (const auto& file : fc.getResults())
                {
                    if (file.existsAsFile())
                    {
                        juce::StringPairArray imgParams;
                        imgParams.set ("imagePath", file.getFullPathName());
                        library.addUserVisual (file.getFileNameWithoutExtension(),
                                               VisualElement::VisualKind::Image,
                                               juce::Colours::white, imgParams);
                    }
                }
                rebuild();
            });
    };
    addAndMakeVisible (importButton);
}

void VisualPanel::rebuild()
{
    tiles.clear();
    tileContainer.removeAllChildren();

    for (auto* vis : library.getAllVisuals())
    {
        auto* tile = tiles.add (new ElementTile (vis));
        tileContainer.addAndMakeVisible (tile);
        if (tileListener)
            tile->addListener (tileListener);
    }

    resized();
}

void VisualPanel::clearSelection()
{
    if (selectedTile != nullptr)
    {
        selectedTile->setSelected (false);
        selectedTile = nullptr;
    }
}

void VisualPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (hovering ? 0xff1a1a3a : 0xff0e0e24));

    // Title bar — smoother style
    auto titleArea = getLocalBounds().removeFromTop (kTitleBarH);
    g.setColour (juce::Colour (0xff1a1a2a));
    g.fillRect (titleArea);

    // Accent line at bottom of title
    g.setColour (juce::Colour (0xff4488ff));
    g.fillRect (titleArea.getX(), titleArea.getBottom() - 2, titleArea.getWidth(), 2);

    g.setColour (juce::Colours::white.withAlpha (0.9f));
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("VISUALS", titleArea.withTrimmedRight (kTitleBarH + 4), juce::Justification::centred);

    // Drop hint — smooth glow border
    if (hovering)
    {
        g.setColour (juce::Colour (0xff4488ff).withAlpha (0.4f));
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 4.0f, 2.0f);
    }
}

void VisualPanel::resized()
{
    auto area = getLocalBounds();
    auto titleBar = area.removeFromTop (kTitleBarH);
    importButton.setBounds (titleBar.removeFromRight (kTitleBarH).reduced (3));

    area.removeFromTop (2);
    viewport.setBounds (area);

    int cols = juce::jmax (1, (area.getWidth() - kGap) / (kTileSize + kGap));
    int rows = ((int) tiles.size() + cols - 1) / cols;
    int containerHeight = rows * (kTileSize + kGap) + kGap;
    tileContainer.setSize (area.getWidth(), containerHeight);

    for (int i = 0; i < tiles.size(); ++i)
    {
        int col = i % cols;
        int row = i / cols;
        tiles[i]->setBounds (kGap + col * (kTileSize + kGap),
                             kGap + row * (kTileSize + kGap),
                             kTileSize, kTileSize);
    }
}

bool VisualPanel::isInterestedInDragSource (const SourceDetails& details)
{
    auto payload = details.description.toString();
    return BrowserDrag::isVisualPresetDrag (payload);
}

void VisualPanel::itemDragEnter (const SourceDetails&) { hovering = true; repaint(); }
void VisualPanel::itemDragExit (const SourceDetails&) { hovering = false; repaint(); }

void VisualPanel::itemDropped (const SourceDetails&)
{
    hovering = false;
    repaint();
}

bool VisualPanel::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& f : files)
    {
        auto ext = juce::File (f).getFileExtension().toLowerCase();
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" || ext == ".bmp")
            return true;
    }
    return false;
}

void VisualPanel::filesDropped (const juce::StringArray& files, int, int)
{
    for (const auto& f : files)
    {
        juce::File file (f);
        auto ext = file.getFileExtension().toLowerCase();
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" || ext == ".bmp")
        {
            juce::StringPairArray imgParams;
            imgParams.set ("imagePath", file.getFullPathName());
            library.addUserVisual (file.getFileNameWithoutExtension(),
                                   VisualElement::VisualKind::Image,
                                   juce::Colours::white, imgParams);
        }
    }
    rebuild();
}

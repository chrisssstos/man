#include "VisualPanel.h"
#include "UI/Browser/BrowserDragSource.h"

VisualPanel::VisualPanel (ElementLibrary& lib)
    : library (lib), youtubeBrowser (lib)
{
    viewport.setViewedComponent (&tileContainer, false);
    viewport.setScrollBarsShown (true, false);
    addAndMakeVisible (viewport);

    // YouTube toggle button
    youtubeButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffcc2222));
    youtubeButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    youtubeButton.setTooltip ("Toggle YouTube search");
    youtubeButton.onClick = [this]
    {
        youtubeMode = ! youtubeMode;
        youtubeBrowser.setVisible (youtubeMode);
        viewport.setVisible (! youtubeMode);
        importButton.setVisible (! youtubeMode);
        youtubeButton.setButtonText (youtubeMode ? "Tiles" : "YT");
        repaint();
    };
    addAndMakeVisible (youtubeButton);

    // YouTube browser (hidden by default)
    addChildComponent (youtubeBrowser);

    // When a YouTube video is downloaded and added to library, rebuild tiles
    youtubeBrowser.onVisualAdded = [this]
    {
        youtubeMode = false;
        youtubeBrowser.setVisible (false);
        viewport.setVisible (true);
        importButton.setVisible (true);
        youtubeButton.setButtonText ("YT");
        rebuild();
    };

    importButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff4488ff));
    importButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    importButton.setTooltip ("Import image or video files...");
    importButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser> (
            "Import Visuals",
            juce::File::getSpecialLocation (juce::File::userDocumentsDirectory));

        fileChooser->launchAsync (juce::FileBrowserComponent::openMode
                                    | juce::FileBrowserComponent::canSelectFiles
                                    | juce::FileBrowserComponent::canSelectMultipleItems,
            [this] (const juce::FileChooser& fc)
            {
                for (const auto& file : fc.getResults())
                {
                    if (! file.existsAsFile())
                        continue;

                    auto ext = file.getFileExtension().toLowerCase();

                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" || ext == ".bmp")
                    {
                        juce::StringPairArray imgParams;
                        imgParams.set ("imagePath", file.getFullPathName());
                        library.addUserVisual (file.getFileNameWithoutExtension(),
                                               VisualElement::VisualKind::Image,
                                               juce::Colours::white, imgParams);
                    }
                    else if (ext == ".mp4" || ext == ".mov" || ext == ".m4v" || ext == ".dxv")
                    {
                        juce::StringPairArray vidParams;
                        vidParams.set ("videoPath", file.getFullPathName());
                        library.addUserVisual (file.getFileNameWithoutExtension(),
                                               VisualElement::VisualKind::Video,
                                               juce::Colours::white, vidParams);
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
    g.fillAll (juce::Colour (hovering ? TouchUI::kBgCard : TouchUI::kBgPanel));

    if (hovering)
    {
        g.setColour (juce::Colour (0xff4488ff).withAlpha (0.4f));
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 4.0f, 2.0f);
    }
}

void VisualPanel::resized()
{
    auto area = getLocalBounds();

    // YouTube toggle button in top-right corner
    int btnSize = 40;
    youtubeButton.setBounds (area.getRight() - btnSize - 4, 4, btnSize, btnSize);
    youtubeButton.toFront (false);

    // YouTube browser fills the whole area (below toggle)
    youtubeBrowser.setBounds (area.withTrimmedTop (btnSize + 8));

    // Floating import button in bottom-right corner
    importButton.setBounds (area.getRight() - TouchUI::kMinTouchTarget - 8,
                            area.getBottom() - TouchUI::kMinTouchTarget - 8,
                            TouchUI::kMinTouchTarget, TouchUI::kMinTouchTarget);
    importButton.toFront (false);

    // Tile grid viewport fills available area
    viewport.setBounds (area);

    int tileSize = TouchUI::kTileSize;
    int gap = TouchUI::kTileGap;
    int cols = juce::jmax (1, (area.getWidth() - gap) / (tileSize + gap));
    int rows = ((int) tiles.size() + cols - 1) / cols;
    int containerHeight = rows * (tileSize + gap) + gap;
    tileContainer.setSize (area.getWidth(), containerHeight);

    for (int i = 0; i < tiles.size(); ++i)
    {
        int col = i % cols;
        int row = i / cols;
        tiles[i]->setBounds (gap + col * (tileSize + gap),
                             gap + row * (tileSize + gap),
                             tileSize, tileSize);
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
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" || ext == ".bmp"
            || ext == ".mp4" || ext == ".mov" || ext == ".m4v" || ext == ".dxv")
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
        else if (ext == ".mp4" || ext == ".mov" || ext == ".m4v" || ext == ".dxv")
        {
            juce::StringPairArray vidParams;
            vidParams.set ("videoPath", file.getFullPathName());
            library.addUserVisual (file.getFileNameWithoutExtension(),
                                   VisualElement::VisualKind::Video,
                                   juce::Colours::white, vidParams);
        }
    }
    rebuild();
}

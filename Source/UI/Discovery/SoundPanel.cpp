#include "SoundPanel.h"
#include "UI/Browser/BrowserDragSource.h"

SoundPanel::SoundPanel (ElementLibrary& lib, AudioEngine& eng)
    : library (lib), audioEngine (eng)
{
    viewport.setViewedComponent (&tileContainer, false);
    viewport.setScrollBarsShown (true, false);
    addAndMakeVisible (viewport);

    importButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff44dd88));
    importButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    importButton.setTooltip ("Import sound files...");
    importButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser> (
            "Import Sounds",
            juce::File::getSpecialLocation (juce::File::userMusicDirectory),
            "*.wav;*.mp3;*.aif;*.aiff;*.flac;*.ogg");

        fileChooser->launchAsync (juce::FileBrowserComponent::openMode
                                    | juce::FileBrowserComponent::canSelectFiles
                                    | juce::FileBrowserComponent::canSelectMultipleItems,
            [this] (const juce::FileChooser& fc)
            {
                for (const auto& file : fc.getResults())
                {
                    if (file.existsAsFile())
                        library.addUserSound (file);
                }
                rebuild();
            });
    };
    addAndMakeVisible (importButton);
}

void SoundPanel::rebuild()
{
    tiles.clear();
    tileContainer.removeAllChildren();

    for (auto* snd : library.getAllSounds())
    {
        auto* tile = tiles.add (new ElementTile (snd, &audioEngine.getSampleManager()));
        tileContainer.addAndMakeVisible (tile);
        if (tileListener)
            tile->addListener (tileListener);
    }

    resized();
}

void SoundPanel::clearSelection()
{
    if (selectedTile != nullptr)
    {
        selectedTile->setSelected (false);
        selectedTile = nullptr;
    }
}

void SoundPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (hovering ? TouchUI::kBgCard : TouchUI::kBgPanel));

    if (hovering)
    {
        g.setColour (juce::Colour (0xffcc4444).withAlpha (0.4f));
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 4.0f, 2.0f);
    }
}

void SoundPanel::resized()
{
    auto area = getLocalBounds();

    // Floating import button in bottom-right corner
    importButton.setBounds (area.getRight() - TouchUI::kMinTouchTarget - 8,
                            area.getBottom() - TouchUI::kMinTouchTarget - 8,
                            TouchUI::kMinTouchTarget, TouchUI::kMinTouchTarget);
    importButton.toFront (false);

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

bool SoundPanel::isInterestedInDragSource (const SourceDetails& details)
{
    auto payload = details.description.toString();
    return BrowserDrag::isSoundFileDrag (payload);
}

void SoundPanel::itemDragEnter (const SourceDetails&) { hovering = true; repaint(); }
void SoundPanel::itemDragExit (const SourceDetails&) { hovering = false; repaint(); }

void SoundPanel::itemDropped (const SourceDetails& details)
{
    hovering = false;
    auto payload = details.description.toString();
    if (BrowserDrag::isSoundFileDrag (payload))
    {
        auto file = BrowserDrag::decodeSoundFile (payload);
        library.addUserSound (file);
        rebuild();
    }
    repaint();
}

bool SoundPanel::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& f : files)
    {
        auto ext = juce::File (f).getFileExtension().toLowerCase();
        if (ext == ".wav" || ext == ".mp3" || ext == ".aif" || ext == ".aiff" || ext == ".flac" || ext == ".ogg")
            return true;
    }
    return false;
}

void SoundPanel::filesDropped (const juce::StringArray& files, int, int)
{
    for (const auto& f : files)
    {
        juce::File file (f);
        auto ext = file.getFileExtension().toLowerCase();
        if (ext == ".wav" || ext == ".mp3" || ext == ".aif" || ext == ".aiff" || ext == ".flac" || ext == ".ogg")
        {
            library.addUserSound (file);
        }
    }
    rebuild();
}

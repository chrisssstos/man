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
    g.fillAll (juce::Colour (hovering ? 0xff1a1a3a : 0xff0e0e24));

    // Title bar — smoother rounded feel
    auto titleArea = getLocalBounds().removeFromTop (kTitleBarH);
    g.setColour (juce::Colour (0xff2a1a1a));
    g.fillRect (titleArea);

    // Accent line at bottom of title
    g.setColour (juce::Colour (0xffcc4444));
    g.fillRect (titleArea.getX(), titleArea.getBottom() - 2, titleArea.getWidth(), 2);

    g.setColour (juce::Colours::white.withAlpha (0.9f));
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("SOUNDS", titleArea.withTrimmedRight (kTitleBarH + 4), juce::Justification::centred);

    // Drop hint — smooth glow border
    if (hovering)
    {
        g.setColour (juce::Colour (0xffcc4444).withAlpha (0.4f));
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 4.0f, 2.0f);
    }
}

void SoundPanel::resized()
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

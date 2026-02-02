#include "SoundBrowserPanel.h"
#include "BrowserDragSource.h"

class SoundBrowserPanel::BookmarkListModel : public juce::ListBoxModel
{
public:
    BookmarkListModel (SoundBrowserPanel& owner) : owner (owner) {}

    int getNumRows() override { return owner.bookmarkedPaths.size(); }

    void paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected) override
    {
        if (row < 0 || row >= owner.bookmarkedPaths.size())
            return;

        if (selected)
            g.fillAll (juce::Colour (0xff2a2a5a));

        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.setFont (12.0f);
        auto path = owner.bookmarkedPaths[row];
        auto name = juce::File (path).getFileName();
        g.drawText (name, 4, 0, width - 8, height, juce::Justification::centredLeft);
    }

    void listBoxItemClicked (int row, const juce::MouseEvent&) override
    {
        if (row >= 0 && row < owner.bookmarkedPaths.size())
        {
            juce::File dir (owner.bookmarkedPaths[row]);
            if (dir.isDirectory())
                owner.fileBrowser->setRoot (dir);
        }
    }

private:
    SoundBrowserPanel& owner;
};

SoundBrowserPanel::SoundBrowserPanel (ElementLibrary& lib, AudioEngine& eng)
    : elementLibrary (lib), audioEngine (eng),
      fileFilter ("*.wav;*.mp3;*.aif;*.aiff;*.flac;*.ogg", "*", "Audio Files")
{
    auto initialDir = juce::File::getSpecialLocation (juce::File::userMusicDirectory);
    fileBrowser = std::make_unique<juce::FileBrowserComponent> (
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        initialDir, &fileFilter, nullptr);
    fileBrowser->addListener (this);
    addAndMakeVisible (*fileBrowser);

    bookmarkModel = std::make_unique<BookmarkListModel> (*this);
    bookmarkList.setModel (bookmarkModel.get());
    bookmarkList.setRowHeight (22);
    bookmarkList.setColour (juce::ListBox::backgroundColourId, juce::Colour (0xff12122a));
    addAndMakeVisible (bookmarkList);

    addBookmarkButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2a5a));
    removeBookmarkButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2a5a));
    addAndMakeVisible (addBookmarkButton);
    addAndMakeVisible (removeBookmarkButton);

    addBookmarkButton.onClick = [this]
    {
        auto currentDir = fileBrowser->getRoot();
        if (currentDir.isDirectory() && ! bookmarkedPaths.contains (currentDir.getFullPathName()))
        {
            bookmarkedPaths.add (currentDir.getFullPathName());
            bookmarkList.updateContent();
            saveState();
        }
    };

    removeBookmarkButton.onClick = [this]
    {
        int selected = bookmarkList.getSelectedRow();
        if (selected >= 0 && selected < bookmarkedPaths.size())
        {
            bookmarkedPaths.remove (selected);
            bookmarkList.updateContent();
            saveState();
        }
    };

    loadState();
}

SoundBrowserPanel::~SoundBrowserPanel()
{
    saveState();
    fileBrowser->removeListener (this);
}

void SoundBrowserPanel::resized()
{
    auto area = getLocalBounds();

    // Bookmarks section at top
    auto bookmarkArea = area.removeFromTop (std::min (100, area.getHeight() / 4));
    auto buttonBar = bookmarkArea.removeFromBottom (22);
    addBookmarkButton.setBounds (buttonBar.removeFromLeft (buttonBar.getWidth() / 2));
    removeBookmarkButton.setBounds (buttonBar);
    bookmarkList.setBounds (bookmarkArea);

    // File browser takes rest
    fileBrowser->setBounds (area);
}

void SoundBrowserPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0e0e24));
}

void SoundBrowserPanel::selectionChanged()
{
    // Preview on selection
    auto selectedFile = fileBrowser->getSelectedFile (0);
    if (selectedFile.existsAsFile())
        audioEngine.previewFile (selectedFile);
}

void SoundBrowserPanel::fileClicked (const juce::File& file, const juce::MouseEvent&)
{
    if (file.existsAsFile())
        audioEngine.previewFile (file);
}

void SoundBrowserPanel::fileDoubleClicked (const juce::File& file)
{
    if (file.existsAsFile())
    {
        elementLibrary.addUserSound (file);
        audioEngine.previewFile (file);
    }
}

void SoundBrowserPanel::browserRootChanged (const juce::File&)
{
    saveState();
}

void SoundBrowserPanel::mouseDrag (const juce::MouseEvent& e)
{
    auto selectedFile = fileBrowser->getSelectedFile (0);
    if (selectedFile.existsAsFile() && e.getDistanceFromDragStart() > 5)
    {
        auto* container = juce::DragAndDropContainer::findParentDragContainerFor (this);
        if (container != nullptr && ! container->isDragAndDropActive())
        {
            container->startDragging (BrowserDrag::encodeSoundFile (selectedFile), this);
        }
    }
}

juce::File SoundBrowserPanel::getSettingsFile() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("man").getChildFile ("browser_state.json");
}

void SoundBrowserPanel::saveState()
{
    auto settingsFile = getSettingsFile();
    settingsFile.getParentDirectory().createDirectory();

    juce::var root (new juce::DynamicObject());
    auto* obj = root.getDynamicObject();
    obj->setProperty ("lastBrowsedPath", fileBrowser->getRoot().getFullPathName());

    juce::var bookmarks;
    for (const auto& path : bookmarkedPaths)
        bookmarks.append (path);
    obj->setProperty ("bookmarks", bookmarks);

    settingsFile.replaceWithText (juce::JSON::toString (root));
}

void SoundBrowserPanel::loadState()
{
    auto settingsFile = getSettingsFile();
    if (! settingsFile.existsAsFile())
        return;

    auto parsed = juce::JSON::parse (settingsFile);
    if (auto* obj = parsed.getDynamicObject())
    {
        auto lastPath = obj->getProperty ("lastBrowsedPath").toString();
        if (juce::File (lastPath).isDirectory())
            fileBrowser->setRoot (juce::File (lastPath));

        bookmarkedPaths.clear();
        if (auto* arr = obj->getProperty ("bookmarks").getArray())
        {
            for (const auto& item : *arr)
                bookmarkedPaths.add (item.toString());
        }
        bookmarkList.updateContent();
    }
}

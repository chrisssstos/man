#pragma once
#include <JuceHeader.h>
#include "Audio/AudioEngine.h"
#include "Model/ElementLibrary.h"

class SoundBrowserPanel : public juce::Component,
                          public juce::FileBrowserListener
{
public:
    SoundBrowserPanel (ElementLibrary& library, AudioEngine& audioEngine);
    ~SoundBrowserPanel() override;

    void resized() override;
    void paint (juce::Graphics& g) override;

    // FileBrowserListener
    void selectionChanged() override;
    void fileClicked (const juce::File& file, const juce::MouseEvent& e) override;
    void fileDoubleClicked (const juce::File& file) override;
    void browserRootChanged (const juce::File& newRoot) override;

    void saveState();
    void loadState();

private:
    ElementLibrary& elementLibrary;
    AudioEngine& audioEngine;

    juce::WildcardFileFilter fileFilter;
    std::unique_ptr<juce::FileBrowserComponent> fileBrowser;

    // Bookmarks
    juce::StringArray bookmarkedPaths;
    juce::ListBox bookmarkList;
    std::unique_ptr<juce::ListBoxModel> bookmarkModel;
    juce::TextButton addBookmarkButton { "+" };
    juce::TextButton removeBookmarkButton { "-" };

    // Drag support
    void mouseDrag (const juce::MouseEvent& e) override;

    juce::File getSettingsFile() const;

    class BookmarkListModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundBrowserPanel)
};

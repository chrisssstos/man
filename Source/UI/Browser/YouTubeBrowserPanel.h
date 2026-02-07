#pragma once
#include <JuceHeader.h>
#include "Network/YouTubeDownloader.h"
#include "Model/ElementLibrary.h"

class YouTubeBrowserPanel : public juce::Component,
                             public YouTubeDownloader::Listener
{
public:
    YouTubeBrowserPanel (ElementLibrary& library);
    ~YouTubeBrowserPanel() override;

    void resized() override;
    void paint (juce::Graphics& g) override;

    // YouTubeDownloader::Listener
    void searchResultsReady (const std::vector<YouTubeDownloader::SearchResult>& results) override;
    void thumbnailLoaded (const juce::String& videoId, const juce::Image& image) override;
    void downloadProgressed (const juce::String& videoId, float progress) override;
    void downloadFinished (const YouTubeDownloader::DownloadResult& result) override;

private:
    ElementLibrary& elementLibrary;
    YouTubeDownloader downloader;

    // Search UI
    juce::TextEditor searchBox;
    juce::TextButton searchButton { "Search" };

    // Results
    juce::ListBox resultsList;
    std::unique_ptr<juce::ListBoxModel> resultsModel;
    std::vector<YouTubeDownloader::SearchResult> results;
    juce::HashMap<juce::String, juce::Image> thumbnails;

    // Action
    juce::TextButton useButton { "Use as Visual" };
    juce::Label statusLabel;

    bool ytDlpAvailable = false;
    bool searching = false;
    bool downloading = false;

public:
    std::function<void()> onVisualAdded;

private:

    void performSearch();
    juce::String formatDuration (int seconds);
    juce::String formatViewCount (int64 count);

    class ResultsListModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (YouTubeBrowserPanel)
};

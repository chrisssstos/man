#pragma once
#include <JuceHeader.h>

class YouTubeDownloader : private juce::Thread
{
public:
    struct SearchResult
    {
        juce::String id;
        juce::String title;
        juce::String url;
        juce::String channel;
        int durationSeconds = 0;
        int64 viewCount = 0;
        juce::String thumbnailUrl;
    };

    struct DownloadResult
    {
        bool success = false;
        juce::String videoId;
        juce::File localFile;
        juce::String errorMessage;
    };

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void searchResultsReady (const std::vector<SearchResult>& results) = 0;
        virtual void thumbnailLoaded (const juce::String& videoId, const juce::Image& image) = 0;
        virtual void downloadProgressed (const juce::String& videoId, float progress) = 0;
        virtual void downloadFinished (const DownloadResult& result) = 0;
    };

    YouTubeDownloader();
    ~YouTubeDownloader() override;

    void searchAsync (const juce::String& query);
    void startDownload (const juce::String& videoUrl, const juce::String& videoId);
    void cancelAll();

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

    static bool isYtDlpAvailable();
    static juce::File getCacheDirectory();

private:
    enum class Task { None, Search, Download, Thumbnails };

    void run() override;
    void runSearch();
    void runDownload();
    void runThumbnailDownloads();

    std::vector<SearchResult> parseSearchResults (const juce::String& output);
    juce::String formatDuration (int seconds);

    juce::ListenerList<Listener> listeners;

    // Protected by lock
    juce::CriticalSection lock;
    Task currentTask = Task::None;
    juce::String searchQuery;
    juce::String downloadUrl;
    juce::String downloadVideoId;

    // Thumbnail state (accessed from thumbnail thread)
    std::vector<SearchResult> pendingThumbnailResults;
    juce::HashMap<juce::String, juce::Image> thumbnailCache;

    // Separate thread for thumbnails so they don't block search/download
    class ThumbnailThread : public juce::Thread
    {
    public:
        ThumbnailThread (YouTubeDownloader& o) : juce::Thread ("YT-Thumbnails"), owner (o) {}
        void run() override { owner.runThumbnailDownloads(); }
    private:
        YouTubeDownloader& owner;
    };
    ThumbnailThread thumbnailThread { *this };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (YouTubeDownloader)
};

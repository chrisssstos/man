#include "YouTubeDownloader.h"

static juce::String findYtDlpPath()
{
    // GUI apps on macOS don't inherit shell PATH, so check common locations
    static const char* paths[] = {
        "/opt/homebrew/bin/yt-dlp",
        "/usr/local/bin/yt-dlp",
        "/usr/bin/yt-dlp",
        nullptr
    };

    for (int i = 0; paths[i] != nullptr; ++i)
        if (juce::File (paths[i]).existsAsFile())
            return paths[i];

    return "yt-dlp"; // fallback to bare name
}

YouTubeDownloader::YouTubeDownloader()
    : juce::Thread ("YT-Downloader")
{
}

YouTubeDownloader::~YouTubeDownloader()
{
    cancelAll();
}

void YouTubeDownloader::cancelAll()
{
    signalThreadShouldExit();
    thumbnailThread.signalThreadShouldExit();
    stopThread (5000);
    thumbnailThread.stopThread (5000);
}

bool YouTubeDownloader::isYtDlpAvailable()
{
    juce::ChildProcess proc;
    if (proc.start (juce::StringArray { findYtDlpPath(), "--version" }))
    {
        proc.waitForProcessToFinish (3000);
        auto output = proc.readAllProcessOutput().trim();
        return output.isNotEmpty() && proc.getExitCode() == 0;
    }
    return false;
}

juce::File YouTubeDownloader::getCacheDirectory()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("man")
                   .getChildFile ("YouTubeCache");
    dir.createDirectory();
    return dir;
}

void YouTubeDownloader::searchAsync (const juce::String& query)
{
    // Stop any running operation
    signalThreadShouldExit();
    stopThread (3000);

    {
        const juce::ScopedLock sl (lock);
        currentTask = Task::Search;
        searchQuery = query;
    }

    startThread();
}

void YouTubeDownloader::startDownload (const juce::String& videoUrl, const juce::String& videoId)
{
    signalThreadShouldExit();
    stopThread (3000);

    {
        const juce::ScopedLock sl (lock);
        currentTask = Task::Download;
        downloadUrl = videoUrl;
        downloadVideoId = videoId;
    }

    startThread();
}

void YouTubeDownloader::run()
{
    Task taskToRun;
    {
        const juce::ScopedLock sl (lock);
        taskToRun = currentTask;
    }

    switch (taskToRun)
    {
        case Task::Search:      runSearch(); break;
        case Task::Download:    runDownload(); break;
        case Task::None:
        case Task::Thumbnails:  break;
    }
}

void YouTubeDownloader::runSearch()
{
    juce::String query;
    {
        const juce::ScopedLock sl (lock);
        query = searchQuery;
    }

    juce::ChildProcess proc;
    juce::String ytdlp = findYtDlpPath();
    juce::StringArray args { ytdlp, "-j", "--flat-playlist", "ytsearch15:" + query };

    if (! proc.start (args))
    {
        juce::MessageManager::callAsync ([this]
        {
            listeners.call (&Listener::searchResultsReady, std::vector<SearchResult>());
        });
        return;
    }

    // Read output incrementally
    juce::String fullOutput;
    char buffer[4096];
    while (! threadShouldExit())
    {
        auto bytesRead = proc.readProcessOutput (buffer, sizeof (buffer));
        if (bytesRead <= 0)
        {
            if (! proc.isRunning())
                break;
            Thread::sleep (50);
            continue;
        }
        fullOutput += juce::String::fromUTF8 (buffer, bytesRead);
    }

    if (threadShouldExit())
    {
        proc.kill();
        return;
    }

    auto results = parseSearchResults (fullOutput);

    // Start thumbnail downloads
    {
        const juce::ScopedLock sl (lock);
        pendingThumbnailResults = results;
    }
    thumbnailThread.signalThreadShouldExit();
    thumbnailThread.stopThread (2000);
    thumbnailThread.startThread();

    auto resultsCopy = results;
    juce::MessageManager::callAsync ([this, resultsCopy]
    {
        listeners.call (&Listener::searchResultsReady, resultsCopy);
    });
}

std::vector<YouTubeDownloader::SearchResult> YouTubeDownloader::parseSearchResults (const juce::String& output)
{
    std::vector<SearchResult> results;
    auto lines = juce::StringArray::fromLines (output);

    for (const auto& line : lines)
    {
        if (line.trimStart().isEmpty())
            continue;

        auto parsed = juce::JSON::parse (line);
        if (auto* obj = parsed.getDynamicObject())
        {
            SearchResult r;
            r.id = obj->getProperty ("id").toString();
            r.title = obj->getProperty ("title").toString();
            r.channel = obj->getProperty ("channel").toString();
            r.durationSeconds = (int) obj->getProperty ("duration");
            r.viewCount = (int64) obj->getProperty ("view_count");

            // Build watch URL from id
            if (r.id.isNotEmpty())
                r.url = "https://www.youtube.com/watch?v=" + r.id;
            else
                r.url = obj->getProperty ("url").toString();

            // Get best thumbnail URL
            auto thumbsVar = obj->getProperty ("thumbnails");
            if (auto* thumbsArr = thumbsVar.getArray())
            {
                if (! thumbsArr->isEmpty())
                {
                    // Use last thumbnail (usually highest res)
                    auto lastThumb = thumbsArr->getLast();
                    if (auto* thumbObj = lastThumb.getDynamicObject())
                        r.thumbnailUrl = thumbObj->getProperty ("url").toString();
                }
            }

            // Fallback thumbnail URL
            if (r.thumbnailUrl.isEmpty() && r.id.isNotEmpty())
                r.thumbnailUrl = "https://i.ytimg.com/vi/" + r.id + "/mqdefault.jpg";

            if (r.id.isNotEmpty() && r.title.isNotEmpty())
                results.push_back (std::move (r));
        }
    }

    return results;
}

void YouTubeDownloader::runThumbnailDownloads()
{
    std::vector<SearchResult> results;
    {
        const juce::ScopedLock sl (lock);
        results = pendingThumbnailResults;
    }

    for (const auto& r : results)
    {
        if (thumbnailThread.threadShouldExit())
            return;

        if (r.thumbnailUrl.isEmpty())
            continue;

        // Check cache
        if (thumbnailCache.contains (r.id))
            continue;

        juce::URL url (r.thumbnailUrl);
        auto stream = url.createInputStream (juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)
                                                 .withConnectionTimeoutMs (5000)
                                                 .withResponseHeaders (nullptr)
                                                 .withStatusCode (nullptr)
                                                 .withNumRedirectsToFollow (5));

        if (stream == nullptr)
            continue;

        juce::MemoryBlock data;
        stream->readIntoMemoryBlock (data);

        auto image = juce::ImageFileFormat::loadFrom (data.getData(), data.getSize());
        if (image.isValid())
        {
            thumbnailCache.set (r.id, image);

            auto videoId = r.id;
            auto imgCopy = image;
            juce::MessageManager::callAsync ([this, videoId, imgCopy]
            {
                listeners.call (&Listener::thumbnailLoaded, videoId, imgCopy);
            });
        }
    }
}

void YouTubeDownloader::runDownload()
{
    juce::String url, videoId;
    {
        const juce::ScopedLock sl (lock);
        url = downloadUrl;
        videoId = downloadVideoId;
    }

    auto cacheDir = getCacheDirectory();
    auto finalFile = cacheDir.getChildFile ("yt_" + videoId + ".mp4");

    // If already cached, return immediately
    if (finalFile.existsAsFile() && finalFile.getSize() > 0)
    {
        DownloadResult result;
        result.success = true;
        result.videoId = videoId;
        result.localFile = finalFile;

        juce::MessageManager::callAsync ([this, result]
        {
            listeners.call (&Listener::downloadFinished, result);
        });
        return;
    }

    auto tempFile = cacheDir.getChildFile ("yt_" + videoId + ".tmp.mp4");

    juce::StringArray dlArgs {
        findYtDlpPath(),
        "-f", "bestvideo[height<=720][ext=mp4]+bestaudio[ext=m4a]/best[height<=720][ext=mp4]/best",
        "--merge-output-format", "mp4",
        "--newline",
        "--no-warnings",
        "--no-playlist",
        "-o", tempFile.getFullPathName(),
        url
    };

    juce::ChildProcess proc;
    if (! proc.start (dlArgs))
    {
        DownloadResult result;
        result.videoId = videoId;
        result.errorMessage = "Failed to start yt-dlp";

        juce::MessageManager::callAsync ([this, result]
        {
            listeners.call (&Listener::downloadFinished, result);
        });
        return;
    }

    // Read output and parse progress
    char dlBuffer[1024];
    while (! threadShouldExit())
    {
        auto bytesRead = proc.readProcessOutput (dlBuffer, sizeof (dlBuffer));
        if (bytesRead <= 0)
        {
            if (! proc.isRunning())
                break;
            Thread::sleep (100);
            continue;
        }

        auto text = juce::String::fromUTF8 (dlBuffer, bytesRead);
        auto lines = juce::StringArray::fromLines (text);

        for (const auto& line : lines)
        {
            // Parse progress: [download]  45.2% of ...
            if (line.contains ("%"))
            {
                auto percentStr = line.fromFirstOccurrenceOf ("[download]", false, true)
                                      .upToFirstOccurrenceOf ("%", false, true)
                                      .trim();
                auto percent = percentStr.getFloatValue();
                if (percent > 0.0f && percent <= 100.0f)
                {
                    auto vid = videoId;
                    auto p = percent / 100.0f;
                    juce::MessageManager::callAsync ([this, vid, p]
                    {
                        listeners.call (&Listener::downloadProgressed, vid, p);
                    });
                }
            }
        }
    }

    if (threadShouldExit())
    {
        proc.kill();
        tempFile.deleteFile();
        return;
    }

    proc.waitForProcessToFinish (5000);

    DownloadResult result;
    result.videoId = videoId;

    if (tempFile.existsAsFile() && tempFile.getSize() > 0)
    {
        tempFile.moveFileTo (finalFile);
        result.success = true;
        result.localFile = finalFile;
    }
    else
    {
        result.errorMessage = "Download failed or produced empty file";
    }

    juce::MessageManager::callAsync ([this, result]
    {
        listeners.call (&Listener::downloadFinished, result);
    });
}

juce::String YouTubeDownloader::formatDuration (int seconds)
{
    if (seconds <= 0)
        return "";
    int mins = seconds / 60;
    int secs = seconds % 60;
    return juce::String (mins) + ":" + juce::String (secs).paddedLeft ('0', 2);
}

#include "YouTubeBrowserPanel.h"

class YouTubeBrowserPanel::ResultsListModel : public juce::ListBoxModel
{
public:
    ResultsListModel (YouTubeBrowserPanel& o) : owner (o) {}

    int getNumRows() override { return (int) owner.results.size(); }

    void paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected) override
    {
        if (row < 0 || row >= (int) owner.results.size())
            return;

        if (selected)
            g.fillAll (juce::Colour (0xff2a2a5a));
        else
            g.fillAll (row % 2 == 0 ? juce::Colour (0xff141430) : juce::Colour (0xff181840));

        auto& result = owner.results[(size_t) row];

        // Thumbnail area (120x68, centered vertically in the row)
        const int thumbW = 120;
        const int thumbH = 68;
        int thumbY = (height - thumbH) / 2;
        auto thumbArea = juce::Rectangle<int> (4, thumbY, thumbW, thumbH);

        if (owner.thumbnails.contains (result.id))
        {
            auto& img = owner.thumbnails.getReference (result.id);
            g.drawImage (img, thumbArea.toFloat(),
                         juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        }
        else
        {
            // Placeholder gradient
            juce::ColourGradient grad (juce::Colour (0xff2a2a5a), (float) thumbArea.getX(), (float) thumbArea.getY(),
                                       juce::Colour (0xff1a1a3a), (float) thumbArea.getRight(), (float) thumbArea.getBottom(),
                                       false);
            g.setGradientFill (grad);
            g.fillRect (thumbArea);
        }

        int textX = thumbW + 12;
        int textW = width - textX - 8;

        // Title
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (13.0f).withStyle ("Bold"));
        g.drawText (result.title, textX, 4, textW, 20, juce::Justification::centredLeft, true);

        // Channel + Duration
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.setFont (juce::FontOptions (11.0f));

        juce::String meta = result.channel;
        if (result.durationSeconds > 0)
            meta += "  " + owner.formatDuration (result.durationSeconds);
        if (result.viewCount > 0)
            meta += "  " + owner.formatViewCount (result.viewCount) + " views";

        g.drawText (meta, textX, 26, textW, 16, juce::Justification::centredLeft, true);

        // Third line: video ID (subtle)
        g.setColour (juce::Colours::white.withAlpha (0.25f));
        g.setFont (juce::FontOptions (10.0f));
        g.drawText (result.id, textX, 44, textW, 14, juce::Justification::centredLeft, true);
    }

    void selectedRowsChanged (int) override {}

private:
    YouTubeBrowserPanel& owner;
};

YouTubeBrowserPanel::YouTubeBrowserPanel (ElementLibrary& lib)
    : elementLibrary (lib)
{
    downloader.addListener (this);

    // Search box
    searchBox.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff1a1a3a));
    searchBox.setColour (juce::TextEditor::textColourId, juce::Colours::white);
    searchBox.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff2a2a5a));
    searchBox.setTextToShowWhenEmpty ("Search YouTube...", juce::Colours::white.withAlpha (0.3f));
    searchBox.onReturnKey = [this] { performSearch(); };
    addAndMakeVisible (searchBox);

    searchButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2a2a5a));
    searchButton.onClick = [this] { performSearch(); };
    addAndMakeVisible (searchButton);

    // Results list
    resultsModel = std::make_unique<ResultsListModel> (*this);
    resultsList.setModel (resultsModel.get());
    resultsList.setRowHeight (72);
    resultsList.setColour (juce::ListBox::backgroundColourId, juce::Colour (0xff12122a));
    addAndMakeVisible (resultsList);

    // Use button
    useButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff5a2a2a));
    useButton.onClick = [this]
    {
        auto selected = resultsList.getSelectedRow();
        if (selected < 0 || selected >= (int) results.size())
            return;

        if (downloading)
            return;

        auto& result = results[(size_t) selected];
        downloading = true;
        statusLabel.setText ("Downloading...", juce::dontSendNotification);
        downloader.startDownload (result.url, result.id);
    };
    addAndMakeVisible (useButton);

    // Status label
    statusLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.7f));
    statusLabel.setFont (juce::FontOptions (11.0f));
    statusLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (statusLabel);

    // Check yt-dlp availability
    ytDlpAvailable = YouTubeDownloader::isYtDlpAvailable();
    if (! ytDlpAvailable)
        statusLabel.setText ("yt-dlp not found", juce::dontSendNotification);
}

YouTubeBrowserPanel::~YouTubeBrowserPanel()
{
    downloader.removeListener (this);
    downloader.cancelAll();
}

void YouTubeBrowserPanel::performSearch()
{
    auto query = searchBox.getText().trim();
    if (query.isEmpty())
        return;

    if (! ytDlpAvailable)
    {
        statusLabel.setText ("Install yt-dlp: brew install yt-dlp", juce::dontSendNotification);
        return;
    }

    searching = true;
    results.clear();
    thumbnails.clear();
    resultsList.updateContent();
    statusLabel.setText ("Searching...", juce::dontSendNotification);
    downloader.searchAsync (query);
}

void YouTubeBrowserPanel::searchResultsReady (const std::vector<YouTubeDownloader::SearchResult>& newResults)
{
    searching = false;
    results = newResults;
    resultsList.updateContent();

    if (results.empty())
        statusLabel.setText ("No results found", juce::dontSendNotification);
    else
        statusLabel.setText (juce::String ((int) results.size()) + " results", juce::dontSendNotification);
}

void YouTubeBrowserPanel::thumbnailLoaded (const juce::String& videoId, const juce::Image& image)
{
    thumbnails.set (videoId, image);
    resultsList.repaint();
}

void YouTubeBrowserPanel::downloadProgressed (const juce::String&, float progress)
{
    int pct = juce::roundToInt (progress * 100.0f);
    statusLabel.setText ("Downloading " + juce::String (pct) + "%...", juce::dontSendNotification);
}

void YouTubeBrowserPanel::downloadFinished (const YouTubeDownloader::DownloadResult& result)
{
    downloading = false;

    if (result.success)
    {
        // Find the title for this video
        juce::String title = result.videoId;
        for (const auto& r : results)
        {
            if (r.id == result.videoId)
            {
                title = r.title;
                break;
            }
        }

        juce::StringPairArray vidParams;
        vidParams.set ("videoPath", result.localFile.getFullPathName());
        elementLibrary.addUserVisual (title, VisualElement::VisualKind::Video,
                                       juce::Colours::white, vidParams);

        statusLabel.setText ("Added to library!", juce::dontSendNotification);

        if (onVisualAdded)
            onVisualAdded();
    }
    else
    {
        statusLabel.setText ("Error: " + result.errorMessage, juce::dontSendNotification);
    }
}

juce::String YouTubeBrowserPanel::formatDuration (int seconds)
{
    if (seconds <= 0)
        return "";
    int mins = seconds / 60;
    int secs = seconds % 60;
    return juce::String (mins) + ":" + juce::String (secs).paddedLeft ('0', 2);
}

juce::String YouTubeBrowserPanel::formatViewCount (int64 count)
{
    if (count >= 1000000)
        return juce::String (count / 1000000) + "M";
    if (count >= 1000)
        return juce::String (count / 1000) + "K";
    return juce::String (count);
}

void YouTubeBrowserPanel::resized()
{
    auto area = getLocalBounds();

    // Search bar at top
    auto searchArea = area.removeFromTop (32);
    searchButton.setBounds (searchArea.removeFromRight (60));
    searchBox.setBounds (searchArea);

    // Action bar at bottom
    auto actionArea = area.removeFromBottom (32);
    useButton.setBounds (actionArea.removeFromLeft (actionArea.getWidth() / 2));
    statusLabel.setBounds (actionArea);

    // Results list fills the rest
    resultsList.setBounds (area);
}

void YouTubeBrowserPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0e0e24));

    if (! ytDlpAvailable && results.empty() && ! searching)
    {
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.setFont (juce::FontOptions (13.0f));

        auto area = getLocalBounds().reduced (20);
        area.removeFromTop (50);
        g.drawFittedText ("yt-dlp is required for YouTube search.\n\n"
                          "Install with:\n"
                          "  brew install yt-dlp\n\n"
                          "Then restart the app.",
                          area, juce::Justification::centredTop, 10);
    }
}

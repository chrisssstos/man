#include "ElementEditor.h"

ElementEditor::ElementEditor (AudioEngine& ae, SampleManager& sm)
    : audioEngine (ae), sampleManager (sm)
{
    setInterceptsMouseClicks (true, true);
}

void ElementEditor::openForSound (SoundElement* element)
{
    soundElement = element;
    visualElement = nullptr;
    isAudioMode = true;
    trimStart = element->getTrimStart();
    trimEnd = element->getTrimEnd();
    waveformBuilt = false;
    visible = true;
    setVisible (true);
    buildWaveform();
    repaint();
}

void ElementEditor::openForVisual (VisualElement* element)
{
    visualElement = element;
    soundElement = nullptr;
    isAudioMode = false;
    trimStart = element->getTrimStart();
    trimEnd = element->getTrimEnd();
    filmstripBuilt = false;
    visible = true;
    setVisible (true);
    buildFilmstrip();
    repaint();
}

void ElementEditor::close()
{
    visible = false;
    setVisible (false);
    soundElement = nullptr;
    visualElement = nullptr;
    waveformBuilt = false;
    filmstripBuilt = false;
    filmstripFrames.clear();
}

juce::Rectangle<int> ElementEditor::getHeaderBounds() const
{
    auto area = getLocalBounds().reduced (20);
    return area.removeFromTop (32);
}

juce::Rectangle<int> ElementEditor::getContentBounds() const
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (32 + 8);
    area.removeFromBottom (40 + 8);
    return area;
}

juce::Rectangle<int> ElementEditor::getFooterBounds() const
{
    auto area = getLocalBounds().reduced (20);
    return area.removeFromBottom (40);
}

juce::Rectangle<int> ElementEditor::getDoneButtonBounds() const
{
    auto header = getHeaderBounds();
    return header.removeFromRight (60);
}

juce::Rectangle<int> ElementEditor::getPlayButtonBounds() const
{
    auto footer = getFooterBounds();
    return footer.removeFromLeft (64);
}

float ElementEditor::xForTrimValue (float val) const
{
    auto content = getContentBounds();
    return content.getX() + val * content.getWidth();
}

float ElementEditor::trimValueForX (float x) const
{
    auto content = getContentBounds();
    return juce::jlimit (0.0f, 1.0f,
        (x - content.getX()) / (float) content.getWidth());
}

void ElementEditor::buildWaveform()
{
    if (soundElement == nullptr) return;

    auto file = resolveSampleFile (soundElement->getSamplePath());
    auto* loaded = sampleManager.loadSample (soundElement->getSamplePath());
    if (loaded == nullptr) return;

    const auto& buf = loaded->buffer;
    int numSamples = buf.getNumSamples();
    auto content = getContentBounds();
    int w = juce::jmax (1, content.getWidth());
    int h = content.getHeight();

    waveformPath.clear();

    float midY = h * 0.5f;

    waveformPath.startNewSubPath (0.0f, midY);

    for (int px = 0; px < w; ++px)
    {
        int startSmp = px * numSamples / w;
        int endSmp = juce::jmin (numSamples, (px + 1) * numSamples / w);
        float maxVal = 0.0f;

        for (int s = startSmp; s < endSmp; ++s)
        {
            float v = std::abs (buf.getSample (0, s));
            if (v > maxVal) maxVal = v;
        }

        float y = midY - maxVal * midY * 0.9f;
        waveformPath.lineTo ((float) px, y);
    }

    // Mirror bottom
    for (int px = w - 1; px >= 0; --px)
    {
        int startSmp = px * numSamples / w;
        int endSmp = juce::jmin (numSamples, (px + 1) * numSamples / w);
        float maxVal = 0.0f;

        for (int s = startSmp; s < endSmp; ++s)
        {
            float v = std::abs (buf.getSample (0, s));
            if (v > maxVal) maxVal = v;
        }

        float y = midY + maxVal * midY * 0.9f;
        waveformPath.lineTo ((float) px, y);
    }

    waveformPath.closeSubPath();
    waveformBuilt = true;
}

void ElementEditor::buildFilmstrip()
{
    if (visualElement == nullptr) return;

    auto videoPath = visualElement->getParams().getValue ("videoPath", "");
    if (videoPath.isEmpty()) return;

    filmstripDecoder.openFile (videoPath);
    if (! filmstripDecoder.isOpen()) return;

    filmstripFrames.clear();
    int numFrames = 12;
    for (int i = 0; i < numFrames; ++i)
    {
        float phase = (float) i / (float) (numFrames - 1);
        auto frame = filmstripDecoder.getFrameAtPhase (phase);
        filmstripFrames.push_back (frame);
    }

    filmstripBuilt = true;
}

void ElementEditor::paint (juce::Graphics& g)
{
    if (! visible) return;

    // Dim overlay background
    g.setColour (juce::Colours::black.withAlpha (0.75f));
    g.fillRect (getLocalBounds());

    // Editor panel
    auto panel = getLocalBounds().reduced (16);
    g.setColour (juce::Colour (TouchUI::kBgPanel));
    g.fillRoundedRectangle (panel.toFloat(), TouchUI::kCornerRadius);

    g.setColour (juce::Colour (TouchUI::kAccentPink).withAlpha (0.4f));
    g.drawRoundedRectangle (panel.toFloat(), TouchUI::kCornerRadius, 1.5f);

    // Header
    auto header = getHeaderBounds();
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (TouchUI::kFontNormal).withStyle ("Bold"));

    juce::String title = isAudioMode
        ? (soundElement != nullptr ? soundElement->getName() : "Sound")
        : (visualElement != nullptr ? visualElement->getName() : "Visual");
    g.drawText (title, header.withTrimmedRight (64), juce::Justification::centredLeft);

    // Done button
    auto doneBounds = getDoneButtonBounds();
    g.setColour (juce::Colour (TouchUI::kAccentPink));
    g.fillRoundedRectangle (doneBounds.toFloat(), 6.0f);
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (TouchUI::kFontSmall).withStyle ("Bold"));
    g.drawText ("Done", doneBounds, juce::Justification::centred);

    // Content area
    auto content = getContentBounds();
    g.setColour (juce::Colour (TouchUI::kBgCard));
    g.fillRoundedRectangle (content.toFloat(), 6.0f);

    if (isAudioMode)
        drawWaveform (g, content);
    else
        drawFilmstrip (g, content);

    drawOverlays (g, content);
    drawTrimHandles (g, content);

    // Footer
    auto footer = getFooterBounds();

    // Play button
    auto playBounds = getPlayButtonBounds();
    g.setColour (juce::Colour (TouchUI::kAccentGreen));
    g.fillRoundedRectangle (playBounds.toFloat(), 6.0f);
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (TouchUI::kFontSmall).withStyle ("Bold"));
    g.drawText ("Play", playBounds, juce::Justification::centred);

    // Time display
    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.setFont (juce::FontOptions (TouchUI::kFontSmall));
    auto timeText = formatTime (trimStart) + " - " + formatTime (trimEnd);
    g.drawText (timeText, footer.withTrimmedLeft (72), juce::Justification::centredLeft);
}

void ElementEditor::drawWaveform (juce::Graphics& g, juce::Rectangle<int> area)
{
    if (! waveformBuilt) return;

    g.saveState();
    g.reduceClipRegion (area);
    g.setColour (juce::Colour (TouchUI::kAccentCyan).withAlpha (0.7f));

    auto transform = juce::AffineTransform::translation (
        (float) area.getX(), (float) area.getY());
    g.fillPath (waveformPath, transform);
    g.restoreState();
}

void ElementEditor::drawFilmstrip (juce::Graphics& g, juce::Rectangle<int> area)
{
    if (! filmstripBuilt || filmstripFrames.empty()) return;

    int numFrames = (int) filmstripFrames.size();
    float frameW = (float) area.getWidth() / numFrames;
    float frameH = (float) area.getHeight();

    for (int i = 0; i < numFrames; ++i)
    {
        auto frameBounds = juce::Rectangle<float> (
            area.getX() + i * frameW, (float) area.getY(), frameW, frameH);

        if (filmstripFrames[(size_t) i].isValid())
        {
            g.setOpacity (1.0f);
            g.drawImage (filmstripFrames[(size_t) i], frameBounds,
                         juce::RectanglePlacement::centred);
        }
        else
        {
            g.setColour (juce::Colour (TouchUI::kBgCard).brighter (0.1f));
            g.fillRect (frameBounds);
        }

        // Frame separator
        g.setColour (juce::Colours::black.withAlpha (0.3f));
        g.drawVerticalLine ((int) (frameBounds.getRight()), (float) area.getY(),
                            (float) area.getBottom());
    }
}

void ElementEditor::drawOverlays (juce::Graphics& g, juce::Rectangle<int> area)
{
    // Dim trimmed-out regions
    float startX = xForTrimValue (trimStart);
    float endX = xForTrimValue (trimEnd);

    g.setColour (juce::Colours::black.withAlpha (0.5f));

    // Left trimmed region
    if (startX > area.getX())
        g.fillRect (juce::Rectangle<float> (
            (float) area.getX(), (float) area.getY(),
            startX - area.getX(), (float) area.getHeight()));

    // Right trimmed region
    if (endX < area.getRight())
        g.fillRect (juce::Rectangle<float> (
            endX, (float) area.getY(),
            area.getRight() - endX, (float) area.getHeight()));
}

void ElementEditor::drawTrimHandles (juce::Graphics& g, juce::Rectangle<int> area)
{
    float startX = xForTrimValue (trimStart);
    float endX = xForTrimValue (trimEnd);
    float handleW = 10.0f;

    // Start handle
    g.setColour (juce::Colour (TouchUI::kAccentGreen).withAlpha (0.9f));
    g.fillRect (juce::Rectangle<float> (
        startX - handleW * 0.5f, (float) area.getY(),
        handleW, (float) area.getHeight()));

    // End handle
    g.setColour (juce::Colour (TouchUI::kAccentPink).withAlpha (0.9f));
    g.fillRect (juce::Rectangle<float> (
        endX - handleW * 0.5f, (float) area.getY(),
        handleW, (float) area.getHeight()));
}

void ElementEditor::resized()
{
    if (visible)
    {
        waveformBuilt = false;
        filmstripBuilt = false;
        if (isAudioMode && soundElement != nullptr)
            buildWaveform();
        else if (! isAudioMode && visualElement != nullptr)
            buildFilmstrip();
    }
}

void ElementEditor::mouseDown (const juce::MouseEvent& e)
{
    auto pos = e.position;

    // Done button
    if (getDoneButtonBounds().toFloat().contains (pos))
    {
        // Apply trim
        if (isAudioMode && soundElement != nullptr)
        {
            soundElement->setTrimRange (trimStart, trimEnd);
            audioEngine.rebuildSounds();
        }
        else if (! isAudioMode && visualElement != nullptr)
        {
            visualElement->setTrimRange (trimStart, trimEnd);
        }
        listeners.call ([] (Listener& l) { l.editorClosed (true); });
        close();
        return;
    }

    // Play button
    if (getPlayButtonBounds().toFloat().contains (pos))
    {
        if (isAudioMode && soundElement != nullptr)
        {
            auto file = resolveSampleFile (soundElement->getSamplePath());
            if (file.existsAsFile())
                audioEngine.previewFileTrimmed (file, trimStart, trimEnd);
        }
        return;
    }

    // Check for trim handle hits
    auto content = getContentBounds();
    if (! content.toFloat().contains (pos)) return;

    float startX = xForTrimValue (trimStart);
    float endX = xForTrimValue (trimEnd);
    float hitZone = 20.0f;

    if (std::abs (pos.x - startX) < hitZone)
        dragTarget = DragTarget::Start;
    else if (std::abs (pos.x - endX) < hitZone)
        dragTarget = DragTarget::End;
    else
        dragTarget = DragTarget::None;
}

void ElementEditor::mouseDrag (const juce::MouseEvent& e)
{
    if (dragTarget == DragTarget::None) return;

    float val = trimValueForX (e.position.x);

    if (dragTarget == DragTarget::Start)
    {
        trimStart = juce::jlimit (0.0f, trimEnd - 0.01f, val);
    }
    else if (dragTarget == DragTarget::End)
    {
        trimEnd = juce::jlimit (trimStart + 0.01f, 1.0f, val);
    }

    repaint();
}

void ElementEditor::mouseUp (const juce::MouseEvent&)
{
    dragTarget = DragTarget::None;
}

juce::String ElementEditor::formatTime (float fraction) const
{
    float totalSeconds = 0.0f;

    if (isAudioMode && soundElement != nullptr)
    {
        auto* loaded = sampleManager.loadSample (soundElement->getSamplePath());
        if (loaded != nullptr)
            totalSeconds = (float) loaded->buffer.getNumSamples() / (float) loaded->sampleRate;
    }
    else if (! isAudioMode && visualElement != nullptr)
    {
        totalSeconds = 10.0f; // default duration estimate for video
    }

    float secs = fraction * totalSeconds;
    int minutes = (int) secs / 60;
    float remainder = secs - minutes * 60.0f;

    return juce::String::formatted ("%02d:%04.1f", minutes, remainder);
}

juce::File ElementEditor::resolveSampleFile (const juce::String& path) const
{
    juce::File file (path);
    if (file.existsAsFile())
        return file;

    auto appBundle = juce::File::getSpecialLocation (juce::File::currentApplicationFile);
    auto samplesDir = appBundle.getChildFile ("Contents/Resources/Samples");
    if (! samplesDir.isDirectory())
    {
        auto exeDir = juce::File::getSpecialLocation (
            juce::File::currentExecutableFile).getParentDirectory();
        samplesDir = exeDir.getChildFile ("Samples");
    }

    return samplesDir.getChildFile (path);
}

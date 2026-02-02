#include "TrackLane.h"

TrackLane::TrackLane (int idx, Sketch& sk, ElementLibrary& lib, SampleManager& sm)
    : trackIndex (idx), sketch (sk), elementLibrary (lib), sampleManager (sm)
{
}

void TrackLane::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();
    int totalW = getWidth();

    // Header background
    auto header = area.removeFromLeft (kHeaderWidth);
    g.setColour (juce::Colour (0xff1a1a3a));
    g.fillRect (header);

    // Track number
    g.setColour (juce::Colours::white.withAlpha (0.5f));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText (juce::String (trackIndex + 1), header.reduced (4, 0),
                juce::Justification::centred, true);

    // Grid area background
    g.setColour (dragHovering ? juce::Colour (0xff141438) : juce::Colour (0xff0e0e24));
    g.fillRect (area);

    // Beat grid lines
    int gridStart = kHeaderWidth;
    int ppb = (int) pixelsPerBeat;
    for (int beat = 0; gridStart + beat * ppb < totalW; ++beat)
    {
        int x = gridStart + beat * ppb;
        bool isMeasure = (beat % 4 == 0);
        g.setColour (isMeasure ? juce::Colour (0xff2a2a4a) : juce::Colour (0xff181838));
        g.drawVerticalLine (x, (float) area.getY(), (float) area.getBottom());
    }

    // Bottom separator
    g.setColour (juce::Colour (0xff1c1c3a));
    g.drawLine (0.0f, (float) getHeight() - 0.5f, (float) totalW, (float) getHeight() - 0.5f);

    // Playhead highlight
    if (playheadBeat >= 0.0)
    {
        int px = kHeaderWidth + (int) (playheadBeat * pixelsPerBeat);
        if (px >= kHeaderWidth && px < totalW)
        {
            g.setColour (juce::Colour (0xff44ff44).withAlpha (0.12f));
            g.fillRect (px - 1, area.getY(), (int) pixelsPerBeat + 2, area.getHeight());

            g.setColour (juce::Colour (0xff44ff44).withAlpha (0.8f));
            g.drawVerticalLine (px, (float) area.getY(), (float) area.getBottom());
        }
    }
}

void TrackLane::paintOverChildren (juce::Graphics& g)
{
    if (dragHovering)
    {
        g.setColour (juce::Colours::white.withAlpha (0.7f));
        g.drawVerticalLine (dragHoverX, 0.0f, (float) getHeight());

        g.setColour (juce::Colours::white.withAlpha (0.15f));
        int snapW = juce::jmax (4, (int) (snapBeats * pixelsPerBeat));
        g.fillRect (dragHoverX, 0, snapW, getHeight());
    }
}

void TrackLane::resized()
{
    layoutClips();
}

void TrackLane::mouseDown (const juce::MouseEvent& e)
{
    juce::ignoreUnused (e);
    // No click-to-add on generic lanes — only drag & drop from palette
}

// --- DragAndDropTarget ---

bool TrackLane::isInterestedInDragSource (const SourceDetails& details)
{
    return details.description.toString().isNotEmpty();
}

void TrackLane::itemDragEnter (const SourceDetails&)
{
    dragHovering = true;
    repaint();
}

void TrackLane::itemDragMove (const SourceDetails& details)
{
    int localX = details.localPosition.x - kHeaderWidth;
    double beat = juce::jmax (0.0, (double) localX / pixelsPerBeat);
    if (snapBeats > 0.0)
        beat = std::floor (beat / snapBeats) * snapBeats;
    dragHoverX = kHeaderWidth + (int) (beat * pixelsPerBeat);
    repaint();
}

void TrackLane::itemDragExit (const SourceDetails&)
{
    dragHovering = false;
    repaint();
}

void TrackLane::itemDropped (const SourceDetails& details)
{
    dragHovering = false;

    int localX = details.localPosition.x - kHeaderWidth;
    double beat = juce::jmax (0.0, (double) localX / pixelsPerBeat);
    if (snapBeats > 0.0)
        beat = std::floor (beat / snapBeats) * snapBeats;

    auto droppedElementId = details.description.toString();
    listeners.call ([&] (Listener& l) { l.trackClipAdded (droppedElementId, trackIndex, beat); });
    repaint();
}

// --- Clips ---

void TrackLane::rebuildClips()
{
    clipComponents.clear();

    const auto& clips = sketch.getClips();
    for (int i = 0; i < (int) clips.size(); ++i)
    {
        const auto& clip = clips[(size_t) i];
        if (clip.track != trackIndex)
            continue;

        // Look up element info for name, colour, sample
        juce::String clipName = clip.elementId.id;
        juce::Colour clipColour (0xff888888);
        juce::String samplePath;

        auto* elem = elementLibrary.getElementById (clip.elementId);
        if (elem != nullptr)
        {
            clipName = elem->getName();
            if (auto* av = dynamic_cast<AudioVisualElement*> (elem))
            {
                clipColour = av->getColour();
                samplePath = av->getSamplePath();
            }
        }

        auto* comp = clipComponents.add (
            new ClipComponent (i, clipName, clipColour, clip.startBeat, clip.durationBeats));
        comp->setPixelsPerBeat (pixelsPerBeat);
        comp->setSnapBeats (snapBeats);
        comp->setHeaderWidth (kHeaderWidth);
        comp->setRowHeight (kRowHeight);
        comp->addListener (this);
        addAndMakeVisible (comp);

        // Load waveform preview if sample exists
        if (samplePath.isNotEmpty())
        {
            if (auto* loaded = sampleManager.loadSample (samplePath))
                comp->setWaveform (&loaded->buffer, loaded->buffer.getNumSamples());
        }
    }

    layoutClips();
}

void TrackLane::setPlayheadBeat (double beat)
{
    if (playheadBeat != beat)
    {
        playheadBeat = beat;
        repaint();
    }
}

void TrackLane::setPixelsPerBeat (double ppb)
{
    pixelsPerBeat = ppb;
    for (auto* comp : clipComponents)
        comp->setPixelsPerBeat (ppb);
    layoutClips();
    repaint();
}

void TrackLane::setSnapBeats (double snap)
{
    snapBeats = snap;
    for (auto* comp : clipComponents)
        comp->setSnapBeats (snap);
}

void TrackLane::clipMoved (int clipIndex, double newBeat)
{
    listeners.call ([clipIndex, newBeat] (Listener& l) { l.trackClipMoved (clipIndex, newBeat); });
    layoutClips();
}

void TrackLane::clipResized (int clipIndex, double newDuration)
{
    listeners.call ([clipIndex, newDuration] (Listener& l) { l.trackClipResized (clipIndex, newDuration); });
    layoutClips();
}

void TrackLane::clipDeleted (int clipIndex)
{
    listeners.call ([clipIndex] (Listener& l) { l.trackClipRemoved (clipIndex); });
}

void TrackLane::clipTrackChanged (int clipIndex, int deltaTrack)
{
    listeners.call ([clipIndex, deltaTrack] (Listener& l) { l.trackClipTrackChanged (clipIndex, deltaTrack); });
}

void TrackLane::layoutClips()
{
    for (auto* comp : clipComponents)
    {
        int x = kHeaderWidth + (int) (comp->getStartBeat() * pixelsPerBeat);
        int w = juce::jmax (4, (int) (comp->getDurationBeats() * pixelsPerBeat));
        comp->setBounds (x, 2, w, getHeight() - 4);
    }
}

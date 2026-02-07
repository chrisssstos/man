#include "Timeline.h"

// --- GridDropTarget ---

bool ArrangementTimeline::GridDropTarget::isInterestedInDragSource (const SourceDetails& details)
{
    return details.description.toString().isNotEmpty();
}

void ArrangementTimeline::GridDropTarget::itemDropped (const SourceDetails& details)
{
    auto elemId = details.description.toString();

    auto* lane = owner.findLaneAtY (details.localPosition.y);
    if (lane == nullptr)
        return;

    int localX = details.localPosition.x - TouchUI::kTrackHeaderWidth;
    double beat = juce::jmax (0.0, (double) localX / owner.pixelsPerBeat);
    if (owner.snapBeats > 0.0)
        beat = std::floor (beat / owner.snapBeats) * owner.snapBeats;

    owner.trackClipAdded (elemId, lane->getTrackIndex(), beat);
}

// --- ArrangementTimeline ---

ArrangementTimeline::ArrangementTimeline (Sketch& sk, ElementLibrary& lib, SampleManager& sm)
    : sketch (sk), elementLibrary (lib), sampleManager (sm), gridContainer (*this)
{
    gridViewport.setViewedComponent (&gridContainer, false);
    gridViewport.setScrollBarsShown (true, true);
    addAndMakeVisible (gridViewport);
}

void ArrangementTimeline::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (TouchUI::kBgDeep));

    auto rulerArea = getLocalBounds().removeFromTop (kRulerHeight);
    paintRuler (g, rulerArea);
}

void ArrangementTimeline::paintRuler (juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour (juce::Colour (TouchUI::kBgCard));
    g.fillRect (area);

    g.setColour (juce::Colour (0xff2a2a4a));
    g.drawLine ((float) area.getX(), (float) area.getBottom() - 0.5f,
                (float) area.getRight(), (float) area.getBottom() - 0.5f);

    int scrollX = gridViewport.getViewPositionX();
    int gridStart = TouchUI::kTrackHeaderWidth - scrollX;
    int totalW = area.getWidth();
    int ppb = (int) pixelsPerBeat;

    int firstBeat = juce::jmax (0, scrollX / juce::jmax (1, ppb));
    int lastBeat = (scrollX + totalW) / juce::jmax (1, ppb) + 2;

    bool showSixteenths = pixelsPerBeat >= 120.0;
    bool showEighths = pixelsPerBeat >= 60.0;

    for (int beat = firstBeat; beat <= lastBeat; ++beat)
    {
        int x = gridStart + beat * ppb;
        if (x < 0 || x > totalW)
            continue;

        bool isMeasure = (beat % 4 == 0);

        if (isMeasure)
        {
            g.setColour (juce::Colour (0xff3a3a5c));
            g.drawVerticalLine (x, (float) area.getY() + 2.0f, (float) area.getBottom());

            int bar = beat / 4 + 1;

            double seconds = (beat / bpm) * 60.0;
            int mins = (int) (seconds / 60.0);
            double secs = seconds - mins * 60.0;
            auto timeStr = juce::String::formatted ("%d:%04.1f", mins, secs);

            g.setFont (juce::FontOptions (TouchUI::kFontSmall));
            g.setColour (juce::Colours::white.withAlpha (0.8f));
            g.drawText (juce::String (bar), x + 4, area.getY(), 30, area.getHeight() / 2,
                        juce::Justification::centredLeft);

            g.setColour (juce::Colours::white.withAlpha (0.45f));
            g.setFont (juce::FontOptions (TouchUI::kFontTiny));
            g.drawText (timeStr, x + 4, area.getY() + area.getHeight() / 2, 50, area.getHeight() / 2,
                        juce::Justification::centredLeft);
        }
        else
        {
            g.setColour (juce::Colour (0xff282848));
            g.drawVerticalLine (x, (float) area.getY() + 8.0f, (float) area.getBottom());
        }

        if (showEighths)
        {
            int halfX = x + ppb / 2;
            if (halfX > 0 && halfX < totalW)
            {
                g.setColour (juce::Colour (0xff1e1e3e));
                g.drawVerticalLine (halfX, (float) area.getY() + 14.0f, (float) area.getBottom());
            }
        }

        if (showSixteenths)
        {
            for (int sub = 1; sub < 4; sub += 2)
            {
                int subX = x + sub * ppb / 4;
                if (subX > 0 && subX < totalW)
                {
                    g.setColour (juce::Colour (0xff181830));
                    g.drawVerticalLine (subX, (float) area.getY() + 18.0f, (float) area.getBottom());
                }
            }
        }
    }

    // Playhead on ruler
    if (playheadBeat >= 0.0)
    {
        int px = gridStart + (int) (playheadBeat * pixelsPerBeat);
        if (px >= 0 && px < totalW)
        {
            g.setColour (juce::Colour (0xff44ff44));
            g.drawVerticalLine (px, (float) area.getY(), (float) area.getBottom());

            juce::Path tri;
            tri.addTriangle ((float) px - 6.0f, (float) area.getY(),
                             (float) px + 6.0f, (float) area.getY(),
                             (float) px, (float) area.getY() + 10.0f);
            g.setColour (juce::Colour (0xff44ff44));
            g.fillPath (tri);
        }
    }
}

void ArrangementTimeline::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop (kRulerHeight);
    gridViewport.setBounds (area);

    int numBeats = 64;
    double totalLen = sketch.getTotalLengthBeats();
    if (totalLen > 0.0)
        numBeats = juce::jmax (numBeats, (int) totalLen + 16);

    int gridW = TouchUI::kTrackHeaderWidth + numBeats * (int) pixelsPerBeat;
    int gridH = (int) trackLanes.size() * TouchUI::kTrackRowHeight;
    gridContainer.setSize (juce::jmax (gridW, area.getWidth()),
                           juce::jmax (gridH, area.getHeight()));

    int y = 0;
    for (auto* lane : trackLanes)
    {
        lane->setBounds (0, y, gridContainer.getWidth(), TouchUI::kTrackRowHeight);
        y += TouchUI::kTrackRowHeight;
    }
}

void ArrangementTimeline::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (e.mods.isCommandDown())
    {
        int scrollX = gridViewport.getViewPositionX();
        double cursorXInGrid = (double) (e.x - TouchUI::kTrackHeaderWidth + scrollX);
        double beatAtCursor = cursorXInGrid / pixelsPerBeat;

        double zoomFactor = 1.0 + wheel.deltaY * 0.3;
        double newPPB = juce::jlimit (kMinPixelsPerBeat, kMaxPixelsPerBeat, pixelsPerBeat * zoomFactor);

        if (newPPB != pixelsPerBeat)
        {
            pixelsPerBeat = newPPB;
            updateTrackLaneZoom();
            resized();

            int newScrollX = (int) (beatAtCursor * pixelsPerBeat) - (e.x - TouchUI::kTrackHeaderWidth);
            gridViewport.setViewPosition (juce::jmax (0, newScrollX), gridViewport.getViewPositionY());

            repaint();
        }
    }
    else
    {
        gridViewport.mouseWheelMove (e, wheel);
    }
}

void ArrangementTimeline::mouseMagnify (const juce::MouseEvent& e, float scaleFactor)
{
    int scrollX = gridViewport.getViewPositionX();
    double cursorXInGrid = (double) (e.x - TouchUI::kTrackHeaderWidth + scrollX);
    double beatAtCursor = cursorXInGrid / pixelsPerBeat;

    double newPPB = juce::jlimit (kMinPixelsPerBeat, kMaxPixelsPerBeat, pixelsPerBeat * (double) scaleFactor);

    if (newPPB != pixelsPerBeat)
    {
        pixelsPerBeat = newPPB;
        updateTrackLaneZoom();
        resized();

        int newScrollX = (int) (beatAtCursor * pixelsPerBeat) - (e.x - TouchUI::kTrackHeaderWidth);
        gridViewport.setViewPosition (juce::jmax (0, newScrollX), gridViewport.getViewPositionY());

        repaint();
    }
}

void ArrangementTimeline::mouseDown (const juce::MouseEvent& e)
{
    auto rulerArea = getLocalBounds().removeFromTop (kRulerHeight);
    if (rulerArea.contains (e.getPosition()))
    {
        draggingPlayhead = true;

        int scrollX = gridViewport.getViewPositionX();
        int gridStart = TouchUI::kTrackHeaderWidth - scrollX;
        double beat = (double) (e.x - gridStart) / pixelsPerBeat;
        beat = juce::jmax (0.0, beat);

        if (snapBeats > 0.0)
            beat = std::round (beat / snapBeats) * snapBeats;

        timelineListeners.call ([beat] (Listener& l) { l.timelinePlayheadClicked (beat); });
    }
}

void ArrangementTimeline::mouseDrag (const juce::MouseEvent& e)
{
    if (draggingPlayhead)
    {
        int scrollX = gridViewport.getViewPositionX();
        int gridStart = TouchUI::kTrackHeaderWidth - scrollX;
        double beat = (double) (e.x - gridStart) / pixelsPerBeat;
        beat = juce::jmax (0.0, beat);

        if (snapBeats > 0.0)
            beat = std::round (beat / snapBeats) * snapBeats;

        timelineListeners.call ([beat] (Listener& l) { l.timelinePlayheadClicked (beat); });
    }
}

void ArrangementTimeline::mouseUp (const juce::MouseEvent&)
{
    draggingPlayhead = false;
}

void ArrangementTimeline::rebuild()
{
    trackLanes.clear();
    gridContainer.removeAllChildren();

    for (int i = 0; i < kNumTracks; ++i)
    {
        auto* lane = trackLanes.add (new TrackLane (i, sketch, elementLibrary, sampleManager));
        lane->setPixelsPerBeat (pixelsPerBeat);
        lane->setSnapBeats (snapBeats);
        lane->addListener (this);
        gridContainer.addAndMakeVisible (lane);
        lane->rebuildClips();
    }

    resized();
}

void ArrangementTimeline::setPlayheadBeat (double beat)
{
    playheadBeat = beat;

    for (auto* lane : trackLanes)
        lane->setPlayheadBeat (beat);

    repaint();
}

void ArrangementTimeline::rebuildClips()
{
    for (auto* lane : trackLanes)
        lane->rebuildClips();
}

void ArrangementTimeline::setPixelsPerBeat (double ppb)
{
    pixelsPerBeat = juce::jlimit (kMinPixelsPerBeat, kMaxPixelsPerBeat, ppb);
    updateTrackLaneZoom();
    resized();
    repaint();
}

void ArrangementTimeline::setSnapBeats (double snap)
{
    snapBeats = snap;
    for (auto* lane : trackLanes)
        lane->setSnapBeats (snap);
}

void ArrangementTimeline::setBPM (double newBPM)
{
    bpm = newBPM;
    repaint();
}

void ArrangementTimeline::updateTrackLaneZoom()
{
    for (auto* lane : trackLanes)
        lane->setPixelsPerBeat (pixelsPerBeat);
}

void ArrangementTimeline::trackClipAdded (const juce::String& elementId, int track, double beat)
{
    Clip clip;
    clip.elementId = { elementId };
    clip.startBeat = beat;
    clip.durationBeats = juce::jmax (1.0, snapBeats);
    clip.track = track;

    sketch.addClip (clip);
    rebuildClips();
    resized();
}

void ArrangementTimeline::trackClipRemoved (int clipIndex)
{
    sketch.removeClip (clipIndex);
    rebuildClips();
}

void ArrangementTimeline::trackClipMoved (int clipIndex, double newBeat)
{
    sketch.moveClip (clipIndex, newBeat, sketch.getClip (clipIndex).track);
}

void ArrangementTimeline::trackClipResized (int clipIndex, double newDuration)
{
    sketch.resizeClip (clipIndex, newDuration);
}

void ArrangementTimeline::trackClipTrackChanged (int clipIndex, int deltaTrack)
{
    auto& clip = sketch.getClip (clipIndex);
    int newTrack = juce::jlimit (0, kNumTracks - 1, clip.track + deltaTrack);
    if (newTrack != clip.track)
    {
        sketch.moveClip (clipIndex, clip.startBeat, newTrack);
        needsRebuildAfterDrag = true;
    }
}

void ArrangementTimeline::trackClipDragEnded (int)
{
    if (needsRebuildAfterDrag)
    {
        needsRebuildAfterDrag = false;
        rebuildClips();
        resized();
    }
}

TrackLane* ArrangementTimeline::findLaneAtY (int y)
{
    for (auto* lane : trackLanes)
        if (y >= lane->getY() && y < lane->getY() + lane->getHeight())
            return lane;
    return nullptr;
}

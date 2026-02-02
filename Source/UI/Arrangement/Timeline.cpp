#include "Timeline.h"

ArrangementTimeline::ArrangementTimeline (Sketch& sk, ElementLibrary& lib)
    : sketch (sk), elementLibrary (lib)
{
    gridViewport.setViewedComponent (&gridContainer, false);
    gridViewport.setScrollBarsShown (true, true);
    addAndMakeVisible (gridViewport);
}

void ArrangementTimeline::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0a1e));

    // Ruler at top
    auto rulerArea = getLocalBounds().removeFromTop (kRulerHeight);
    paintRuler (g, rulerArea);
}

void ArrangementTimeline::paintRuler (juce::Graphics& g, juce::Rectangle<int> area)
{
    // Smooth dark ruler background
    g.setColour (juce::Colour (0xff1a1a3a));
    g.fillRect (area);

    // Subtle bottom border
    g.setColour (juce::Colour (0xff2a2a4a));
    g.drawLine ((float) area.getX(), (float) area.getBottom() - 0.5f,
                (float) area.getRight(), (float) area.getBottom() - 0.5f);

    g.setFont (juce::FontOptions (11.0f));

    // Account for viewport scroll
    int scrollX = gridViewport.getViewPositionX();
    int gridStart = TrackLane::kHeaderWidth - scrollX;

    int totalW = area.getWidth();
    int ppb = TrackLane::kPixelsPerBeat;

    int firstBeat = juce::jmax (0, scrollX / ppb);
    int lastBeat = (scrollX + totalW) / ppb + 2;

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
            g.setColour (juce::Colours::white.withAlpha (0.8f));
            g.drawText (juce::String (bar), x + 4, area.getY(), 30, area.getHeight(),
                        juce::Justification::centredLeft);
        }
        else
        {
            g.setColour (juce::Colour (0xff282848));
            g.drawVerticalLine (x, (float) area.getY() + 8.0f, (float) area.getBottom());
        }
    }

    // Playhead on ruler — smooth green marker
    if (playheadBeat >= 0.0)
    {
        int px = gridStart + (int) (playheadBeat * ppb);
        if (px >= 0 && px < totalW)
        {
            // Playhead line
            g.setColour (juce::Colour (0xff44ff44));
            g.drawVerticalLine (px, (float) area.getY(), (float) area.getBottom());

            // Smooth triangle marker
            juce::Path tri;
            tri.addTriangle ((float) px - 5.0f, (float) area.getY(),
                             (float) px + 5.0f, (float) area.getY(),
                             (float) px, (float) area.getY() + 8.0f);
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

    int gridW = TrackLane::kHeaderWidth + numBeats * TrackLane::kPixelsPerBeat;
    int gridH = (int) trackLanes.size() * TrackLane::kRowHeight;
    gridContainer.setSize (juce::jmax (gridW, area.getWidth()),
                           juce::jmax (gridH, area.getHeight()));

    int y = 0;
    for (auto* lane : trackLanes)
    {
        lane->setBounds (0, y, gridContainer.getWidth(), TrackLane::kRowHeight);
        y += TrackLane::kRowHeight;
    }
}

void ArrangementTimeline::rebuild()
{
    trackLanes.clear();
    gridContainer.removeAllChildren();

    auto avElements = elementLibrary.getAllDiscoveredAV();

    for (auto* av : avElements)
    {
        auto elemId = av->getId().id;

        auto* lane = trackLanes.add (new TrackLane (
            elemId, av->getName(), av->getColour(), sketch));
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

    repaint(); // For ruler playhead
}

void ArrangementTimeline::rebuildClips()
{
    for (auto* lane : trackLanes)
        lane->rebuildClips();
}

void ArrangementTimeline::trackClipAdded (const juce::String& elementId, double beat)
{
    Clip clip;
    clip.elementId = { elementId };
    clip.startBeat = beat;
    clip.durationBeats = 1.0;

    for (int i = 0; i < trackLanes.size(); ++i)
    {
        if (trackLanes[i]->getElementId() == elementId)
        {
            clip.track = i;
            break;
        }
    }

    sketch.addClip (clip);
    rebuildClips();
    resized();
}

void ArrangementTimeline::trackClipRemoved (int clipIndex)
{
    sketch.removeClip (clipIndex);
    rebuildClips();
}

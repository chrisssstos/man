#include "TrackLane.h"

TrackLane::TrackLane (const juce::String& elemId, const juce::String& n,
                      juce::Colour c, Sketch& sk)
    : elementId (elemId), name (n), colour (c), sketch (sk)
{
}

void TrackLane::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();
    int totalW = getWidth();

    // Header background — smooth dark panel
    auto header = area.removeFromLeft (kHeaderWidth);
    g.setColour (juce::Colour (0xff1a1a3a));
    g.fillRect (header);

    // Colour indicator strip — rounded left edge
    g.setColour (colour);
    g.fillRoundedRectangle (header.removeFromLeft (6).toFloat(), 2.0f);

    // Element name
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (13.0f));
    g.drawText (name, header.reduced (8, 0), juce::Justification::centredLeft, true);

    // Grid area background
    g.setColour (juce::Colour (0xff0e0e24));
    g.fillRect (area);

    // Beat grid lines — softer
    int gridStart = kHeaderWidth;
    for (int beat = 0; gridStart + beat * kPixelsPerBeat < totalW; ++beat)
    {
        int x = gridStart + beat * kPixelsPerBeat;
        bool isMeasure = (beat % 4 == 0);
        g.setColour (isMeasure ? juce::Colour (0xff2a2a4a) : juce::Colour (0xff181838));
        g.drawVerticalLine (x, (float) area.getY(), (float) area.getBottom());
    }

    // Bottom separator — subtle
    g.setColour (juce::Colour (0xff1c1c3a));
    g.drawLine (0.0f, (float) getHeight() - 0.5f, (float) totalW, (float) getHeight() - 0.5f);

    // Playhead highlight — smooth glow
    if (playheadBeat >= 0.0)
    {
        int px = kHeaderWidth + (int) (playheadBeat * kPixelsPerBeat);
        if (px >= kHeaderWidth && px < totalW)
        {
            // Soft glow column
            g.setColour (juce::Colour (0xff44ff44).withAlpha (0.12f));
            g.fillRect (px - 1, area.getY(), kPixelsPerBeat + 2, area.getHeight());

            // Crisp playhead line
            g.setColour (juce::Colour (0xff44ff44).withAlpha (0.8f));
            g.drawVerticalLine (px, (float) area.getY(), (float) area.getBottom());
        }
    }
}

void TrackLane::resized()
{
    layoutClips();
}

void TrackLane::mouseDown (const juce::MouseEvent& e)
{
    int localX = e.x - kHeaderWidth;
    if (localX < 0)
        return;

    double beatClicked = (double) localX / (double) kPixelsPerBeat;
    beatClicked = std::floor (beatClicked); // Snap to beat

    // Check if we clicked on an existing clip
    for (auto* comp : clipComponents)
    {
        if (beatClicked >= comp->getStartBeat() &&
            beatClicked < comp->getStartBeat() + comp->getDurationBeats())
        {
            listeners.call ([&] (Listener& l) { l.trackClipRemoved (comp->getClipIndex()); });
            return;
        }
    }

    // Add a new clip at this beat
    listeners.call ([&] (Listener& l) { l.trackClipAdded (elementId, beatClicked); });
}

void TrackLane::rebuildClips()
{
    clipComponents.clear();

    const auto& clips = sketch.getClips();
    for (int i = 0; i < (int) clips.size(); ++i)
    {
        const auto& clip = clips[(size_t) i];
        if (clip.elementId.id == elementId)
        {
            auto* comp = clipComponents.add (
                new ClipComponent (i, name, colour, clip.startBeat, clip.durationBeats));
            addAndMakeVisible (comp);
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

void TrackLane::layoutClips()
{
    for (auto* comp : clipComponents)
    {
        int x = kHeaderWidth + (int) (comp->getStartBeat() * kPixelsPerBeat);
        int w = juce::jmax (4, (int) (comp->getDurationBeats() * kPixelsPerBeat));
        comp->setBounds (x, 2, w, getHeight() - 4);
    }
}

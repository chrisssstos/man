#include "ClipComponent.h"

ClipComponent::ClipComponent (int idx, const juce::String& n, juce::Colour c,
                               double start, double dur)
    : clipIndex (idx), name (n), colour (c), startBeat (start), durationBeats (dur)
{
}

void ClipComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);

    // Filled rounded rectangle
    auto fillColour = colour.withAlpha (hovering ? 0.75f : 0.65f);
    g.setColour (fillColour);
    g.fillRoundedRectangle (bounds, 6.0f);

    // Top highlight for depth
    auto topSlice = bounds.removeFromTop (bounds.getHeight() * 0.35f);
    g.setColour (colour.brighter (0.3f).withAlpha (0.25f));
    g.fillRoundedRectangle (topSlice.withBottom (topSlice.getBottom() + 6.0f), 6.0f);

    // Border
    bounds = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (colour.brighter (0.5f));
    g.drawRoundedRectangle (bounds, 6.0f, 1.5f);

    // Resize handle indicator — vertical lines on right edge
    if (bounds.getWidth() > 20.0f)
    {
        float handleX = bounds.getRight() - (float) kResizeHandleWidth;
        g.setColour (juce::Colours::white.withAlpha (0.3f));
        g.drawVerticalLine ((int) (handleX + 2.0f), bounds.getY() + 4.0f, bounds.getBottom() - 4.0f);
        g.drawVerticalLine ((int) (handleX + 5.0f), bounds.getY() + 4.0f, bounds.getBottom() - 4.0f);
    }

    // Waveform preview behind text
    if (! waveformPath.isEmpty() && bounds.getWidth() > 10.0f)
    {
        g.setColour (colour.brighter (0.4f).withAlpha (0.3f));
        auto wfBounds = bounds.reduced (2.0f, 4.0f);
        auto pathBounds = waveformPath.getBounds();
        if (! pathBounds.isEmpty())
        {
            auto transform = juce::AffineTransform::scale (
                wfBounds.getWidth() / pathBounds.getWidth(),
                wfBounds.getHeight() / pathBounds.getHeight())
                .translated (wfBounds.getX() - pathBounds.getX() * (wfBounds.getWidth() / pathBounds.getWidth()),
                             wfBounds.getY() - pathBounds.getY() * (wfBounds.getHeight() / pathBounds.getHeight()));
            g.fillPath (waveformPath, transform);
        }
    }

    // Clip name text
    if (bounds.getWidth() > 24.0f)
    {
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (11.0f));
        g.drawText (name, bounds.reduced (5.0f, 0.0f),
                    juce::Justification::centredLeft, true);
    }
}

void ClipComponent::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        listeners.call ([this] (Listener& l) { l.clipDeleted (clipIndex); });
        return;
    }

    if (isInResizeZone (e.getPosition()))
    {
        resizing = true;
        dragStartDuration = durationBeats;
    }
    else
    {
        dragging = true;
        // Record offset from the clip's left edge in beats
        dragOffsetBeats = (double) e.x / pixelsPerBeat;
        dragStartBeat = startBeat;
        dragAccumulatedY = 0;
    }
}

void ClipComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (resizing)
    {
        // e.getDistanceFromDragStartX() gives pixels moved since mouseDown
        double deltaPx = (double) e.getDistanceFromDragStartX();
        double deltaBeats = deltaPx / pixelsPerBeat;
        double newDuration = snapBeat (dragStartDuration + deltaBeats);
        newDuration = juce::jmax (snapValue > 0.0 ? snapValue : 0.25, newDuration);

        if (newDuration != durationBeats)
        {
            durationBeats = newDuration;
            listeners.call ([this, newDuration] (Listener& l) { l.clipResized (clipIndex, newDuration); });
        }
    }
    else if (dragging)
    {
        // Horizontal movement — position relative to parent (the TrackLane)
        auto posInParent = e.getEventRelativeTo (getParentComponent()).position;
        double beatAtCursor = (posInParent.x - (float) headerWidth) / pixelsPerBeat;
        double newStart = snapBeat (beatAtCursor - dragOffsetBeats);
        newStart = juce::jmax (0.0, newStart);

        if (newStart != startBeat)
        {
            startBeat = newStart;
            listeners.call ([this, newStart] (Listener& l) { l.clipMoved (clipIndex, newStart); });
        }

        // Vertical movement — detect crossing into adjacent track
        int yFromStart = (int) (e.position.y - e.mouseDownPosition.y);
        int trackDelta = (yFromStart - dragAccumulatedY) / rowHeight;
        if (trackDelta != 0)
        {
            dragAccumulatedY += trackDelta * rowHeight;
            listeners.call ([this, trackDelta] (Listener& l) { l.clipTrackChanged (clipIndex, trackDelta); });
        }
    }
}

void ClipComponent::mouseUp (const juce::MouseEvent&)
{
    dragging = false;
    resizing = false;
}

void ClipComponent::mouseMove (const juce::MouseEvent& e)
{
    if (isInResizeZone (e.getPosition()))
        setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
    else
        setMouseCursor (juce::MouseCursor::DraggingHandCursor);
}

void ClipComponent::mouseEnter (const juce::MouseEvent&)
{
    hovering = true;
    repaint();
}

void ClipComponent::mouseExit (const juce::MouseEvent&)
{
    hovering = false;
    setMouseCursor (juce::MouseCursor::NormalCursor);
    repaint();
}

bool ClipComponent::isInResizeZone (const juce::Point<int>& pos) const
{
    return pos.x >= getWidth() - kResizeHandleWidth;
}

double ClipComponent::snapBeat (double beat) const
{
    if (snapValue <= 0.0)
        return beat;
    return std::round (beat / snapValue) * snapValue;
}

void ClipComponent::setWaveform (const juce::AudioBuffer<float>* buffer, int numSamples)
{
    waveformPath.clear();

    if (buffer == nullptr || numSamples <= 0)
        return;

    const int numPoints = juce::jmin (128, numSamples);
    const int samplesPerPoint = numSamples / numPoints;
    const float* data = buffer->getReadPointer (0);

    float midY = 0.5f;

    waveformPath.startNewSubPath (0.0f, midY);

    for (int i = 0; i < numPoints; ++i)
    {
        int start = i * samplesPerPoint;
        int end = juce::jmin (start + samplesPerPoint, numSamples);

        float maxVal = 0.0f;
        for (int s = start; s < end; ++s)
            maxVal = juce::jmax (maxVal, std::abs (data[s]));

        float x = (float) i / (float) (numPoints - 1);
        waveformPath.lineTo (x, midY - maxVal * midY);
    }

    // Mirror back for filled shape
    for (int i = numPoints - 1; i >= 0; --i)
    {
        int start = i * samplesPerPoint;
        int end = juce::jmin (start + samplesPerPoint, numSamples);

        float maxVal = 0.0f;
        for (int s = start; s < end; ++s)
            maxVal = juce::jmax (maxVal, std::abs (data[s]));

        float x = (float) i / (float) (numPoints - 1);
        waveformPath.lineTo (x, midY + maxVal * midY);
    }

    waveformPath.closeSubPath();
}

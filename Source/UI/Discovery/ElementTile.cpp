#include "ElementTile.h"

ElementTile::ElementTile (Element* elem, SampleManager* sm)
    : element (elem), sampleMgr (sm)
{
    if (auto* vis = dynamic_cast<VisualElement*> (elem))
    {
        elemType = ElementType::Visual;
        renderer = VisualRenderer::create (vis->getVisualKind());
        tileColour = vis->getBaseColour();
        visualParams = vis->getParams();
        needsAnimation = true;
    }
    else if (auto* av = dynamic_cast<AudioVisualElement*> (elem))
    {
        elemType = ElementType::AudioVisual;
        renderer = VisualRenderer::create (av->getVisualKind());
        tileColour = av->getColour();
        visualParams = av->getVisualParams();
        needsAnimation = true;
    }
    else
    {
        elemType = ElementType::Sound;
        tileColour = juce::Colour (0xff44dd88);
        needsAnimation = false;
    }

    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setRepaintsOnMouseActivity (false);

    // Match TriggerPad: 30fps for animated tiles
    if (needsAnimation)
        startTimerHz (30);
}

ElementTile::~ElementTile()
{
    stopTimer();
}

void ElementTile::timerCallback()
{
    animPhase += 1.0f / (30.0f * 3.0f); // 3-second cycle at 30fps
    if (animPhase >= 1.0f)
        animPhase -= 1.0f;

    if (selected)
    {
        dashOffset += 0.4f;
        if (dashOffset >= 12.0f)
            dashOffset -= 12.0f;
    }

    repaint();
}

juce::Rectangle<float> ElementTile::getPlayButtonBounds() const
{
    auto bounds = getLocalBounds().toFloat();
    float btnSize = 22.0f;
    return { bounds.getRight() - btnSize - 4.0f, bounds.getY() + 4.0f, btnSize, btnSize };
}

void ElementTile::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background — match TriggerPad style with 8px radius
    auto bgColour = (selected || active) ? juce::Colour (0xff3a3a5c) : juce::Colour (0xff1a1a3a);
    g.setColour (bgColour);
    g.fillRoundedRectangle (bounds, 8.0f);

    // Active state (press-and-hold glow)
    if (active)
    {
        g.setColour (tileColour.withAlpha (0.3f));
        g.fillRoundedRectangle (bounds, 8.0f);

        g.setColour (tileColour);
        g.drawRoundedRectangle (bounds.reduced (1.0f), 8.0f, 3.0f);
    }

    // Border — match TriggerPad: element colour, thicker when selected
    if (selected)
    {
        // Pulsing glow fill
        float pulse = 0.7f + 0.3f * std::sin (dashOffset * 0.52f);
        g.setColour (tileColour.withAlpha (pulse * 0.2f));
        g.fillRoundedRectangle (bounds, 8.0f);

        // Bright coloured border like TriggerPad active state
        g.setColour (tileColour);
        g.drawRoundedRectangle (bounds.reduced (1.0f), 8.0f, 3.0f);

        // Inner white accent
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.drawRoundedRectangle (bounds.reduced (3.5f), 6.0f, 1.0f);
    }
    else
    {
        g.setColour (tileColour.withAlpha (0.4f));
        g.drawRoundedRectangle (bounds.reduced (1.0f), 8.0f, 1.0f);
    }

    // Visual content area — more space like TriggerPad
    auto visualArea = bounds.reduced (8.0f).withTrimmedBottom (20.0f);
    if (renderer)
    {
        renderer->paint (g, visualArea, animPhase, tileColour, visualParams);
    }
    else if (elemType == ElementType::Sound)
    {
        // Build waveform path from sample data
        if (! waveformBuilt)
        {
            waveformPath.clear();
            const juce::AudioBuffer<float>* audioData = nullptr;

            if (sampleMgr != nullptr)
            {
                if (auto* snd = dynamic_cast<SoundElement*> (element))
                {
                    auto* loaded = sampleMgr->loadSample (snd->getSamplePath());
                    if (loaded != nullptr)
                        audioData = &loaded->buffer;
                }
            }

            if (audioData != nullptr && audioData->getNumSamples() > 0)
            {
                const float* samples = audioData->getReadPointer (0);
                int numSamples = audioData->getNumSamples();
                int numPoints = (int) visualArea.getWidth();
                if (numPoints < 2) numPoints = 2;
                int samplesPerPoint = numSamples / numPoints;
                if (samplesPerPoint < 1) samplesPerPoint = 1;

                float midY = visualArea.getCentreY();
                float halfH = visualArea.getHeight() * 0.45f;

                for (int i = 0; i < numPoints; ++i)
                {
                    int start = i * samplesPerPoint;
                    int end = juce::jmin (start + samplesPerPoint, numSamples);
                    float maxVal = 0.0f;
                    for (int s = start; s < end; ++s)
                        maxVal = juce::jmax (maxVal, std::abs (samples[s]));

                    float x = visualArea.getX() + (float) i;
                    float top = midY - maxVal * halfH;

                    if (i == 0)
                    {
                        waveformPath.startNewSubPath (x, top);
                    }
                    else
                    {
                        waveformPath.lineTo (x, top);
                    }

                    // Store bottom points for the return path
                }

                // Return path along the bottom
                for (int i = numPoints - 1; i >= 0; --i)
                {
                    int start = i * samplesPerPoint;
                    int end = juce::jmin (start + samplesPerPoint, numSamples);
                    float maxVal = 0.0f;
                    for (int s = start; s < end; ++s)
                        maxVal = juce::jmax (maxVal, std::abs (samples[s]));

                    float x = visualArea.getX() + (float) i;
                    float bot = midY + maxVal * halfH;
                    waveformPath.lineTo (x, bot);
                }

                waveformPath.closeSubPath();
            }

            waveformBuilt = true;
        }

        if (! waveformPath.isEmpty())
        {
            g.setColour (tileColour.withAlpha (0.6f));
            g.fillPath (waveformPath);
            g.setColour (tileColour);
            g.strokePath (waveformPath, juce::PathStrokeType (0.5f));
        }
        else
        {
            // Fallback if no sample data
            g.setColour (tileColour.withAlpha (0.5f));
            g.setFont (juce::FontOptions (12.0f));
            g.drawText ("SND", visualArea, juce::Justification::centred);
        }
    }

    // Play/preview button
    bool hasAudio = (elemType == ElementType::Sound || elemType == ElementType::AudioVisual);
    if (hasAudio)
    {
        auto playBounds = getPlayButtonBounds();
        g.setColour (juce::Colour (playButtonHovered ? 0xcc44dd88u : 0x8833aa66u));
        g.fillEllipse (playBounds);

        // Play triangle
        juce::Path triangle;
        float cx = playBounds.getCentreX() + 1.0f;
        float cy = playBounds.getCentreY();
        float sz = 6.0f;
        triangle.addTriangle (cx - sz * 0.5f, cy - sz, cx - sz * 0.5f, cy + sz, cx + sz, cy);
        g.setColour (juce::Colours::white);
        g.fillPath (triangle);
    }

    // Name label
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (11.0f));
    auto textArea = bounds.removeFromBottom (18.0f);
    g.drawText (element->getName(), textArea, juce::Justification::centred, true);
}

void ElementTile::mouseDown (const juce::MouseEvent& e)
{
    bool hasAudio = (elemType == ElementType::Sound || elemType == ElementType::AudioVisual);
    if (hasAudio && getPlayButtonBounds().contains (e.position))
    {
        listeners.call ([this] (Listener& l) { l.tilePlayClicked (this); });
        return;
    }

    if (elemType == ElementType::AudioVisual)
    {
        setActive (true);
        listeners.call ([this] (Listener& l) { l.tilePressed (this); });
        return;
    }

    listeners.call ([this] (Listener& l) { l.tileClicked (this); });
}

void ElementTile::mouseUp (const juce::MouseEvent& e)
{
    if (elemType == ElementType::AudioVisual && active)
    {
        setActive (false);
        listeners.call ([this] (Listener& l) { l.tileReleased (this); });
    }

    bool hasAudio = (elemType == ElementType::Sound || elemType == ElementType::AudioVisual);
    bool overPlay = hasAudio && getPlayButtonBounds().contains (e.position);
    if (playButtonHovered != overPlay)
    {
        playButtonHovered = overPlay;
        repaint();
    }
}

void ElementTile::mouseMove (const juce::MouseEvent& e)
{
    bool hasAudio = (elemType == ElementType::Sound || elemType == ElementType::AudioVisual);
    bool overPlay = hasAudio && getPlayButtonBounds().contains (e.position);
    if (playButtonHovered != overPlay)
    {
        playButtonHovered = overPlay;
        repaint();
    }
}

void ElementTile::mouseExit (const juce::MouseEvent&)
{
    if (playButtonHovered)
    {
        playButtonHovered = false;
        repaint();
    }
}

void ElementTile::mouseDrag (const juce::MouseEvent& e)
{
    if (e.getDistanceFromDragStart() > 5)
    {
        if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor (this))
            container->startDragging (element->getId().id, this);
    }
}

void ElementTile::mouseDoubleClick (const juce::MouseEvent&)
{
    listeners.call ([this] (Listener& l) { l.tileDoubleClicked (this); });
}

void ElementTile::setActive (bool a)
{
    if (active != a)
    {
        active = a;
        repaint();
    }
}

void ElementTile::setSelected (bool s)
{
    if (selected != s)
    {
        selected = s;
        dashOffset = 0.0f;

        // Sound tiles: start/stop timer based on selection
        if (! needsAnimation)
        {
            if (s)
                startTimerHz (30);
            else
                stopTimer();
        }

        repaint();
    }
}

#include "VisualCanvas.h"

VisualCanvas::VisualCanvas (ElementLibrary& library)
    : elementLibrary (library)
{
    renderers[VisualElement::VisualKind::Waveform]  = VisualRenderer::create (VisualElement::VisualKind::Waveform);
    renderers[VisualElement::VisualKind::Geometric] = VisualRenderer::create (VisualElement::VisualKind::Geometric);
    renderers[VisualElement::VisualKind::Particle]  = VisualRenderer::create (VisualElement::VisualKind::Particle);
    renderers[VisualElement::VisualKind::Image]     = VisualRenderer::create (VisualElement::VisualKind::Image);
    renderers[VisualElement::VisualKind::Video]     = VisualRenderer::create (VisualElement::VisualKind::Video);

    startTimerHz (30);
}

VisualCanvas::~VisualCanvas()
{
    stopTimer();
}

void VisualCanvas::timerCallback()
{
    bool hasActive = false;

    if (getActiveElements)
    {
        auto ids = getActiveElements();
        hasActive = ! ids.empty();
    }

    if (hasActive)
    {
        animationPhase += 1.0f / (30.0f * 3.0f);
        if (animationPhase >= 1.0f)
            animationPhase -= 1.0f;
        repaint();
    }
}

void VisualCanvas::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0a1a));

    if (! getActiveElements)
    {
        // Draw placeholder text when idle
        g.setColour (juce::Colours::white.withAlpha (0.15f));
        g.setFont (juce::FontOptions (14.0f));
        g.drawText ("Visual Preview", getLocalBounds(), juce::Justification::centred);
        return;
    }

    auto activeIds = getActiveElements();
    if (activeIds.empty())
    {
        g.setColour (juce::Colours::white.withAlpha (0.15f));
        g.setFont (juce::FontOptions (14.0f));
        g.drawText ("Visual Preview", getLocalBounds(), juce::Justification::centred);
        return;
    }

    auto bounds = getLocalBounds().toFloat().reduced (10);

    for (const auto& idStr : activeIds)
    {
        auto* elem = dynamic_cast<AudioVisualElement*> (
            elementLibrary.getElementById ({ idStr }));
        if (elem == nullptr)
            continue;

        auto kind = elem->getVisualKind();
        auto it = renderers.find (kind);
        if (it != renderers.end())
        {
            it->second->paint (g, bounds, animationPhase,
                              elem->getColour(), elem->getVisualParams());
        }
    }
}

void VisualCanvas::setActiveElementsProvider (ActiveElementsProvider provider)
{
    getActiveElements = std::move (provider);
}

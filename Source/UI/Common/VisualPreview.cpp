#include "VisualPreview.h"

VisualPreview::VisualPreview()
{
    startTimerHz (30);
}

VisualPreview::~VisualPreview()
{
    stopTimer();
}

void VisualPreview::setElement (VisualElement* visual)
{
    if (visual != nullptr)
    {
        renderer = VisualRenderer::create (visual->getVisualKind());
        colour = visual->getBaseColour();
        params = visual->getParams();
        hasElement = true;
    }
    else
    {
        clearElement();
    }
}

void VisualPreview::setElement (VisualElement::VisualKind kind, juce::Colour c, const juce::StringPairArray& p)
{
    renderer = VisualRenderer::create (kind);
    colour = c;
    params = p;
    hasElement = true;
}

void VisualPreview::setAVElement (AudioVisualElement* av)
{
    if (av != nullptr)
    {
        renderer = VisualRenderer::create (av->getVisualKind());
        colour = av->getColour();
        params = av->getVisualParams();
        hasElement = true;
    }
    else
    {
        clearElement();
    }
}

void VisualPreview::clearElement()
{
    renderer = nullptr;
    hasElement = false;
}

void VisualPreview::timerCallback()
{
    if (hasElement)
    {
        phase += 1.0f / (30.0f * 3.0f);
        if (phase >= 1.0f)
            phase -= 1.0f;
        repaint();
    }
}

void VisualPreview::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0a1a));

    if (hasElement && renderer)
        renderer->paint (g, getLocalBounds().toFloat().reduced (4), phase, colour, params);
}

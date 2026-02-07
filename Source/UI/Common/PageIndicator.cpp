#include "PageIndicator.h"
#include "TouchConstants.h"

PageIndicator::PageIndicator()
{
    setInterceptsMouseClicks (false, false);
}

void PageIndicator::setNumPages (int n)
{
    numPages = n;
    repaint();
}

void PageIndicator::setActivePage (int page)
{
    if (activePage != page)
    {
        activePage = page;
        repaint();
    }
}

void PageIndicator::paint (juce::Graphics& g)
{
    if (numPages <= 1)
        return;

    float dotDiameter = 10.0f;
    float dotSpacing = 20.0f;
    float totalW = (float) numPages * dotSpacing - (dotSpacing - dotDiameter);
    float startX = ((float) getWidth() - totalW) * 0.5f;
    float cy = (float) getHeight() * 0.5f;

    for (int i = 0; i < numPages; ++i)
    {
        float cx = startX + (float) i * dotSpacing + dotDiameter * 0.5f;
        if (i == activePage)
            g.setColour (juce::Colour (TouchUI::kAccentPink));
        else
            g.setColour (juce::Colours::white.withAlpha (0.3f));

        g.fillEllipse (cx - dotDiameter * 0.5f, cy - dotDiameter * 0.5f,
                        dotDiameter, dotDiameter);
    }
}

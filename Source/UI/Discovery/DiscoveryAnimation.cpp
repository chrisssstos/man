#include "DiscoveryAnimation.h"

DiscoveryAnimation::DiscoveryAnimation()
{
    setInterceptsMouseClicks (false, false);
}

void DiscoveryAnimation::show (const juce::String& name, juce::Colour colour)
{
    displayName = name;
    displayColour = colour;
    progress = 0.0f;
    animating = true;
    setVisible (true);
    startTimerHz (60);
}

void DiscoveryAnimation::timerCallback()
{
    progress += 1.0f / (60.0f * 1.5f); // 1.5 second animation
    if (progress >= 1.0f)
    {
        animating = false;
        setVisible (false);
        stopTimer();
    }
    repaint();
}

void DiscoveryAnimation::paint (juce::Graphics& g)
{
    if (! animating)
        return;

    float alpha = progress < 0.3f ? progress / 0.3f : 1.0f - (progress - 0.3f) / 0.7f;
    float scale = 0.5f + progress * 0.5f;

    auto area = getLocalBounds().toFloat();
    auto centre = area.getCentre();

    // Glow
    g.setColour (displayColour.withAlpha (alpha * 0.3f));
    float glowSize = 200.0f * scale;
    g.fillEllipse (centre.x - glowSize / 2, centre.y - glowSize / 2, glowSize, glowSize);

    // Text
    g.setColour (juce::Colours::white.withAlpha (alpha));
    g.setFont (juce::FontOptions (28.0f * scale));
    g.drawText ("Discovered: " + displayName, area, juce::Justification::centred);
}

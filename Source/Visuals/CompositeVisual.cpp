#include "CompositeVisual.h"

CompositeVisual::CompositeVisual (std::unique_ptr<VisualRenderer> base, juce::Colour glow)
    : baseRenderer (std::move (base)), glowColour (glow)
{
}

void CompositeVisual::paint (juce::Graphics& g, juce::Rectangle<float> bounds,
                              float phase, juce::Colour colour,
                              const juce::StringPairArray& params)
{
    // Draw glow background
    float glowAlpha = 0.15f + 0.1f * std::sin (phase * juce::MathConstants<float>::twoPi * 2.0f);
    g.setColour (glowColour.withAlpha (glowAlpha));
    g.fillRoundedRectangle (bounds, 6.0f);

    // Draw the base visual
    if (baseRenderer)
        baseRenderer->paint (g, bounds.reduced (4), phase, colour, params);
}

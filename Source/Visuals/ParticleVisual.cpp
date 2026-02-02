#include "ParticleVisual.h"

void ParticleVisual::paint (juce::Graphics& g, juce::Rectangle<float> bounds,
                             float phase, juce::Colour colour,
                             const juce::StringPairArray& params)
{
    int count = params.getValue ("count", "20").getIntValue();
    float speed = params.getValue ("speed", "1.0").getFloatValue();
    float size = params.getValue ("size", "4.0").getFloatValue();

    juce::Random rng (42);
    for (int i = 0; i < count; ++i)
    {
        float baseX = rng.nextFloat();
        float baseY = rng.nextFloat();
        float orbitRadius = rng.nextFloat() * 0.1f;
        float orbitAngle = (phase * speed + float (i) * 0.618f)
                           * juce::MathConstants<float>::twoPi;

        float px = bounds.getX() + (baseX + orbitRadius * std::cos (orbitAngle)) * bounds.getWidth();
        float py = bounds.getY() + (baseY + orbitRadius * std::sin (orbitAngle)) * bounds.getHeight();

        g.setColour (colour.withAlpha (0.5f + 0.5f * std::sin (orbitAngle)));
        g.fillEllipse (px - size / 2, py - size / 2, size, size);
    }
}

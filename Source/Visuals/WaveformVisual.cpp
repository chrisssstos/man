#include "WaveformVisual.h"

void WaveformVisual::paint (juce::Graphics& g, juce::Rectangle<float> bounds,
                             float phase, juce::Colour colour,
                             const juce::StringPairArray& params)
{
    float freq = params.getValue ("frequency", "2.0").getFloatValue();
    float amp = params.getValue ("amplitude", "0.5").getFloatValue();
    auto waveType = params.getValue ("type", "sine");

    juce::Path path;
    float w = bounds.getWidth();
    float h = bounds.getHeight();
    float cy = bounds.getCentreY();

    path.startNewSubPath (bounds.getX(), cy);

    for (float x = 0; x < w; x += 1.0f)
    {
        float normX = x / w;
        float t = (normX * freq + phase) * juce::MathConstants<float>::twoPi;
        float y;

        if (waveType == "saw")
            y = cy + (2.0f * std::fmod (t / juce::MathConstants<float>::twoPi, 1.0f) - 1.0f) * (h * 0.4f * amp);
        else if (waveType == "square")
            y = cy + (std::sin (t) >= 0 ? 1.0f : -1.0f) * (h * 0.3f * amp);
        else // sine
            y = cy + std::sin (t) * (h * 0.4f * amp);

        path.lineTo (bounds.getX() + x, y);
    }

    g.setColour (colour);
    g.strokePath (path, juce::PathStrokeType (2.0f));
}

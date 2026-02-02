#include "GeometricVisual.h"

void GeometricVisual::paint (juce::Graphics& g, juce::Rectangle<float> bounds,
                              float phase, juce::Colour colour,
                              const juce::StringPairArray& params)
{
    int sides = params.getValue ("sides", "4").getIntValue();
    float rotSpeed = params.getValue ("rotation_speed", "1.0").getFloatValue();
    bool fill = params.getValue ("fill", "true") == "true";

    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();
    float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.4f;
    float angle = phase * rotSpeed * juce::MathConstants<float>::twoPi;

    juce::Path polygon;
    for (int i = 0; i <= sides; ++i)
    {
        float a = angle + (float (i) / float (sides)) * juce::MathConstants<float>::twoPi;
        float px = cx + radius * std::cos (a);
        float py = cy + radius * std::sin (a);
        if (i == 0)
            polygon.startNewSubPath (px, py);
        else
            polygon.lineTo (px, py);
    }
    polygon.closeSubPath();

    g.setColour (colour);
    if (fill)
        g.fillPath (polygon);
    else
        g.strokePath (polygon, juce::PathStrokeType (2.0f));
}

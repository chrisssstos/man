#include "ImageVisual.h"

void ImageVisual::paint (juce::Graphics& g, juce::Rectangle<float> bounds,
                          float phase, juce::Colour colour,
                          const juce::StringPairArray& params)
{
    auto imagePath = params.getValue ("imagePath", "");

    if (imagePath.isNotEmpty() && imagePath != cachedPath)
    {
        cachedImage = juce::ImageFileFormat::loadFrom (juce::File (imagePath));
        cachedPath = imagePath;
    }

    if (cachedImage.isValid())
    {
        float scale = 1.0f + 0.05f * std::sin (phase * juce::MathConstants<float>::twoPi);
        float w = bounds.getWidth() * scale;
        float h = bounds.getHeight() * scale;
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();

        juce::Rectangle<float> drawBounds (cx - w * 0.5f, cy - h * 0.5f, w, h);

        g.setOpacity (1.0f);
        g.drawImage (cachedImage, drawBounds,
                     juce::RectanglePlacement::centred);

        // Colour tint overlay
        g.setColour (colour.withAlpha (0.25f));
        g.fillRect (drawBounds);
    }
    else
    {
        // Fallback: draw a placeholder
        g.setColour (colour.withAlpha (0.4f));
        g.drawRect (bounds, 2.0f);
        g.setColour (colour);
        g.drawText ("IMG", bounds, juce::Justification::centred);
    }
}

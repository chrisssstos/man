#include "VideoVisual.h"

void VideoVisual::paint (juce::Graphics& g, juce::Rectangle<float> bounds,
                          float phase, juce::Colour colour,
                          const juce::StringPairArray& params)
{
    auto videoPath = params.getValue ("videoPath", "");

    if (videoPath.isNotEmpty() && videoPath != cachedPath)
    {
        decoder.openFile (videoPath);
        cachedPath = videoPath;
    }

    if (decoder.isOpen())
    {
        // Apply trim range from params
        float ts = params.getValue ("trimStart", "0").getFloatValue();
        float te = params.getValue ("trimEnd", "1").getFloatValue();
        if (te <= ts) te = 1.0f;
        float adjustedPhase = ts + phase * (te - ts);
        auto frame = decoder.getFrameAtPhase (adjustedPhase);

        if (frame.isValid())
        {
            g.setOpacity (1.0f);
            g.drawImage (frame, bounds, juce::RectanglePlacement::centred);

            // Colour tint overlay at 15% alpha
            g.setColour (colour.withAlpha (0.15f));
            g.fillRect (bounds);
        }
        else
        {
            g.setColour (colour.withAlpha (0.4f));
            g.drawRect (bounds, 2.0f);
            g.setColour (colour);
            g.drawText ("VID", bounds, juce::Justification::centred);
        }
    }
    else
    {
        g.setColour (colour.withAlpha (0.4f));
        g.drawRect (bounds, 2.0f);
        g.setColour (colour);
        g.drawText ("VID", bounds, juce::Justification::centred);
    }
}

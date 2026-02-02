#pragma once
#include "VisualRenderer.h"
#include "VideoDecoder.h"

class VideoVisual : public VisualRenderer
{
public:
    void paint (juce::Graphics& g, juce::Rectangle<float> bounds,
                float phase, juce::Colour colour,
                const juce::StringPairArray& params) override;

private:
    VideoDecoder decoder;
    juce::String cachedPath;
};

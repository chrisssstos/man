#pragma once
#include <JuceHeader.h>
#include <memory>

class VideoDecoder
{
public:
    VideoDecoder();
    ~VideoDecoder();

    bool openFile (const juce::String& path);
    bool isOpen() const;
    double getDurationSeconds() const;
    juce::Image getFrameAtPhase (float phase);

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};

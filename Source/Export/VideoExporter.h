#pragma once
#include <JuceHeader.h>
#include "Model/Sketch.h"
#include "Model/ElementLibrary.h"

class VideoExporter
{
public:
    static bool exportToVideo (const juce::File& outputDir,
                               const Sketch& sketch,
                               ElementLibrary& library,
                               double bpm,
                               int width = 1280,
                               int height = 720,
                               int fps = 30);
};

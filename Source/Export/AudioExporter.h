#pragma once
#include <JuceHeader.h>
#include "Model/Sketch.h"
#include "Model/ElementLibrary.h"
#include "Audio/SampleManager.h"

class AudioExporter
{
public:
    static bool exportToWav (const juce::File& outputFile,
                             const Sketch& sketch,
                             ElementLibrary& library,
                             SampleManager& sampleManager,
                             double bpm,
                             double sampleRate = 44100.0);
};

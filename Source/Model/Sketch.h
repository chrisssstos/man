#pragma once
#include "Clip.h"
#include <vector>

class Sketch
{
public:
    Sketch() = default;

    void addClip (Clip clip);
    void removeClip (int index);
    void moveClip (int index, double newStartBeat, int newTrack);
    void resizeClip (int index, double newDuration);

    const std::vector<Clip>& getClips() const { return clips; }
    int getNumClips() const { return (int) clips.size(); }
    Clip& getClip (int index) { return clips[(size_t) index]; }
    const Clip& getClip (int index) const { return clips[(size_t) index]; }

    double getTotalLengthBeats() const;
    void clear();

    juce::ValueTree toValueTree() const;
    void loadFromValueTree (const juce::ValueTree& tree);

    void saveToFile (const juce::File& file) const;
    bool loadFromFile (const juce::File& file);

private:
    std::vector<Clip> clips;
};

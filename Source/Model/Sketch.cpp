#include "Sketch.h"

void Sketch::addClip (Clip clip)
{
    clips.push_back (std::move (clip));
}

void Sketch::removeClip (int index)
{
    if (index >= 0 && index < (int) clips.size())
        clips.erase (clips.begin() + index);
}

void Sketch::moveClip (int index, double newStartBeat, int newTrack)
{
    if (index >= 0 && index < (int) clips.size())
    {
        clips[(size_t) index].startBeat = newStartBeat;
        clips[(size_t) index].track = newTrack;
    }
}

void Sketch::resizeClip (int index, double newDuration)
{
    if (index >= 0 && index < (int) clips.size())
        clips[(size_t) index].durationBeats = newDuration;
}

double Sketch::getTotalLengthBeats() const
{
    double maxEnd = 0.0;
    for (const auto& clip : clips)
        maxEnd = std::max (maxEnd, clip.startBeat + clip.durationBeats);
    return maxEnd;
}

void Sketch::clear()
{
    clips.clear();
}

juce::ValueTree Sketch::toValueTree() const
{
    juce::ValueTree tree ("Sketch");
    for (const auto& clip : clips)
    {
        juce::ValueTree child ("Clip");
        child.setProperty ("elementId", clip.elementId.id, nullptr);
        child.setProperty ("startBeat", clip.startBeat, nullptr);
        child.setProperty ("durationBeats", clip.durationBeats, nullptr);
        child.setProperty ("track", clip.track, nullptr);
        tree.addChild (child, -1, nullptr);
    }
    return tree;
}

void Sketch::loadFromValueTree (const juce::ValueTree& tree)
{
    clips.clear();
    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        auto child = tree.getChild (i);
        if (child.hasType ("Clip"))
        {
            Clip c;
            c.elementId = { child.getProperty ("elementId").toString() };
            c.startBeat = child.getProperty ("startBeat");
            c.durationBeats = child.getProperty ("durationBeats");
            c.track = child.getProperty ("track");
            clips.push_back (c);
        }
    }
}

void Sketch::saveToFile (const juce::File& file) const
{
    auto tree = toValueTree();
    auto xml = tree.createXml();
    if (xml != nullptr)
        xml->writeTo (file);
}

bool Sketch::loadFromFile (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml != nullptr)
    {
        auto tree = juce::ValueTree::fromXml (*xml);
        if (tree.isValid())
        {
            loadFromValueTree (tree);
            return true;
        }
    }
    return false;
}

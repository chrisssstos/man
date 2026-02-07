#pragma once
#include <JuceHeader.h>

enum class ElementType { Visual, Sound, AudioVisual };

struct ElementID
{
    juce::String id;
    bool operator== (const ElementID& other) const { return id == other.id; }
    bool operator!= (const ElementID& other) const { return id != other.id; }
    bool operator<  (const ElementID& other) const { return id < other.id; }
    bool isValid() const { return id.isNotEmpty(); }
};

class Element
{
public:
    Element (ElementID id, juce::String name, ElementType type);
    virtual ~Element() = default;

    const ElementID& getId() const      { return elementId; }
    const juce::String& getName() const { return name; }
    ElementType getType() const         { return type; }
    bool isDiscovered() const           { return discovered; }
    void setDiscovered (bool d)         { discovered = d; }

private:
    ElementID elementId;
    juce::String name;
    ElementType type;
    bool discovered = false;
};

class VisualElement : public Element
{
public:
    enum class VisualKind { Waveform, Geometric, Particle, Image, Video };

    VisualElement (ElementID id, juce::String name, VisualKind kind,
                   juce::Colour baseColour, juce::StringPairArray params);

    VisualKind getVisualKind() const                  { return visualKind; }
    juce::Colour getBaseColour() const                { return baseColour; }
    const juce::StringPairArray& getParams() const    { return params; }

    float getTrimStart() const { return trimStart; }
    float getTrimEnd() const   { return trimEnd; }
    void setTrimRange (float start, float end)
    {
        trimStart = juce::jlimit (0.0f, 1.0f, start);
        trimEnd   = juce::jlimit (0.0f, 1.0f, end);
        if (trimEnd <= trimStart) trimEnd = trimStart + 0.001f;
    }

private:
    VisualKind visualKind;
    juce::Colour baseColour;
    juce::StringPairArray params;
    float trimStart = 0.0f;
    float trimEnd   = 1.0f;
};

class SoundElement : public Element
{
public:
    SoundElement (ElementID id, juce::String name, juce::String samplePath);

    const juce::String& getSamplePath() const { return samplePath; }

    float getTrimStart() const { return trimStart; }
    float getTrimEnd() const   { return trimEnd; }
    void setTrimRange (float start, float end)
    {
        trimStart = juce::jlimit (0.0f, 1.0f, start);
        trimEnd   = juce::jlimit (0.0f, 1.0f, end);
        if (trimEnd <= trimStart) trimEnd = trimStart + 0.001f;
    }

private:
    juce::String samplePath;
    float trimStart = 0.0f;
    float trimEnd   = 1.0f;
};

class AudioVisualElement : public Element
{
public:
    AudioVisualElement (ElementID id, juce::String name,
                        ElementID visualSourceId, ElementID soundSourceId,
                        juce::String samplePath,
                        VisualElement::VisualKind visualKind,
                        juce::Colour colour,
                        juce::StringPairArray visualParams);

    const ElementID& getVisualSourceId() const            { return visualSourceId; }
    const ElementID& getSoundSourceId() const             { return soundSourceId; }
    const juce::String& getSamplePath() const             { return samplePath; }
    VisualElement::VisualKind getVisualKind() const       { return visualKind; }
    juce::Colour getColour() const                        { return colour; }
    const juce::StringPairArray& getVisualParams() const  { return visualParams; }

    int getMidiNote() const        { return midiNote; }
    void setMidiNote (int note)    { midiNote = note; }

private:
    ElementID visualSourceId, soundSourceId;
    juce::String samplePath;
    VisualElement::VisualKind visualKind;
    juce::Colour colour;
    juce::StringPairArray visualParams;
    int midiNote = -1;
};

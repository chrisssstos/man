#pragma once
#include "Element.h"
#include <map>
#include <vector>
#include <memory>

class ElementLibrary
{
public:
    ElementLibrary() = default;

    void addElement (std::unique_ptr<Element> element);
    Element* getElementById (const ElementID& id) const;

    std::vector<VisualElement*> getAllVisuals() const;
    std::vector<SoundElement*> getAllSounds() const;
    std::vector<AudioVisualElement*> getAllAudioVisuals() const;
    std::vector<AudioVisualElement*> getAllDiscoveredAV() const;

    // User-loaded elements from browser
    SoundElement* addUserSound (const juce::File& audioFile);
    VisualElement* addUserVisual (const juce::String& name,
                                  VisualElement::VisualKind kind,
                                  juce::Colour colour,
                                  const juce::StringPairArray& params);
    bool hasElementForFile (const juce::File& file) const;

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void elementAdded (const ElementID& id) = 0;
    };
    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    std::map<juce::String, std::unique_ptr<Element>> elements;
    juce::ListenerList<Listener> listeners;
};

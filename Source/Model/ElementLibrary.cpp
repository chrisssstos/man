#include "ElementLibrary.h"

void ElementLibrary::addElement (std::unique_ptr<Element> element)
{
    auto key = element->getId().id;
    elements[key] = std::move (element);
}

Element* ElementLibrary::getElementById (const ElementID& id) const
{
    auto it = elements.find (id.id);
    if (it != elements.end())
        return it->second.get();
    return nullptr;
}

std::vector<VisualElement*> ElementLibrary::getAllVisuals() const
{
    std::vector<VisualElement*> result;
    for (auto& [key, elem] : elements)
        if (auto* v = dynamic_cast<VisualElement*> (elem.get()))
            result.push_back (v);
    return result;
}

std::vector<SoundElement*> ElementLibrary::getAllSounds() const
{
    std::vector<SoundElement*> result;
    for (auto& [key, elem] : elements)
        if (auto* s = dynamic_cast<SoundElement*> (elem.get()))
            result.push_back (s);
    return result;
}

std::vector<AudioVisualElement*> ElementLibrary::getAllAudioVisuals() const
{
    std::vector<AudioVisualElement*> result;
    for (auto& [key, elem] : elements)
        if (auto* av = dynamic_cast<AudioVisualElement*> (elem.get()))
            result.push_back (av);
    return result;
}

std::vector<AudioVisualElement*> ElementLibrary::getAllDiscoveredAV() const
{
    std::vector<AudioVisualElement*> result;
    for (auto& [key, elem] : elements)
        if (auto* av = dynamic_cast<AudioVisualElement*> (elem.get()))
            if (av->isDiscovered())
                result.push_back (av);
    return result;
}

SoundElement* ElementLibrary::addUserSound (const juce::File& audioFile)
{
    juce::String idStr = "snd_user_" + juce::String (audioFile.getFullPathName().hashCode64());
    ElementID eid { idStr };

    if (auto* existing = getElementById (eid))
        return dynamic_cast<SoundElement*> (existing);

    auto elem = std::make_unique<SoundElement> (eid,
        audioFile.getFileNameWithoutExtension(),
        audioFile.getFullPathName());
    elem->setDiscovered (true);
    auto* ptr = elem.get();
    addElement (std::move (elem));
    listeners.call ([&] (Listener& l) { l.elementAdded (eid); });
    return dynamic_cast<SoundElement*> (ptr);
}

VisualElement* ElementLibrary::addUserVisual (const juce::String& name,
                                               VisualElement::VisualKind kind,
                                               juce::Colour colour,
                                               const juce::StringPairArray& params)
{
    juce::String idStr = "vis_user_" + juce::String ((name + juce::String (juce::Time::currentTimeMillis())).hashCode64());
    ElementID eid { idStr };

    auto elem = std::make_unique<VisualElement> (eid, name, kind, colour, params);
    elem->setDiscovered (true);
    auto* ptr = elem.get();
    addElement (std::move (elem));
    listeners.call ([&] (Listener& l) { l.elementAdded (eid); });
    return dynamic_cast<VisualElement*> (ptr);
}

bool ElementLibrary::hasElementForFile (const juce::File& file) const
{
    juce::String idStr = "snd_user_" + juce::String (file.getFullPathName().hashCode64());
    return getElementById (ElementID { idStr }) != nullptr;
}

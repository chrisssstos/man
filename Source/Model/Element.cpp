#include "Element.h"

Element::Element (ElementID id, juce::String name, ElementType type)
    : elementId (std::move (id)), name (std::move (name)), type (type)
{
}

VisualElement::VisualElement (ElementID id, juce::String name, VisualKind kind,
                               juce::Colour baseColour, juce::StringPairArray params)
    : Element (std::move (id), std::move (name), ElementType::Visual),
      visualKind (kind), baseColour (baseColour), params (std::move (params))
{
    setDiscovered (true); // Base visuals are always available
}

SoundElement::SoundElement (ElementID id, juce::String name, juce::String samplePath)
    : Element (std::move (id), std::move (name), ElementType::Sound),
      samplePath (std::move (samplePath))
{
    setDiscovered (true); // Base sounds are always available
}

AudioVisualElement::AudioVisualElement (ElementID id, juce::String name,
                                        ElementID visualSourceId, ElementID soundSourceId,
                                        juce::String samplePath,
                                        VisualElement::VisualKind visualKind,
                                        juce::Colour colour,
                                        juce::StringPairArray visualParams)
    : Element (std::move (id), std::move (name), ElementType::AudioVisual),
      visualSourceId (std::move (visualSourceId)),
      soundSourceId (std::move (soundSourceId)),
      samplePath (std::move (samplePath)),
      visualKind (visualKind),
      colour (colour),
      visualParams (std::move (visualParams))
{
    // AV elements start undiscovered
}

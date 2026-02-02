#pragma once
#include <JuceHeader.h>
#include "Visuals/VisualRenderer.h"
#include "Model/Element.h"

class VisualPreview : public juce::Component, public juce::Timer
{
public:
    VisualPreview();
    ~VisualPreview() override;

    void setElement (VisualElement* visual);
    void setElement (VisualElement::VisualKind kind, juce::Colour c, const juce::StringPairArray& p);
    void setAVElement (AudioVisualElement* av);
    void clearElement();

    void paint (juce::Graphics& g) override;
    void timerCallback() override;

private:
    std::unique_ptr<VisualRenderer> renderer;
    juce::Colour colour;
    juce::StringPairArray params;
    float phase = 0.0f;
    bool hasElement = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VisualPreview)
};

#pragma once
#include <JuceHeader.h>
#include "VisualRenderer.h"
#include "Model/ElementLibrary.h"
#include <set>
#include <functional>

class VisualCanvas : public juce::Component, public juce::Timer
{
public:
    VisualCanvas (ElementLibrary& library);
    ~VisualCanvas() override;

    void paint (juce::Graphics& g) override;
    void timerCallback() override;

    using ActiveElementsProvider = std::function<std::set<juce::String>()>;
    void setActiveElementsProvider (ActiveElementsProvider provider);

private:
    ElementLibrary& elementLibrary;
    ActiveElementsProvider getActiveElements;
    float animationPhase = 0.0f;

    std::map<VisualElement::VisualKind, std::unique_ptr<VisualRenderer>> renderers;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VisualCanvas)
};

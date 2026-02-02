#pragma once
#include <JuceHeader.h>
#include "Model/Element.h"

class VisualParamEditor : public juce::Component,
                          public juce::Slider::Listener,
                          public juce::ComboBox::Listener
{
public:
    VisualParamEditor();

    void setKind (VisualElement::VisualKind kind, juce::StringPairArray& params, juce::Colour& colour);
    void resized() override;

    void sliderValueChanged (juce::Slider* slider) override;
    void comboBoxChanged (juce::ComboBox* combo) override;

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void paramChanged() = 0;
    };
    void addListener (Listener* l) { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::Label> labels;
    std::unique_ptr<juce::ComboBox> typeCombo;
    std::unique_ptr<juce::Label> typeLabel;

    juce::StringPairArray* currentParams = nullptr;
    juce::Colour* currentColour = nullptr;
    VisualElement::VisualKind currentKind = VisualElement::VisualKind::Waveform;

    juce::ListenerList<Listener> listeners;

    struct ParamDef
    {
        juce::String key;
        juce::String label;
        double min, max, defaultVal;
    };
    std::vector<ParamDef> getParamDefs (VisualElement::VisualKind kind);
    void rebuildControls();
    void notifyChange();
};

#include "VisualParamEditor.h"

VisualParamEditor::VisualParamEditor()
{
}

std::vector<VisualParamEditor::ParamDef> VisualParamEditor::getParamDefs (VisualElement::VisualKind kind)
{
    switch (kind)
    {
        case VisualElement::VisualKind::Waveform:
            return {
                { "frequency", "Frequency", 0.1, 10.0, 2.0 },
                { "amplitude", "Amplitude", 0.0, 1.0, 0.5 }
            };
        case VisualElement::VisualKind::Geometric:
            return {
                { "sides", "Sides", 3.0, 12.0, 4.0 },
                { "rotation_speed", "Rotation", 0.0, 5.0, 1.0 }
            };
        case VisualElement::VisualKind::Particle:
            return {
                { "count", "Count", 5.0, 100.0, 20.0 },
                { "speed", "Speed", 0.1, 5.0, 1.0 },
                { "size", "Size", 1.0, 10.0, 4.0 }
            };
        case VisualElement::VisualKind::Image:
            return {};
    }
    return {};
}

void VisualParamEditor::setKind (VisualElement::VisualKind kind, juce::StringPairArray& params, juce::Colour& colour)
{
    currentKind = kind;
    currentParams = &params;
    currentColour = &colour;
    rebuildControls();
}

void VisualParamEditor::rebuildControls()
{
    sliders.clear();
    labels.clear();
    typeCombo.reset();
    typeLabel.reset();

    if (currentParams == nullptr)
        return;

    // Type combo for Waveform
    if (currentKind == VisualElement::VisualKind::Waveform)
    {
        typeLabel = std::make_unique<juce::Label> ("", "Type");
        typeLabel->setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.8f));
        typeLabel->setFont (12.0f);
        addAndMakeVisible (*typeLabel);

        typeCombo = std::make_unique<juce::ComboBox>();
        typeCombo->addItem ("sine", 1);
        typeCombo->addItem ("saw", 2);
        typeCombo->addItem ("square", 3);
        auto currentType = currentParams->getValue ("type", "sine");
        if (currentType == "sine") typeCombo->setSelectedId (1, juce::dontSendNotification);
        else if (currentType == "saw") typeCombo->setSelectedId (2, juce::dontSendNotification);
        else typeCombo->setSelectedId (3, juce::dontSendNotification);
        typeCombo->addListener (this);
        addAndMakeVisible (*typeCombo);
    }

    // Geometric fill toggle as a combo
    if (currentKind == VisualElement::VisualKind::Geometric)
    {
        typeLabel = std::make_unique<juce::Label> ("", "Fill");
        typeLabel->setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.8f));
        typeLabel->setFont (12.0f);
        addAndMakeVisible (*typeLabel);

        typeCombo = std::make_unique<juce::ComboBox>();
        typeCombo->addItem ("Filled", 1);
        typeCombo->addItem ("Outline", 2);
        auto fill = currentParams->getValue ("fill", "true");
        typeCombo->setSelectedId (fill == "true" ? 1 : 2, juce::dontSendNotification);
        typeCombo->addListener (this);
        addAndMakeVisible (*typeCombo);
    }

    auto defs = getParamDefs (currentKind);
    for (const auto& def : defs)
    {
        auto* label = labels.add (new juce::Label());
        label->setText (def.label, juce::dontSendNotification);
        label->setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.8f));
        label->setFont (12.0f);
        addAndMakeVisible (label);

        auto* slider = sliders.add (new juce::Slider());
        slider->setRange (def.min, def.max, (def.key == "sides" || def.key == "count") ? 1.0 : 0.01);
        slider->setSliderStyle (juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle (juce::Slider::TextBoxRight, false, 45, 20);
        slider->setColour (juce::Slider::thumbColourId, juce::Colour (0xffe94560));
        slider->setColour (juce::Slider::trackColourId, juce::Colour (0xff2a2a5a));
        slider->setName (def.key);

        double val = currentParams->getValue (def.key, juce::String (def.defaultVal)).getDoubleValue();
        slider->setValue (val, juce::dontSendNotification);
        slider->addListener (this);
        addAndMakeVisible (slider);
    }

    resized();
}

void VisualParamEditor::resized()
{
    auto area = getLocalBounds().reduced (4);
    int rowH = 28;

    if (typeCombo != nullptr && typeLabel != nullptr)
    {
        auto row = area.removeFromTop (rowH);
        typeLabel->setBounds (row.removeFromLeft (70));
        typeCombo->setBounds (row);
        area.removeFromTop (2);
    }

    for (int i = 0; i < sliders.size(); ++i)
    {
        auto row = area.removeFromTop (rowH);
        labels[i]->setBounds (row.removeFromLeft (70));
        sliders[i]->setBounds (row);
        area.removeFromTop (2);
    }
}

void VisualParamEditor::sliderValueChanged (juce::Slider* slider)
{
    if (currentParams == nullptr)
        return;

    currentParams->set (slider->getName(), juce::String (slider->getValue()));
    notifyChange();
}

void VisualParamEditor::comboBoxChanged (juce::ComboBox*)
{
    if (currentParams == nullptr || typeCombo == nullptr)
        return;

    if (currentKind == VisualElement::VisualKind::Waveform)
    {
        juce::String types[] = { "", "sine", "saw", "square" };
        currentParams->set ("type", types[typeCombo->getSelectedId()]);
    }
    else if (currentKind == VisualElement::VisualKind::Geometric)
    {
        currentParams->set ("fill", typeCombo->getSelectedId() == 1 ? "true" : "false");
    }

    notifyChange();
}

void VisualParamEditor::notifyChange()
{
    listeners.call ([] (Listener& l) { l.paramChanged(); });
}

#include "TriggerGrid.h"

const juce::StringArray& TriggerGrid::getKeyLabels()
{
    static juce::StringArray labels { "Q", "W", "E", "R", "T",
                                       "A", "S", "D", "F", "G",
                                       "Z", "X", "C", "V", "B" };
    return labels;
}

const std::vector<int>& TriggerGrid::getKeyCodes()
{
    static std::vector<int> codes {
        'Q', 'W', 'E', 'R', 'T',
        'A', 'S', 'D', 'F', 'G',
        'Z', 'X', 'C', 'V', 'B'
    };
    return codes;
}

TriggerGrid::TriggerGrid (ElementLibrary& lib)
    : library (lib)
{
    rebuild();
}

void TriggerGrid::rebuild()
{
    pads.clear();

    auto discovered = library.getAllDiscoveredAV();
    auto& labels = getKeyLabels();

    for (int i = 0; i < (int) discovered.size() && i < labels.size(); ++i)
    {
        auto* pad = pads.add (new TriggerPad (discovered[(size_t) i], labels[i]));
        addAndMakeVisible (pad);
        pad->addListener (this);
    }

    resized();
}

void TriggerGrid::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (TouchUI::kBgDeep));
}

void TriggerGrid::resized()
{
    int gap = TouchUI::kTileGap;
    auto area = getLocalBounds().reduced (gap);

    int minPadSize = TouchUI::kLargeTouchTarget; // 80px min
    int availW = area.getWidth();

    // Responsive columns: ensure pads never shrink below 80px
    int cols = juce::jmax (1, (availW + gap) / (minPadSize + gap));
    cols = juce::jmin (cols, 5); // Max 5 columns

    int rows = ((int) pads.size() + cols - 1) / cols;
    int padW = (availW - (cols - 1) * gap) / cols;
    int padH = rows > 0 ? (area.getHeight() - (rows - 1) * gap) / rows : 0;

    // Enforce minimum size
    padW = juce::jmax (minPadSize, padW);
    padH = juce::jmax (minPadSize, padH);

    for (int i = 0; i < pads.size(); ++i)
    {
        int col = i % cols;
        int row = i / cols;
        pads[i]->setBounds (area.getX() + col * (padW + gap),
                            area.getY() + row * (padH + gap),
                            padW, padH);
    }
}

void TriggerGrid::padPressed (TriggerPad* pad)
{
    if (pad->getElement())
        listeners.call ([&] (Listener& l) { l.padTriggered (pad->getElement()->getId()); });
}

void TriggerGrid::padReleased (TriggerPad* pad)
{
    if (pad->getElement())
        listeners.call ([&] (Listener& l) { l.padReleased (pad->getElement()->getId()); });
}

TriggerPad* TriggerGrid::getPadForKey (int keyCode) const
{
    auto& codes = getKeyCodes();
    for (int i = 0; i < (int) codes.size() && i < pads.size(); ++i)
    {
        if (codes[(size_t) i] == keyCode)
            return pads[i];
    }
    return nullptr;
}

#include "LiveView.h"

LiveView::LiveView (ElementLibrary& lib, AudioEngine& eng, Sketch& sk)
    : elementLibrary (lib), audioEngine (eng), sketch (sk),
      visualCanvas (lib), triggerGrid (lib)
{
    addAndMakeVisible (visualCanvas);
    addAndMakeVisible (triggerGrid);
    addAndMakeVisible (recordButton);
    addAndMakeVisible (stopButton);

    triggerGrid.addListener (this);

    recordButton.setColour (juce::TextButton::buttonColourId, juce::Colour (TouchUI::kAccentPink));
    stopButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xffe74c3c));

    recordButton.onClick = [this]
    {
        audioEngine.startRecording();
        recordButton.setEnabled (false);
        stopButton.setEnabled (true);
    };

    stopButton.onClick = [this]
    {
        audioEngine.stopRecording();
        recordButton.setEnabled (true);
        stopButton.setEnabled (false);

        auto events = audioEngine.getRecordedEvents();
        int track = 0;
        for (const auto& event : events)
        {
            Clip clip;
            clip.elementId = event.elementId;
            clip.startBeat = event.beatPosition;
            clip.durationBeats = event.durationBeats;
            clip.track = track % 4;
            sketch.addClip (clip);
            track++;
        }
        audioEngine.clearRecordedEvents();
    };

    stopButton.setEnabled (false);

    visualCanvas.setActiveElementsProvider ([this]() { return audioEngine.getActiveElementIds(); });

    setWantsKeyboardFocus (true);
    addKeyListener (this);
}

LiveView::~LiveView()
{
    removeKeyListener (this);
}

void LiveView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (TouchUI::kBgDeep));
}

void LiveView::resized()
{
    auto area = getLocalBounds();

    // Transport bar: 64px with large buttons
    auto topBar = area.removeFromTop (TouchUI::kTransportHeight);
    int btnW = 120;
    int btnH = TouchUI::kMinTouchTarget;
    recordButton.setBounds (topBar.removeFromLeft (btnW + 8).withSizeKeepingCentre (btnW, btnH));
    stopButton.setBounds (topBar.removeFromLeft (btnW + 8).withSizeKeepingCentre (btnW, btnH));

    // VisualCanvas: 55% of remaining
    int canvasH = area.getHeight() * 55 / 100;
    visualCanvas.setBounds (area.removeFromTop (canvasH));

    // TriggerGrid: remaining
    triggerGrid.setBounds (area);
}

void LiveView::padTriggered (const ElementID& id)
{
    audioEngine.triggerElement (id);
}

void LiveView::padReleased (const ElementID& id)
{
    audioEngine.stopElement (id);
}

bool LiveView::keyPressed (const juce::KeyPress& key, juce::Component*)
{
    int code = juce::CharacterFunctions::toUpperCase ((juce::juce_wchar) key.getKeyCode());
    if (heldKeys.count (code) > 0)
        return true;

    auto* pad = triggerGrid.getPadForKey (code);
    if (pad != nullptr && pad->getElement() != nullptr)
    {
        heldKeys.insert (code);
        pad->setActive (true);
        audioEngine.triggerElement (pad->getElement()->getId());
        return true;
    }
    return false;
}

bool LiveView::keyStateChanged (bool, juce::Component*)
{
    std::vector<int> toRemove;
    for (int code : heldKeys)
    {
        if (! juce::KeyPress::isKeyCurrentlyDown (code))
        {
            toRemove.push_back (code);
            auto* pad = triggerGrid.getPadForKey (code);
            if (pad != nullptr && pad->getElement() != nullptr)
            {
                pad->setActive (false);
                audioEngine.stopElement (pad->getElement()->getId());
            }
        }
    }
    for (int code : toRemove)
        heldKeys.erase (code);

    return ! toRemove.empty();
}

void LiveView::rebuild()
{
    triggerGrid.rebuild();
}

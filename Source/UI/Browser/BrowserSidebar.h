#pragma once
#include <JuceHeader.h>
#include "SoundBrowserPanel.h"
#include "VisualBrowserPanel.h"
#include "Model/ElementLibrary.h"
#include "Audio/AudioEngine.h"

class BrowserSidebar : public juce::Component
{
public:
    BrowserSidebar (ElementLibrary& library, AudioEngine& audioEngine);

    void resized() override;
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

    bool isCollapsed() const { return collapsed; }
    void setCollapsed (bool c);
    int getDesiredWidth() const;

private:
    ElementLibrary& elementLibrary;
    AudioEngine& audioEngine;

    bool collapsed = false;
    int panelWidth = 250;
    static constexpr int collapsedWidth = 22;
    static constexpr int resizeEdge = 5;

    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
    std::unique_ptr<SoundBrowserPanel> soundBrowser;
    std::unique_ptr<VisualBrowserPanel> visualBrowser;

    juce::TextButton collapseButton;

    bool resizing = false;
    int dragStartWidth = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BrowserSidebar)
};

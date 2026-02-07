#include "BrowserSidebar.h"

BrowserSidebar::BrowserSidebar (ElementLibrary& lib, AudioEngine& eng)
    : elementLibrary (lib), audioEngine (eng)
{
    soundBrowser = std::make_unique<SoundBrowserPanel> (elementLibrary, audioEngine);
    visualBrowser = std::make_unique<VisualBrowserPanel> (elementLibrary);
    youtubeBrowser = std::make_unique<YouTubeBrowserPanel> (elementLibrary);

    tabs.addTab ("Sounds", juce::Colour (0xff1a1a3a), soundBrowser.get(), false);
    tabs.addTab ("Visuals", juce::Colour (0xff1a1a3a), visualBrowser.get(), false);
    tabs.addTab ("YouTube", juce::Colour (0xff1a1a3a), youtubeBrowser.get(), false);
    tabs.setTabBarDepth (28);
    addAndMakeVisible (tabs);

    collapseButton.setButtonText ("<");
    collapseButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1a1a3a));
    collapseButton.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    collapseButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible (collapseButton);

    collapseButton.onClick = [this]
    {
        setCollapsed (! collapsed);
        if (auto* parent = getParentComponent())
            parent->resized();
    };
}

void BrowserSidebar::setCollapsed (bool c)
{
    collapsed = c;
    tabs.setVisible (! collapsed);
    collapseButton.setButtonText (collapsed ? ">" : "<");
}

int BrowserSidebar::getDesiredWidth() const
{
    return collapsed ? collapsedWidth : panelWidth;
}

void BrowserSidebar::resized()
{
    auto area = getLocalBounds();

    if (collapsed)
    {
        collapseButton.setBounds (area);
    }
    else
    {
        collapseButton.setBounds (area.removeFromTop (28).removeFromRight (22));
        tabs.setBounds (area);
    }
}

void BrowserSidebar::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0e0e24));

    if (! collapsed)
    {
        // Right edge resize indicator
        g.setColour (juce::Colour (0xff2a2a5a));
        g.fillRect (getWidth() - resizeEdge, 0, resizeEdge, getHeight());
    }
}

void BrowserSidebar::mouseDown (const juce::MouseEvent& e)
{
    if (! collapsed && e.x >= getWidth() - resizeEdge)
    {
        resizing = true;
        dragStartWidth = panelWidth;
    }
}

void BrowserSidebar::mouseDrag (const juce::MouseEvent& e)
{
    if (resizing)
    {
        panelWidth = juce::jlimit (180, 450, dragStartWidth + e.getDistanceFromDragStartX());
        if (auto* parent = getParentComponent())
            parent->resized();
    }
}

void BrowserSidebar::mouseUp (const juce::MouseEvent&)
{
    resizing = false;
}

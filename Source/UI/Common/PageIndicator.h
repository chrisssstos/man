#pragma once
#include <JuceHeader.h>

class PageIndicator : public juce::Component
{
public:
    PageIndicator();

    void setNumPages (int n);
    void setActivePage (int page);

    void paint (juce::Graphics& g) override;

private:
    int numPages = 0;
    int activePage = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PageIndicator)
};

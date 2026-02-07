#pragma once
#include <JuceHeader.h>
#include "PageIndicator.h"
#include "TouchConstants.h"

class SwipeContainer : public juce::Component
{
public:
    SwipeContainer();

    void addPage (juce::Component* page);
    void setCurrentPage (int index, bool animate = true);
    int getCurrentPage() const { return currentPage; }
    int getNumPages() const { return (int) pages.size(); }

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void swipePageChanged (int newPage) = 0;
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    std::vector<juce::Component*> pages;
    juce::Component pageContainer;
    PageIndicator pageIndicator;

    int currentPage = 0;
    bool swiping = false;
    float swipeStartX = 0.0f;
    float swipeStartY = 0.0f;
    float swipeDeltaX = 0.0f;
    bool swipeDecided = false;
    bool swipeIsHorizontal = false;

    juce::ListenerList<Listener> listeners;

    void animateToPage (int page);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SwipeContainer)
};

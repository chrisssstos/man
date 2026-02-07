#include "SwipeContainer.h"

SwipeContainer::SwipeContainer()
{
    addAndMakeVisible (pageContainer);
    addAndMakeVisible (pageIndicator);
    pageIndicator.setNumPages (0);
}

void SwipeContainer::addPage (juce::Component* page)
{
    pages.push_back (page);
    pageContainer.addAndMakeVisible (page);
    pageIndicator.setNumPages ((int) pages.size());

    if (pages.size() == 1)
    {
        currentPage = 0;
        pageIndicator.setActivePage (0);
    }

    resized();
}

void SwipeContainer::setCurrentPage (int index, bool animate)
{
    index = juce::jlimit (0, juce::jmax (0, (int) pages.size() - 1), index);
    if (index == currentPage)
        return;

    currentPage = index;
    pageIndicator.setActivePage (currentPage);

    if (animate)
        animateToPage (currentPage);
    else
        resized();
}

void SwipeContainer::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (TouchUI::kBgDeep));
}

void SwipeContainer::resized()
{
    auto area = getLocalBounds();
    auto indicatorArea = area.removeFromBottom (32);
    pageIndicator.setBounds (indicatorArea);

    int pageW = area.getWidth();
    int pageH = area.getHeight();
    int totalW = pageW * (int) pages.size();

    pageContainer.setBounds (area.getX() - currentPage * pageW, area.getY(), totalW, pageH);

    for (int i = 0; i < (int) pages.size(); ++i)
        pages[(size_t) i]->setBounds (i * pageW, 0, pageW, pageH);
}

void SwipeContainer::mouseDown (const juce::MouseEvent& e)
{
    swiping = true;
    swipeStartX = e.position.x;
    swipeStartY = e.position.y;
    swipeDeltaX = 0.0f;
    swipeDecided = false;
    swipeIsHorizontal = false;
}

void SwipeContainer::mouseDrag (const juce::MouseEvent& e)
{
    if (! swiping)
        return;

    float dx = e.position.x - swipeStartX;
    float dy = e.position.y - swipeStartY;

    if (! swipeDecided)
    {
        if (std::abs (dx) > 10.0f || std::abs (dy) > 10.0f)
        {
            swipeDecided = true;
            swipeIsHorizontal = (std::abs (dx) > std::abs (dy));
        }
    }

    if (swipeDecided && swipeIsHorizontal)
    {
        swipeDeltaX = dx;
        auto area = getLocalBounds();
        area.removeFromBottom (32);
        int pageW = area.getWidth();
        int targetX = area.getX() - currentPage * pageW + (int) swipeDeltaX;
        pageContainer.setTopLeftPosition (targetX, area.getY());
    }
}

void SwipeContainer::mouseUp (const juce::MouseEvent&)
{
    if (! swiping)
        return;

    swiping = false;

    if (! swipeDecided || ! swipeIsHorizontal)
        return;

    int newPage = currentPage;
    if (swipeDeltaX < -(float) TouchUI::kSwipeThreshold && currentPage < (int) pages.size() - 1)
        newPage = currentPage + 1;
    else if (swipeDeltaX > (float) TouchUI::kSwipeThreshold && currentPage > 0)
        newPage = currentPage - 1;

    currentPage = newPage;
    pageIndicator.setActivePage (currentPage);
    animateToPage (currentPage);

    listeners.call ([newPage] (Listener& l) { l.swipePageChanged (newPage); });
}

void SwipeContainer::animateToPage (int page)
{
    auto area = getLocalBounds();
    area.removeFromBottom (32);
    int pageW = area.getWidth();
    int totalW = pageW * (int) pages.size();
    int targetX = area.getX() - page * pageW;

    auto& animator = juce::Desktop::getInstance().getAnimator();
    animator.animateComponent (&pageContainer,
                               { targetX, area.getY(), totalW, area.getHeight() },
                               1.0f, 250, false, 0.5, 0.0);
}

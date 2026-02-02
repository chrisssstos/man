#pragma once
#include "Element.h"
#include <set>

class DiscoveryLog
{
public:
    DiscoveryLog() = default;

    bool isDiscovered (const ElementID& id) const;
    void markDiscovered (const ElementID& id);
    const std::set<ElementID>& getAllDiscovered() const { return discoveredIds; }
    int getDiscoveryCount() const { return (int) discoveredIds.size(); }

    juce::ValueTree toValueTree() const;
    void loadFromValueTree (const juce::ValueTree& tree);

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void onNewDiscovery (const ElementID& id) = 0;
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    std::set<ElementID> discoveredIds;
    juce::ListenerList<Listener> listeners;
};

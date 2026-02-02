#include "DiscoveryLog.h"

bool DiscoveryLog::isDiscovered (const ElementID& id) const
{
    return discoveredIds.count (id) > 0;
}

void DiscoveryLog::markDiscovered (const ElementID& id)
{
    if (discoveredIds.insert (id).second)
        listeners.call ([&] (Listener& l) { l.onNewDiscovery (id); });
}

juce::ValueTree DiscoveryLog::toValueTree() const
{
    juce::ValueTree tree ("Discoveries");
    for (const auto& id : discoveredIds)
    {
        juce::ValueTree child ("Discovered");
        child.setProperty ("id", id.id, nullptr);
        tree.addChild (child, -1, nullptr);
    }
    return tree;
}

void DiscoveryLog::loadFromValueTree (const juce::ValueTree& tree)
{
    discoveredIds.clear();
    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        auto child = tree.getChild (i);
        if (child.hasType ("Discovered"))
            discoveredIds.insert ({ child.getProperty ("id").toString() });
    }
}

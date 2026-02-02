#pragma once
#include "Element.h"

struct Clip
{
    ElementID elementId;
    double startBeat = 0.0;
    double durationBeats = 1.0;
    int track = 0;
};

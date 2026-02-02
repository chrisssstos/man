#pragma once
#include <JuceHeader.h>

namespace BrowserDrag
{
    inline juce::String encodeSoundFile (const juce::File& file)
    {
        return "browser_sound:" + file.getFullPathName();
    }

    inline bool isSoundFileDrag (const juce::String& payload)
    {
        return payload.startsWith ("browser_sound:");
    }

    inline juce::File decodeSoundFile (const juce::String& payload)
    {
        return juce::File (payload.fromFirstOccurrenceOf ("browser_sound:", false, false));
    }

    inline juce::String encodeVisualPreset (const juce::String& presetId)
    {
        return "browser_visual:" + presetId;
    }

    inline bool isVisualPresetDrag (const juce::String& payload)
    {
        return payload.startsWith ("browser_visual:");
    }

    inline juce::String decodeVisualPreset (const juce::String& payload)
    {
        return payload.fromFirstOccurrenceOf ("browser_visual:", false, false);
    }
}

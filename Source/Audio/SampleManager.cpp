#include "SampleManager.h"

SampleManager::SampleManager()
{
    formatManager.registerBasicFormats();
}

const SampleManager::LoadedSample* SampleManager::loadSample (const juce::String& path)
{
    juce::ScopedLock sl (lock);

    auto it = cache.find (path);
    if (it != cache.end())
        return it->second.get();

    // Try absolute path first, then relative to base sample directory
    juce::File file (path);
    if (! file.existsAsFile() && baseSampleDir.exists())
        file = baseSampleDir.getChildFile (path);

    if (! file.existsAsFile())
        return nullptr;

    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    if (reader == nullptr)
        return nullptr;

    auto sample = std::make_unique<LoadedSample>();
    sample->sampleRate = reader->sampleRate;
    sample->buffer.setSize ((int) reader->numChannels, (int) reader->lengthInSamples);
    reader->read (&sample->buffer, 0, (int) reader->lengthInSamples, 0, true, true);

    auto* ptr = sample.get();
    cache[path] = std::move (sample);
    return ptr;
}

void SampleManager::loadDirectory (const juce::File& directory)
{
    if (! directory.isDirectory())
        return;

    for (const auto& file : directory.findChildFiles (juce::File::findFiles, false, "*.wav;*.mp3;*.aif;*.aiff"))
    {
        loadSample (file.getFullPathName());
    }
}

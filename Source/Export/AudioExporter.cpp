#include "AudioExporter.h"
#include "Audio/SampleSound.h"
#include "Audio/SampleVoice.h"

bool AudioExporter::exportToWav (const juce::File& outputFile,
                                  const Sketch& sketch,
                                  ElementLibrary& library,
                                  SampleManager& sampleManager,
                                  double bpm,
                                  double sampleRate)
{
    double totalBeats = sketch.getTotalLengthBeats();
    if (totalBeats <= 0.0)
        return false;

    double secondsPerBeat = 60.0 / bpm;
    double totalSeconds = totalBeats * secondsPerBeat;
    int totalSamples = (int) (totalSeconds * sampleRate);

    // Set up offline synthesiser
    juce::Synthesiser synth;
    synth.setCurrentPlaybackSampleRate (sampleRate);
    for (int i = 0; i < 16; ++i)
        synth.addVoice (new SampleVoice());

    int noteNumber = 36;
    std::map<juce::String, int> noteMap;
    for (auto* av : library.getAllDiscoveredAV())
    {
        auto* loaded = sampleManager.loadSample (av->getSamplePath());
        if (loaded != nullptr)
        {
            juce::AudioBuffer<float> buf;
            buf.makeCopyOf (loaded->buffer);
            synth.addSound (new SampleSound (av->getId(), std::move (buf),
                                             loaded->sampleRate, noteNumber));
            noteMap[av->getId().id] = noteNumber;
            noteNumber++;
        }
    }

    // Render
    juce::AudioBuffer<float> output (2, totalSamples);
    output.clear();

    int blockSize = 512;
    double beatsPerSample = bpm / (60.0 * sampleRate);

    std::set<int> activeClips;

    for (int pos = 0; pos < totalSamples; pos += blockSize)
    {
        int samplesThisBlock = juce::jmin (blockSize, totalSamples - pos);
        double beatStart = pos * beatsPerSample;
        double beatEnd = (pos + samplesThisBlock) * beatsPerSample;

        juce::MidiBuffer midi;

        for (int ci = 0; ci < sketch.getNumClips(); ++ci)
        {
            const auto& clip = sketch.getClip (ci);
            double clipEnd = clip.startBeat + clip.durationBeats;
            bool wasActive = activeClips.count (ci) > 0;
            bool shouldStart = (clip.startBeat >= beatStart && clip.startBeat < beatEnd);
            bool shouldEnd = (clipEnd >= beatStart && clipEnd < beatEnd);

            auto it = noteMap.find (clip.elementId.id);
            if (it == noteMap.end())
                continue;

            int note = it->second;
            if (shouldStart && ! wasActive)
            {
                int sampleOffset = (int) ((clip.startBeat - beatStart) / beatsPerSample);
                midi.addEvent (juce::MidiMessage::noteOn (1, note, 1.0f),
                              juce::jmax (0, sampleOffset));
                activeClips.insert (ci);
            }
            if (shouldEnd && wasActive)
            {
                int sampleOffset = (int) ((clipEnd - beatStart) / beatsPerSample);
                midi.addEvent (juce::MidiMessage::noteOff (1, note),
                              juce::jmin (samplesThisBlock - 1, juce::jmax (0, sampleOffset)));
                activeClips.erase (ci);
            }
        }

        juce::AudioBuffer<float> block (output.getArrayOfWritePointers(), 2, pos, samplesThisBlock);
        synth.renderNextBlock (block, midi, 0, samplesThisBlock);
    }

    // Write WAV
    outputFile.deleteFile();
    std::unique_ptr<juce::FileOutputStream> stream (outputFile.createOutputStream());
    if (stream == nullptr)
        return false;

    juce::WavAudioFormat wav;
    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4996)
    std::unique_ptr<juce::AudioFormatWriter> writer (
        wav.createWriterFor (stream.get(), sampleRate, 2, 16, {}, 0));
    JUCE_END_IGNORE_WARNINGS_MSVC
    if (writer == nullptr)
        return false;

    stream.release(); // writer owns it now
    writer->writeFromAudioSampleBuffer (output, 0, totalSamples);
    return true;
}

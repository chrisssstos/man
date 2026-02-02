#include "AudioEngine.h"

AudioEngine::AudioEngine (ElementLibrary& library, SampleManager& sm)
    : elementLibrary (library), sampleManager (sm)
{
    previewFormatManager.registerBasicFormats();
    initialiseAudio();
}

AudioEngine::~AudioEngine()
{
    deviceManager.removeAudioCallback (this);
}

void AudioEngine::initialiseAudio()
{
    auto result = deviceManager.initialiseWithDefaultDevices (0, 2);
    if (result.isNotEmpty())
        DBG ("Audio init error: " + result);

    deviceManager.addAudioCallback (this);
}

void AudioEngine::audioDeviceAboutToStart (juce::AudioIODevice* device)
{
    sampleRate = device->getCurrentSampleRate();
    synthesiser.setCurrentPlaybackSampleRate (sampleRate);
    previewTransport.prepareToPlay (device->getCurrentBufferSizeSamples(), sampleRate);
}

void AudioEngine::audioDeviceStopped()
{
    previewTransport.releaseResources();
}

void AudioEngine::audioDeviceIOCallbackWithContext (
    const float* const*, int,
    float* const* outputChannelData, int numOutputChannels,
    int numSamples, const juce::AudioIODeviceCallbackContext&)
{
    // Clear output
    for (int ch = 0; ch < numOutputChannels; ++ch)
        juce::FloatVectorOperations::clear (outputChannelData[ch], numSamples);

    // If playing a sketch, trigger clips at the right time
    if (playing.load() && currentSketch != nullptr)
    {
        double bpm = currentBPM.load();
        double beatsPerSample = bpm / (60.0 * sampleRate);

        for (int i = 0; i < numSamples; ++i)
        {
            double beat = currentBeat.load();

            // Check each clip
            for (int ci = 0; ci < currentSketch->getNumClips(); ++ci)
            {
                const auto& clip = currentSketch->getClip (ci);
                double clipEnd = clip.startBeat + clip.durationBeats;

                bool wasActive = activeClipIndices.count (ci) > 0;
                bool shouldBeActive = (beat >= clip.startBeat && beat < clipEnd);

                if (shouldBeActive && ! wasActive)
                {
                    // Trigger note on
                    auto* elem = dynamic_cast<AudioVisualElement*> (
                        elementLibrary.getElementById (clip.elementId));
                    if (elem != nullptr && elem->getMidiNote() >= 0)
                    {
                        synthesiser.noteOn (1, elem->getMidiNote(), 1.0f);
                        activeClipIndices.insert (ci);

                        juce::ScopedLock sl (activeElementsLock);
                        activeElements.insert (clip.elementId.id);
                    }
                }
                else if (! shouldBeActive && wasActive)
                {
                    auto* elem = dynamic_cast<AudioVisualElement*> (
                        elementLibrary.getElementById (clip.elementId));
                    if (elem != nullptr && elem->getMidiNote() >= 0)
                    {
                        synthesiser.noteOff (1, elem->getMidiNote(), 1.0f, false);
                        activeClipIndices.erase (ci);

                        juce::ScopedLock sl (activeElementsLock);
                        activeElements.erase (clip.elementId.id);
                    }
                }
            }

            currentBeat.store (beat + beatsPerSample);
        }

        // Check if we've passed the end
        if (currentBeat.load() >= currentSketch->getTotalLengthBeats())
        {
            stop();
        }
    }

    // Process pending note-offs
    {
        juce::ScopedLock sl (noteOffLock);
        for (auto it = pendingNoteOffs.begin(); it != pendingNoteOffs.end(); )
        {
            it->samplesRemaining -= numSamples;
            if (it->samplesRemaining <= 0)
            {
                synthesiser.noteOff (1, it->midiNote, 1.0f, true);
                {
                    juce::ScopedLock sl2 (activeElementsLock);
                    activeElements.erase (it->elementId.id);
                }
                it = pendingNoteOffs.erase (it);
            }
            else
            {
                ++it;
            }
        }
    }

    // Render synth audio
    juce::AudioBuffer<float> buffer (outputChannelData, numOutputChannels, numSamples);
    juce::MidiBuffer emptyMidi;
    synthesiser.renderNextBlock (buffer, emptyMidi, 0, numSamples);

    // Mix in preview audio
    {
        juce::ScopedLock sl (previewLock);
        if (previewTransport.isPlaying())
        {
            juce::AudioSourceChannelInfo info (&buffer, 0, numSamples);
            previewTransport.getNextAudioBlock (info);
        }
    }
}

void AudioEngine::play()
{
    activeClipIndices.clear();
    playing.store (true);
}

void AudioEngine::stop()
{
    playing.store (false);
    recording.store (false);
    activeClipIndices.clear();

    synthesiser.allNotesOff (1, false);

    juce::ScopedLock sl (activeElementsLock);
    activeElements.clear();
}

void AudioEngine::setPosition (double beats)
{
    currentBeat.store (beats);
}

void AudioEngine::setBPM (double bpm)
{
    currentBPM.store (bpm);
}

void AudioEngine::startRecording()
{
    juce::ScopedLock sl (recordLock);
    recordedLiveEvents.clear();
    recording.store (true);
    playing.store (true);
    currentBeat.store (0.0);
}

void AudioEngine::stopRecording()
{
    recording.store (false);
    playing.store (false);
}

void AudioEngine::triggerElement (const ElementID& id)
{
    auto* elem = dynamic_cast<AudioVisualElement*> (elementLibrary.getElementById (id));
    if (elem != nullptr && elem->getMidiNote() >= 0)
    {
        // Stop any existing note-off for this element first
        {
            juce::ScopedLock sl (noteOffLock);
            pendingNoteOffs.erase (
                std::remove_if (pendingNoteOffs.begin(), pendingNoteOffs.end(),
                    [&] (const PendingNoteOff& p) { return p.elementId.id == id.id; }),
                pendingNoteOffs.end());
        }

        synthesiser.noteOn (1, elem->getMidiNote(), 1.0f);

        {
            juce::ScopedLock sl (activeElementsLock);
            activeElements.insert (id.id);
        }

        // Schedule auto note-off after ~2 seconds
        {
            juce::ScopedLock sl (noteOffLock);
            int samplesForDuration = (int) (sampleRate * 2.0);
            pendingNoteOffs.push_back ({ id, elem->getMidiNote(), samplesForDuration });
        }

        if (recording.load())
        {
            juce::ScopedLock sl (recordLock);
            recordedLiveEvents.push_back ({ id, currentBeat.load(), true });
        }
    }
}

void AudioEngine::stopElement (const ElementID& id)
{
    auto* elem = dynamic_cast<AudioVisualElement*> (elementLibrary.getElementById (id));
    if (elem != nullptr && elem->getMidiNote() >= 0)
    {
        synthesiser.noteOff (1, elem->getMidiNote(), 1.0f, true);

        {
            juce::ScopedLock sl (activeElementsLock);
            activeElements.erase (id.id);
        }

        if (recording.load())
        {
            juce::ScopedLock sl (recordLock);
            recordedLiveEvents.push_back ({ id, currentBeat.load(), false });
        }
    }
}

void AudioEngine::setSketch (Sketch* sketch)
{
    currentSketch = sketch;
}

void AudioEngine::rebuildSounds()
{
    synthesiser.clearSounds();
    synthesiser.clearVoices();

    for (int i = 0; i < 16; ++i)
        synthesiser.addVoice (new SampleVoice());

    int noteNumber = 36;
    for (auto* av : elementLibrary.getAllDiscoveredAV())
    {
        auto* loaded = sampleManager.loadSample (av->getSamplePath());
        if (loaded != nullptr)
        {
            av->setMidiNote (noteNumber);
            juce::AudioBuffer<float> bufferCopy;
            bufferCopy.makeCopyOf (loaded->buffer);
            synthesiser.addSound (new SampleSound (
                av->getId(), std::move (bufferCopy),
                loaded->sampleRate, noteNumber));
            noteNumber++;
        }
    }
}

std::vector<AudioEngine::RecordedEvent> AudioEngine::getRecordedEvents() const
{
    std::vector<RecordedEvent> events;
    // Match note-ons with note-offs to create clip events
    juce::ScopedLock sl (recordLock);

    std::map<juce::String, double> pendingNoteOns;
    for (const auto& e : recordedLiveEvents)
    {
        if (e.noteOn)
        {
            pendingNoteOns[e.elementId.id] = e.startBeat;
        }
        else
        {
            auto it = pendingNoteOns.find (e.elementId.id);
            if (it != pendingNoteOns.end())
            {
                double duration = e.startBeat - it->second;
                if (duration > 0.0)
                    events.push_back ({ e.elementId, it->second, duration });
                pendingNoteOns.erase (it);
            }
        }
    }

    return events;
}

void AudioEngine::clearRecordedEvents()
{
    juce::ScopedLock sl (recordLock);
    recordedLiveEvents.clear();
}

std::set<juce::String> AudioEngine::getActiveElementIds() const
{
    juce::ScopedLock sl (activeElementsLock);
    return activeElements;
}

void AudioEngine::previewFile (const juce::File& file)
{
    stopPreview();

    auto* reader = previewFormatManager.createReaderFor (file);
    if (reader == nullptr)
        return;

    juce::ScopedLock sl (previewLock);
    previewReaderSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
    previewTransport.setSource (previewReaderSource.get(), 0, nullptr, reader->sampleRate);
    previewTransport.setPosition (0.0);
    previewTransport.start();
}

void AudioEngine::stopPreview()
{
    juce::ScopedLock sl (previewLock);
    previewTransport.stop();
    previewTransport.setSource (nullptr);
    previewReaderSource.reset();
}

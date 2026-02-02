#pragma once
#include <JuceHeader.h>
#include "SampleManager.h"
#include "SampleSound.h"
#include "SampleVoice.h"
#include "Model/ElementLibrary.h"
#include "Model/Sketch.h"
#include "Model/Clip.h"
#include <set>
#include <atomic>

class AudioEngine : public juce::AudioIODeviceCallback
{
public:
    AudioEngine (ElementLibrary& library, SampleManager& sampleManager);
    ~AudioEngine() override;

    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                           int numInputChannels,
                                           float* const* outputChannelData,
                                           int numOutputChannels,
                                           int numSamples,
                                           const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart (juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

    juce::AudioDeviceManager& getDeviceManager() { return deviceManager; }
    SampleManager& getSampleManager() { return sampleManager; }

    // Transport
    void play();
    void stop();
    void setPosition (double beats);
    void setBPM (double bpm);
    double getBPM() const               { return currentBPM.load(); }
    double getCurrentBeat() const       { return currentBeat.load(); }
    bool isPlaying() const              { return playing.load(); }
    bool isRecording() const            { return recording.load(); }

    // Looping
    void setLoopRange (double startBeat, double endBeat);
    void setLooping (bool shouldLoop);
    bool isLooping() const              { return looping.load(); }
    void startRecording();
    void stopRecording();

    // Triggering
    void triggerElement (const ElementID& id);
    void stopElement (const ElementID& id);

    // Sketch playback
    void setSketch (Sketch* sketch);

    // Rebuild synth sounds from discovered AV elements
    void rebuildSounds();

    // Audio file preview (for browser)
    void previewFile (const juce::File& file);
    void stopPreview();

    // Recorded events (from live mode)
    struct RecordedEvent
    {
        ElementID elementId;
        double beatPosition;
        double durationBeats;
    };
    std::vector<RecordedEvent> getRecordedEvents() const;
    void clearRecordedEvents();

    // Active elements (for visual canvas)
    std::set<juce::String> getActiveElementIds() const;

private:
    juce::AudioDeviceManager deviceManager;
    juce::Synthesiser synthesiser;
    ElementLibrary& elementLibrary;
    SampleManager& sampleManager;

    std::atomic<double> currentBPM { 120.0 };
    std::atomic<double> currentBeat { 0.0 };
    std::atomic<bool> playing { false };
    std::atomic<bool> recording { false };
    std::atomic<bool> looping { true };
    std::atomic<double> loopStartBeat { 0.0 };
    std::atomic<double> loopEndBeat { 16.0 };

    Sketch* currentSketch = nullptr;
    double sampleRate = 44100.0;

    // Track which clips are currently active during playback
    std::set<int> activeClipIndices;

    // Recording data
    struct LiveEvent
    {
        ElementID elementId;
        double startBeat;
        bool noteOn;
    };
    std::vector<LiveEvent> recordedLiveEvents;
    juce::CriticalSection recordLock;

    // Track active trigger elements
    std::set<juce::String> activeElements;
    juce::CriticalSection activeElementsLock;

    // Audio preview (buffer-based, no transport)
    juce::AudioFormatManager previewFormatManager;
    std::unique_ptr<juce::AudioBuffer<float>> previewBuffer;
    double previewSampleRate = 44100.0;
    int previewPosition = 0;
    std::atomic<bool> previewPlaying { false };
    juce::CriticalSection previewLock;

    void initialiseAudio();

    // Auto note-off timers for triggered elements
    struct PendingNoteOff
    {
        ElementID elementId;
        int midiNote;
        int samplesRemaining;
    };
    std::vector<PendingNoteOff> pendingNoteOffs;
    juce::CriticalSection noteOffLock;
};

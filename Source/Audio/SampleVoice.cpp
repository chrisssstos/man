#include "SampleVoice.h"
#include "SampleSound.h"

SampleVoice::SampleVoice()
{
    adsr.setParameters ({ 0.01f, 0.0f, 1.0f, 0.1f });
}

bool SampleVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<SampleSound*> (sound) != nullptr;
}

void SampleVoice::startNote (int, float velocity,
                              juce::SynthesiserSound*, int)
{
    samplePosition = 0.0;
    gain = velocity;
    isPlaying = true;
    adsr.noteOn();
}

void SampleVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        isPlaying = false;
        adsr.reset();
    }
}

void SampleVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                                    int startSample, int numSamples)
{
    if (! isPlaying)
        return;

    auto* sound = dynamic_cast<SampleSound*> (getCurrentlyPlayingSound().get());
    if (sound == nullptr)
    {
        isPlaying = false;
        clearCurrentNote();
        return;
    }

    const auto& data = sound->getAudioData();
    const int totalSamples = data.getNumSamples();
    const int numChannels = data.getNumChannels();
    const double ratioCorrection = sound->getSourceSampleRate() / getSampleRate();

    for (int i = 0; i < numSamples; ++i)
    {
        auto pos = (int) samplePosition;
        if (pos >= totalSamples)
        {
            isPlaying = false;
            clearCurrentNote();
            adsr.reset();
            break;
        }

        float adsrValue = adsr.getNextSample();
        float amplitude = gain * adsrValue;

        for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
        {
            int srcCh = ch % numChannels;
            outputBuffer.addSample (ch, startSample + i,
                                    data.getSample (srcCh, pos) * amplitude);
        }

        samplePosition += ratioCorrection;
    }

    if (! adsr.isActive())
    {
        isPlaying = false;
        clearCurrentNote();
    }
}

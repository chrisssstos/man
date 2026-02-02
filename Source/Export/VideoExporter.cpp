#include "VideoExporter.h"
#include "Visuals/VisualRenderer.h"

bool VideoExporter::exportToVideo (const juce::File& outputDir,
                                    const Sketch& sketch,
                                    ElementLibrary& library,
                                    double bpm,
                                    int width, int height, int fps)
{
    double totalBeats = sketch.getTotalLengthBeats();
    if (totalBeats <= 0.0)
        return false;

    double secondsPerBeat = 60.0 / bpm;
    double totalSeconds = totalBeats * secondsPerBeat;
    int totalFrames = (int) (totalSeconds * fps);

    outputDir.createDirectory();
    auto framesDir = outputDir.getChildFile ("frames");
    framesDir.createDirectory();

    // Create renderers
    std::map<VisualElement::VisualKind, std::unique_ptr<VisualRenderer>> renderers;
    renderers[VisualElement::VisualKind::Waveform]  = VisualRenderer::create (VisualElement::VisualKind::Waveform);
    renderers[VisualElement::VisualKind::Geometric] = VisualRenderer::create (VisualElement::VisualKind::Geometric);
    renderers[VisualElement::VisualKind::Particle]  = VisualRenderer::create (VisualElement::VisualKind::Particle);

    for (int frame = 0; frame < totalFrames; ++frame)
    {
        double currentTime = (double) frame / fps;
        double currentBeat = currentTime / secondsPerBeat;
        float phase = std::fmod ((float) currentTime / 3.0f, 1.0f);

        juce::Image img (juce::Image::ARGB, width, height, true);
        juce::Graphics g (img);
        g.fillAll (juce::Colour (0xff0a0a1a));

        auto bounds = juce::Rectangle<float> (0, 0, (float) width, (float) height).reduced (20);

        // Render active clips
        for (int ci = 0; ci < sketch.getNumClips(); ++ci)
        {
            const auto& clip = sketch.getClip (ci);
            double clipEnd = clip.startBeat + clip.durationBeats;

            if (currentBeat >= clip.startBeat && currentBeat < clipEnd)
            {
                auto* elem = dynamic_cast<AudioVisualElement*> (
                    library.getElementById (clip.elementId));
                if (elem != nullptr)
                {
                    auto it = renderers.find (elem->getVisualKind());
                    if (it != renderers.end())
                    {
                        it->second->paint (g, bounds, phase,
                                          elem->getColour(), elem->getVisualParams());
                    }
                }
            }
        }

        // Save frame as PNG
        auto file = framesDir.getChildFile (juce::String::formatted ("frame_%05d.png", frame));
        juce::PNGImageFormat png;
        std::unique_ptr<juce::FileOutputStream> stream (file.createOutputStream());
        if (stream != nullptr)
            png.writeImageToStream (img, *stream);
    }

    return true;
}

#include "VisualRenderer.h"
#include "WaveformVisual.h"
#include "GeometricVisual.h"
#include "ParticleVisual.h"
#include "ImageVisual.h"
#include "VideoVisual.h"

std::unique_ptr<VisualRenderer> VisualRenderer::create (VisualElement::VisualKind kind)
{
    switch (kind)
    {
        case VisualElement::VisualKind::Waveform:  return std::make_unique<WaveformVisual>();
        case VisualElement::VisualKind::Geometric: return std::make_unique<GeometricVisual>();
        case VisualElement::VisualKind::Particle:  return std::make_unique<ParticleVisual>();
        case VisualElement::VisualKind::Image:     return std::make_unique<ImageVisual>();
        case VisualElement::VisualKind::Video:     return std::make_unique<VideoVisual>();
    }
    return std::make_unique<WaveformVisual>();
}

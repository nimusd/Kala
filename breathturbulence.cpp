#include "breathturbulence.h"

BreathTurbulence::BreathTurbulence()
    : blend(0.10)  // Default: 10% noise
    , blendCurve(BlendCurve::Sqrt)  // Default: sqrt curve
{
}

void BreathTurbulence::setBlend(double blend)
{
    this->blend = std::clamp(blend, 0.0, 1.0);
}

void BreathTurbulence::setBlendCurve(BlendCurve curve)
{
    this->blendCurve = curve;
}

double BreathTurbulence::applyBlendCurve(double blend) const
{
    switch (blendCurve) {
        case BlendCurve::Linear:
            return blend;

        case BlendCurve::Sqrt:
            // Square root curve - gentler at low values, steeper at high
            return std::sqrt(blend);

        case BlendCurve::Log:
            // Logarithmic curve - very gentle increase
            // Map 0-1 to logarithmic scale
            if (blend <= 0.0) return 0.0;
            if (blend >= 1.0) return 1.0;
            // Using log10: log10(1 + 9*x) maps 0->0 and 1->1
            return std::log10(1.0 + 9.0 * blend);

        default:
            return blend;
    }
}

double BreathTurbulence::processSample(double voiceIn, double noiseIn)
{
    // Apply blend curve to the mix parameter
    double effectiveBlend = applyBlendCurve(blend);

    // Simple blend: output = voiceIn × (1 - blend) + noiseIn × blend
    double output = voiceIn * (1.0 - effectiveBlend) + noiseIn * effectiveBlend;

    return output;
}

#ifndef BREATHTURBULENCE_H
#define BREATHTURBULENCE_H

#include <algorithm>
#include <cmath>

/**
 * BreathTurbulence - Blends harmonic content with noise
 *
 * Adds the "breath" that makes sound human - the critical ingredient
 * that transforms electronic sound into something with lungs, throat, and body.
 *
 * Parameters:
 * - blend: Mix amount (0 = pure voice, 1 = pure noise)
 * - blendCurve: Mapping function (linear, sqrt, log)
 *
 * Critical discovery: Just 5-15% noise creates organic quality.
 */
class BreathTurbulence
{
public:
    enum class BlendCurve {
        Linear,
        Sqrt,
        Log
    };

    BreathTurbulence();

    // Process a single sample (blend voice and noise)
    double processSample(double voiceIn, double noiseIn);

    // Parameter setters
    void setBlend(double blend);
    void setBlendCurve(BlendCurve curve);

    // Parameter getters
    double getBlend() const { return blend; }
    BlendCurve getBlendCurve() const { return blendCurve; }

private:
    double applyBlendCurve(double blend) const;

    double blend;
    BlendCurve blendCurve;
};

#endif // BREATHTURBULENCE_H

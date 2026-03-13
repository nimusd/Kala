#include "frequencymapper.h"
#include <cmath>
#include <algorithm>

double FrequencyMapper::normalize(double pitchHz) const
{
    double clamped = std::max(lowFreq, std::min(pitchHz, highFreq));

    double t;
    if (curveType == Logarithmic) {
        // Normalize in log space for perceptually uniform mapping
        double logLow = std::log(lowFreq);
        double logHigh = std::log(highFreq);
        t = (std::log(clamped) - logLow) / (logHigh - logLow);
    } else {
        t = (clamped - lowFreq) / (highFreq - lowFreq);
    }

    return std::max(0.0, std::min(t, 1.0));
}

double FrequencyMapper::applyCurve(double t) const
{
    switch (curveType) {
        case Linear:
        case Logarithmic:
            // Linear and Logarithmic use the normalization directly
            return t;
        case Exponential:
            return t * t;
        case SCurve:
            // Smooth S-curve (Hermite interpolation)
            return t * t * (3.0 - 2.0 * t);
        default:
            return t;
    }
}

double FrequencyMapper::map(double pitchHz) const
{
    double t = normalize(pitchHz);
    t = applyCurve(t);

    if (invert)
        t = 1.0 - t;

    return outputMin + t * (outputMax - outputMin);
}

double FrequencyMapper::map(double pitchHz, double externalCurve) const
{
    double t = normalize(pitchHz);

    // External curve reshapes the transfer: use it instead of built-in curve
    // externalCurve (0-1) acts as the shaped value at position t
    // Blend: lerp between linear t and externalCurve based on t as position
    t = externalCurve;

    if (invert)
        t = 1.0 - t;

    return outputMin + t * (outputMax - outputMin);
}

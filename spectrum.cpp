#include "spectrum.h"
#include <algorithm>

Spectrum::Spectrum(int numHarmonics)
    : harmonics(numHarmonics, 0.0)
{
}

void Spectrum::setAmplitude(int harmonicIndex, double amplitude)
{
    if (harmonicIndex >= 0 && harmonicIndex < static_cast<int>(harmonics.size())) {
        harmonics[harmonicIndex] = amplitude;
    }
}

double Spectrum::getAmplitude(int harmonicIndex) const
{
    if (harmonicIndex >= 0 && harmonicIndex < static_cast<int>(harmonics.size())) {
        return harmonics[harmonicIndex];
    }
    return 0.0;
}

void Spectrum::resize(int numHarmonics)
{
    harmonics.resize(numHarmonics, 0.0);
}

void Spectrum::clear()
{
    std::fill(harmonics.begin(), harmonics.end(), 0.0);
}

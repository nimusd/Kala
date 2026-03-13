#include "spectrumblender.h"
#include <algorithm>

SpectrumBlender::SpectrumBlender()
    : position(0.5)
{
}

void SpectrumBlender::process(const Spectrum &spectrumA, const Spectrum &spectrumB,
                              double position, Spectrum &output)
{
    int sizeA = spectrumA.getNumHarmonics();
    int sizeB = spectrumB.getNumHarmonics();
    int outSize = std::max(sizeA, sizeB);

    output.resize(outSize);

    double posB = position;
    double posA = 1.0 - position;

    for (int h = 0; h < outSize; h++) {
        double ampA = (h < sizeA) ? spectrumA.getAmplitude(h) : 0.0;
        double ampB = (h < sizeB) ? spectrumB.getAmplitude(h) : 0.0;
        output.setAmplitude(h, ampA * posA + ampB * posB);
    }
}

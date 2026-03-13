#ifndef SPECTRUMBLENDER_H
#define SPECTRUMBLENDER_H

#include "spectrum.h"

/**
 * SpectrumBlender - Per-harmonic linear interpolation between two spectra
 *
 * Takes two spectrum inputs (A and B) and a position control.
 * Position 0.0 = pure A, 1.0 = pure B, 0.5 = 50/50 blend.
 * Drive position with an Envelope Engine or LFO to morph between timbres.
 */
class SpectrumBlender
{
public:
    SpectrumBlender();

    void process(const Spectrum &spectrumA, const Spectrum &spectrumB,
                 double position, Spectrum &output);

    void setPosition(double pos) { this->position = pos; }
    double getPosition() const { return position; }

private:
    double position;  // 0.0 = pure A, 1.0 = pure B
};

#endif // SPECTRUMBLENDER_H

#include "bandpasseq.h"
#include <algorithm>

BandpassEQ::BandpassEQ(double sampleRate)
    : sampleRate(sampleRate)
{
    gains.fill(1.0);
    qValues.fill(2.0);

    for (int i = 0; i < NUM_BANDS; i++) {
        updateCoefficients(i);
    }
}

void BandpassEQ::reset()
{
    for (auto &f : filters) {
        f.reset();
    }
}

void BandpassEQ::setSampleRate(double rate)
{
    sampleRate = rate;
    for (int i = 0; i < NUM_BANDS; i++) {
        updateCoefficients(i);
    }
}

void BandpassEQ::setGain(int band, double gain)
{
    if (band < 0 || band >= NUM_BANDS) return;
    gains[band] = std::clamp(gain, 0.0, 2.0);
}

void BandpassEQ::setQ(int band, double q)
{
    if (band < 0 || band >= NUM_BANDS) return;
    qValues[band] = std::clamp(q, 0.5, 20.0);
    updateCoefficients(band);
}

double BandpassEQ::getGain(int band) const
{
    if (band < 0 || band >= NUM_BANDS) return 1.0;
    return gains[band];
}

double BandpassEQ::getQ(int band) const
{
    if (band < 0 || band >= NUM_BANDS) return 2.0;
    return qValues[band];
}

void BandpassEQ::updateCoefficients(int band)
{
    if (band < 0 || band >= NUM_BANDS) return;

    double freq = CENTER_FREQS[band];
    double q = qValues[band];

    // Clamp frequency to valid range for the sample rate
    double nyquist = sampleRate * 0.5;
    if (freq >= nyquist) freq = nyquist * 0.95;

    // Bandpass biquad coefficients (Audio EQ Cookbook by Robert Bristow-Johnson)
    double omega = 2.0 * M_PI * freq / sampleRate;
    double sinOmega = std::sin(omega);
    double cosOmega = std::cos(omega);
    double alpha = sinOmega / (2.0 * q);

    double a0 = 1.0 + alpha;
    filters[band].b0 = alpha / a0;
    filters[band].b1 = 0.0;
    filters[band].b2 = -alpha / a0;
    filters[band].a1 = -2.0 * cosOmega / a0;
    filters[band].a2 = (1.0 - alpha) / a0;
}

double BandpassEQ::processSample(double input)
{
    double output = 0.0;
    for (int i = 0; i < NUM_BANDS; i++) {
        output += filters[i].process(input) * gains[i];
    }
    return output;
}

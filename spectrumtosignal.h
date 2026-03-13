#ifndef SPECTRUMTOSIGNAL_H
#define SPECTRUMTOSIGNAL_H

#include "spectrum.h"
#include <vector>
#include <cmath>

/**
 * SpectrumToSignal - Converts spectrum to audio signal
 *
 * Essential container - must have one to produce sound.
 * Sums oscillators for each harmonic to create audio output.
 */
class SpectrumToSignal
{
public:
    SpectrumToSignal(double sampleRate = 44100.0);

    // Generate a single audio sample from spectrum
    double generateSample(const Spectrum &spectrum, double pitch);

    // Reset phase accumulators (call when starting new note)
    void reset();

    // Parameter setters
    void setSampleRate(double rate);
    void setNormalize(double normalize) { this->normalize = normalize; }

    // PADsynth wavetable playback
    void setWavetable(const std::vector<float>* table, double fundamentalHz);
    void clearWavetable();
    bool hasWavetable() const { return wavetablePtr != nullptr && !wavetablePtr->empty(); }

private:
    double sampleRate;
    double normalize;  // 0 = off, 1 = full auto-normalize

    std::vector<double> phases;  // Phase accumulator per harmonic

    // PADsynth wavetable playback
    const std::vector<float>* wavetablePtr = nullptr;
    double wavetableFundamentalHz = 0.0;
    double wavetablePhase = 0.0;
};

#endif // SPECTRUMTOSIGNAL_H

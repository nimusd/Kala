#ifndef ROLLOFFPROCESSOR_H
#define ROLLOFFPROCESSOR_H

#include "spectrum.h"

/**
 * RolloffProcessor - Applies brightness curve to spectrum
 *
 * The "magic ingredient" that prevents static sound.
 * Supports dual-band rolloff with different powers for low and high harmonics.
 * Higher rolloff = darker sound (more attenuation of harmonics)
 */
class RolloffProcessor
{
public:
    RolloffProcessor();

    // Process a spectrum with single rolloff power (legacy/simple mode)
    void processSpectrum(const Spectrum &input, Spectrum &output, double rolloffPower);

    // Process a spectrum with dual-band rolloff
    void processSpectrumDualBand(const Spectrum &input, Spectrum &output,
                                  double lowRolloff, double highRolloff,
                                  int crossoverHarmonic, int transitionWidth);

    // Parameter setters/getters
    void setRolloffPower(double rolloff) { this->rolloffPower = rolloff; }
    double getRolloffPower() const { return rolloffPower; }

    void setLowRolloff(double rolloff) { this->lowRolloff = rolloff; }
    double getLowRolloff() const { return lowRolloff; }

    void setHighRolloff(double rolloff) { this->highRolloff = rolloff; }
    double getHighRolloff() const { return highRolloff; }

    void setCrossoverHarmonic(int harmonic) { this->crossoverHarmonic = harmonic; }
    int getCrossoverHarmonic() const { return crossoverHarmonic; }

    void setTransitionWidth(int width) { this->transitionWidth = width; }
    int getTransitionWidth() const { return transitionWidth; }

private:
    double rolloffPower;       // Single rolloff value (legacy mode)
    double lowRolloff;         // Rolloff power for harmonics below crossover
    double highRolloff;        // Rolloff power for harmonics above crossover
    int crossoverHarmonic;     // Harmonic number where rolloff behavior changes
    int transitionWidth;       // Number of harmonics to blend across (smooth transition)
};

#endif // ROLLOFFPROCESSOR_H

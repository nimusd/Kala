#ifndef NOISECOLORFILTER_H
#define NOISECOLORFILTER_H

#include <cmath>
#include <random>

/**
 * NoiseColorFilter - Shapes noise spectrum for breath effects
 *
 * From dark breath to bright fricatives. Generates white noise internally
 * and filters it to create colored noise.
 *
 * Parameters:
 * - color: Filter cutoff frequency (100-12000 Hz)
 * - filterQ: Filter resonance (1.0-20.0)
 * - filterType: lowpass, highpass, or bandpass
 *
 * Processing:
 * - Low color (100 Hz) = dark, breathy rumble
 * - Mid (2000 Hz) = 'SH' territory
 * - High (8000 Hz) = 'S' territory, hissy
 */
class NoiseColorFilter
{
public:
    enum class FilterType {
        Lowpass,
        Highpass,
        Bandpass
    };

    enum class NoiseType {
        White,
        Pink,
        Brown
    };

    NoiseColorFilter(double sampleRate = 44100.0);

    // Generate a single sample of colored noise
    double generateSample();

    // Process external noise input (when not using internal generator)
    double processSample(double noiseIn);

    // Reset filter state
    void reset();

    // Parameter setters
    void setColor(double freq);
    void setFilterQ(double q);
    void setFilterType(FilterType type);
    void setNoiseType(NoiseType type);
    void setUseInternal(bool useInternal);
    void setSampleRate(double rate);

    // Parameter getters
    double getColor() const { return color; }
    double getFilterQ() const { return filterQ; }
    FilterType getFilterType() const { return filterType; }
    NoiseType getNoiseType() const { return noiseType; }
    bool getUseInternal() const { return useInternal; }

private:
    // Biquad filter for noise shaping
    struct BiquadFilter {
        // Filter coefficients
        double b0, b1, b2;  // Numerator
        double a1, a2;      // Denominator (a0 is normalized to 1)

        // Filter state (delayed samples)
        double x1, x2;  // Input history
        double y1, y2;  // Output history

        BiquadFilter() : b0(1), b1(0), b2(0), a1(0), a2(0),
                         x1(0), x2(0), y1(0), y2(0) {}

        void reset() {
            x1 = x2 = y1 = y2 = 0.0;
        }

        double process(double input) {
            // Direct Form I: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
            double output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

            // Update state
            x2 = x1;
            x1 = input;
            y2 = y1;
            y1 = output;

            return output;
        }
    };

    void updateFilterCoefficients();
    double generateWhiteNoise();
    double generatePinkNoise();
    double generateBrownNoise();

    double sampleRate;
    double color;
    double filterQ;
    FilterType filterType;
    NoiseType noiseType;
    bool useInternal;

    BiquadFilter filter;

    // White noise generator
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;

    // Pink noise state (Paul Kellet's implementation)
    double pinkB0, pinkB1, pinkB2, pinkB3, pinkB4, pinkB5, pinkB6;

    // Brown noise state
    double brownLast;
};

#endif // NOISECOLORFILTER_H

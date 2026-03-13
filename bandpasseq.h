#ifndef BANDPASSEQ_H
#define BANDPASSEQ_H

#include <cmath>
#include <array>

/**
 * BandpassEQ - 10-Band Parametric Equalizer
 *
 * 10 parallel bandpass filters at ISO center frequencies:
 * 31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000 Hz
 *
 * Each band has gain (0 = silent, 1 = unity, 2 = boost) and Q (width) control.
 * Input signal is run through all 10 filters in parallel, each output
 * scaled by its gain, then summed.
 */
class BandpassEQ
{
public:
    static constexpr int NUM_BANDS = 10;
    static constexpr double CENTER_FREQS[NUM_BANDS] = {
        31.0, 63.0, 125.0, 250.0, 500.0,
        1000.0, 2000.0, 4000.0, 8000.0, 16000.0
    };

    BandpassEQ(double sampleRate = 44100.0);

    double processSample(double input);
    void reset();
    void setSampleRate(double rate);

    void setGain(int band, double gain);
    void setQ(int band, double q);

    double getGain(int band) const;
    double getQ(int band) const;

private:
    struct BiquadFilter {
        double b0 = 0, b1 = 0, b2 = 0;
        double a1 = 0, a2 = 0;
        double x1 = 0, x2 = 0;
        double y1 = 0, y2 = 0;

        void reset() { x1 = x2 = y1 = y2 = 0.0; }

        double process(double input) {
            double output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1; x1 = input;
            y2 = y1; y1 = output;
            return output;
        }
    };

    void updateCoefficients(int band);

    double sampleRate;
    std::array<BiquadFilter, NUM_BANDS> filters;
    std::array<double, NUM_BANDS> gains;
    std::array<double, NUM_BANDS> qValues;
};

#endif // BANDPASSEQ_H

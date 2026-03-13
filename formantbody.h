#ifndef FORMANTBODY_H
#define FORMANTBODY_H

#include <cmath>

/**
 * FormantBody - Resonant filtering creating vowel-like character
 *
 * The "throat" - uses two parallel resonant bandpass filters (biquad)
 * to create formant peaks that give the sound vowel-like qualities.
 *
 * Parameters:
 * - f1Freq, f2Freq: Formant center frequencies (Hz)
 * - f1Q, f2Q: Formant resonance (higher Q = narrower, more resonant)
 * - directMix: Dry/wet blend (0 = all filtered, 1 = all direct)
 * - f1f2Balance: Balance between F1 and F2 (0 = F2 only, 1 = F1 only)
 */
class FormantBody
{
public:
    FormantBody(double sampleRate = 44100.0);

    // Process a single audio sample
    double processSample(double input);

    // Reset filter state (call when starting a new note)
    void reset();

    // Parameter setters
    void setF1Freq(double freq);
    void setF2Freq(double freq);
    void setF1Q(double q);
    void setF2Q(double q);
    void setDirectMix(double mix);
    void setF1F2Balance(double balance);
    void setSampleRate(double rate);

    // Parameter getters
    double getF1Freq() const { return f1Freq; }
    double getF2Freq() const { return f2Freq; }
    double getF1Q() const { return f1Q; }
    double getF2Q() const { return f2Q; }
    double getDirectMix() const { return directMix; }
    double getF1F2Balance() const { return f1f2Balance; }

private:
    // Biquad filter for resonant bandpass
    struct BiquadFilter {
        // Filter coefficients
        double b0, b1, b2;  // Numerator
        double a1, a2;      // Denominator (a0 is normalized to 1)

        // Filter state (delayed samples)
        double x1, x2;  // Input history
        double y1, y2;  // Output history

        BiquadFilter() : b0(0), b1(0), b2(0), a1(0), a2(0),
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

    void updateF1Coefficients();
    void updateF2Coefficients();

    double sampleRate;
    double f1Freq;
    double f2Freq;
    double f1Q;
    double f2Q;
    double directMix;
    double f1f2Balance;

    BiquadFilter f1Filter;
    BiquadFilter f2Filter;
};

#endif // FORMANTBODY_H

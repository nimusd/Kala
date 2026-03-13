#ifndef DRIFTENGINE_H
#define DRIFTENGINE_H

#include <algorithm>
#include <cmath>
#include <vector>

/**
 * DriftEngine - Adds micro-detuning for organic quality
 *
 * Creates subtle beating between partials - the sound "breathes" even on
 * sustained notes. Slow-moving LFOs produce detuning multipliers near 1.0.
 *
 * Parameters:
 * - amount: Drift intensity (0 = none, 0.005 = subtle, 0.1 = extreme)
 * - rate: How quickly drift wanders (Hz)
 *
 * Processing:
 * - Generates slow LFO oscillations
 * - Output: detuning multiplier (around 1.0 ± amount)
 * - Future: Support per-harmonic drift for richer beating
 */
class DriftEngine
{
public:
    enum class DriftPattern {
        Sine,       // Simple sine wave LFO
        Random,     // Sample-and-hold random values
        Perlin      // Smooth noise (simulated with multiple sine waves)
    };

    DriftEngine(double sampleRate = 44100.0);

    // Generate drift value for this sample
    double generateSample();

    // Reset drift state
    void reset();

    // Parameter setters
    void setAmount(double amount);
    void setRate(double rateHz);
    void setDriftPattern(DriftPattern pattern);
    void setSampleRate(double rate);

    // Parameter getters
    double getAmount() const { return amount; }
    double getRate() const { return rate; }
    DriftPattern getDriftPattern() const { return driftPattern; }

private:
    double sampleRate;
    double amount;      // Drift intensity
    double rate;        // Drift rate in Hz
    DriftPattern driftPattern;

    // LFO state
    double phase;       // Main LFO phase
    double phase2;      // Secondary phase for Perlin-style noise
    double phase3;      // Tertiary phase for Perlin-style noise

    // Random state for Random pattern
    double randomValue;
    double randomCounter;
    double randomUpdateRate;
};

#endif // DRIFTENGINE_H

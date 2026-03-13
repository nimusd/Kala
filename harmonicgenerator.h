#ifndef HARMONICGENERATOR_H
#define HARMONICGENERATOR_H

#include <cmath>
#include <vector>
#include <cstdint>
#include <QString>

/**
 * HarmonicGenerator - Minimal additive synthesis (Phase 3: Option A)
 *
 * Generates rich harmonic content shaped by rolloff.
 * This is the "bones" from the lab - harmonics only, no formants yet.
 *
 * Parameters:
 * - numHarmonics: Number of harmonics to generate (1-64)
 * - fundamentalHz: Base frequency in Hz
 * - rolloffPower: Controls brightness (low = bright/buzzy, high = dark/mellow)
 */
class HarmonicGenerator
{
public:
    HarmonicGenerator(double sampleRate = 44100.0);

    // Generate a single audio sample
    double generateSample();

    // Reset phase (call when starting a new note)
    void reset();

    // Parameter setters
    void setNumHarmonics(int count);
    void setFundamentalHz(double hz);
    void setRolloffPower(double power);
    void setSampleRate(double rate);
    void setDnaPreset(int preset);  // 0-5 for presets (-1 for custom, 5 for pure sine test)
    void setPurity(double purity);  // 0.0-1.0, controls harmonic purity
    void setDrift(double drift);    // 0.0-0.1, controls frequency drift amount
    void setCustomAmplitudes(const std::vector<double> &amplitudes);  // Set custom harmonic pattern
    void setDigitString(const QString &digits, int initialOffset = 0); // Store digit string for window modulation
    void setDigitWindowOffset(int offset);  // Shift window into digit string and recompute amplitudes
    const QString& getDigitString() const { return digitString; }
    int getDigitWindowOffset() const { return currentDigitOffset; }

    // PADsynth setters
    void setPadEnabled(bool enabled);
    void setPadBandwidth(double cents);
    void setPadBandwidthScale(double scale);
    void setPadProfileShape(int shape);
    void setPadFftSize(int size);

    // PADsynth wavetable generation
    void generatePadWavetable(double fundamentalHz, double sampleRate);
    bool hasPadWavetable() const { return !padWavetable.empty(); }
    const std::vector<float>& getPadWavetable() const { return padWavetable; }

    // PADsynth getters
    bool getPadEnabled() const { return padEnabled; }
    double getPadBandwidth() const { return padBandwidth; }
    double getPadBandwidthScale() const { return padBandwidthScale; }
    int getPadProfileShape() const { return padProfileShape; }
    int getPadFftSize() const { return padFftSize; }

    // Parameter getters
    int getNumHarmonics() const { return numHarmonics; }
    double getFundamentalHz() const { return fundamentalHz; }
    double getRolloffPower() const { return rolloffPower; }
    int getDnaPreset() const { return dnaPreset; }

    // Get pre-calculated harmonic amplitude for a specific harmonic
    double getHarmonicAmplitude(int harmonicIndex) const {
        if (harmonicIndex >= 0 && harmonicIndex < harmonicAmplitudes.size()) {
            return harmonicAmplitudes[harmonicIndex];
        }
        return 0.0;
    }

private:
    double sampleRate;
    int numHarmonics;
    double fundamentalHz;
    double rolloffPower;
    int dnaPreset;  // -1 = custom, 0-4 = presets
    double purity;  // 0.0-1.0, controls harmonic purity
    double drift;   // 0.0-0.1, controls frequency drift

    double phase;  // Master phase accumulator
    std::vector<double> harmonicPhases;  // Phase for each harmonic
    std::vector<double> harmonicAmplitudes;  // Cached amplitudes for each harmonic
    std::vector<double> driftOffsets;  // Random drift offsets for each harmonic
    std::vector<double> customDnaPattern;  // Stored custom DNA pattern (when dnaPreset == -1)
    QString digitString;        // Fractional digit string for window-offset mode
    int     currentDigitOffset; // Last applied offset (-1 = unset)

    // PADsynth members
    bool padEnabled;
    double padBandwidth;       // bandwidth in cents
    double padBandwidthScale;  // how bandwidth scales with harmonic number
    int padProfileShape;       // 0=Gaussian, 1=Raised Cosine, 2=Square
    int padFftSize;            // FFT size (power of 2)
    std::vector<float> padWavetable;  // pre-computed wavetable

    double getBandwidthProfile(double fi, double bwi, int shape) const;

    // Recalculate harmonic amplitudes based on DNA preset
    void updateHarmonicAmplitudes();
};

#endif // HARMONICGENERATOR_H

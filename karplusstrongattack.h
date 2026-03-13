#ifndef KARPLUSSTRONGATTACK_H
#define KARPLUSSTRONGATTACK_H

#include <vector>
#include <cstddef>

/**
 * KSMode - Operating mode for Karplus-Strong
 */
enum class KSMode {
    String = 0,  // K-S is the entire sound (damped string behavior)
    Attack = 1   // K-S adds pluck transient to an input signal
};

/**
 * KSExcitation - Excitation type for delay line initialization
 */
enum class KSExcitation {
    Noise = 0,        // Filtered white noise (classic KS)
    SoftPluck = 1,    // Raised cosine (mallet / marimba)
    Blend = 2,        // Mix of noise and soft pluck
    FingerPluck = 3   // Rounded triangle (realistic finger pluck)
};

/**
 * KarplusStrongAttack - Extended Karplus-Strong string synthesis
 *
 * Implements the EKS framework (Jaffe & Smith 1983) with:
 * - Tunable first-order loop filter (brightness control)
 * - Shaped excitations (noise, soft pluck, blend)
 * - Pick direction filter
 * - Body resonance (biquad bandpass)
 *
 * Parameters:
 * - mode: Operating mode (String or Attack)
 * - attackPortion: How much of the decay to include (0.0-1.0)
 * - damping: Decay rate — controls feedback gain rho (0=longest, 1=quickest)
 * - pluckPosition: Where on the string (0.0=bridge/bright, 1.0=neck/mellow)
 * - mix: For Attack mode — prominence of pluck attack (0-1)
 * - brightness: Loop filter stretching factor (0=dark, 1=bright)
 * - excitationType: Noise/SoftPluck/Blend
 * - excitationSoftness: Noise LP cutoff / blend ratio (0-1)
 * - pickDirection: 0=downstroke, 0.5=neutral, 1=upstroke
 * - bodyResonance: Body resonator wet/dry mix (0=off)
 * - bodyFreq: Body resonator center frequency (80-400 Hz)
 */
class KarplusStrongAttack
{
public:
    KarplusStrongAttack(double sampleRate = 44100.0);

    /**
     * Trigger a new pluck at the given pitch
     * @param pitch Fundamental frequency in Hz
     */
    void trigger(double pitch);

    /**
     * Generate the next audio sample
     * @param inputSignal Optional input signal (used in Attack mode)
     * @return Sample value (0.0 if attack is complete in String mode)
     */
    double generateSample(double inputSignal = 0.0);

    /**
     * Reset all state (called when starting a new note)
     */
    void reset();

    /**
     * Check if the attack is still generating samples
     */
    bool isActive() const { return active; }

    // Parameter setters
    void setMode(KSMode m) { mode = m; }
    void setAttackPortion(double portion);   // 0.0-1.0
    void setDamping(double damping);         // 0.0-1.0
    void setPluckPosition(double position);  // 0.0-1.0
    void setMix(double m);                   // 0.0-1.0 (Attack mode only)
    void setSampleRate(double rate);
    void setBrightness(double b);            // 0.0-1.0
    void setExcitationType(int type);        // 0=Noise, 1=SoftPluck, 2=Blend
    void setExcitationSoftness(double s);    // 0.0-1.0
    void setPluckHardness(double h);         // 0.0-1.0
    void setPickDirection(double d);         // 0.0-1.0
    void setBodyResonance(double r);         // 0.0-1.0
    void setBodyFreq(double f);              // 80-400 Hz
    void setPitch(double pitch);             // Update fractional delay for continuous pitch

    // Parameter getters
    KSMode getMode() const { return mode; }
    double getAttackPortion() const { return attackPortion; }
    double getDamping() const { return damping; }
    double getPluckPosition() const { return pluckPosition; }
    double getMix() const { return mix; }
    double getBrightness() const { return brightness; }
    KSExcitation getExcitationType() const { return excitationType; }
    double getExcitationSoftness() const { return excitationSoftness; }
    double getPluckHardness() const { return pluckHardness; }
    double getPickDirection() const { return pickDirection; }
    double getBodyResonance() const { return bodyResonance; }
    double getBodyFreq() const { return bodyFreq; }

private:
    double sampleRate;

    // Mode
    KSMode mode;            // Operating mode (String or Attack)

    // Parameters
    double attackPortion;   // How much of decay to include (0-1)
    double damping;         // Decay rate — maps to feedback gain rho (0-1)
    double pluckPosition;   // Position on string (0=bridge, 1=neck)
    double mix;             // Attack mode: prominence of pluck (0-1)
    double brightness;      // Loop filter stretching factor (0-1)
    KSExcitation excitationType;  // Excitation waveform type
    double excitationSoftness;    // Noise LP cutoff / blend ratio
    double pluckHardness;         // Pluck hump width (0=wide/soft, 1=narrow/hard)
    double pickDirection;   // Pick direction (0=down, 0.5=neutral, 1=up)
    double bodyResonance;   // Body resonator wet/dry mix
    double bodyFreq;        // Body resonator center frequency (Hz)

    // Delay line (the "string")
    std::vector<double> delayLine;
    size_t writeIndex;
    size_t delayLength;
    size_t maxDelayLength;  // Allocated buffer capacity (allows pitch sweeps)
    double currentDelay;    // Fractional delay in samples (sampleRate / pitch)
    double prevSample;      // Previous sample for loop filter

    // Body resonance biquad state
    double bqX1, bqX2;     // Input history
    double bqY1, bqY2;     // Output history
    double bqB0, bqB1, bqB2; // Feedforward coefficients
    double bqA1, bqA2;     // Feedback coefficients

    // Attack state tracking
    bool active;
    size_t sampleCount;     // Samples generated since trigger
    size_t attackSamples;   // Total samples for attack portion
    double peakAmplitude;   // For normalization

    // Internal helpers
    void fillDelayLineWithNoise();
    void fillDelayLineWithExcitation();
    void fillWithSoftPluck();
    void fillWithBlendedExcitation();
    void fillWithFingerPluck();
    void applyPickDirectionToExcitation();
    void computeBodyResonanceCoeffs();
    double processBodyResonance(double sample);
    double applyLowPassFilter(double sample, double cutoffHz);

    // Filter state for noise initialization
    double filterState;
};

#endif // KARPLUSSTRONGATTACK_H

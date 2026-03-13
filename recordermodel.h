#ifndef RECORDERMODEL_H
#define RECORDERMODEL_H

#include <vector>
#include <cstddef>

/**
 * RecorderModel - Physical model of a recorder (end-blown flute)
 *
 * Implements the STK Flute digital waveguide algorithm as a standalone
 * class with no STK framework dependency.
 *
 * Signal path per sample:
 *   breathPressure (+ noise + vibrato)
 *     → jet feedback (jetReflection)
 *     → jet delay line
 *     → jet nonlinearity  (x^3 - x, a la Cook)
 *     → DC blocker
 *     → + bore-end reflection (endReflection)
 *     → bore delay line
 *     → one-pole lowpass (bore-end loss)
 *     → (loop back)
 *   output = bore delay output × gain
 *
 * Reference: STK Flute by Perry R. Cook and Gary P. Scavone (1995–2021).
 */
class RecorderModel
{
public:
    explicit RecorderModel(double sampleRate = 44100.0);

    // Reset state for a new note.  isLegato = true preserves delay-line contents.
    void reset(bool isLegato = false);

    // Generate one output sample.
    //   pitch        – fundamental frequency in Hz
    //   noteProgress – 0.0 (note start) to 1.0 (note end); drives breath envelope
    //   isLegato     – skip attack ramp (breath immediately at full pressure)
    //   tailMode     – silence excitation so bore rings down naturally
    double tick(double pitch, double noteProgress,
                bool isLegato = false, bool tailMode = false);

    // --- Parameter setters (safe to call every sample for modulation) ---

    // Overall breath pressure  [0.0 – 1.0]
    void setBreathPressure(double p) { breathPressure = clamp(p, 0.0, 1.0); }

    // Ratio of jet delay length to bore delay length  [0.05 – 0.60]
    // Controls overblowing / register.  STK default: 0.22
    void setJetRatio(double r);

    // Breath noise amount (random breath component)  [0.0 – 0.5]
    void setNoiseGain(double g) { noiseGain = clamp(g, 0.0, 0.5); }

    // Vibrato frequency in Hz  [0.0 – 20.0]
    void setVibratoFreq(double f) { vibratoFreq = clamp(f, 0.0, 20.0); }

    // Vibrato depth (amplitude)  [0.0 – 1.0]
    void setVibratoGain(double g) { vibratoGain = clamp(g, 0.0, 1.0); }

    // Reflection coefficient at bore end  [-1.0 – 1.0]; STK default: 0.6
    void setEndReflection(double r) { endReflection = clamp(r, -1.0, 1.0); }

    // Reflection coefficient at jet end  [-1.0 – 1.0]; STK default: 0.8
    void setJetReflection(double r) { jetReflection = clamp(r, -1.0, 1.0); }

private:
    static constexpr double TWO_PI = 6.283185307179586;
    static constexpr size_t MAX_DELAY = 8192; // supports freqs down to ~5 Hz at 44100

    static double clamp(double v, double lo, double hi)
    { return v < lo ? lo : (v > hi ? hi : v); }

    double sampleRate;

    // --- Controllable parameters ---
    double breathPressure;
    double jetRatio;
    double noiseGain;
    double vibratoFreq;
    double vibratoGain;
    double endReflection;
    double jetReflection;

    // --- Bore delay line (linear interpolation) ---
    std::vector<double> boreBuffer;
    size_t boreInPoint  = 0;
    size_t boreOutPoint = 0;
    double boreAlpha    = 0.0;
    double boreOmAlpha  = 1.0;
    double boreLastOut  = 0.0;

    void   setBoreDelay(double delaySamples);
    double boreTick(double input);

    // --- Jet delay line (linear interpolation) ---
    std::vector<double> jetBuffer;
    size_t jetInPoint  = 0;
    size_t jetOutPoint = 0;
    double jetAlpha    = 0.0;
    double jetOmAlpha  = 1.0;

    void   setJetDelayLen(double delaySamples);
    double jetDelayTick(double input);

    // --- One-pole lowpass filter (bore-end loss) ---
    // H(z) = (1 − |pole|) / (1 − pole·z⁻¹)
    double filterPole  = 0.0;
    double filterState = 0.0;

    double filterPhaseDelay(double frequency) const;
    double filterTick(double input);

    // --- DC blocker  H(z) = (1 − z⁻¹) / (1 − 0.99·z⁻¹) ---
    double dcX1 = 0.0;
    double dcY1 = 0.0;
    static constexpr double DC_COEFF = 0.99;
    double dcTick(double input);

    // --- Jet nonlinearity  (x³ − x, clamped to ±1) ---
    static double jetTable(double x);

    // --- Vibrato oscillator ---
    double vibratoPhase = 0.0;

    // --- White noise (32-bit LCG) ---
    unsigned int rngState = 12345u;
    double nextRandom();

    // --- Internal state ---
    double lastPitch             = -1.0;
    double currentBoreDelaySamps = 0.0;  // remembered for jetRatio changes

    void   setFrequency(double freq);
    double breathEnvelope(double noteProgress, bool isLegato) const;
};

#endif // RECORDERMODEL_H

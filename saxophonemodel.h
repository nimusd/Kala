#ifndef SAXOPHONEMODEL_H
#define SAXOPHONEMODEL_H

#include <vector>
#include <cstddef>

/**
 * SaxophoneModel - Physical model of a saxophone-like reed instrument
 *
 * Reimplements the STK Saxofony digital waveguide algorithm as a standalone
 * class with no STK framework dependency.
 *
 * The bore is split at a blowing position into two delay lines: a long segment
 * (blow point → open end) and a short segment (blow point → reed/mouthpiece).
 * The reed is modelled as a memoryless nonlinear spring (ReedTable, Smith 1986).
 * A OneZero low-pass filter provides the lossy open-end reflection.
 *
 * At blow positions close to 0 (near the mouthpiece) the timbre resembles a
 * saxophone; at 0.5 (bore centre) it resembles a clarinet.  See Scavone (2002).
 *
 * Reference: STK Saxofony — Perry R. Cook and Gary P. Scavone (1995–2021).
 *
 * Signal path per sample:
 *   breathPressure  (envelope × (breathP + noise + vibrato))
 *   temp            = −0.95 × OneZero(delay0.lastOut)   ← open-end reflection
 *   output          = temp − delay1.lastOut              ← pressure at blow point
 *   pressureDiff    = breathPressure − output            ← differential across reed
 *   delay1  ←  temp
 *   delay0  ←  breathPressure − pressureDiff × reedTable(pressureDiff) − temp
 *   return output × outputGain
 */
class SaxophoneModel
{
public:
    explicit SaxophoneModel(double sampleRate = 44100.0);

    // Reset state for a new note.  isLegato = true preserves delay-line contents.
    void reset(bool isLegato = false);

    // Generate one output sample.
    //   pitch        – fundamental frequency in Hz
    //   noteProgress – 0.0 (note start) to 1.0 (note end); drives breath envelope
    //   isLegato     – skip attack ramp (blow immediately at full pressure)
    //   tailMode     – silence excitation so bore rings down naturally
    double tick(double pitch, double noteProgress,
                bool isLegato = false, bool tailMode = false);

    // --- Parameter setters (safe to call every sample for modulation) ---

    // Overall breath pressure  [0.0 – 1.0]
    void setBreathPressure(double p) { breathPressure = clamp(p, 0.0, 1.0); }

    // Reed stiffness: soft reed = more harmonic, stiff reed = buzzy  [0.0 – 1.0]
    // Maps to STK slope: 0.1 + 0.4 × stiffness  (default: 0.5 → slope 0.3)
    void setReedStiffness(double s) { reedStiffness = clamp(s, 0.0, 1.0); }

    // Reed aperture: controls initial reed tip opening  [0.0 – 1.0]
    // Maps to STK offset: 0.4 + 0.6 × aperture  (default: 0.5 → offset 0.7)
    void setReedAperture(double a) { reedAperture = clamp(a, 0.0, 1.0); }

    // Blow position along the bore  [0.0 – 1.0]
    // 0.0 = near mouthpiece (saxophone-like), 0.5 = centre (clarinet-like)
    void setBlowPosition(double pos);

    // Breath noise amount  [0.0 – 0.4]
    void setNoiseGain(double g) { noiseGain = clamp(g, 0.0, 0.4); }

    // Vibrato frequency in Hz  [0.0 – 12.0]
    void setVibratoFreq(double f) { vibratoFreq = clamp(f, 0.0, 12.0); }

    // Vibrato depth  [0.0 – 0.5]
    void setVibratoGain(double g) { vibratoGain = clamp(g, 0.0, 0.5); }

    // --- Nonlinear allpass parameters (Romain Michon extension) ---

    // Modulation type for allpass coefficient θ  [0 – 4]
    //   0 = θ ← open-end signal directly
    //   1 = θ ← leaky average of open-end signal
    //   2 = θ ← open-end signal²
    //   3 = θ ← sine wave at nlFreqMod Hz
    //   4 = θ ← sine wave at fundamental pitch Hz
    void setNLType(int t)      { nlType    = clamp(t, 0, 4); }

    // Amount of nonlinear filtering  [0.0 – 1.0]  (0 = fully bypassed)
    void setNLAmount(double a)  { nlAmount  = clamp(a, 0.0, 1.0); }

    // Modulation frequency used by types 3 and 4  [0 – 200 Hz]
    void setNLFreqMod(double f) { nlFreqMod = clamp(f, 0.0, 200.0); }

    // Ramp-in duration for the nonlinearity at note start  [0 – 2 s]
    void setNLAttack(double a)  { nlAttack  = clamp(a, 0.001, 2.0); }

private:
    static constexpr double TWO_PI    = 6.283185307179586;
    static constexpr size_t MAX_DELAY = 8192; // supports freqs down to ~5 Hz at 44100

    static double clamp(double v, double lo, double hi)
    { return v < lo ? lo : (v > hi ? hi : v); }

    double sampleRate;

    // --- Controllable parameters ---
    double breathPressure = 0.7;
    double reedStiffness  = 0.5;  // → slope  = 0.3 (matches STK constructor default)
    double reedAperture   = 0.5;  // → offset = 0.7 (matches STK constructor default)
    double blowPosition   = 0.2;  // near mouthpiece → saxophone
    double noiseGain      = 0.2;
    double vibratoFreq    = 5.735;
    double vibratoGain    = 0.0;

    // --- Delay line 0: long segment (blow point → open end) ---
    std::vector<double> delay0Buffer;
    size_t delay0InPoint  = 0;
    size_t delay0OutPoint = 0;
    double delay0Alpha    = 0.0;
    double delay0OmAlpha  = 1.0;
    double delay0LastOut  = 0.0;  // output from previous tick (read before writing)

    void   setDelay0(double delaySamples);
    double delay0Tick(double input);

    // --- Delay line 1: short segment (blow point → reed/mouthpiece) ---
    std::vector<double> delay1Buffer;
    size_t delay1InPoint  = 0;
    size_t delay1OutPoint = 0;
    double delay1Alpha    = 0.0;
    double delay1OmAlpha  = 1.0;
    double delay1LastOut  = 0.0;  // output from previous tick (read before writing)

    void   setDelay1(double delaySamples);
    double delay1Tick(double input);

    // --- OneZero low-pass filter (zero at −1, phase delay = 0.5 samples) ---
    // y[n] = 0.5·x[n] + 0.5·x[n−1]
    double oneZeroX1 = 0.0;
    double oneZeroTick(double input);

    // --- Reed table (Smith 1986): output = clamp(offset + slope·input, −1, 1) ---
    double reedTableTick(double input) const;

    // --- Vibrato oscillator ---
    double vibratoPhase = 0.0;

    // --- White noise (32-bit LCG) ---
    unsigned int rngState = 12345u;
    double nextRandom();

    // --- Nonlinear allpass filter (Michon) ---
    // H(z) = (θ + z⁻¹) / (1 + θ·z⁻¹)
    // y[n] = θ·(x[n] − y[n−1]) + x[n−1]
    int    nlType    = 0;
    double nlAmount  = 0.0;
    double nlFreqMod = 10.0;
    double nlAttack  = 0.1;

    double nlX1        = 0.0;   // x[n-1]
    double nlY1        = 0.0;   // y[n-1]
    double nlSinePhase = 0.0;   // oscillator phase for types 3 & 4
    double nlLeaky     = 0.0;   // leaky integrator state for type 1

    // Ramp-in envelope (0 → 1 over nlAttack seconds at note start)
    double nlEnvGain   = 0.0;
    bool   nlEnvActive = false;

    double computeNLCoeff(double signal, double pitch) const;
    double nlFilterTick(double input, double coeff);

    // --- Internal state ---
    double lastPitch         = -1.0;
    double totalDelaySamples = 0.0; // remembered so setBlowPosition can re-split

    void   setFrequency(double freq);
    double breathEnvelope(double noteProgress, bool isLegato) const;
};

#endif // SAXOPHONEMODEL_H

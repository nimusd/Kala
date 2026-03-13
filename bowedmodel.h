#ifndef BOWEDMODEL_H
#define BOWEDMODEL_H

#include <vector>
#include <cstddef>

/**
 * BowedModel - Physical model of a bowed string instrument
 *
 * Reimplements the STK Bowed waveguide model as a standalone class
 * with no STK framework dependency, extended with the Romain Michon
 * nonlinear allpass filter (from his Faust/STK extended bowed model).
 *
 * References:
 *   STK Bowed — Perry R. Cook and Gary P. Scavone (1995–2021)
 *   Body filter coefficients — Esteban Maestre (2011)
 *   Nonlinear waveguide extension — Romain Michon (Faust/STK, 2011+)
 *
 * Signal path per sample:
 *   ADSR envelope → bowVelocity
 *   stringVelocity = bridgeReflection + nutReflection
 *   deltaV         = bowVelocity − stringVelocity
 *   newVelocity    = deltaV × BowTable(deltaV)          ← friction
 *   neckDelay  ← nutReflection  + newVelocity
 *   bridgeDelay← bridgeReflection + newVelocity
 *
 *   bridgeSig      = bridgeDelay.lastOut()
 *   nlSig          = nonlinearAllpass(bridgeSig, θ)     ← Michon extension
 *   bridgeReflection = −stringFilter(nlSig)
 *   output         = bodyFilters(nlSig) × 0.1248
 *
 * Nonlinear allpass:  H(z) = (θ + z⁻¹) / (1 + θ·z⁻¹)
 *   θ is derived from one of five modulation types, scaled by nlAmount
 *   and ramped in over nlAttack seconds at note start.
 */
class BowedModel
{
public:
    explicit BowedModel(double sampleRate = 44100.0);

    // Reset state for a new note.  isLegato = true preserves delay-line contents.
    void reset(bool isLegato = false);

    // Generate one output sample.
    //   pitch        – fundamental frequency in Hz
    //   noteProgress – 0.0 (note start) to 1.0 (note end); drives bow envelope
    //   isLegato     – skip attack ramp (bow immediately at full pressure)
    //   tailMode     – silence excitation so string rings down naturally
    double tick(double pitch, double noteProgress,
                bool isLegato = false, bool tailMode = false);

    // --- Bow parameter setters ---

    // Bow pressure / friction force  [0.0 – 1.0]
    // Maps to BowTable slope: slope = 5.0 − 4.0 × pressure
    void setBowPressure(double p) { bowPressure = clamp(p, 0.0, 1.0); }

    // Maximum bow velocity amplitude  [0.0 – 1.0]
    void setBowVelocity(double v) { bowVelocity = clamp(v, 0.0, 1.0); }

    // Bow contact position along the string  [0.0 – 1.0]
    // 0 = bridge end (sul ponticello), 1 = nut end (sul tasto)
    void setBowPosition(double pos) { bowPosition = clamp(pos, 0.0, 1.0); }

    // --- Nonlinear waveguide parameter setters (Michon extension) ---

    // Modulation type for allpass coefficient θ  [0 – 4]
    //   0 = θ ← bridge signal
    //   1 = θ ← leaky average of bridge signal
    //   2 = θ ← bridge signal²
    //   3 = θ ← sine wave at nlFreqMod Hz
    //   4 = θ ← sine wave at fundamental pitch Hz
    void setNLType(int t) { nlType = clamp(t, 0, 4); }

    // Amount of nonlinear filtering  [0.0 – 1.0]  (0 = fully bypassed)
    void setNLAmount(double a) { nlAmount = clamp(a, 0.0, 1.0); }

    // Modulation frequency used by types 3 and 4  [0 – 200 Hz]
    void setNLFreqMod(double f) { nlFreqMod = clamp(f, 0.0, 200.0); }

    // Ramp-in duration for the nonlinearity at note start  [0 – 2 s]
    void setNLAttack(double a) { nlAttack = clamp(a, 0.001, 2.0); }

private:
    static constexpr double TWO_PI    = 6.283185307179586;
    static constexpr size_t MAX_DELAY = 8192; // supports freqs down to ~5 Hz at 44100

    static double clamp(double v, double lo, double hi)
    { return v < lo ? lo : (v > hi ? hi : v); }

    static int clamp(int v, int lo, int hi)
    { return v < lo ? lo : (v > hi ? hi : v); }

    double sampleRate;

    // --- Bow parameters ---
    double bowPressure  = 0.75;
    double bowVelocity  = 0.5;
    double bowPosition  = 0.127236;

    // --- Nonlinear parameters ---
    int    nlType    = 0;
    double nlAmount  = 0.0;
    double nlFreqMod = 10.0;
    double nlAttack  = 0.1;

    // --- Neck delay line (bow → nut) with linear interpolation ---
    std::vector<double> neckBuffer;
    size_t neckInPoint  = 0;
    size_t neckOutPoint = 0;
    double neckAlpha    = 0.0;
    double neckOmAlpha  = 1.0;
    double neckLastOut  = 0.0;

    void   setNeckDelay(double delaySamples);
    double neckTick(double input);

    // --- Bridge delay line (bow → bridge) with linear interpolation ---
    std::vector<double> bridgeBuffer;
    size_t bridgeInPoint  = 0;
    size_t bridgeOutPoint = 0;
    double bridgeAlpha    = 0.0;
    double bridgeOmAlpha  = 1.0;
    double bridgeLastOut  = 0.0;

    void   setBridgeDelay(double delaySamples);
    double bridgeTick(double input);

    // --- OnePole string loss filter ---
    // Matches STK OnePole: y[n] = b0×x[n] + pole×y[n−1]
    // b0 = gain×(1−|pole|) ensures DC loop gain = gain = 0.95 < 1
    double stringFilterPole  = 0.0;
    double stringFilterGain  = 0.95;
    double stringFilterB0    = 0.0;
    double stringFilterState = 0.0;
    double stringFilterTick(double input);

    // --- 6-stage cascaded BiQuad body resonance filter ---
    // Coefficients from STK Bowed constructor (Esteban Maestre violin body model)
    struct BiQuadState {
        double b0, b1, b2;
        double a1, a2;
        double x1 = 0.0, x2 = 0.0;
        double y1 = 0.0, y2 = 0.0;
    };
    BiQuadState bodyFilters[6];
    void   initBodyFilters();
    double bodyFilterTick(double input);

    // --- Nonlinear allpass filter (Michon) ---
    // H(z) = (θ + z⁻¹) / (1 + θ·z⁻¹)
    // y[n] = θ·(x[n] − y[n−1]) + x[n−1]
    double nlX1         = 0.0;   // x[n-1]
    double nlY1         = 0.0;   // y[n-1]
    double nlSinePhase  = 0.0;   // oscillator phase for types 3 & 4
    double nlLeaky      = 0.0;   // leaky integrator state for type 1

    // Nonlinearity ramp-in envelope (0 → 1 over nlAttack seconds at note start)
    double nlEnvGain   = 0.0;
    bool   nlEnvActive = false;

    double computeNLCoeff(double bridgeSample, double pitch) const;
    double nlFilterTick(double input, double coeff);

    // --- ADSR bow envelope ---
    enum class AdsrState { Idle, Attack, Decay, Sustain, Release };
    AdsrState adsrState  = AdsrState::Idle;
    double    adsrValue  = 0.0;

    static constexpr double ADSR_ATTACK_TIME  = 0.02;
    static constexpr double ADSR_DECAY_TIME   = 0.005;
    static constexpr double ADSR_SUSTAIN      = 0.9;
    static constexpr double ADSR_RELEASE_TIME = 0.01;

    double adsrTick();
    void   adsrNoteOn();
    void   adsrNoteOff();

    // --- Internal state ---
    double lastPitch = -1.0;
    double baseDelay = 0.0;
    bool   bowActive = false;

    void setFrequency(double freq);
};

#endif // BOWEDMODEL_H

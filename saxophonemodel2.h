#ifndef SAXOPHONEMODEL2_H
#define SAXOPHONEMODEL2_H

#include <vector>
#include <cstddef>

/**
 * SaxophoneModel2 - Enhanced physical model of a saxophone reed instrument
 *
 * Builds on the STK Saxofony conical-bore waveguide (SaxophoneModel) and adds:
 *
 *  1. Vocal tract resonance — 2nd-order RBJ bandpass applied to the mouthpiece
 *     pressure (delay1LastOut), fed back into breath to simulate the player's
 *     throat/mouth cavity amplifying frequencies it resonates with.  This is
 *     the key "vocal warmth" missing from the original model.
 *
 *  2. Jaw vibrato — aperture-based vibrato (jawVibratoBlend blends from pure
 *     breath tremolo to pure jaw modulation of reedAperture) for an authentic
 *     saxophone vibrato as opposed to the tremolo-like original.
 *
 *  3. Growl — AM modulation of breath pressure at growlFreq Hz, simulating
 *     the player humming a low tone into the instrument.
 *
 *  4. Configurable bell reflection — replaces the hardcoded -0.95 loss factor.
 *     Lower values = brighter/more open, higher = darker/more resonant.
 *
 *  5. S-curve (smoothstep) breath attack — 3t²−2t³ ramp instead of linear
 *     for a more natural, accelerating onset.
 *
 *  6. Pressure-scaled output — output level scales with breath pressure so
 *     dynamics affect both character and amplitude.
 *
 * Reference: STK Saxofony — Perry R. Cook and Gary P. Scavone (1995–2021).
 */
class SaxophoneModel2
{
public:
    explicit SaxophoneModel2(double sampleRate = 44100.0);

    // Reset state for a new note.  isLegato = true preserves delay-line contents.
    void reset(bool isLegato = false);

    // Generate one output sample.
    double tick(double pitch, double noteProgress,
                bool isLegato = false, bool tailMode = false);

    // ── Core reed parameters (identical range/semantics to SaxophoneModel) ──

    void setBreathPressure(double p) { breathPressure = clamp(p, 0.0, 1.0); }
    void setReedStiffness(double s)  { reedStiffness  = clamp(s, 0.0, 1.0); }
    void setReedAperture(double a)   { reedAperture   = clamp(a, 0.0, 1.0); }
    void setBlowPosition(double pos);
    void setNoiseGain(double g)      { noiseGain      = clamp(g, 0.0, 0.4); }
    void setVibratoFreq(double f)    { vibratoFreq    = clamp(f, 0.0, 12.0); }
    void setVibratoGain(double g)    { vibratoGain    = clamp(g, 0.0, 0.5); }
    void setNLType(int t)            { nlType         = clamp(t, 0, 4); }
    void setNLAmount(double a)       { nlAmount       = clamp(a, 0.0, 1.0); }
    void setNLFreqMod(double f)      { nlFreqMod      = clamp(f, 0.0, 200.0); }
    void setNLAttack(double a)       { nlAttack       = clamp(a, 0.001, 2.0); }

    // ── New saxophone-specific parameters ───────────────────────────────────

    // Vocal tract resonance frequency (Hz) [200–3000]
    // Tune to the vowel you want the bore to "sing through": ~500 Hz = "oh",
    // ~700 Hz = "ah", ~1200 Hz = "ee".  For alto sax, 600–800 Hz works well.
    void setVocalTractFreq(double f);

    // Vocal tract Q factor [0.5–10]
    // Higher Q = narrower, more pronounced resonance peak.
    void setVocalTractQ(double q);

    // Vocal tract feedback gain [0–1]
    // 0 = off (identical to SaxophoneModel), 0.2–0.4 = subtle warmth,
    // 0.6–0.8 = prominent vocal coloring.
    void setVocalTractGain(double g) { vocalTractGain = clamp(g, 0.0, 1.0); }

    // Jaw vibrato blend [0–1]
    // 0 = breath tremolo (original), 1 = pure jaw (aperture) vibrato.
    // 0.7–1.0 gives the relaxed, rolling vibrato characteristic of sax players.
    void setJawVibratoBlend(double b) { jawVibratoBlend = clamp(b, 0.0, 1.0); }

    // Growl modulation frequency (Hz) [20–200]
    // 60–120 Hz is typical; 20–40 Hz = flutter tongue effect.
    void setGrowlFreq(double f) { growlFreq = clamp(f, 20.0, 200.0); }

    // Growl depth [0–0.5]
    // 0 = off; 0.1 = subtle rasp; 0.3 = audible growl; 0.45 = intense.
    void setGrowlDepth(double d) { growlDepth = clamp(d, 0.0, 0.5); }

    // Open-end (bell) reflection coefficient [0.5–0.99]
    // 0.95 = original STK value (quite resonant/dark).
    // Lower = more energy escapes = brighter, more open sound.
    void setBellReflection(double r) { bellReflection = clamp(r, 0.5, 0.99); }

    // ── Onset / attack parameters ────────────────────────────────────────────

    // Attack chiff gain [0–10]
    // Boosts noise at note onset to simulate the tongue-release turbulence burst.
    // 0 = off, 4–6 = natural chiff, 8–10 = very pronounced attack transient.
    void setAttackChiffGain(double g) { attackChiffGain = clamp(g, 0.0, 10.0); }

    // Attack overpressure [0–1]
    // Brief breath-pressure boost at tongue release, decays in ~3 ms.
    // 0 = off, 0.3–0.5 = realistic tongue attack, 1.0 = hard accent.
    void setAttackOverpressure(double p) { attackOverpressure = clamp(p, 0.0, 1.0); }

    // Reed saturation [0–1]
    // Scales the reed-table slope so the reed clips more readily, generating
    // harmonic distortion throughout normal operation rather than only at peaks.
    // 0 = original linear reed, 0.5 = moderate drive, 1.0 = full saturation.
    void setReedSaturation(double s) { reedSaturation = clamp(s, 0.0, 1.0); }

private:
    static constexpr double TWO_PI    = 6.283185307179586;
    static constexpr size_t MAX_DELAY = 8192;

    template<typename T>
    static T clamp(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); }

    double sampleRate;

    // ── Core parameters ─────────────────────────────────────────────────────
    double breathPressure = 0.7;
    double reedStiffness  = 0.5;
    double reedAperture   = 0.5;
    double blowPosition   = 0.2;
    double noiseGain      = 0.2;
    double vibratoFreq    = 5.735;
    double vibratoGain    = 0.0;

    // ── New parameters ───────────────────────────────────────────────────────
    double vocalTractFreq  = 650.0;
    double vocalTractQ     = 2.0;
    double vocalTractGain  = 0.3;
    double jawVibratoBlend = 0.7;
    double growlFreq       = 85.0;
    double growlDepth      = 0.0;
    double bellReflection  = 0.85;

    double attackChiffGain    = 5.0;
    double attackOverpressure = 0.4;
    double reedSaturation     = 0.6;

    int    m_onsetSamples = 0;   // samples elapsed since note-on (non-legato only)

    // ── Delay line 0 (long, blow point → open end) ──────────────────────────
    std::vector<double> delay0Buffer;
    size_t delay0InPoint = 0, delay0OutPoint = 0;
    double delay0Alpha = 0.0, delay0OmAlpha = 1.0, delay0LastOut = 0.0;
    void   setDelay0(double delaySamples);
    double delay0Tick(double input);

    // ── Delay line 1 (short, blow point → reed/mouthpiece) ──────────────────
    std::vector<double> delay1Buffer;
    size_t delay1InPoint = 0, delay1OutPoint = 0;
    double delay1Alpha = 0.0, delay1OmAlpha = 1.0, delay1LastOut = 0.0;
    void   setDelay1(double delaySamples);
    double delay1Tick(double input);

    // ── OneZero low-pass filter ──────────────────────────────────────────────
    double oneZeroX1 = 0.0;
    double oneZeroTick(double input);

    // ── Reed table ───────────────────────────────────────────────────────────
    // Takes aperture as a parameter so jaw vibrato can modulate it per-sample.
    double reedTableTick(double input, double aperture) const;

    // ── Vibrato oscillator ───────────────────────────────────────────────────
    double vibratoPhase = 0.0;

    // ── Growl oscillator ─────────────────────────────────────────────────────
    double growlPhase = 0.0;

    // ── White noise (32-bit LCG) ─────────────────────────────────────────────
    unsigned int rngState = 12345u;
    double nextRandom();

    // ── Nonlinear allpass (Michon) ───────────────────────────────────────────
    int    nlType    = 0;
    double nlAmount  = 0.0;
    double nlFreqMod = 10.0;
    double nlAttack  = 0.1;
    double nlX1 = 0.0, nlY1 = 0.0;
    double nlSinePhase = 0.0, nlLeaky = 0.0;
    double nlEnvGain = 0.0;
    bool   nlEnvActive = false;
    double computeNLCoeff(double signal, double pitch) const;
    double nlFilterTick(double input, double coeff);

    // ── Vocal tract biquad bandpass (RBJ) ────────────────────────────────────
    // Coefficients are pre-computed; recomputed when freq or Q changes.
    double vtB0 = 0.0, vtB2 = 0.0;  // b1 = 0 for BPF
    double vtA1 = 0.0, vtA2 = 0.0;
    double vtX1 = 0.0, vtX2 = 0.0;  // input delay state
    double vtY1 = 0.0, vtY2 = 0.0;  // output delay state
    void   updateVocalTractCoeffs();
    double vocalTractFilter(double input);

    // ── Internal ─────────────────────────────────────────────────────────────
    double lastPitch = -1.0, totalDelaySamples = 0.0;
    double m_tailDecay = 1.0;  // per-sample gain applied to bore ring-down in tail mode
    void   setFrequency(double freq);
    double breathEnvelope(double noteProgress, bool isLegato) const;
};

#endif // SAXOPHONEMODEL2_H

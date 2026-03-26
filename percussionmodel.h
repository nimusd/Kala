#ifndef PERCUSSIONMODEL_H
#define PERCUSSIONMODEL_H

#include <cstddef>

/**
 * PercussionModel - Modal synthesis percussion
 *
 * Models a struck resonating body using a bank of damped sinusoidal
 * oscillators (modes). Covers the full range from tabla/dayan to gong.
 *
 * Each mode is a second-order IIR resonator:
 *   y[n] = cosCoeff * y[n-1]  -  r² * y[n-2]  +  excitation * modeAmp
 *
 * Mode frequency ratios are interpolated across three anchor sets:
 *   inharmonicity = 0.0  →  harmonic series (tabla syahi-tuned, near-integer)
 *   inharmonicity = 0.5  →  circular membrane Bessel zeros (raw drum skin)
 *   inharmonicity = 1.0  →  bell / gong inharmonic set
 *
 * Instrument recipes:
 *   Tabla/Dayan :  inharmonicity=0.05, decayTime=0.6,  highDecayRate=2.5, strikePos=0.4
 *   Timpani     :  inharmonicity=0.3,  decayTime=4.0,  highDecayRate=1.5, strikePos=0.3
 *   Tar / frame :  inharmonicity=0.1,  decayTime=0.4,  highDecayRate=3.0, strikePos=0.7
 *   Gong        :  inharmonicity=0.9,  decayTime=10.0, highDecayRate=0.4, strikePos=0.5
 *   Bell        :  inharmonicity=0.8,  decayTime=6.0,  highDecayRate=0.6, strikePos=0.5
 */
class PercussionModel
{
public:
    static constexpr int MAX_MODES = 16;

    explicit PercussionModel(double sampleRate = 44100.0);

    // Reset state for a new note (called at note-on).
    void reset();

    // Generate one output sample.
    //   pitch    – fundamental frequency in Hz (used on first tick after reset)
    //   tailMode – excitation is suppressed; modes ring down naturally
    double tick(double pitch, bool tailMode = false);

    // -- Parameter setters --

    // Strike position [0–1]: 0 = centre of membrane, 1 = rim/edge.
    // Centre emphasises low modes; rim excites all modes equally.
    void setStrikePosition(double p);

    // Mallet / hand contact duration [1–50 ms].
    // Short = hard stick; long = padded mallet or palm slap.
    void setStrikeDuration(double ms);

    // Timbral character [0–1]:
    //   0.0 = harmonic (tabla with syahi)
    //   0.5 = raw circular membrane (Bessel zeros)
    //   1.0 = inharmonic bell / gong
    void setInharmonicity(double v);

    // Fundamental decay time [0.05–15 s].
    void setDecayTime(double s);

    // High-mode decay exponent [0.1–5.0].
    // τ[i] = decayTime × (f₀/fᵢ)^highDecayRate
    //   0.3–0.5 = gong / bell (all modes ring equally long)
    //   1.5–2.0 = tabla (high modes die much faster)
    void setHighDecayRate(double r);

    // Number of modal resonators to compute [2–16].
    void setNumModes(int n);

    // Broadband noise burst at onset (skin texture, stick click) [0–1].
    void setNoiseGain(double g);

    // Pitch ratio [0.25–4.0]: 1.0 = note pitch, 2.0 = octave up.
    void setPitchMultiplier(double m);

private:
    static constexpr double TWO_PI = 6.283185307179586;

    template<typename T>
    static T clamp(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); }

    double sampleRate;

    // Parameters (set before reset())
    double strikePosition   = 0.5;
    double strikeDurationMs = 5.0;
    double inharmonicity    = 0.0;
    double decayTime        = 1.0;
    double highDecayRate    = 1.5;
    int    numModes         = 12;
    double noiseGain        = 0.15;
    double pitchMultiplier  = 1.0;

    // Per-mode resonator state
    struct Mode {
        double s1       = 0.0;  // y[n-1]
        double s2       = 0.0;  // y[n-2]
        double cosCoeff = 0.0;  // 2·r·cos(ω)
        double r2       = 0.0;  // r²
        double amp      = 0.0;  // excitation amplitude for this mode
    } modes[MAX_MODES];

    // Excitation state (initialised in reset(), consumed in tick())
    bool modesReady           = false;  // true after first tick computes mode coeffs
    int  strikeSamplesTotal   = 0;
    int  strikeSamplesRemaining = 0;

    // Noise burst state
    int    noiseSamplesRemaining = 0;
    double noiseCurrentAmp      = 0.0;
    double noiseDecayPerSample  = 1.0;

    // Noise PRNG (linear congruential)
    unsigned int rngState = 12345u;
    double nextRandom();

    // Mode ratio anchor tables (16 entries each, index 0 = fundamental)
    static const double ratioHarmonic[MAX_MODES];
    static const double ratioMembrane[MAX_MODES];
    static const double ratioBell[MAX_MODES];

    double getModeRatio(int i) const;
    double getModeAmplitude(int i) const;
    void   computeModes(double pitch);
};

#endif // PERCUSSIONMODEL_H

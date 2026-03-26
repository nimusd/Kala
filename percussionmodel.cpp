#include "percussionmodel.h"
#include <cmath>

// ---------------------------------------------------------------------------
// Mode frequency ratio tables
// ---------------------------------------------------------------------------

// Harmonic series — tabla with syahi tuning paste creates near-integer overtones
const double PercussionModel::ratioHarmonic[MAX_MODES] = {
    1.000,  2.000,  3.000,  4.000,  5.000,  6.000,  7.000,  8.000,
    9.000, 10.000, 11.000, 12.000, 13.000, 14.000, 15.000, 16.000
};

// Circular membrane Bessel-function zeros, normalised to the (0,1) mode
// Sources: J₀, J₁, J₂, J₃, J₄ first positive zeros combined
const double PercussionModel::ratioMembrane[MAX_MODES] = {
    1.000, 1.593, 2.136, 2.296, 2.653, 2.917, 3.156, 3.500,
    3.600, 3.900, 4.154, 4.230, 4.516, 4.610, 4.832, 4.918
};

// Bell / gong inharmonic set — empirical, stays within a musical frequency
// range so high modes don't become ultrasonic at typical pitches
const double PercussionModel::ratioBell[MAX_MODES] = {
    1.000, 1.505, 2.000, 2.390, 2.876, 3.100, 3.596, 4.015,
    4.456, 4.892, 5.310, 5.826, 6.237, 6.742, 7.115, 7.640
};

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

PercussionModel::PercussionModel(double sr)
    : sampleRate(sr)
{
}

// ---------------------------------------------------------------------------
// reset() — called at note-on
// ---------------------------------------------------------------------------

void PercussionModel::reset()
{
    // Clear resonator state
    for (int i = 0; i < MAX_MODES; i++) {
        modes[i].s1 = 0.0;
        modes[i].s2 = 0.0;
    }

    // Mode coefficients are computed on the first tick() when pitch is known
    modesReady = false;

    // Excitation envelope (half-cosine decay, mallet contact)
    strikeSamplesTotal     = clamp(static_cast<int>(strikeDurationMs * 0.001 * sampleRate), 1, static_cast<int>(sampleRate));
    strikeSamplesRemaining = strikeSamplesTotal;

    // Broadband noise burst (fixed 10 ms window, scaled by noiseGain)
    int noiseSamplesTotal  = static_cast<int>(0.010 * sampleRate);
    noiseSamplesRemaining  = (noiseGain > 0.0) ? noiseSamplesTotal : 0;
    noiseCurrentAmp        = noiseGain;
    noiseDecayPerSample    = (noiseSamplesTotal > 1)
                             ? std::exp(-6.0 / double(noiseSamplesTotal))
                             : 0.0;
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

double PercussionModel::getModeRatio(int i) const
{
    double t = clamp(inharmonicity, 0.0, 1.0);
    if (t <= 0.5) {
        double u = t * 2.0;
        return ratioHarmonic[i] * (1.0 - u) + ratioMembrane[i] * u;
    } else {
        double u = (t - 0.5) * 2.0;
        return ratioMembrane[i] * (1.0 - u) + ratioBell[i] * u;
    }
}

double PercussionModel::getModeAmplitude(int i) const
{
    // Base rolloff: lower modes carry more energy
    double base = 1.0 / std::sqrt(double(i + 1));

    // Strike position: centre emphasises axisymmetric (low) modes.
    // Rim strike excites all modes roughly equally.
    // The weight of mode i from the centre fades exponentially with i.
    double centerFactor = std::exp(-double(i) * 0.35 * (1.0 - strikePosition));
    double rimFactor    = 0.5 + 0.5 / (1.0 + double(i) * 0.15);
    double posWeight    = (1.0 - strikePosition) * centerFactor + strikePosition * rimFactor;

    return base * posWeight;
}

void PercussionModel::computeModes(double pitch)
{
    const double nyquist = sampleRate * 0.495;  // slight headroom below Nyquist
    const double effectivePitch = pitch * pitchMultiplier;
    const int    n = clamp(numModes, 2, MAX_MODES);

    for (int i = 0; i < n; i++) {
        const double ratio = getModeRatio(i);
        const double freq  = effectivePitch * ratio;

        if (freq >= nyquist) {
            // Mode above Nyquist — silence it
            modes[i].cosCoeff = 0.0;
            modes[i].r2       = 0.0;
            modes[i].amp      = 0.0;
            continue;
        }

        // Decay time for this mode: τ[i] = decayTime × (1/ratio)^highDecayRate
        double modeDecay = decayTime * std::pow(1.0 / ratio, highDecayRate);
        modeDecay = clamp(modeDecay, 0.001, 120.0);

        const double r     = std::exp(-1.0 / (modeDecay * sampleRate));
        const double omega = TWO_PI * freq / sampleRate;

        modes[i].cosCoeff = 2.0 * r * std::cos(omega);
        modes[i].r2       = r * r;
        modes[i].amp      = getModeAmplitude(i);
    }

    // Silence any unused mode slots
    for (int i = n; i < MAX_MODES; i++) {
        modes[i].cosCoeff = 0.0;
        modes[i].r2       = 0.0;
        modes[i].amp      = 0.0;
    }

    modesReady = true;
}

// ---------------------------------------------------------------------------
// tick()
// ---------------------------------------------------------------------------

double PercussionModel::tick(double pitch, bool tailMode)
{
    // Compute mode coefficients on the first sample of each note
    if (!modesReady) {
        computeModes(pitch);
    }

    // Mallet excitation: half-cosine decay (1 → 0 over strikeDuration).
    // Continues even in tail mode so a strike that started before tail mode
    // (e.g. when note duration < strikeDuration) completes naturally.
    double excitation = 0.0;
    if (strikeSamplesRemaining > 0) {
        const double progress = 1.0 - double(strikeSamplesRemaining) / double(strikeSamplesTotal);
        excitation = std::cos(progress * (TWO_PI * 0.25));  // cos(0..π/2) = 1..0
        strikeSamplesRemaining--;
    }

    // Drive resonators
    double output = 0.0;
    const int n = clamp(numModes, 2, MAX_MODES);
    for (int i = 0; i < n; i++) {
        const double in   = excitation * modes[i].amp;
        const double next = modes[i].cosCoeff * modes[i].s1
                          - modes[i].r2       * modes[i].s2
                          + in;
        modes[i].s2 = modes[i].s1;
        modes[i].s1 = next;
        output += next;
    }

    // Broadband noise burst — skin texture / stick attack.
    // Also continues through tail mode for the same reason as excitation above.
    if (noiseSamplesRemaining > 0) {
        output += noiseCurrentAmp * nextRandom();
        noiseCurrentAmp *= noiseDecayPerSample;
        noiseSamplesRemaining--;
    }

    return output;
}

// ---------------------------------------------------------------------------
// PRNG
// ---------------------------------------------------------------------------

double PercussionModel::nextRandom()
{
    rngState = rngState * 1664525u + 1013904223u;
    return double(rngState) / double(0xFFFFFFFFu) * 2.0 - 1.0;
}

// ---------------------------------------------------------------------------
// Setters
// ---------------------------------------------------------------------------

void PercussionModel::setStrikePosition(double p)  { strikePosition   = clamp(p, 0.0, 1.0); }
void PercussionModel::setStrikeDuration(double ms) { strikeDurationMs = clamp(ms, 0.5, 50.0); }
void PercussionModel::setInharmonicity(double v)   { inharmonicity    = clamp(v, 0.0, 1.0); }
void PercussionModel::setDecayTime(double s)       { decayTime        = clamp(s, 0.005, 15.0); }
void PercussionModel::setHighDecayRate(double r)   { highDecayRate    = clamp(r, 0.1, 5.0); }
void PercussionModel::setNumModes(int n)           { numModes         = clamp(n, 2, MAX_MODES); }
void PercussionModel::setNoiseGain(double g)       { noiseGain        = clamp(g, 0.0, 1.0); }
void PercussionModel::setPitchMultiplier(double m) { pitchMultiplier  = clamp(m, 0.25, 4.0); }

#include "bowedmodel.h"
#include <cmath>
#include <algorithm>

// ────────────────────────────────────────────────────────────────────────────
// Constructor
// ────────────────────────────────────────────────────────────────────────────

BowedModel::BowedModel(double sampleRate)
    : sampleRate(sampleRate)
{
    neckBuffer.assign(MAX_DELAY, 0.0);
    bridgeBuffer.assign(MAX_DELAY, 0.0);

    // String loss filter — same pole formula as STK Bowed
    stringFilterPole = 0.75 - (0.2 * 22050.0 / sampleRate);
    // b0 = gain × (1 − |pole|) ensures DC loop gain = gain = 0.95 < 1
    stringFilterB0 = stringFilterGain * (1.0 - std::fabs(stringFilterPole));

    initBodyFilters();
    setFrequency(220.0);
}

// ────────────────────────────────────────────────────────────────────────────
// Body filter initialisation (violin body — Esteban Maestre, 2011)
// Coefficients taken directly from STK Bowed.cpp constructor
// ────────────────────────────────────────────────────────────────────────────

void BowedModel::initBodyFilters()
{
    bodyFilters[0] = { 1.0,  1.5667,  0.3133, -0.5509, -0.3925 };
    bodyFilters[1] = { 1.0, -1.9537,  0.9542, -1.6357,  0.8697 };
    bodyFilters[2] = { 1.0, -1.6683,  0.8852, -1.7674,  0.8735 };
    bodyFilters[3] = { 1.0, -1.8585,  0.9653, -1.8498,  0.9516 };
    bodyFilters[4] = { 1.0, -1.9299,  0.9621, -1.9354,  0.9590 };
    bodyFilters[5] = { 1.0, -1.9800,  0.9888, -1.9867,  0.9923 };
}

// ────────────────────────────────────────────────────────────────────────────
// Frequency / delay line setup
// ────────────────────────────────────────────────────────────────────────────

void BowedModel::setFrequency(double freq)
{
    if (freq <= 0.0) freq = 20.0;
    lastPitch = freq;

    baseDelay = sampleRate / freq - 4.0;
    if (baseDelay < 2.0) baseDelay = 2.0;

    double beta = clamp(bowPosition, 0.001, 0.999);
    setBridgeDelay(baseDelay * beta);
    setNeckDelay(baseDelay * (1.0 - beta));
}

// ────────────────────────────────────────────────────────────────────────────
// Delay line helpers
// ────────────────────────────────────────────────────────────────────────────

void BowedModel::setNeckDelay(double delaySamples)
{
    size_t intDelay = static_cast<size_t>(delaySamples);
    neckAlpha    = delaySamples - static_cast<double>(intDelay);
    neckOmAlpha  = 1.0 - neckAlpha;
    neckOutPoint = (neckInPoint + MAX_DELAY - intDelay) % MAX_DELAY;
}

void BowedModel::setBridgeDelay(double delaySamples)
{
    size_t intDelay = static_cast<size_t>(delaySamples);
    bridgeAlpha    = delaySamples - static_cast<double>(intDelay);
    bridgeOmAlpha  = 1.0 - bridgeAlpha;
    bridgeOutPoint = (bridgeInPoint + MAX_DELAY - intDelay) % MAX_DELAY;
}

double BowedModel::neckTick(double input)
{
    neckBuffer[neckInPoint] = input;
    neckInPoint = (neckInPoint + 1) % MAX_DELAY;

    size_t next = (neckOutPoint + 1) % MAX_DELAY;
    neckLastOut = neckBuffer[neckOutPoint] * neckOmAlpha
                + neckBuffer[next]         * neckAlpha;
    neckOutPoint = (neckOutPoint + 1) % MAX_DELAY;
    return neckLastOut;
}

double BowedModel::bridgeTick(double input)
{
    bridgeBuffer[bridgeInPoint] = input;
    bridgeInPoint = (bridgeInPoint + 1) % MAX_DELAY;

    size_t next = (bridgeOutPoint + 1) % MAX_DELAY;
    bridgeLastOut = bridgeBuffer[bridgeOutPoint] * bridgeOmAlpha
                  + bridgeBuffer[next]           * bridgeAlpha;
    bridgeOutPoint = (bridgeOutPoint + 1) % MAX_DELAY;
    return bridgeLastOut;
}

// ────────────────────────────────────────────────────────────────────────────
// String loss filter
// ────────────────────────────────────────────────────────────────────────────

double BowedModel::stringFilterTick(double input)
{
    double out = stringFilterB0 * input + stringFilterPole * stringFilterState;
    stringFilterState = out;
    return out;
}

// ────────────────────────────────────────────────────────────────────────────
// Body filter cascade  (6 × BiQuad)
// ────────────────────────────────────────────────────────────────────────────

double BowedModel::bodyFilterTick(double input)
{
    double s = input;
    for (int i = 0; i < 6; ++i) {
        BiQuadState &f = bodyFilters[i];
        double y = f.b0 * s + f.b1 * f.x1 + f.b2 * f.x2
                             - f.a1 * f.y1 - f.a2 * f.y2;
        f.x2 = f.x1;  f.x1 = s;
        f.y2 = f.y1;  f.y1 = y;
        s = y;
    }
    return s;
}

// ────────────────────────────────────────────────────────────────────────────
// Nonlinear allpass filter (Michon)
//
// H(z) = (θ + z⁻¹) / (1 + θ·z⁻¹)
// Difference equation: y[n] = θ·(x[n] − y[n−1]) + x[n−1]
//
// θ is derived from the bridge signal using one of five modulation types,
// scaled by nlAmount and the ramp-in envelope.
// ────────────────────────────────────────────────────────────────────────────

double BowedModel::computeNLCoeff(double bridgeSample, double pitch) const
{
    double modSig = 0.0;

    switch (nlType) {
    case 0:
        // θ ← bridge signal directly
        modSig = bridgeSample;
        break;
    case 1:
        // θ ← leaky time-average of bridge signal
        // (nlLeakyState updated in tick; read here as const)
        modSig = nlLeaky;
        break;
    case 2:
        // θ ← bridge signal squared (always positive, range [0, 1])
        modSig = bridgeSample * bridgeSample;
        break;
    case 3:
        // θ ← sine at nlFreqMod Hz  (phase driven externally in tick)
        modSig = std::sin(nlSinePhase);
        break;
    case 4:
        // θ ← sine at fundamental pitch Hz  (phase driven externally in tick)
        modSig = std::sin(nlSinePhase);
        break;
    }

    // Scale by amount and ramp-in envelope; clamp for allpass stability
    double coeff = modSig * nlAmount * nlEnvGain;
    return clamp(coeff, -0.999, 0.999);
}

double BowedModel::nlFilterTick(double input, double coeff)
{
    // y[n] = θ·(x[n] − y[n−1]) + x[n−1]
    double output = coeff * (input - nlY1) + nlX1;
    nlX1 = input;
    nlY1 = output;
    return output;
}

// ────────────────────────────────────────────────────────────────────────────
// ADSR bow envelope
// ────────────────────────────────────────────────────────────────────────────

void BowedModel::adsrNoteOn()
{
    adsrState = AdsrState::Attack;
    bowActive = true;
}

void BowedModel::adsrNoteOff()
{
    adsrState = AdsrState::Release;
    bowActive = false;
}

double BowedModel::adsrTick()
{
    const double attackRate  = 1.0 / (ADSR_ATTACK_TIME  * sampleRate);
    const double decayRate   = 1.0 / (ADSR_DECAY_TIME   * sampleRate);
    const double releaseRate = 1.0 / (ADSR_RELEASE_TIME * sampleRate);

    switch (adsrState) {
    case AdsrState::Attack:
        adsrValue += attackRate;
        if (adsrValue >= 1.0) { adsrValue = 1.0; adsrState = AdsrState::Decay; }
        break;
    case AdsrState::Decay:
        adsrValue -= decayRate;
        if (adsrValue <= ADSR_SUSTAIN) { adsrValue = ADSR_SUSTAIN; adsrState = AdsrState::Sustain; }
        break;
    case AdsrState::Sustain:
        adsrValue = ADSR_SUSTAIN;
        break;
    case AdsrState::Release:
        adsrValue -= releaseRate;
        if (adsrValue <= 0.0) { adsrValue = 0.0; adsrState = AdsrState::Idle; }
        break;
    case AdsrState::Idle:
        adsrValue = 0.0;
        break;
    }
    return adsrValue;
}

// ────────────────────────────────────────────────────────────────────────────
// reset()
// ────────────────────────────────────────────────────────────────────────────

void BowedModel::reset(bool isLegato)
{
    if (!isLegato) {
        std::fill(neckBuffer.begin(),   neckBuffer.end(),   0.0);
        std::fill(bridgeBuffer.begin(), bridgeBuffer.end(), 0.0);
        neckLastOut       = 0.0;
        bridgeLastOut     = 0.0;
        stringFilterState = 0.0;

        for (int i = 0; i < 6; ++i) {
            bodyFilters[i].x1 = bodyFilters[i].x2 = 0.0;
            bodyFilters[i].y1 = bodyFilters[i].y2 = 0.0;
        }

        // Nonlinear filter state
        nlX1        = 0.0;
        nlY1        = 0.0;
        nlSinePhase = 0.0;
        nlLeaky     = 0.0;
        nlEnvGain   = 0.0;
        nlEnvActive = false;

        adsrValue = 0.0;
        adsrState = AdsrState::Idle;
        lastPitch = -1.0;
        bowActive = false;
    }
    // Legato: preserve delay-line and NL filter state so the string keeps
    // vibrating across the note boundary. NL envelope keeps its current level.
}

// ────────────────────────────────────────────────────────────────────────────
// tick() — generate one output sample
// ────────────────────────────────────────────────────────────────────────────

double BowedModel::tick(double pitch, double noteProgress,
                        bool isLegato, bool tailMode)
{
    // ── Pitch update ─────────────────────────────────────────────────────────
    if (pitch != lastPitch && pitch > 0.0) {
        setFrequency(pitch);
    }

    // ── Bow envelope management ───────────────────────────────────────────────
    if (tailMode) {
        if (adsrState != AdsrState::Release && adsrState != AdsrState::Idle)
            adsrNoteOff();
    } else if (noteProgress < 0.001) {
        if (isLegato) {
            adsrValue = ADSR_SUSTAIN;
            adsrState = AdsrState::Sustain;
            bowActive = true;
        } else {
            adsrNoteOn();
        }
    } else if (noteProgress > 0.98) {
        if (adsrState == AdsrState::Sustain || adsrState == AdsrState::Decay)
            adsrNoteOff();
    }

    // ── Nonlinearity ramp-in envelope ────────────────────────────────────────
    // Arm the ramp when a new (non-legato) note starts
    if (noteProgress < 0.001 && !isLegato && !tailMode) {
        nlEnvGain   = 0.0;
        nlEnvActive = (nlAmount > 0.0);
    }
    if (nlEnvActive) {
        double rate = 1.0 / (nlAttack * sampleRate);
        nlEnvGain += rate;
        if (nlEnvGain >= 1.0) {
            nlEnvGain   = 1.0;
            nlEnvActive = false;
        }
    }

    // ── Advance NL oscillator phase (types 3 and 4) ──────────────────────────
    double freqForPhase = (nlType == 4) ? pitch : nlFreqMod;
    nlSinePhase += TWO_PI * freqForPhase / sampleRate;
    if (nlSinePhase >= TWO_PI) nlSinePhase -= TWO_PI;

    // ── Update leaky integrator (type 1) ─────────────────────────────────────
    // Updated with previous bridgeLastOut so computeNLCoeff can read it
    nlLeaky = 0.999 * nlLeaky + 0.001 * bridgeLastOut;

    // ── Bow excitation ───────────────────────────────────────────────────────
    const double slope  = 5.0 - 4.0 * bowPressure;          // [5→1]
    const double maxVel = 0.03 + 0.2 * bowVelocity;
    const double bowAmp = adsrTick() * maxVel;

    const double nutReflection    = -neckLastOut;
    const double bridgeReflection = -stringFilterTick(bridgeLastOut);
    const double stringVel        = bridgeReflection + nutReflection;
    const double deltaV           = bowAmp - stringVel;

    double newVelocity = 0.0;
    if (!tailMode) {
        double x       = deltaV * slope + 0.001;
        double friction = std::pow(std::fabs(x) + 0.75, -4.0);
        friction = clamp(friction, 0.01, 0.98);
        newVelocity = deltaV * friction;
    }

    neckTick(bridgeReflection + newVelocity);
    bridgeTick(nutReflection  + newVelocity);

    // ── Nonlinear allpass (Michon) ────────────────────────────────────────────
    // Applied to the bridge output before the body filter.
    // Operates inside the perceptual output path; feeding back through the
    // string filter on the next sample creates the waveguide colouration.
    double bridgeSig = bridgeLastOut;
    if (nlAmount > 0.0 && nlEnvGain > 0.0) {
        double theta = computeNLCoeff(bridgeSig, pitch);
        bridgeSig    = nlFilterTick(bridgeSig, theta);
    }

    // ── Body resonance → output ───────────────────────────────────────────────
    double output = bodyFilterTick(bridgeSig) * 0.1248;
    return clamp(output, -1.0, 1.0);
}

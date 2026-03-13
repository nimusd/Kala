#include "saxophonemodel.h"
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────────────────────────────────────

SaxophoneModel::SaxophoneModel(double sampleRate)
    : sampleRate(sampleRate)
{
    delay0Buffer.assign(MAX_DELAY + 1, 0.0);
    delay1Buffer.assign(MAX_DELAY + 1, 0.0);
    setFrequency(220.0);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Reset
// ─────────────────────────────────────────────────────────────────────────────

void SaxophoneModel::reset(bool isLegato)
{
    if (!isLegato) {
        std::fill(delay0Buffer.begin(), delay0Buffer.end(), 0.0);
        std::fill(delay1Buffer.begin(), delay1Buffer.end(), 0.0);
        delay0InPoint  = 0;
        delay0OutPoint = 0;
        delay0LastOut  = 0.0;
        delay1InPoint  = 0;
        delay1OutPoint = 0;
        delay1LastOut  = 0.0;
        oneZeroX1      = 0.0;

        // Nonlinear filter state
        nlX1        = 0.0;
        nlY1        = 0.0;
        nlSinePhase = 0.0;
        nlLeaky     = 0.0;
        nlEnvGain   = 0.0;
        nlEnvActive = false;
    }
    vibratoPhase = 0.0;
    lastPitch    = -1.0;  // force setFrequency() on first tick
}

// ─────────────────────────────────────────────────────────────────────────────
//  Main sample generator
// ─────────────────────────────────────────────────────────────────────────────

double SaxophoneModel::tick(double pitch, double noteProgress,
                             bool isLegato, bool tailMode)
{
    // Tail mode: stop blowing; let whatever residual energy drain naturally.
    if (tailMode) {
        double d0   = delay0LastOut;
        double d1   = delay1LastOut;
        double temp = -0.95 * oneZeroTick(d0);
        double out  = temp - d1;
        delay1Tick(temp);
        delay0Tick(-temp);  // no breath, just closed-end reflection
        return out * 0.3;
    }

    // Update delay-line lengths when pitch changes (handles portamento / curves).
    if (lastPitch <= 0.0 || std::fabs(pitch - lastPitch) > lastPitch * 0.0003) {
        setFrequency(pitch);
        lastPitch = pitch;
    }

    // ── Nonlinearity ramp-in envelope ────────────────────────────────────────
    if (noteProgress < 0.001 && !isLegato) {
        nlEnvGain   = 0.0;
        nlEnvActive = (nlAmount > 0.0);
    }
    if (nlEnvActive) {
        nlEnvGain += 1.0 / (nlAttack * sampleRate);
        if (nlEnvGain >= 1.0) { nlEnvGain = 1.0; nlEnvActive = false; }
    }

    // ── Advance NL oscillator phase (types 3 and 4) ──────────────────────────
    double freqForPhase = (nlType == 4) ? pitch : nlFreqMod;
    nlSinePhase += TWO_PI * freqForPhase / sampleRate;
    if (nlSinePhase >= TWO_PI) nlSinePhase -= TWO_PI;

    // ── Update leaky integrator (type 1) ─────────────────────────────────────
    nlLeaky = 0.999 * nlLeaky + 0.001 * delay0LastOut;

    // Build breath pressure (envelope + noise + vibrato).
    double env    = breathEnvelope(noteProgress, isLegato);
    double breathP = breathPressure * env;

    breathP += breathP * noiseGain * nextRandom();

    if (vibratoGain > 0.0)
        breathP += breathP * vibratoGain * std::sin(vibratoPhase);
    vibratoPhase += TWO_PI * vibratoFreq / sampleRate;
    if (vibratoPhase >= TWO_PI) vibratoPhase -= TWO_PI;

    // Read previous delay outputs (both before any writes — matches STK tick order).
    double d0 = delay0LastOut;
    double d1 = delay1LastOut;

    // ── Nonlinear allpass (Michon) — applied to open-end feedback ────────────
    // Inserting inside the waveguide loop (before the OneZero) creates genuine
    // spectral enrichment, not just output shaping.
    if (nlAmount > 0.0 && nlEnvGain > 0.0) {
        double theta = computeNLCoeff(d0, pitch);
        d0 = nlFilterTick(d0, theta);
    }

    // Open-end reflection: lossy (-0.95) + low-pass (OneZero).
    double temp = -0.95 * oneZeroTick(d0);

    // Pressure at the blow point (sum of reflections from both terminations).
    double output = temp - d1;

    // Differential pressure across the reed.
    double pressureDiff = breathP - output;

    // Reed reflection coefficient (nonlinear spring, Smith 1986).
    double reedCoeff = reedTableTick(pressureDiff);

    // Write new values into both delay lines.
    delay1Tick(temp);
    delay0Tick(breathP - (pressureDiff * reedCoeff) - temp);

    return output * 0.3;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Frequency / delay-line setup
// ─────────────────────────────────────────────────────────────────────────────

void SaxophoneModel::setFrequency(double freq)
{
    if (freq <= 0.0) freq = 20.0;
    lastPitch = freq;

    // OneZero phase delay = 0.5 samples; subtract it plus one latency sample.
    double totalDelay = sampleRate / freq - 1.5;
    if (totalDelay < 2.0) totalDelay = 2.0;
    totalDelaySamples = totalDelay;

    double pos = clamp(blowPosition, 0.001, 0.999);
    setDelay0((1.0 - pos) * totalDelay);  // long segment (open end)
    setDelay1(pos          * totalDelay); // short segment (reed end)
}

void SaxophoneModel::setBlowPosition(double pos)
{
    blowPosition = clamp(pos, 0.001, 0.999);
    if (totalDelaySamples > 0.0) {
        setDelay0((1.0 - blowPosition) * totalDelaySamples);
        setDelay1(blowPosition          * totalDelaySamples);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Delay line 0 (long, open end)
// ─────────────────────────────────────────────────────────────────────────────

void SaxophoneModel::setDelay0(double delaySamples)
{
    double outPointer = static_cast<double>(delay0InPoint) - delaySamples;
    const double bufSize = static_cast<double>(delay0Buffer.size());
    while (outPointer < 0.0) outPointer += bufSize;

    delay0OutPoint = static_cast<size_t>(outPointer);
    delay0Alpha    = outPointer - static_cast<double>(delay0OutPoint);
    delay0OmAlpha  = 1.0 - delay0Alpha;
    if (delay0OutPoint >= delay0Buffer.size()) delay0OutPoint = 0;
}

double SaxophoneModel::delay0Tick(double input)
{
    delay0Buffer[delay0InPoint] = input;
    if (++delay0InPoint >= delay0Buffer.size()) delay0InPoint = 0;

    size_t next = delay0OutPoint + 1;
    if (next >= delay0Buffer.size()) next = 0;
    delay0LastOut = delay0Buffer[delay0OutPoint] * delay0OmAlpha
                  + delay0Buffer[next]           * delay0Alpha;

    if (++delay0OutPoint >= delay0Buffer.size()) delay0OutPoint = 0;
    return delay0LastOut;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Delay line 1 (short, reed end)
// ─────────────────────────────────────────────────────────────────────────────

void SaxophoneModel::setDelay1(double delaySamples)
{
    double outPointer = static_cast<double>(delay1InPoint) - delaySamples;
    const double bufSize = static_cast<double>(delay1Buffer.size());
    while (outPointer < 0.0) outPointer += bufSize;

    delay1OutPoint = static_cast<size_t>(outPointer);
    delay1Alpha    = outPointer - static_cast<double>(delay1OutPoint);
    delay1OmAlpha  = 1.0 - delay1Alpha;
    if (delay1OutPoint >= delay1Buffer.size()) delay1OutPoint = 0;
}

double SaxophoneModel::delay1Tick(double input)
{
    delay1Buffer[delay1InPoint] = input;
    if (++delay1InPoint >= delay1Buffer.size()) delay1InPoint = 0;

    size_t next = delay1OutPoint + 1;
    if (next >= delay1Buffer.size()) next = 0;
    delay1LastOut = delay1Buffer[delay1OutPoint] * delay1OmAlpha
                  + delay1Buffer[next]           * delay1Alpha;

    if (++delay1OutPoint >= delay1Buffer.size()) delay1OutPoint = 0;
    return delay1LastOut;
}

// ─────────────────────────────────────────────────────────────────────────────
//  OneZero low-pass filter (zero at −1)
//  y[n] = 0.5·x[n] + 0.5·x[n−1]   (phase delay = 0.5 samples at all frequencies)
// ─────────────────────────────────────────────────────────────────────────────

double SaxophoneModel::oneZeroTick(double input)
{
    double out = 0.5 * input + 0.5 * oneZeroX1;
    oneZeroX1  = input;
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Reed table (Smith 1986)
//  output = clamp(offset + slope · input, −1, 1)
//  offset ∈ [0.4, 1.0]: larger = more open reed tip
//  slope  ∈ [0.1, 0.5]: larger = stiffer reed, harder sound
// ─────────────────────────────────────────────────────────────────────────────

double SaxophoneModel::reedTableTick(double input) const
{
    double offset = 0.4 + 0.6 * reedAperture;   // [0.4, 1.0]; STK default 0.7
    double slope  = 0.1 + 0.4 * reedStiffness;  // [0.1, 0.5]; STK default 0.3
    double out    = offset + slope * input;
    if (out >  1.0) out =  1.0;
    if (out < -1.0) out = -1.0;
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Nonlinear allpass filter (Michon)
//  H(z) = (θ + z⁻¹) / (1 + θ·z⁻¹)
//  Difference equation: y[n] = θ·(x[n] − y[n−1]) + x[n−1]
//
//  θ is derived from the open-end signal using one of five modulation types,
//  scaled by nlAmount and the ramp-in envelope.
// ─────────────────────────────────────────────────────────────────────────────

double SaxophoneModel::computeNLCoeff(double signal, double pitch) const
{
    double modSig = 0.0;

    switch (nlType) {
    case 0:
        modSig = signal;                       // signal directly
        break;
    case 1:
        modSig = nlLeaky;                      // leaky time-average
        break;
    case 2:
        modSig = signal * signal;              // signal squared
        break;
    case 3:
        modSig = std::sin(nlSinePhase);        // sine at nlFreqMod Hz
        break;
    case 4:
        modSig = std::sin(nlSinePhase);        // sine at pitch Hz
        break;
    }

    double coeff = modSig * nlAmount * nlEnvGain;
    return clamp(coeff, -0.999, 0.999);
}

double SaxophoneModel::nlFilterTick(double input, double coeff)
{
    double output = coeff * (input - nlY1) + nlX1;
    nlX1 = input;
    nlY1 = output;
    return output;
}

// ─────────────────────────────────────────────────────────────────────────────
//  White noise (LCG, returns uniform in [−1, 1])
// ─────────────────────────────────────────────────────────────────────────────

double SaxophoneModel::nextRandom()
{
    rngState = rngState * 1664525u + 1013904223u;  // Knuth multiplier
    return static_cast<double>(static_cast<int>(rngState)) / 2147483648.0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Breath envelope (noteProgress-based)
//  Attack: ramp from 0 to 1 over first 8% of note (skipped for legato)
//  Release: ramp from 1 to 0 over last 5% of note
// ─────────────────────────────────────────────────────────────────────────────

double SaxophoneModel::breathEnvelope(double noteProgress, bool isLegato) const
{
    constexpr double ATK = 0.08;
    constexpr double REL = 0.05;

    if (!isLegato && noteProgress < ATK)
        return noteProgress / ATK;

    if (noteProgress > (1.0 - REL))
        return (1.0 - noteProgress) / REL;

    return 1.0;
}

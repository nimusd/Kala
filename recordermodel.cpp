#include "recordermodel.h"
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────

RecorderModel::RecorderModel(double sampleRate)
    : sampleRate(sampleRate)
    , breathPressure(0.81)
    , jetRatio(0.35)
    , noiseGain(0.015)
    , vibratoFreq(0.0)
    , vibratoGain(0.0)
    , endReflection(0.73)
    , jetReflection(0.59)
    , filterPole(0.7 - 0.1 * 2050.0 / sampleRate)  // STK formula
    , rngState(12345u)
{
    boreBuffer.assign(MAX_DELAY + 1, 0.0);
    jetBuffer .assign(MAX_DELAY + 1, 0.0);
}

// ─────────────────────────────────────────────
//  Reset
// ─────────────────────────────────────────────

void RecorderModel::reset(bool isLegato)
{
    if (!isLegato) {
        std::fill(boreBuffer.begin(), boreBuffer.end(), 0.0);
        std::fill(jetBuffer .begin(), jetBuffer .end(), 0.0);
        boreInPoint  = 0;
        boreOutPoint = 0;
        boreLastOut  = 0.0;
        jetInPoint   = 0;
        jetOutPoint  = 0;
        filterState  = 0.0;
        dcX1 = 0.0;
        dcY1 = 0.0;
    }
    vibratoPhase = 0.0;
    lastPitch    = -1.0;     // force setFrequency() on first tick
}

// ─────────────────────────────────────────────
//  Main sample generator
// ─────────────────────────────────────────────

double RecorderModel::tick(double pitch, double noteProgress,
                           bool isLegato, bool tailMode)
{
    // --- Tail mode: wind instrument stops sounding the moment the player stops blowing.
    //     The jet disappears, so bypass the entire feedback algorithm and just drain
    //     whatever residual energy remains in the bore delay buffer to silence.
    if (tailMode) {
        boreLastOut = boreTick(0.0);
        return 0.2 * boreLastOut;
    }

    // --- Update frequency when pitch changes (handles portamento / pitch curves) ---
    if (lastPitch <= 0.0 || std::fabs(pitch - lastPitch) > lastPitch * 0.0003) {
        setFrequency(pitch);
        lastPitch = pitch;
    }

    // --- Build breath pressure (excitation) ---
    double env = breathEnvelope(noteProgress, isLegato);
    double breathP = breathPressure * env;

    // Add breath noise
    breathP += breathP * noiseGain * nextRandom();

    // Add vibrato
    if (vibratoGain > 0.0) {
        breathP += vibratoGain * 0.05 * std::sin(vibratoPhase);
    }
    vibratoPhase += TWO_PI * vibratoFreq / sampleRate;
    if (vibratoPhase >= TWO_PI) vibratoPhase -= TWO_PI;

    // --- Digital waveguide loop (matches STK Flute::tick exactly) ---

    // Feedback from bore end (previous bore delay output, one-pole filtered)
    double temp = -filterTick(boreLastOut);

    // Jet feedback mixing
    double pressureDiff = breathP - (jetReflection * temp);

    // Jet delay
    pressureDiff = jetDelayTick(pressureDiff);

    // Jet nonlinearity + DC block + bore-end reflection
    // Coefficients 1.2 and 1.02 match STK Flute (2020 revision by GPS)
    pressureDiff = dcTick(jetTable(pressureDiff)) * 1.2
                 + (endReflection * temp) * 1.02;

    // Bore delay — store UNSCALED output for feedback, scale only the audio output
    boreLastOut = boreTick(pressureDiff);

    return 0.2 * boreLastOut;
}

// ─────────────────────────────────────────────
//  Parameter setters that require recomputation
// ─────────────────────────────────────────────

void RecorderModel::setJetRatio(double r)
{
    jetRatio = clamp(r, 0.05, 0.60);
    if (currentBoreDelaySamps > 0.0)
        setJetDelayLen(currentBoreDelaySamps * jetRatio);
}

// ─────────────────────────────────────────────
//  Frequency / delay tuning
// ─────────────────────────────────────────────

void RecorderModel::setFrequency(double freq)
{
    // STK: slight overblowing correction
    double playFreq = freq * 0.96666;
    if (playFreq < 10.0) playFreq = 10.0;

    // Bore delay length: compensate for one-pole phase delay and one-sample latency
    double boreSamps = sampleRate / playFreq - filterPhaseDelay(playFreq) - 1.0;
    boreSamps = clamp(boreSamps, 1.0, static_cast<double>(MAX_DELAY - 2));

    currentBoreDelaySamps = boreSamps;
    setBoreDelay(boreSamps);
    setJetDelayLen(boreSamps * jetRatio);
}

// ─────────────────────────────────────────────
//  Bore delay line
// ─────────────────────────────────────────────

void RecorderModel::setBoreDelay(double delaySamples)
{
    // Compute output pointer from current input pointer and requested delay
    double outPointer = static_cast<double>(boreInPoint) - delaySamples;
    const double bufSize = static_cast<double>(boreBuffer.size());
    while (outPointer < 0.0) outPointer += bufSize;

    boreOutPoint = static_cast<size_t>(outPointer);
    boreAlpha    = outPointer - static_cast<double>(boreOutPoint);
    boreOmAlpha  = 1.0 - boreAlpha;

    if (boreOutPoint >= boreBuffer.size()) boreOutPoint = 0;
}

double RecorderModel::boreTick(double input)
{
    boreBuffer[boreInPoint] = input;
    if (++boreInPoint >= boreBuffer.size()) boreInPoint = 0;

    // Linear interpolation between outPoint and outPoint+1
    size_t next = boreOutPoint + 1;
    if (next >= boreBuffer.size()) next = 0;
    double out = boreBuffer[boreOutPoint] * boreOmAlpha
               + boreBuffer[next]         * boreAlpha;

    if (++boreOutPoint >= boreBuffer.size()) boreOutPoint = 0;
    return out;
}

// ─────────────────────────────────────────────
//  Jet delay line
// ─────────────────────────────────────────────

void RecorderModel::setJetDelayLen(double delaySamples)
{
    double outPointer = static_cast<double>(jetInPoint) - delaySamples;
    const double bufSize = static_cast<double>(jetBuffer.size());
    while (outPointer < 0.0) outPointer += bufSize;

    jetOutPoint = static_cast<size_t>(outPointer);
    jetAlpha    = outPointer - static_cast<double>(jetOutPoint);
    jetOmAlpha  = 1.0 - jetAlpha;

    if (jetOutPoint >= jetBuffer.size()) jetOutPoint = 0;
}

double RecorderModel::jetDelayTick(double input)
{
    jetBuffer[jetInPoint] = input;
    if (++jetInPoint >= jetBuffer.size()) jetInPoint = 0;

    size_t next = jetOutPoint + 1;
    if (next >= jetBuffer.size()) next = 0;
    double out = jetBuffer[jetOutPoint] * jetOmAlpha
               + jetBuffer[next]        * jetAlpha;

    if (++jetOutPoint >= jetBuffer.size()) jetOutPoint = 0;
    return out;
}

// ─────────────────────────────────────────────
//  One-pole lowpass filter
// ─────────────────────────────────────────────

double RecorderModel::filterPhaseDelay(double frequency) const
{
    // H(z) = a0 / (1 − pole·z⁻¹)
    // Phase delay (samples) = atan2(pole·sin ω, 1 − pole·cos ω) / ω
    // where ω = 2π·f / sr
    double w = TWO_PI * frequency / sampleRate;
    if (w < 1e-10) return 0.0;
    double phase = std::atan2(filterPole * std::sin(w),
                              1.0 - filterPole * std::cos(w));
    return phase / w;
}

double RecorderModel::filterTick(double input)
{
    // y[n] = a0·x[n] + pole·y[n−1],   a0 = 1 − |pole|
    double a0 = 1.0 - std::fabs(filterPole);
    filterState = a0 * input + filterPole * filterState;
    return filterState;
}

// ─────────────────────────────────────────────
//  DC blocker
// ─────────────────────────────────────────────

double RecorderModel::dcTick(double input)
{
    // y[n] = x[n] − x[n−1] + DC_COEFF·y[n−1]
    double output = input - dcX1 + DC_COEFF * dcY1;
    dcX1 = input;
    dcY1 = output;
    return output;
}

// ─────────────────────────────────────────────
//  Jet nonlinearity
// ─────────────────────────────────────────────

double RecorderModel::jetTable(double x)
{
    // Polynomial sigmoid approximation (Cook)
    double out = x * (x * x - 1.0);
    if (out >  1.0) out =  1.0;
    if (out < -1.0) out = -1.0;
    return out;
}

// ─────────────────────────────────────────────
//  White noise (LCG, returns uniform in [−1, 1])
// ─────────────────────────────────────────────

double RecorderModel::nextRandom()
{
    rngState = rngState * 1664525u + 1013904223u;  // Knuth
    return static_cast<double>(static_cast<int>(rngState)) / 2147483648.0;
}

// ─────────────────────────────────────────────
//  Breath envelope (noteProgress-based)
// ─────────────────────────────────────────────

double RecorderModel::breathEnvelope(double noteProgress, bool isLegato) const
{
    // Attack: ramp from 0 to 1 over first 8% of note (skipped for legato)
    // Release: ramp from 1 to 0 over last 5% of note
    constexpr double ATK = 0.08;
    constexpr double REL = 0.05;

    if (!isLegato && noteProgress < ATK)
        return noteProgress / ATK;

    if (noteProgress > (1.0 - REL))
        return (1.0 - noteProgress) / REL;

    return 1.0;
}

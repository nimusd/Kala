#include "saxophonemodel2.h"
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────────────────────────────────────

SaxophoneModel2::SaxophoneModel2(double sampleRate)
    : sampleRate(sampleRate)
{
    delay0Buffer.assign(MAX_DELAY + 1, 0.0);
    delay1Buffer.assign(MAX_DELAY + 1, 0.0);
    updateVocalTractCoeffs();
    setFrequency(220.0);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Reset
// ─────────────────────────────────────────────────────────────────────────────

void SaxophoneModel2::reset(bool isLegato)
{
    if (!isLegato) {
        std::fill(delay0Buffer.begin(), delay0Buffer.end(), 0.0);
        std::fill(delay1Buffer.begin(), delay1Buffer.end(), 0.0);
        delay0InPoint  = delay0OutPoint = 0;
        delay0LastOut  = 0.0;
        delay1InPoint  = delay1OutPoint = 0;
        delay1LastOut  = 0.0;
        oneZeroX1      = 0.0;

        nlX1 = nlY1 = 0.0;
        nlSinePhase = nlLeaky = 0.0;
        nlEnvGain   = 0.0;
        nlEnvActive = false;

        vtX1 = vtX2 = vtY1 = vtY2 = 0.0;

        m_onsetSamples = 0;
        m_tailDecay    = 1.0;
    }
    vibratoPhase = 0.0;
    growlPhase   = 0.0;
    lastPitch    = -1.0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Main sample generator
// ─────────────────────────────────────────────────────────────────────────────

double SaxophoneModel2::tick(double pitch, double noteProgress,
                              bool isLegato, bool tailMode)
{
    // Tail mode: stop blowing; let the bore ring down naturally.
    // Apply per-sample exponential decay so the bore becomes inaudible in ~15 ms.
    // This eliminates the square-wave artifact from the odd-harmonic bore ring-down
    // without touching the internal bore state (no mode-hop click).
    // The Note Tail container continues to cleanly fade the HG's ongoing tone.
    if (tailMode) {
        m_tailDecay *= 0.99;
        double d0   = delay0LastOut;
        double d1   = delay1LastOut;
        double temp = -bellReflection * oneZeroTick(d0);
        double out  = temp - d1;
        delay1Tick(temp);
        delay0Tick(-temp);
        return out * 0.3 * m_tailDecay;
    }

    // Update delay-line lengths when pitch changes (handles portamento/curves).
    if (lastPitch <= 0.0 || std::fabs(pitch - lastPitch) > lastPitch * 0.0003) {
        setFrequency(pitch);
        lastPitch = pitch;
    }

    // ── Pre-charge: seed delay lines at the very first sample of a new note ──
    // A waveguide starting from silence needs many cycles to build oscillation —
    // that gradual build-up is what makes it sound bowed.  Seeding the bore with
    // a low-amplitude half-sine gives the reed immediate back-pressure so it
    // starts producing output from sample 1.
    if (!isLegato && m_onsetSamples == 0 && totalDelaySamples > 1.0) {
        double amp0 = breathPressure * 0.07;
        size_t N0   = static_cast<size_t>((1.0 - blowPosition) * totalDelaySamples);
        size_t N1   = static_cast<size_t>(blowPosition          * totalDelaySamples);

        // Sine wave at the fundamental period (π/2 phase offset so the first
        // read position is at the wave's peak, not its zero-crossing).
        // A monotonic/DC pre-charge causes output = temp − d1 → 0 after
        // delay1_length samples; a periodic pre-charge avoids that cancellation.
        const double phaseStep = TWO_PI / totalDelaySamples;
        const double phase0    = TWO_PI * 0.25;          // start at sin peak
        const double phase1    = phase0 + N0 * phaseStep; // delay1 picks up where delay0 left off

        for (size_t i = 0; i < N0 && i < MAX_DELAY; ++i) {
            double v = amp0 * std::sin(phase0 + i * phaseStep);
            delay0Buffer[(delay0OutPoint + i) % delay0Buffer.size()] = v;
        }
        for (size_t i = 0; i < N1 && i < MAX_DELAY; ++i) {
            double v = amp0 * 0.4 * std::sin(phase1 + i * phaseStep);
            delay1Buffer[(delay1OutPoint + i) % delay1Buffer.size()] = v;
        }

        // Refresh lastOut so first tick() reads the seeded values, not zero.
        {
            size_t nx = (delay0OutPoint + 1) % delay0Buffer.size();
            delay0LastOut = delay0Buffer[delay0OutPoint] * delay0OmAlpha
                          + delay0Buffer[nx]             * delay0Alpha;
        }
        {
            size_t nx = (delay1OutPoint + 1) % delay1Buffer.size();
            delay1LastOut = delay1Buffer[delay1OutPoint] * delay1OmAlpha
                          + delay1Buffer[nx]             * delay1Alpha;
        }
    }

    // Advance onset sample counter (non-legato only).
    if (!isLegato) m_onsetSamples++;

    // ── Nonlinearity ramp-in ──────────────────────────────────────────────────
    if (noteProgress < 0.001 && !isLegato) {
        nlEnvGain   = 0.0;
        nlEnvActive = (nlAmount > 0.0);
    }
    if (nlEnvActive) {
        nlEnvGain += 1.0 / (nlAttack * sampleRate);
        if (nlEnvGain >= 1.0) { nlEnvGain = 1.0; nlEnvActive = false; }
    }

    // ── Advance oscillator phases ─────────────────────────────────────────────
    double freqForNL = (nlType == 4) ? pitch : nlFreqMod;
    nlSinePhase += TWO_PI * freqForNL / sampleRate;
    if (nlSinePhase >= TWO_PI) nlSinePhase -= TWO_PI;

    growlPhase += TWO_PI * growlFreq / sampleRate;
    if (growlPhase >= TWO_PI) growlPhase -= TWO_PI;

    // ── Leaky integrator (NL type 1) ──────────────────────────────────────────
    nlLeaky = 0.999 * nlLeaky + 0.001 * delay0LastOut;

    // ── Build breath pressure ─────────────────────────────────────────────────
    // Full breathPressure from sample 1 — a ramp that starts at zero starves
    // the bore and prevents self-oscillation.  The attack shape comes from the
    // reed transient and the chiff burst below.
    //
    // No release ramp on breathP: ramping it down causes the bore to cross the
    // self-oscillation threshold (~0.416) and mode-hop, producing a click that
    // is worse than the artifact it was meant to cure.  Instead, the output
    // sample is faded via a cosine window (see below) while the bore continues
    // running at full pressure — no threshold crossing, no mode-hop.
    double breathP = breathPressure;

    // ── Onset envelope: chiff burst + tongue-release overpressure ─────────────
    // These two effects combine to create the characteristic blown attack of a
    // reed instrument: a brief noise transient (chiff) followed by immediate
    // full amplitude, as opposed to the gradual swell of a bowed string.
    if (!isLegato) {
        const double onsetWindow = 0.015 * sampleRate;   // 15 ms window
        if (m_onsetSamples <= static_cast<int>(onsetWindow)) {
            double t = static_cast<double>(m_onsetSamples) / onsetWindow;

            // Chiff: noise burst decays over ~15 ms (peaks at note-on)
            if (attackChiffGain > 0.0) {
                double chiffEnv = std::exp(-t * 4.0);
                breathP += attackChiffGain * 0.03 * chiffEnv * nextRandom();
            }

            // Overpressure: tongue-release pressure spike, very fast decay (~2 ms)
            if (attackOverpressure > 0.0) {
                double opEnv = std::exp(-t * 18.0);
                breathP *= (1.0 + attackOverpressure * opEnv);
            }
        }
    }

    breathP += breathP * noiseGain * nextRandom();

    // ── Growl: AM modulation (player humming into instrument) ─────────────────
    if (growlDepth > 0.0)
        breathP *= (1.0 + growlDepth * std::sin(growlPhase));

    // ── Vibrato ───────────────────────────────────────────────────────────────
    double vibratoVal = std::sin(vibratoPhase);
    vibratoPhase += TWO_PI * vibratoFreq / sampleRate;
    if (vibratoPhase >= TWO_PI) vibratoPhase -= TWO_PI;

    // Breath tremolo component (jawVibratoBlend < 1): modulates breathP
    if (vibratoGain > 0.0 && jawVibratoBlend < 1.0)
        breathP += breathP * vibratoGain * (1.0 - jawVibratoBlend) * vibratoVal;

    // Clamp breathP to the reed model's valid range.  Contributions from noise,
    // growl, and overpressure can push it above 1.0 at dynamics peaks, which
    // causes the bore to mode-hop to the octave.
    if (breathP > 1.0) breathP = 1.0;
    if (breathP < 0.0) breathP = 0.0;

    // ── Read previous delay outputs (both before any write — STK tick order) ──
    double d0 = delay0LastOut;
    double d1 = delay1LastOut;

    // ── Nonlinear allpass (Michon) in the waveguide loop ─────────────────────
    if (nlAmount > 0.0 && nlEnvGain > 0.0) {
        double theta = computeNLCoeff(d0, pitch);
        d0 = nlFilterTick(d0, theta);
    }

    // ── Open-end (bell) reflection: lossy + low-pass ─────────────────────────
    double temp = -bellReflection * oneZeroTick(d0);

    // ── Junction pressure (STK Saxofony formulation) ─────────────────────────
    // output = bore pressure at the reed junction, used for VT and audio output.
    // pressureDiff = breathP - (-(return wave)) = breathP + d1 (STK sign convention).
    // Using breathP - output introduced the bell-reflection term into pressureDiff,
    // which shifted the reed's operating point and prevented self-oscillation.
    double output       = temp - d1;              // junction pressure (for output / VT)
    double pressureDiff = breathP + d1;           // STK: differential pressure at reed

    // ── Reed: jaw vibrato modulates aperture (blend mode) ────────────────────
    double effectiveAperture = reedAperture;
    if (vibratoGain > 0.0 && jawVibratoBlend > 0.0) {
        effectiveAperture += jawVibratoBlend * vibratoGain * vibratoVal * 0.15;
        if (effectiveAperture < 0.0) effectiveAperture = 0.0;
        if (effectiveAperture > 1.0) effectiveAperture = 1.0;
    }

    double reedCoeff = reedTableTick(pressureDiff, effectiveAperture);

    // ── Write new values into both delay lines (STK Saxofony junction) ───────
    // delay1 receives the bell reflection (traveling back toward reed).
    // delay0 receives the reed-modulated pressure traveling toward the bell:
    //   pipeIn = reedCoeff * pressDiff + d1  (STK formula)
    delay1Tick(temp);
    delay0Tick(reedCoeff * pressureDiff + d1);

    // ── Pressure-scaled output ────────────────────────────────────────────────
    // Output amplitude scales with breath pressure so dynamics affect the
    // signal level as well as the waveguide character.
    double pressureScale = 0.5 + 0.5 * breathPressure;
    double outSample = output * 0.3 * pressureScale;

    // ── Vocal tract as output formant ─────────────────────────────────────────
    // Previously the vocal tract fed back into breathP (creating regenerative
    // resonance that made the instrument sound bowed/sustained).  Instead we
    // apply it as a spectral formant on the output: the player's vocal cavity
    // shapes the sound that exits the bell without locking the waveguide into
    // a resonant feedback loop.
    if (vocalTractGain > 0.0) {
        double vtFormed = vocalTractFilter(output);   // filter unscaled bore signal
        outSample += vocalTractGain * 0.25 * pressureScale * vtFormed;
    }

    return outSample;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Frequency / delay-line setup
// ─────────────────────────────────────────────────────────────────────────────

void SaxophoneModel2::setFrequency(double freq)
{
    if (freq <= 0.0) freq = 20.0;
    lastPitch = freq;

    double totalDelay = sampleRate / freq - 1.5;  // subtract OneZero (0.5) + 1 latency
    if (totalDelay < 2.0) totalDelay = 2.0;
    totalDelaySamples = totalDelay;

    double pos = clamp(blowPosition, 0.001, 0.999);
    setDelay0((1.0 - pos) * totalDelay);  // long segment → open end
    setDelay1(pos          * totalDelay); // short segment → reed
}

void SaxophoneModel2::setBlowPosition(double pos)
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

void SaxophoneModel2::setDelay0(double delaySamples)
{
    double outPointer = static_cast<double>(delay0InPoint) - delaySamples;
    const double bufSize = static_cast<double>(delay0Buffer.size());
    while (outPointer < 0.0) outPointer += bufSize;
    delay0OutPoint = static_cast<size_t>(outPointer);
    delay0Alpha    = outPointer - static_cast<double>(delay0OutPoint);
    delay0OmAlpha  = 1.0 - delay0Alpha;
    if (delay0OutPoint >= delay0Buffer.size()) delay0OutPoint = 0;
}

double SaxophoneModel2::delay0Tick(double input)
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

void SaxophoneModel2::setDelay1(double delaySamples)
{
    double outPointer = static_cast<double>(delay1InPoint) - delaySamples;
    const double bufSize = static_cast<double>(delay1Buffer.size());
    while (outPointer < 0.0) outPointer += bufSize;
    delay1OutPoint = static_cast<size_t>(outPointer);
    delay1Alpha    = outPointer - static_cast<double>(delay1OutPoint);
    delay1OmAlpha  = 1.0 - delay1Alpha;
    if (delay1OutPoint >= delay1Buffer.size()) delay1OutPoint = 0;
}

double SaxophoneModel2::delay1Tick(double input)
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
//  OneZero low-pass filter (zero at −1, phase delay = 0.5 samples)
//  y[n] = 0.5·x[n] + 0.5·x[n−1]
// ─────────────────────────────────────────────────────────────────────────────

double SaxophoneModel2::oneZeroTick(double input)
{
    double out = 0.5 * input + 0.5 * oneZeroX1;
    oneZeroX1  = input;
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Reed table (Smith 1986)
//  output = clamp(offset + slope · input, −1, 1)
//  aperture is passed per-sample so jaw vibrato can modulate it each tick.
// ─────────────────────────────────────────────────────────────────────────────

double SaxophoneModel2::reedTableTick(double input, double aperture) const
{
    // Negative slope (STK Saxofony / Smith 1986 valve model):
    // When bore pressure is LOW (pressureDiff large+), reedCoeff is LOW →
    // reed opens wide, lots of breath enters the bore.
    // When bore pressure is HIGH (pressureDiff small or negative), reedCoeff
    // is HIGH → reed closes, breath injection stops.
    // This valve behaviour is what allows the bore to self-oscillate.
    // A POSITIVE slope does the opposite and starves the bore → silence.
    double offset = 0.4  + 0.6  * aperture;         // [0.4, 1.0]
    double slope  = -(0.1 + 0.4 * reedStiffness);   // [−0.1, −0.5] — NEGATIVE
    double out    = offset + slope * input;

    // reedSaturation softly limits how far the reed can open (reedCoeff < 0
    // means fully open / over-driven).  Limiting the minimum adds odd-harmonic
    // drive without breaking the self-oscillation condition.
    if (reedSaturation > 0.0) {
        double minVal = -reedSaturation;
        if (out < minVal) out = minVal;
    }

    if (out >  1.0) out =  1.0;
    if (out < -1.0) out = -1.0;
    return out;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Nonlinear allpass (Michon)
// ─────────────────────────────────────────────────────────────────────────────

double SaxophoneModel2::computeNLCoeff(double signal, double pitch) const
{
    double modSig = 0.0;
    switch (nlType) {
    case 0: modSig = signal;                 break;
    case 1: modSig = nlLeaky;                break;
    case 2: modSig = signal * signal;        break;
    case 3: modSig = std::sin(nlSinePhase);  break;
    case 4: modSig = std::sin(nlSinePhase);  break;
    }
    double coeff = modSig * nlAmount * nlEnvGain;
    return clamp(coeff, -0.999, 0.999);
}

double SaxophoneModel2::nlFilterTick(double input, double coeff)
{
    double output = coeff * (input - nlY1) + nlX1;
    nlX1 = input;
    nlY1 = output;
    return output;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Vocal tract biquad bandpass (RBJ Audio EQ Cookbook, BPF constant 0dB peak)
//
//  H(z) = (b0 + b2·z⁻²) / (a0 + a1·z⁻¹ + a2·z⁻²)
//  w0    = 2π·f/fs
//  α     = sin(w0) / (2Q)
//  b0    =  α,  b1 = 0,  b2 = −α
//  a0    = 1+α, a1 = −2·cos(w0), a2 = 1−α
//  (all divided by a0 before storing)
// ─────────────────────────────────────────────────────────────────────────────

void SaxophoneModel2::updateVocalTractCoeffs()
{
    double w0    = TWO_PI * vocalTractFreq / sampleRate;
    double sinW0 = std::sin(w0);
    double cosW0 = std::cos(w0);
    double alpha = sinW0 / (2.0 * vocalTractQ);
    double a0    = 1.0 + alpha;
    vtB0 =  alpha / a0;   // b0/a0
    vtB2 = -alpha / a0;   // b2/a0  (b1 = 0)
    vtA1 = -2.0 * cosW0 / a0;
    vtA2 = (1.0 - alpha) / a0;
}

void SaxophoneModel2::setVocalTractFreq(double f)
{
    vocalTractFreq = clamp(f, 200.0, 3000.0);
    updateVocalTractCoeffs();
}

void SaxophoneModel2::setVocalTractQ(double q)
{
    vocalTractQ = clamp(q, 0.5, 10.0);
    updateVocalTractCoeffs();
}

double SaxophoneModel2::vocalTractFilter(double input)
{
    double output = vtB0 * input + vtB2 * vtX2
                  - vtA1 * vtY1 - vtA2 * vtY2;
    vtX2 = vtX1;  vtX1 = input;
    vtY2 = vtY1;  vtY1 = output;
    return output;
}

// ─────────────────────────────────────────────────────────────────────────────
//  White noise (LCG, returns uniform in [−1, 1])
// ─────────────────────────────────────────────────────────────────────────────

double SaxophoneModel2::nextRandom()
{
    rngState = rngState * 1664525u + 1013904223u;
    return static_cast<double>(static_cast<int>(rngState)) / 2147483648.0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Breath envelope — S-curve (smoothstep) attack, linear release
//  Attack: 3t²−2t³ over first 8% of note (skipped for legato)
//  Release: linear from 1→0 over last 5% of note
// ─────────────────────────────────────────────────────────────────────────────

double SaxophoneModel2::breathEnvelope(double noteProgress, bool isLegato) const
{
    constexpr double ATK = 0.08;
    constexpr double REL = 0.05;

    if (!isLegato && noteProgress < ATK) {
        double t = noteProgress / ATK;
        return std::sqrt(t);  // concave: fast initial rise, decelerates to full pressure
    }
    if (noteProgress > (1.0 - REL))
        return (1.0 - noteProgress) / REL;
    return 1.0;
}

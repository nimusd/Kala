#include "karplusstrongattack.h"
#include <cmath>
#include <random>
#include <algorithm>

// Constants
static constexpr double PI = 3.14159265358979323846;
static constexpr double TYPICAL_DECAY_CYCLES = 1500.0;  // Cycles for full K-S decay
static constexpr double TARGET_PEAK = 0.8;              // Normalized output level
static constexpr double FADE_PORTION = 0.1;             // Final 10% gets cosine fade

KarplusStrongAttack::KarplusStrongAttack(double sampleRate)
    : sampleRate(sampleRate)
    , mode(KSMode::String)
    , attackPortion(0.15)
    , damping(0.5)
    , pluckPosition(0.5)
    , mix(0.8)
    , brightness(0.5)
    , excitationType(KSExcitation::Noise)
    , excitationSoftness(0.3)
    , pluckHardness(0.0)
    , pickDirection(0.5)
    , bodyResonance(0.0)
    , bodyFreq(200.0)
    , writeIndex(0)
    , delayLength(0)
    , maxDelayLength(0)
    , currentDelay(0.0)
    , prevSample(0.0)
    , bqX1(0.0), bqX2(0.0)
    , bqY1(0.0), bqY2(0.0)
    , bqB0(0.0), bqB1(0.0), bqB2(0.0)
    , bqA1(0.0), bqA2(0.0)
    , active(false)
    , sampleCount(0)
    , attackSamples(0)
    , peakAmplitude(0.001)  // Avoid division by zero
    , filterState(0.0)
{
}

void KarplusStrongAttack::trigger(double pitch)
{
    // Clamp pitch to reasonable range
    pitch = std::clamp(pitch, 20.0, 20000.0);

    // Calculate delay line length from pitch (integer, for excitation fill)
    delayLength = static_cast<size_t>(std::round(sampleRate / pitch));
    if (delayLength < 2) {
        delayLength = 2;  // Minimum delay length
    }

    // Fractional delay for interpolated read
    currentDelay = sampleRate / pitch;

    // Max buffer allows pitch sweeps down to ~20 Hz
    size_t minForSweep = static_cast<size_t>(sampleRate / 20.0) + 2;
    maxDelayLength = std::max(delayLength, minForSweep);

    // Resize buffer, zero it, then fill first delayLength samples with excitation
    delayLine.assign(maxDelayLength, 0.0);
    fillDelayLineWithExcitation();

    // Apply pick direction comb filter to excitation (shapes initial harmonic content)
    applyPickDirectionToExcitation();

    // Calculate how many samples before K-S goes silent.
    // String mode: let natural decay + renderNoteImpl silence detection control length.
    //   A 30-second safety cap matches the render cap in Track::renderNoteImpl.
    // Attack mode: attackPortion controls how much of the pluck decay to include.
    if (mode == KSMode::String) {
        attackSamples = static_cast<size_t>(30.0 * sampleRate);
    } else {
        attackSamples = static_cast<size_t>(attackPortion * TYPICAL_DECAY_CYCLES * delayLength);
        if (attackSamples < delayLength) {
            attackSamples = delayLength;  // At least one cycle
        }
    }

    // Reset state
    // Start writing AFTER the excitation region so the read pointer
    // (at writeIndex - currentDelay) lands on position ~0, reading the
    // excitation data before it gets overwritten by the feedback loop.
    writeIndex = delayLength;
    prevSample = delayLine[delayLength - 1];
    sampleCount = 0;
    peakAmplitude = 0.001;
    active = true;

    // Compute body resonance biquad coefficients
    computeBodyResonanceCoeffs();
    bqX1 = bqX2 = bqY1 = bqY2 = 0.0;
}

double KarplusStrongAttack::generateSample(double inputSignal)
{
    // EKS loop filter parameters (computed once per sample, shared by both modes)
    // S: stretching factor — controls frequency-dependent damping
    //    Higher S = more previous sample weight = less HF loss = brighter
    double S = 0.5 + brightness * 0.49;  // Range: 0.50 - 0.99

    // rho: feedback gain — controls frequency-independent decay
    //    damping=0 => rho=0.99998 (~8s natural tail)
    //    damping=1 => rho=0.990   (~15ms — dead stop)
    double rho = 0.990 + (1.0 - damping) * 0.00998;  // Range: 0.990 - 0.99998

    // In Attack mode, always pass through input signal even when K-S is inactive
    if (mode == KSMode::Attack) {
        if (!active || maxDelayLength == 0) {
            return inputSignal;
        }

        // Fractional delay read with linear interpolation
        double readPos = static_cast<double>(writeIndex) - currentDelay;
        if (readPos < 0.0) readPos += static_cast<double>(maxDelayLength);
        size_t idx0 = static_cast<size_t>(readPos);
        size_t idx1 = (idx0 + 1) % maxDelayLength;
        double frac = readPos - std::floor(readPos);
        double currentSample = delayLine[idx0] * (1.0 - frac) + delayLine[idx1] * frac;

        // EKS loop filter: weighted average with feedback gain
        double filtered = ((1.0 - S) * currentSample + S * prevSample) * rho;

        // Write filtered sample back to delay line
        delayLine[writeIndex] = filtered;

        // Advance write index with wraparound
        writeIndex = (writeIndex + 1) % maxDelayLength;

        // Store for next iteration's filter
        prevSample = currentSample;

        // Track peak amplitude for normalization
        double absSample = std::abs(currentSample);
        if (absSample > peakAmplitude) {
            peakAmplitude = absSample;
        }

        // Normalize K-S output
        double ksOutput = 0.0;
        if (peakAmplitude > 0.001) {
            ksOutput = currentSample / peakAmplitude * TARGET_PEAK;
        }

        // Apply body resonance (post-output, not in feedback loop)
        if (bodyResonance > 0.001) {
            double wet = processBodyResonance(ksOutput);
            ksOutput = (1.0 - bodyResonance) * ksOutput + bodyResonance * wet;
        }

        // Calculate progress through attack phase
        double progress = static_cast<double>(sampleCount) / static_cast<double>(attackSamples);

        // Attack mode blending
        double ksWeight = (1.0 - progress) * mix;
        double inputWeight = 1.0 - ksWeight;

        // Apply cosine fade in final portion for smooth transition
        if (progress > (1.0 - FADE_PORTION)) {
            double fadeProgress = (progress - (1.0 - FADE_PORTION)) / FADE_PORTION;
            double fadeFactor = 0.5 * (1.0 + std::cos(fadeProgress * PI));
            ksWeight *= fadeFactor;
            inputWeight = 1.0 - ksWeight;
        }

        double output = ksOutput * ksWeight + inputSignal * inputWeight;

        sampleCount++;
        if (sampleCount >= attackSamples) {
            active = false;
        }

        return output;
    }

    // String mode: K-S is the entire sound
    if (!active || maxDelayLength == 0) {
        return 0.0;
    }

    // Fractional delay read with linear interpolation
    double readPos = static_cast<double>(writeIndex) - currentDelay;
    if (readPos < 0.0) readPos += static_cast<double>(maxDelayLength);
    size_t idx0 = static_cast<size_t>(readPos);
    size_t idx1 = (idx0 + 1) % maxDelayLength;
    double frac = readPos - std::floor(readPos);
    double currentSample = delayLine[idx0] * (1.0 - frac) + delayLine[idx1] * frac;

    // EKS loop filter: weighted average with feedback gain
    double filtered = ((1.0 - S) * currentSample + S * prevSample) * rho;

    // Write filtered sample back to delay line
    delayLine[writeIndex] = filtered;

    // Advance write index with wraparound
    writeIndex = (writeIndex + 1) % maxDelayLength;

    // Store for next iteration's filter
    prevSample = currentSample;

    // Track peak amplitude for normalization
    double absSample = std::abs(currentSample);
    if (absSample > peakAmplitude) {
        peakAmplitude = absSample;
    }

    // Normalize output
    double output = 0.0;
    if (peakAmplitude > 0.001) {
        output = currentSample / peakAmplitude * TARGET_PEAK;
    }

    // Apply body resonance (post-output, not in feedback loop)
    if (bodyResonance > 0.001) {
        double wet = processBodyResonance(output);
        output = (1.0 - bodyResonance) * output + bodyResonance * wet;
    }

    // Apply cosine fade in final portion
    double progress = static_cast<double>(sampleCount) / static_cast<double>(attackSamples);
    if (progress > (1.0 - FADE_PORTION)) {
        double fadeProgress = (progress - (1.0 - FADE_PORTION)) / FADE_PORTION;
        double fadeFactor = 0.5 * (1.0 + std::cos(fadeProgress * PI));
        output *= fadeFactor;
    }

    sampleCount++;
    if (sampleCount >= attackSamples) {
        active = false;
    }

    return output;
}

void KarplusStrongAttack::reset()
{
    active = false;
    sampleCount = 0;
    writeIndex = 0;
    prevSample = 0.0;
    peakAmplitude = 0.001;
    filterState = 0.0;
    currentDelay = 0.0;
    maxDelayLength = 0;
    bqX1 = bqX2 = bqY1 = bqY2 = 0.0;

    // Clear delay line
    std::fill(delayLine.begin(), delayLine.end(), 0.0);
}

// ========== Excitation Methods ==========

void KarplusStrongAttack::fillDelayLineWithExcitation()
{
    switch (excitationType) {
    case KSExcitation::SoftPluck:
        fillWithSoftPluck();
        break;
    case KSExcitation::Blend:
        fillWithBlendedExcitation();
        break;
    case KSExcitation::FingerPluck:
        fillWithFingerPluck();
        break;
    case KSExcitation::Noise:
    default:
        fillDelayLineWithNoise();
        break;
    }
}

void KarplusStrongAttack::fillDelayLineWithNoise()
{
    // Random number generator
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> dist(-1.0, 1.0);

    // Calculate filter cutoff based on pluck position
    // Bridge (0.0) = bright = high cutoff
    // Neck (1.0) = mellow = low cutoff
    double cutoffHz = 200.0 + (1.0 - pluckPosition) * 8000.0;

    // Reset filter state
    filterState = 0.0;

    // Fill delay line with filtered noise
    for (size_t i = 0; i < delayLength; ++i) {
        double noise = dist(gen);
        double filtered = applyLowPassFilter(noise, cutoffHz);
        delayLine[i] = filtered;
    }

    // Normalize the initial noise to prevent clipping
    double maxVal = 0.0;
    for (size_t i = 0; i < delayLength; ++i) {
        double absVal = std::abs(delayLine[i]);
        if (absVal > maxVal) {
            maxVal = absVal;
        }
    }
    if (maxVal > 0.0) {
        for (size_t i = 0; i < delayLength; ++i) {
            delayLine[i] /= maxVal;
        }
    }
}

void KarplusStrongAttack::fillWithSoftPluck()
{
    // Width-controlled raised cosine hump, centered in the delay buffer.
    // pluckHardness 0 => widthSamples = N  (full period, current soft behavior)
    // pluckHardness 1 => widthSamples = 0.1*N (narrow pick impulse)
    double N = static_cast<double>(delayLength);
    double widthSamples = N * (0.1 + 0.9 * (1.0 - pluckHardness));
    if (widthSamples < 2.0) widthSamples = 2.0;

    double center = N * 0.5;
    double halfWidth = widthSamples * 0.5;

    // Generate raised cosine hump (0 outside, 0→1→0 inside)
    for (size_t i = 0; i < delayLength; ++i) {
        double pos = static_cast<double>(i);
        if (pos >= center - halfWidth && pos <= center + halfWidth) {
            double t = (pos - (center - halfWidth)) / widthSamples;
            delayLine[i] = 0.5 * (1.0 - std::cos(2.0 * PI * t));
        } else {
            delayLine[i] = 0.0;
        }
    }

    // DC-remove: subtract mean so waveform is centered around 0
    double sum = 0.0;
    for (size_t i = 0; i < delayLength; ++i) {
        sum += delayLine[i];
    }
    double mean = sum / N;
    for (size_t i = 0; i < delayLength; ++i) {
        delayLine[i] -= mean;
    }

    // Normalize peak to 1.0
    double maxVal = 0.0;
    for (size_t i = 0; i < delayLength; ++i) {
        double absVal = std::abs(delayLine[i]);
        if (absVal > maxVal) maxVal = absVal;
    }
    if (maxVal > 0.0) {
        for (size_t i = 0; i < delayLength; ++i) {
            delayLine[i] /= maxVal;
        }
    }
}

void KarplusStrongAttack::fillWithBlendedExcitation()
{
    // First fill with noise
    fillDelayLineWithNoise();

    // Generate width-controlled soft pluck and blend with noise
    double N = static_cast<double>(delayLength);
    double widthSamples = N * (0.1 + 0.9 * (1.0 - pluckHardness));
    if (widthSamples < 2.0) widthSamples = 2.0;

    double center = N * 0.5;
    double halfWidth = widthSamples * 0.5;
    double blend = excitationSoftness;  // 0 = all noise, 1 = all soft pluck

    // Generate width-controlled soft pluck into temp buffer, DC-remove, normalize
    std::vector<double> softBuf(delayLength, 0.0);
    for (size_t i = 0; i < delayLength; ++i) {
        double pos = static_cast<double>(i);
        if (pos >= center - halfWidth && pos <= center + halfWidth) {
            double t = (pos - (center - halfWidth)) / widthSamples;
            softBuf[i] = 0.5 * (1.0 - std::cos(2.0 * PI * t));
        }
    }
    double sum = 0.0;
    for (size_t i = 0; i < delayLength; ++i) sum += softBuf[i];
    double mean = sum / N;
    for (size_t i = 0; i < delayLength; ++i) softBuf[i] -= mean;
    double spMax = 0.0;
    for (size_t i = 0; i < delayLength; ++i) {
        double a = std::abs(softBuf[i]);
        if (a > spMax) spMax = a;
    }
    if (spMax > 0.0) {
        for (size_t i = 0; i < delayLength; ++i) softBuf[i] /= spMax;
    }

    // Blend noise and soft pluck
    for (size_t i = 0; i < delayLength; ++i) {
        delayLine[i] = (1.0 - blend) * delayLine[i] + blend * softBuf[i];
    }

    // Re-normalize
    double maxVal = 0.0;
    for (size_t i = 0; i < delayLength; ++i) {
        double absVal = std::abs(delayLine[i]);
        if (absVal > maxVal) maxVal = absVal;
    }
    if (maxVal > 0.0) {
        for (size_t i = 0; i < delayLength; ++i) {
            delayLine[i] /= maxVal;
        }
    }
}

void KarplusStrongAttack::fillWithFingerPluck()
{
    // Rounded triangle excitation — models a real plucked string displacement.
    //
    // A finger pluck creates a triangular displacement where the string lifts
    // off the fingertip. The triangle provides harmonic richness (1/n² rolloff)
    // while cosine rounding at the apex softens the harshest harmonics,
    // simulating the finite width of the finger's flesh.
    //
    // pluckHardness controls:
    //   - Width: 0 = wide (0.6*N, thick thumb), 1 = narrow (0.08*N, nail/pick)
    //   - Rounding: 0 = heavily rounded apex, 1 = sharp apex

    double N = static_cast<double>(delayLength);

    // Width: from 60% of period (soft thumb) to 8% (hard nail)
    double widthSamples = N * (0.08 + 0.52 * (1.0 - pluckHardness));
    if (widthSamples < 3.0) widthSamples = 3.0;

    double center = N * 0.5;
    double halfWidth = widthSamples * 0.5;

    // Rounding radius: heavily rounded at low hardness, sharp at high
    // Expressed as fraction of half-width to round at the peak
    double roundFraction = 0.4 * (1.0 - pluckHardness);  // 0.4 at soft, 0.0 at hard
    double roundSamples = halfWidth * roundFraction;

    // Generate rounded triangle
    for (size_t i = 0; i < delayLength; ++i) {
        double pos = static_cast<double>(i);
        double dist = std::abs(pos - center);

        if (dist > halfWidth) {
            // Outside the triangle
            delayLine[i] = 0.0;
        } else if (dist > halfWidth - roundSamples && roundSamples > 0.5) {
            // In the rounding zone at the base — cosine taper to zero
            double t = (dist - (halfWidth - roundSamples)) / roundSamples;
            double triangleVal = 1.0 - dist / halfWidth;
            double taperVal = triangleVal * 0.5 * (1.0 + std::cos(t * PI));
            delayLine[i] = taperVal;
        } else if (dist < roundSamples && roundSamples > 0.5) {
            // In the rounding zone at the apex — cosine smoothing
            double t = dist / roundSamples;
            // Blend between flat top (1.0) and triangle slope
            double triangleVal = 1.0 - dist / halfWidth;
            double flatVal = 1.0;
            double blend = 0.5 * (1.0 - std::cos(t * PI));  // 0 at center, 1 at edge of round zone
            delayLine[i] = flatVal * (1.0 - blend) + triangleVal * blend;
        } else {
            // Pure triangle slope
            delayLine[i] = 1.0 - dist / halfWidth;
        }
    }

    // DC-remove: subtract mean so waveform is centered around 0
    double sum = 0.0;
    for (size_t i = 0; i < delayLength; ++i) sum += delayLine[i];
    double mean = sum / N;
    for (size_t i = 0; i < delayLength; ++i) delayLine[i] -= mean;

    // Normalize peak to 1.0
    double maxVal = 0.0;
    for (size_t i = 0; i < delayLength; ++i) {
        double absVal = std::abs(delayLine[i]);
        if (absVal > maxVal) maxVal = absVal;
    }
    if (maxVal > 0.0) {
        for (size_t i = 0; i < delayLength; ++i) delayLine[i] /= maxVal;
    }
}

// ========== Pick Direction (Excitation Comb Filter) ==========

void KarplusStrongAttack::applyPickDirectionToExcitation()
{
    // EKS pick direction: a one-sample-delay comb filter applied to the
    // excitation signal.  This creates spectral notches that shape which
    // harmonics are present at the moment of the pluck, modelling how
    // different pick angles excite different harmonics.
    //
    // H(z) = 1 + p * z^-1
    //   p = 0  → neutral (no filtering)
    //   p > 0  → notch at Nyquist  → warmer / rounder
    //   p < 0  → notch at DC       → thinner / brighter
    //
    // pickDirection 0.0 → p = -0.9  (bright, plectrum-like)
    // pickDirection 0.5 → p =  0.0  (neutral, bypassed)
    // pickDirection 1.0 → p = +0.9  (warm, flesh-like)

    double p = (pickDirection - 0.5) * 2.0 * 0.9;  // range: -0.9 to +0.9

    // Skip if negligible
    if (std::abs(p) < 0.001) return;

    // Apply feedforward comb filter in-place to delay line
    // Process backwards so we read unfiltered values
    double prev = delayLine[(delayLength > 0) ? delayLength - 1 : 0];  // wrap: last sample feeds into first
    // We need to process forward, keeping track of the previous unfiltered sample
    double prevUnfiltered = prev;
    for (size_t i = 0; i < delayLength; ++i) {
        double current = delayLine[i];
        delayLine[i] = current + p * prevUnfiltered;
        prevUnfiltered = current;
    }

    // Re-normalize to prevent amplitude change
    double maxVal = 0.0;
    for (size_t i = 0; i < delayLength; ++i) {
        double absVal = std::abs(delayLine[i]);
        if (absVal > maxVal) maxVal = absVal;
    }
    if (maxVal > 0.0) {
        for (size_t i = 0; i < delayLength; ++i) {
            delayLine[i] /= maxVal;
        }
    }
}

// ========== Body Resonance ==========

void KarplusStrongAttack::computeBodyResonanceCoeffs()
{
    // Biquad bandpass filter coefficients
    // Based on Audio EQ Cookbook (Robert Bristow-Johnson)
    double Q = 5.0;  // Fixed Q for body resonance
    double w0 = 2.0 * PI * bodyFreq / sampleRate;
    double alpha = std::sin(w0) / (2.0 * Q);

    double b0 = alpha;
    double b1 = 0.0;
    double b2 = -alpha;
    double a0 = 1.0 + alpha;
    double a1 = -2.0 * std::cos(w0);
    double a2 = 1.0 - alpha;

    // Normalize by a0
    bqB0 = b0 / a0;
    bqB1 = b1 / a0;
    bqB2 = b2 / a0;
    bqA1 = a1 / a0;
    bqA2 = a2 / a0;
}

double KarplusStrongAttack::processBodyResonance(double sample)
{
    // Direct Form II biquad
    double out = bqB0 * sample + bqB1 * bqX1 + bqB2 * bqX2
                 - bqA1 * bqY1 - bqA2 * bqY2;

    // Update state
    bqX2 = bqX1;
    bqX1 = sample;
    bqY2 = bqY1;
    bqY1 = out;

    return out;
}

// ========== Low-Pass Filter ==========

double KarplusStrongAttack::applyLowPassFilter(double sample, double cutoffHz)
{
    // Simple one-pole low-pass filter
    double coefficient = 1.0 - std::exp(-2.0 * PI * cutoffHz / sampleRate);
    filterState += coefficient * (sample - filterState);
    return filterState;
}

// ========== Parameter Setters ==========

void KarplusStrongAttack::setAttackPortion(double portion)
{
    attackPortion = std::clamp(portion, 0.01, 1.0);
}

void KarplusStrongAttack::setDamping(double d)
{
    damping = std::clamp(d, 0.0, 1.0);
}

void KarplusStrongAttack::setPluckPosition(double position)
{
    pluckPosition = std::clamp(position, 0.0, 1.0);
}

void KarplusStrongAttack::setMix(double m)
{
    mix = std::clamp(m, 0.0, 1.0);
}

void KarplusStrongAttack::setSampleRate(double rate)
{
    if (rate > 0.0) {
        sampleRate = rate;
    }
}

void KarplusStrongAttack::setBrightness(double b)
{
    brightness = std::clamp(b, 0.0, 1.0);
}

void KarplusStrongAttack::setExcitationType(int type)
{
    type = std::clamp(type, 0, 3);
    excitationType = static_cast<KSExcitation>(type);
}

void KarplusStrongAttack::setExcitationSoftness(double s)
{
    excitationSoftness = std::clamp(s, 0.0, 1.0);
}

void KarplusStrongAttack::setPluckHardness(double h)
{
    pluckHardness = std::clamp(h, 0.0, 1.0);
}

void KarplusStrongAttack::setPickDirection(double d)
{
    pickDirection = std::clamp(d, 0.0, 1.0);
}

void KarplusStrongAttack::setBodyResonance(double r)
{
    bodyResonance = std::clamp(r, 0.0, 1.0);
}

void KarplusStrongAttack::setBodyFreq(double f)
{
    bodyFreq = std::clamp(f, 80.0, 400.0);
}

void KarplusStrongAttack::setPitch(double pitch)
{
    if (maxDelayLength == 0) return;
    currentDelay = sampleRate / std::clamp(pitch, 20.0, 20000.0);
    currentDelay = std::clamp(currentDelay, 2.0, static_cast<double>(maxDelayLength - 1));
}

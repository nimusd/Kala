#include "envelopeengine.h"

EnvelopeEngine::EnvelopeEngine()
    : envelopeType(EnvelopeType::Linear)
    , timeScale(1.0)
    , valueScale(1.0)
    , valueOffset(0.0)
    , loopMode(LoopMode::None)
    , attackTime(0.1)
    , decayTime(0.2)
    , sustainLevel(0.7)
    , releaseTime(0.2)
    , fadeTime(0.5)
{
}

void EnvelopeEngine::setEnvelopeType(EnvelopeType type)
{
    envelopeType = type;
}

void EnvelopeEngine::setEnvelopeSelect(int index)
{
    // Map index to envelope type (matching UI: Linear, Attack-Decay, ADSR, Fade In, Fade Out, Log Fade In, Log Fade Out, Custom)
    switch (index) {
        case 0: envelopeType = EnvelopeType::Linear; break;
        case 1: envelopeType = EnvelopeType::AttackDecay; break;
        case 2: envelopeType = EnvelopeType::SustainRelease; break;   // ADSR
        case 3: envelopeType = EnvelopeType::ExponentialRise; break;  // Fade In
        case 4: envelopeType = EnvelopeType::ExponentialFall; break;  // Fade Out
        case 5: envelopeType = EnvelopeType::LogarithmicRise; break;  // Log Fade In
        case 6: envelopeType = EnvelopeType::LogarithmicFall; break;  // Log Fade Out
        case 7: envelopeType = EnvelopeType::Custom; break;           // Custom envelope
        default: envelopeType = EnvelopeType::Linear; break;
    }
}

void EnvelopeEngine::setTimeScale(double scale)
{
    timeScale = std::clamp(scale, 0.1, 10.0);
}

void EnvelopeEngine::setValueScale(double scale)
{
    valueScale = std::clamp(scale, 0.0, 10.0);
}

void EnvelopeEngine::setValueOffset(double offset)
{
    valueOffset = std::clamp(offset, -10.0, 10.0);
}

void EnvelopeEngine::setLoopMode(LoopMode mode)
{
    loopMode = mode;
}

void EnvelopeEngine::setAttackTime(double time)
{
    attackTime = std::clamp(time, 0.001, 5.0);
}

void EnvelopeEngine::setDecayTime(double time)
{
    decayTime = std::clamp(time, 0.001, 5.0);
}

void EnvelopeEngine::setSustainLevel(double level)
{
    sustainLevel = std::clamp(level, 0.0, 1.0);
}

void EnvelopeEngine::setReleaseTime(double time)
{
    releaseTime = std::clamp(time, 0.001, 5.0);
}

void EnvelopeEngine::setFadeTime(double time)
{
    fadeTime = std::clamp(time, 0.0, 1.0);
}

void EnvelopeEngine::setCustomEnvelope(const QVector<EnvelopePoint> &points)
{
    customEnvelopePoints = points;
    // Ensure points are sorted by time
    std::sort(customEnvelopePoints.begin(), customEnvelopePoints.end(),
              [](const EnvelopePoint &a, const EnvelopePoint &b) {
                  return a.time < b.time;
              });
}

double EnvelopeEngine::applyLoopMode(double position) const
{
    if (position < 0.0) position = 0.0;

    switch (loopMode) {
        case LoopMode::None:
            // Clamp to [0, 1]
            return std::clamp(position, 0.0, 1.0);

        case LoopMode::Loop:
            // Wrap around: take fractional part
            if (position >= 1.0) {
                position = position - std::floor(position);
            }
            return position;

        case LoopMode::Pingpong: {
            // Bounce back and forth
            if (position >= 1.0) {
                int cycles = static_cast<int>(std::floor(position));
                double frac = position - cycles;

                // Even cycles: forward, odd cycles: backward
                if (cycles % 2 == 0) {
                    return frac;
                } else {
                    return 1.0 - frac;
                }
            }
            return position;
        }

        default:
            return std::clamp(position, 0.0, 1.0);
    }
}

double EnvelopeEngine::evaluateEnvelope(double position) const
{
    // Ensure position is in valid range [0, 1]
    position = std::clamp(position, 0.0, 1.0);

    switch (envelopeType) {
        case EnvelopeType::Linear:
            // Simple linear ramp: 0 -> 1
            return position;

        case EnvelopeType::ExponentialRise: {
            // Fade In: uses fadeTime parameter
            if (position < fadeTime) {
                // Fading in from 0 to 1
                return position / fadeTime;
            } else {
                // After fade, stay at 1
                return 1.0;
            }
        }

        case EnvelopeType::ExponentialFall: {
            // Fade Out: uses fadeTime parameter
            if (position < (1.0 - fadeTime)) {
                // Before fade, stay at 1
                return 1.0;
            } else {
                // Fading out from 1 to 0
                double fadeProgress = (position - (1.0 - fadeTime)) / fadeTime;
                return 1.0 - fadeProgress;
            }
        }

        case EnvelopeType::Sine: {
            // Smooth S-curve using sine
            // sin((x - 0.5) * π) * 0.5 + 0.5
            // Maps 0->0 and 1->1 with smooth acceleration/deceleration
            double sineValue = std::sin((position - 0.5) * M_PI) * 0.5 + 0.5;
            return sineValue;
        }

        case EnvelopeType::AttackDecay: {
            // Use attackTime and decayTime parameters
            double totalTime = attackTime + decayTime;
            if (totalTime <= 0.0) return 0.0;

            double scaledTime = position * totalTime;
            if (scaledTime < attackTime) {
                // Attack phase: 0 -> 1
                return scaledTime / attackTime;
            } else {
                // Decay phase: 1 -> 0
                double decayProgress = (scaledTime - attackTime) / decayTime;
                return 1.0 - decayProgress;
            }
        }

        case EnvelopeType::SustainRelease: {
            // ADSR envelope using all timing parameters
            double totalTime = attackTime + decayTime + releaseTime;
            double sustainTime = 1.0 - totalTime;
            if (sustainTime < 0.0) sustainTime = 0.0;

            double scaledTime = position;

            if (scaledTime < attackTime) {
                // Attack phase: 0 -> 1
                return scaledTime / attackTime;
            } else if (scaledTime < attackTime + decayTime) {
                // Decay phase: 1 -> sustain level
                double decayProgress = (scaledTime - attackTime) / decayTime;
                return 1.0 - decayProgress * (1.0 - sustainLevel);
            } else if (scaledTime < attackTime + decayTime + sustainTime) {
                // Sustain phase: hold at sustain level
                return sustainLevel;
            } else {
                // Release phase: sustain level -> 0
                double releaseProgress = (scaledTime - attackTime - decayTime - sustainTime) / releaseTime;
                return sustainLevel * (1.0 - releaseProgress);
            }
        }

        case EnvelopeType::LogarithmicRise:
            // Rises quickly at the start, then gradually flattens: log10(1 + 9x)
            return std::log10(1.0 + 9.0 * position);

        case EnvelopeType::LogarithmicFall:
            // Mirror of LogarithmicRise: stays high early, then drops off sharply
            return 1.0 - std::log10(1.0 + 9.0 * position);

        case EnvelopeType::Custom:
            // User-defined multi-point envelope
            return evaluateCustomEnvelope(position);

        default:
            return position;
    }
}

double EnvelopeEngine::evaluateCustomEnvelope(double position) const
{
    if (customEnvelopePoints.isEmpty()) return 0.0;
    if (customEnvelopePoints.size() == 1) return customEnvelopePoints.first().value;

    // Clamp position to valid range
    if (position <= customEnvelopePoints.first().time) {
        return customEnvelopePoints.first().value;
    }
    if (position >= customEnvelopePoints.last().time) {
        return customEnvelopePoints.last().value;
    }

    // Find the two points we're between
    for (int i = 0; i < customEnvelopePoints.size() - 1; ++i) {
        const EnvelopePoint &p1 = customEnvelopePoints[i];
        const EnvelopePoint &p2 = customEnvelopePoints[i + 1];

        if (position >= p1.time && position <= p2.time) {
            // Calculate interpolation parameter
            double t = (position - p1.time) / (p2.time - p1.time);

            // Interpolate based on curve type
            if (p1.curveType == 0) {
                // Linear interpolation
                return p1.value + t * (p2.value - p1.value);
            } else if (p1.curveType == 1) {
                // Smooth (cosine) interpolation
                double smoothT = (1.0 - std::cos(t * M_PI)) * 0.5;
                return p1.value + smoothT * (p2.value - p1.value);
            } else {
                // Step (hold first value until next point)
                return p1.value;
            }
        }
    }

    return 0.0;
}

double EnvelopeEngine::process(double noteProgress, bool isLegato)
{
    // For legato notes, skip the attack phase and start at the post-attack level
    double effectiveProgress = noteProgress;

    if (isLegato) {
        // Calculate where the attack phase ends for each envelope type
        // and offset the progress to start from there
        double attackEndPosition = 0.0;

        switch (envelopeType) {
            case EnvelopeType::Linear:
                // No attack concept - start at a reasonable level (0.3)
                attackEndPosition = 0.3;
                break;

            case EnvelopeType::ExponentialRise:
                // Fade In: attack ends at fadeTime
                attackEndPosition = fadeTime;
                break;

            case EnvelopeType::ExponentialFall:
                // Fade Out: no attack phase, starts at 1.0
                attackEndPosition = 0.0;
                break;

            case EnvelopeType::Sine:
                // S-curve: skip to middle of curve (0.5 gives value ~0.5)
                attackEndPosition = 0.3;
                break;

            case EnvelopeType::AttackDecay:
                // Skip attack phase, start at peak
                attackEndPosition = attackTime / (attackTime + decayTime);
                break;

            case EnvelopeType::SustainRelease:
                // ADSR: skip attack and decay, start at sustain
                attackEndPosition = attackTime + decayTime;
                break;

            case EnvelopeType::LogarithmicRise:
                // Most of the rise happens early; skip to where curve has levelled off
                attackEndPosition = 0.3;
                break;

            case EnvelopeType::LogarithmicFall:
                // No attack phase — starts at full value
                attackEndPosition = 0.0;
                break;

            case EnvelopeType::Custom:
                // For custom envelopes, skip the first 10% as a rough estimate
                // Could be improved to analyze the curve
                attackEndPosition = 0.1;
                break;

            default:
                attackEndPosition = 0.0;
                break;
        }

        // Remap progress: 0->attackEnd, 1->1
        // So legato notes start at post-attack and still reach the end normally
        effectiveProgress = attackEndPosition + noteProgress * (1.0 - attackEndPosition);
    }

    // Apply time scaling
    double scaledPosition = effectiveProgress * timeScale;

    // Apply loop mode
    double loopedPosition = applyLoopMode(scaledPosition);

    // Evaluate envelope curve
    double envelopeValue = evaluateEnvelope(loopedPosition);

    // Apply value scaling and offset
    double finalValue = envelopeValue * valueScale + valueOffset;

    return finalValue;
}

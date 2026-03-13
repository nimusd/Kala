#include "gateprocessor.h"

GateProcessor::GateProcessor(double sampleRate)
    : sampleRate(sampleRate)
    , velocity(1.0)
    , attackTime(0.01)
    , releaseTime(0.1)
    , velocitySens(0.5)
    , attackCurve(CurveType::Linear)
    , releaseCurve(CurveType::Linear)
    , state(GateState::Off)
    , statePosition(0.0)
    , envelopeOut(0.0)
    , attackTrigger(false)
    , releaseTrigger(false)
{
}

void GateProcessor::reset()
{
    state = GateState::Off;
    statePosition = 0.0;
    envelopeOut = 0.0;
    attackTrigger = false;
    releaseTrigger = false;
}

void GateProcessor::setVelocity(double vel)
{
    velocity = std::clamp(vel, 0.0, 1.0);
}

void GateProcessor::setAttackTime(double seconds)
{
    attackTime = std::clamp(seconds, 0.001, 5.0);
}

void GateProcessor::setReleaseTime(double seconds)
{
    releaseTime = std::clamp(seconds, 0.01, 10.0);
}

void GateProcessor::setAttackCurve(int curveIndex)
{
    switch (curveIndex) {
        case 0: attackCurve = CurveType::Linear; break;
        case 1: attackCurve = CurveType::Exponential; break;
        case 2: attackCurve = CurveType::Logarithmic; break;
        case 3: attackCurve = CurveType::SCurve; break;
        default: attackCurve = CurveType::Linear; break;
    }
}

void GateProcessor::setReleaseCurve(int curveIndex)
{
    switch (curveIndex) {
        case 0: releaseCurve = CurveType::Linear; break;
        case 1: releaseCurve = CurveType::Exponential; break;
        case 2: releaseCurve = CurveType::Logarithmic; break;
        case 3: releaseCurve = CurveType::SCurve; break;
        default: releaseCurve = CurveType::Linear; break;
    }
}

void GateProcessor::setVelocitySens(double sens)
{
    velocitySens = std::clamp(sens, 0.0, 1.0);
}

void GateProcessor::setSampleRate(double rate)
{
    sampleRate = rate;
}

void GateProcessor::noteOn(double vel, bool isLegato)
{
    velocity = std::clamp(vel, 0.0, 1.0);

    if (isLegato) {
        // For legato notes, skip attack and go directly to sustain
        // This prevents re-triggering attacks on connected notes
        state = GateState::Sustain;
        statePosition = 0.0;
        envelopeOut = 1.0;  // Start at full sustain level
        attackTrigger = false;  // Don't fire attack trigger for legato
    } else {
        // Normal note - start attack phase
        state = GateState::Attack;
        statePosition = 0.0;
        attackTrigger = true;  // Fire attack trigger
    }
    releaseTrigger = false;
}

void GateProcessor::noteOff()
{
    if (state == GateState::Attack || state == GateState::Sustain) {
        state = GateState::Release;
        statePosition = 0.0;
        releaseTrigger = true;  // Fire release trigger
        attackTrigger = false;
    }
}

double GateProcessor::applyCurve(double position, CurveType curve) const
{
    position = std::clamp(position, 0.0, 1.0);

    switch (curve) {
        case CurveType::Linear:
            return position;

        case CurveType::Exponential:
            // Fast start, slow end
            return 1.0 - std::exp(-5.0 * position);

        case CurveType::Logarithmic:
            // Slow start, fast end
            return (std::exp(position) - 1.0) / (M_E - 1.0);

        case CurveType::SCurve:
            // Smooth S-curve
            return (std::sin((position - 0.5) * M_PI) + 1.0) * 0.5;

        default:
            return position;
    }
}

void GateProcessor::processSample()
{
    // Clear triggers (they only fire for one sample)
    attackTrigger = false;
    releaseTrigger = false;

    switch (state) {
        case GateState::Off:
            envelopeOut = 0.0;
            statePosition = 0.0;
            break;

        case GateState::Attack: {
            // Advance position based on attack time
            double increment = 1.0 / (attackTime * sampleRate);
            statePosition += increment;

            if (statePosition >= 1.0) {
                // Attack complete, move to sustain
                statePosition = 0.0;
                state = GateState::Sustain;
                envelopeOut = 1.0;
            } else {
                // Apply attack curve
                double curvedPosition = applyCurve(statePosition, attackCurve);
                envelopeOut = curvedPosition;
            }
            break;
        }

        case GateState::Sustain:
            // Hold at maximum
            envelopeOut = 1.0;
            break;

        case GateState::Release: {
            // Advance position based on release time
            double increment = 1.0 / (releaseTime * sampleRate);
            statePosition += increment;

            if (statePosition >= 1.0) {
                // Release complete, move to off
                state = GateState::Off;
                statePosition = 0.0;
                envelopeOut = 0.0;
            } else {
                // Apply release curve (inverted - goes from 1 to 0)
                double curvedPosition = applyCurve(statePosition, releaseCurve);
                envelopeOut = 1.0 - curvedPosition;
            }
            break;
        }
    }

    // Apply velocity sensitivity
    // velocitySens = 0: envelope not affected by velocity
    // velocitySens = 1: envelope fully scaled by velocity
    double velocityScale = 1.0 - velocitySens + (velocitySens * velocity);
    envelopeOut *= velocityScale;
}

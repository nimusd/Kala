#ifndef GATEPROCESSOR_H
#define GATEPROCESSOR_H

#include <algorithm>
#include <cmath>

/**
 * GateProcessor - Handles note on/off lifecycle
 *
 * The birth and death of sound. State machine manages attack, sustain, and
 * release phases with configurable curves.
 *
 * Parameters:
 * - velocity: Note velocity (0-1)
 * - attackTime: Attack duration in seconds
 * - releaseTime: Release duration in seconds
 * - attackCurve: Easing function index for attack
 * - releaseCurve: Easing function index for release
 * - velocitySens: How much velocity affects output (0-1)
 *
 * Outputs:
 * - envelopeOut: Current envelope value (0-1)
 * - stateOut: State code (0=off, 1=attack, 2=sustain, 3=release)
 * - attackTrigger: Fires once at note start (impulse for consonants)
 * - releaseTrigger: Fires once at note end
 *
 * State machine: OFF → ATTACK → SUSTAIN → RELEASE → OFF
 */
class GateProcessor
{
public:
    enum class GateState {
        Off = 0,
        Attack = 1,
        Sustain = 2,
        Release = 3
    };

    enum class CurveType {
        Linear,
        Exponential,
        Logarithmic,
        SCurve
    };

    GateProcessor(double sampleRate = 44100.0);

    // Process one sample and update state
    void processSample();

    // Trigger note on (start attack)
    // isLegato: if true, skip attack phase and go directly to sustain
    void noteOn(double velocity = 1.0, bool isLegato = false);

    // Trigger note off (start release)
    void noteOff();

    // Reset to off state
    void reset();

    // Parameter setters
    void setVelocity(double vel);
    void setAttackTime(double seconds);
    void setReleaseTime(double seconds);
    void setAttackCurve(int curveIndex);
    void setReleaseCurve(int curveIndex);
    void setVelocitySens(double sens);
    void setSampleRate(double rate);

    // Output getters
    double getEnvelopeOut() const { return envelopeOut; }
    int getStateOut() const { return static_cast<int>(state); }
    bool getAttackTrigger() const { return attackTrigger; }
    bool getReleaseTrigger() const { return releaseTrigger; }

    // State query
    GateState getState() const { return state; }
    bool isActive() const { return state != GateState::Off; }

private:
    double applyCurve(double position, CurveType curve) const;

    double sampleRate;
    double velocity;
    double attackTime;
    double releaseTime;
    double velocitySens;
    CurveType attackCurve;
    CurveType releaseCurve;

    // State
    GateState state;
    double statePosition;      // Position within current state (0-1)
    double envelopeOut;        // Current envelope value
    bool attackTrigger;        // True for one sample at attack start
    bool releaseTrigger;       // True for one sample at release start
};

#endif // GATEPROCESSOR_H

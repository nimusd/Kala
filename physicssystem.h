#ifndef PHYSICSSYSTEM_H
#define PHYSICSSYSTEM_H

#include <algorithm>
#include <cmath>

/**
 * PhysicsSystem - Applies mass/spring/damping to parameter changes
 *
 * The core "organic without random" mechanism - prevents clicks from
 * abrupt parameter changes AND creates smooth, physics-based movement.
 *
 * Parameters:
 * - targetValue: Where the parameter wants to go
 * - mass: Resistance to change (0 = instant, higher = more inertia)
 * - springK: Spring constant (pull toward target)
 * - damping: Energy loss per sample (0-1, close to 1 = less damping)
 * - impulse: Trigger to add velocity kick
 * - impulseAmount: Strength of velocity kick
 *
 * Processing (per sample):
 * - force = (target - current) × spring
 * - velocity += force / mass  (or force if mass is 0)
 * - velocity *= damping
 * - current += velocity
 *
 * Dual purpose:
 * - Expression: organic movement when modulating parameters
 * - Stability: prevents clicks from abrupt changes (e.g., formant frequencies)
 */
class PhysicsSystem
{
public:
    PhysicsSystem();

    // Process one sample/step and return current smoothed value
    double processSample(double targetValue);

    // Apply an impulse (velocity kick)
    void applyImpulse(double amount);

    // Reset state (call when starting a new note)
    void reset();
    void reset(double initialValue);

    // Parameter setters
    void setMass(double mass);
    void setSpringK(double springK);
    void setDamping(double damping);
    void setImpulseAmount(double amount);

    // Parameter getters
    double getMass() const { return mass; }
    double getSpringK() const { return springK; }
    double getDamping() const { return damping; }
    double getImpulseAmount() const { return impulseAmount; }
    double getCurrentValue() const { return currentValue; }
    double getVelocity() const { return velocity; }

private:
    // Parameters
    double mass;
    double springK;
    double damping;
    double impulseAmount;

    // State
    double currentValue;
    double velocity;

    // Safety limits
    static constexpr double MAX_VELOCITY = 10000.0;
};

#endif // PHYSICSSYSTEM_H

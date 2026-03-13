#include "physicssystem.h"

PhysicsSystem::PhysicsSystem()
    : mass(0.5)
    , springK(0.001)
    , damping(0.995)
    , impulseAmount(100.0)
    , currentValue(0.0)
    , velocity(0.0)
{
}

void PhysicsSystem::reset()
{
    currentValue = 0.0;
    velocity = 0.0;
}

void PhysicsSystem::reset(double initialValue)
{
    currentValue = initialValue;
    velocity = 0.0;
}

void PhysicsSystem::setMass(double mass)
{
    this->mass = std::clamp(mass, 0.0, 10.0);
}

void PhysicsSystem::setSpringK(double springK)
{
    this->springK = std::clamp(springK, 0.0001, 1.0);
}

void PhysicsSystem::setDamping(double damping)
{
    this->damping = std::clamp(damping, 0.5, 0.9999);
}

void PhysicsSystem::setImpulseAmount(double amount)
{
    this->impulseAmount = std::clamp(amount, 0.0, 1000.0);
}

void PhysicsSystem::applyImpulse(double amount)
{
    velocity += amount;

    // Clamp velocity to prevent instability
    velocity = std::clamp(velocity, -MAX_VELOCITY, MAX_VELOCITY);
}

double PhysicsSystem::processSample(double targetValue)
{
    // Physics simulation:
    // force = (target - current) × spring
    double force = (targetValue - currentValue) * springK;

    // Apply force to velocity (accounting for mass)
    if (mass > 0.0001) {
        // With mass: acceleration = force / mass
        velocity += force / mass;
    } else {
        // No mass: instant response
        velocity += force;
    }

    // Apply damping (energy loss)
    velocity *= damping;

    // Clamp velocity to prevent instability
    velocity = std::clamp(velocity, -MAX_VELOCITY, MAX_VELOCITY);

    // Update position
    currentValue += velocity;

    return currentValue;
}

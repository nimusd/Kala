#include "driftengine.h"
#include <cstdlib>
#include <ctime>

DriftEngine::DriftEngine(double sampleRate)
    : sampleRate(sampleRate)
    , amount(0.005)
    , rate(0.5)
    , driftPattern(DriftPattern::Perlin)
    , phase(0.0)
    , phase2(0.0)
    , phase3(0.0)
    , randomValue(0.0)
    , randomCounter(0.0)
    , randomUpdateRate(0.1)
{
    // Seed random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

void DriftEngine::reset()
{
    phase = 0.0;
    phase2 = 0.0;
    phase3 = 0.0;
    randomValue = 0.0;
    randomCounter = 0.0;
}

void DriftEngine::setAmount(double amount)
{
    this->amount = std::clamp(amount, 0.0, 0.1);
}

void DriftEngine::setRate(double rateHz)
{
    this->rate = std::clamp(rateHz, 0.01, 11.0);
}

void DriftEngine::setDriftPattern(DriftPattern pattern)
{
    this->driftPattern = pattern;
}

void DriftEngine::setSampleRate(double rate)
{
    this->sampleRate = rate;
}

double DriftEngine::generateSample()
{
    double driftValue = 0.0;

    switch (driftPattern) {
        case DriftPattern::Sine: {
            // Simple sine wave LFO
            driftValue = std::sin(phase);

            // Advance phase
            phase += 2.0 * M_PI * rate / sampleRate;
            if (phase > 2.0 * M_PI * 1000.0) {
                phase -= 2.0 * M_PI * 1000.0;
            }
            break;
        }

        case DriftPattern::Random: {
            // Sample-and-hold random values
            randomCounter += 1.0 / sampleRate;

            // Update random value at specified rate
            if (randomCounter >= randomUpdateRate) {
                randomValue = (static_cast<double>(std::rand()) / RAND_MAX) * 2.0 - 1.0;
                randomCounter = 0.0;
            }

            driftValue = randomValue;
            break;
        }

        case DriftPattern::Perlin: {
            // Simulated Perlin noise using multiple sine waves at different frequencies
            // Creates smooth, organic-looking drift
            double sine1 = std::sin(phase);
            double sine2 = std::sin(phase2 * 1.7) * 0.5;
            double sine3 = std::sin(phase3 * 2.3) * 0.25;

            driftValue = sine1 + sine2 + sine3;
            driftValue /= 1.75;  // Normalize

            // Advance phases at slightly different rates for organic quality
            phase += 2.0 * M_PI * rate / sampleRate;
            phase2 += 2.0 * M_PI * rate * 0.83 / sampleRate;
            phase3 += 2.0 * M_PI * rate * 1.27 / sampleRate;

            // Wrap phases
            if (phase > 2.0 * M_PI * 1000.0) {
                phase -= 2.0 * M_PI * 1000.0;
            }
            if (phase2 > 2.0 * M_PI * 1000.0) {
                phase2 -= 2.0 * M_PI * 1000.0;
            }
            if (phase3 > 2.0 * M_PI * 1000.0) {
                phase3 -= 2.0 * M_PI * 1000.0;
            }
            break;
        }

        default:
            driftValue = 0.0;
            break;
    }

    // Convert to detuning multiplier: 1.0 ± (amount × driftValue)
    // Output range: approximately [1.0 - amount, 1.0 + amount]
    double detuningMultiplier = 1.0 + (driftValue * amount);

    return detuningMultiplier;
}

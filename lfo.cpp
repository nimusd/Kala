#include "lfo.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

LFO::LFO(double sampleRate)
    : sampleRate(sampleRate)
    , frequency(1.0)    // Default 1 Hz
    , amplitude(1.0)    // Default full amplitude
    , waveType(Sine)    // Default sine wave
    , phase(0.0)
{
}

void LFO::setFrequency(double freq)
{
    // Clamp to reasonable LFO range (0.01 Hz to 100 Hz)
    frequency = std::max(0.01, std::min(freq, 100.0));
}

void LFO::setAmplitude(double amp)
{
    // Clamp to 0.0 - 1.0
    amplitude = std::max(0.0, std::min(amp, 1.0));
}

void LFO::setWaveType(WaveType type)
{
    waveType = type;
}

void LFO::setWaveType(int type)
{
    if (type >= 0 && type <= 3) {
        waveType = static_cast<WaveType>(type);
    }
}

double LFO::process()
{
    // Generate the waveform value at current phase
    double value = generateWave(phase) * amplitude;

    // Advance phase
    double phaseIncrement = frequency / sampleRate;
    phase += phaseIncrement;

    // Wrap phase to [0, 1)
    while (phase >= 1.0) {
        phase -= 1.0;
    }

    return value;
}

void LFO::reset()
{
    phase = 0.0;
}

double LFO::generateWave(double p) const
{
    switch (waveType) {
        case Sine:
            // Sine wave: -1 to +1
            return std::sin(2.0 * M_PI * p);

        case Triangle:
            // Triangle wave: starts at 0, rises to 1, falls to -1, returns to 0
            if (p < 0.25) {
                return 4.0 * p;  // 0 to 1
            } else if (p < 0.75) {
                return 2.0 - 4.0 * p;  // 1 to -1
            } else {
                return -4.0 + 4.0 * p;  // -1 to 0
            }

        case Square:
            // Square wave: +1 for first half, -1 for second half
            return (p < 0.5) ? 1.0 : -1.0;

        case Sawtooth:
            // Sawtooth wave: rises from -1 to +1
            return 2.0 * p - 1.0;

        default:
            return 0.0;
    }
}

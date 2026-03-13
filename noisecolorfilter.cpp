#include "noisecolorfilter.h"
#include <algorithm>
#include <chrono>

NoiseColorFilter::NoiseColorFilter(double sampleRate)
    : sampleRate(sampleRate)
    , color(2000.0)
    , filterQ(1.0)
    , filterType(FilterType::Highpass)
    , noiseType(NoiseType::White)
    , useInternal(true)
    , dist(-1.0, 1.0)
    , pinkB0(0.0), pinkB1(0.0), pinkB2(0.0), pinkB3(0.0), pinkB4(0.0), pinkB5(0.0), pinkB6(0.0)
    , brownLast(0.0)
{
    // Initialize random number generator with time-based seed
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    rng.seed(static_cast<unsigned int>(seed));

    updateFilterCoefficients();
}

void NoiseColorFilter::reset()
{
    filter.reset();
    // Reset pink noise state
    pinkB0 = pinkB1 = pinkB2 = pinkB3 = pinkB4 = pinkB5 = pinkB6 = 0.0;
    // Reset brown noise state
    brownLast = 0.0;
}

void NoiseColorFilter::setColor(double freq)
{
    color = std::clamp(freq, 100.0, 12000.0);
    updateFilterCoefficients();
}

void NoiseColorFilter::setFilterQ(double q)
{
    filterQ = std::clamp(q, 0.5, 20.0);
    updateFilterCoefficients();
}

void NoiseColorFilter::setFilterType(FilterType type)
{
    filterType = type;
    updateFilterCoefficients();
}

void NoiseColorFilter::setNoiseType(NoiseType type)
{
    noiseType = type;
}

void NoiseColorFilter::setUseInternal(bool useInternal)
{
    this->useInternal = useInternal;
}

void NoiseColorFilter::setSampleRate(double rate)
{
    sampleRate = rate;
    updateFilterCoefficients();
}

void NoiseColorFilter::updateFilterCoefficients()
{
    // Biquad filter coefficients
    // Reference: Audio EQ Cookbook by Robert Bristow-Johnson

    double omega = 2.0 * M_PI * color / sampleRate;
    double sinOmega = std::sin(omega);
    double cosOmega = std::cos(omega);
    double alpha = sinOmega / (2.0 * filterQ);

    double a0;

    switch (filterType) {
        case FilterType::Lowpass:
            // Lowpass filter
            a0 = 1.0 + alpha;
            filter.b0 = ((1.0 - cosOmega) / 2.0) / a0;
            filter.b1 = (1.0 - cosOmega) / a0;
            filter.b2 = ((1.0 - cosOmega) / 2.0) / a0;
            filter.a1 = (-2.0 * cosOmega) / a0;
            filter.a2 = (1.0 - alpha) / a0;
            break;

        case FilterType::Highpass:
            // Highpass filter
            a0 = 1.0 + alpha;
            filter.b0 = ((1.0 + cosOmega) / 2.0) / a0;
            filter.b1 = -(1.0 + cosOmega) / a0;
            filter.b2 = ((1.0 + cosOmega) / 2.0) / a0;
            filter.a1 = (-2.0 * cosOmega) / a0;
            filter.a2 = (1.0 - alpha) / a0;
            break;

        case FilterType::Bandpass:
            // Bandpass filter (constant 0 dB peak gain)
            a0 = 1.0 + alpha;
            filter.b0 = alpha / a0;
            filter.b1 = 0.0;
            filter.b2 = -alpha / a0;
            filter.a1 = -2.0 * cosOmega / a0;
            filter.a2 = (1.0 - alpha) / a0;
            break;
    }
}

double NoiseColorFilter::generateWhiteNoise()
{
    return dist(rng);
}

double NoiseColorFilter::generatePinkNoise()
{
    // Paul Kellet's implementation of pink noise
    // http://www.firstpr.com.au/dsp/pink-noise/
    double white = generateWhiteNoise();

    pinkB0 = 0.99886 * pinkB0 + white * 0.0555179;
    pinkB1 = 0.99332 * pinkB1 + white * 0.0750759;
    pinkB2 = 0.96900 * pinkB2 + white * 0.1538520;
    pinkB3 = 0.86650 * pinkB3 + white * 0.3104856;
    pinkB4 = 0.55000 * pinkB4 + white * 0.5329522;
    pinkB5 = -0.7616 * pinkB5 - white * 0.0168980;

    double pink = pinkB0 + pinkB1 + pinkB2 + pinkB3 + pinkB4 + pinkB5 + pinkB6 + white * 0.5362;
    pinkB6 = white * 0.115926;

    return pink * 0.11; // Scale to approximately [-1, 1]
}

double NoiseColorFilter::generateBrownNoise()
{
    // Brownian noise (random walk)
    double white = generateWhiteNoise();
    brownLast = (brownLast + (0.02 * white)) / 1.02;

    return brownLast * 3.5; // Scale to approximately [-1, 1]
}

double NoiseColorFilter::generateSample()
{
    // Generate internal noise based on selected type
    double noise;
    switch (noiseType) {
        case NoiseType::Pink:
            noise = generatePinkNoise();
            break;
        case NoiseType::Brown:
            noise = generateBrownNoise();
            break;
        case NoiseType::White:
        default:
            noise = generateWhiteNoise();
            break;
    }

    // Filter the noise
    return filter.process(noise);
}

double NoiseColorFilter::processSample(double noiseIn)
{
    // Process external noise input
    return filter.process(noiseIn);
}

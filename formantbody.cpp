#include "formantbody.h"
#include <algorithm>

FormantBody::FormantBody(double sampleRate)
    : sampleRate(sampleRate)
    , f1Freq(500.0)
    , f2Freq(1500.0)
    , f1Q(8.0)
    , f2Q(10.0)
    , directMix(0.3)
    , f1f2Balance(0.6)
{
    updateF1Coefficients();
    updateF2Coefficients();
}

void FormantBody::reset()
{
    f1Filter.reset();
    f2Filter.reset();
}

void FormantBody::setF1Freq(double freq)
{
    f1Freq = std::clamp(freq, 200.0, 1000.0);
    updateF1Coefficients();
}

void FormantBody::setF2Freq(double freq)
{
    f2Freq = std::clamp(freq, 500.0, 4000.0);
    updateF2Coefficients();
}

void FormantBody::setF1Q(double q)
{
    f1Q = std::clamp(q, 1.0, 20.0);
    updateF1Coefficients();
}

void FormantBody::setF2Q(double q)
{
    f2Q = std::clamp(q, 1.0, 20.0);
    updateF2Coefficients();
}

void FormantBody::setDirectMix(double mix)
{
    directMix = std::clamp(mix, 0.0, 1.0);
}

void FormantBody::setF1F2Balance(double balance)
{
    f1f2Balance = std::clamp(balance, 0.0, 1.0);
}

void FormantBody::setSampleRate(double rate)
{
    sampleRate = rate;
    updateF1Coefficients();
    updateF2Coefficients();
}

void FormantBody::updateF1Coefficients()
{
    // Bandpass biquad filter coefficients
    // Reference: Audio EQ Cookbook by Robert Bristow-Johnson

    double omega = 2.0 * M_PI * f1Freq / sampleRate;
    double sinOmega = std::sin(omega);
    double cosOmega = std::cos(omega);
    double alpha = sinOmega / (2.0 * f1Q);

    // Bandpass filter (constant 0 dB peak gain)
    double a0 = 1.0 + alpha;
    f1Filter.b0 = alpha / a0;
    f1Filter.b1 = 0.0;
    f1Filter.b2 = -alpha / a0;
    f1Filter.a1 = -2.0 * cosOmega / a0;
    f1Filter.a2 = (1.0 - alpha) / a0;
}

void FormantBody::updateF2Coefficients()
{
    // Bandpass biquad filter coefficients

    double omega = 2.0 * M_PI * f2Freq / sampleRate;
    double sinOmega = std::sin(omega);
    double cosOmega = std::cos(omega);
    double alpha = sinOmega / (2.0 * f2Q);

    // Bandpass filter (constant 0 dB peak gain)
    double a0 = 1.0 + alpha;
    f2Filter.b0 = alpha / a0;
    f2Filter.b1 = 0.0;
    f2Filter.b2 = -alpha / a0;
    f2Filter.a1 = -2.0 * cosOmega / a0;
    f2Filter.a2 = (1.0 - alpha) / a0;
}

double FormantBody::processSample(double input)
{
    // Process both formant filters in parallel
    double f1Out = f1Filter.process(input);
    double f2Out = f2Filter.process(input);

    // Blend F1 and F2 based on balance parameter
    // f1f2Balance: 0 = F2 only, 1 = F1 only
    double formantBlend = f1Out * f1f2Balance + f2Out * (1.0 - f1f2Balance);

    // Mix direct and filtered signal
    // directMix: 0 = all filtered, 1 = all direct
    double output = input * directMix + formantBlend * (1.0 - directMix);

    return output;
}

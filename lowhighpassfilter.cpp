#include "lowhighpassfilter.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

LowHighPassFilter::LowHighPassFilter(double sampleRate)
    : m_sampleRate(sampleRate)
    , m_mode(LowPass)
    , m_cutoff(1000.0)
    , m_resonance(0.707)
    , m_b0(1.0), m_b1(0.0), m_b2(0.0)
    , m_a1(0.0), m_a2(0.0)
    , m_x1(0.0), m_x2(0.0)
    , m_y1(0.0), m_y2(0.0)
{
    updateCoefficients();
}

void LowHighPassFilter::setSampleRate(double rate)
{
    m_sampleRate = rate;
    updateCoefficients();
    reset();
}

void LowHighPassFilter::reset()
{
    m_x1 = m_x2 = 0.0;
    m_y1 = m_y2 = 0.0;
}

void LowHighPassFilter::setMode(int mode)
{
    int newMode = (mode == HighPass) ? HighPass : LowPass;
    if (newMode != m_mode) {
        m_mode = newMode;
        updateCoefficients();
    }
}

void LowHighPassFilter::setCutoff(double hz)
{
    hz = std::clamp(hz, 20.0, 20000.0);
    if (hz != m_cutoff) {
        m_cutoff = hz;
        updateCoefficients();
    }
}

void LowHighPassFilter::setResonance(double q)
{
    q = std::clamp(q, 0.5, 20.0);
    if (q != m_resonance) {
        m_resonance = q;
        updateCoefficients();
    }
}

void LowHighPassFilter::updateCoefficients()
{
    // Audio EQ Cookbook by Robert Bristow-Johnson
    double w0 = 2.0 * M_PI * m_cutoff / m_sampleRate;
    double cosw0 = cos(w0);
    double sinw0 = sin(w0);
    double alpha = sinw0 / (2.0 * m_resonance);

    double a0;

    if (m_mode == LowPass) {
        // LPF: H(s) = 1 / (s^2 + s/Q + 1)
        m_b0 = (1.0 - cosw0) / 2.0;
        m_b1 = 1.0 - cosw0;
        m_b2 = (1.0 - cosw0) / 2.0;
        a0   = 1.0 + alpha;
        m_a1 = -2.0 * cosw0;
        m_a2 = 1.0 - alpha;
    } else {
        // HPF: H(s) = s^2 / (s^2 + s/Q + 1)
        m_b0 = (1.0 + cosw0) / 2.0;
        m_b1 = -(1.0 + cosw0);
        m_b2 = (1.0 + cosw0) / 2.0;
        a0   = 1.0 + alpha;
        m_a1 = -2.0 * cosw0;
        m_a2 = 1.0 - alpha;
    }

    // Normalize by a0
    m_b0 /= a0;
    m_b1 /= a0;
    m_b2 /= a0;
    m_a1 /= a0;
    m_a2 /= a0;
}

double LowHighPassFilter::processSample(double input)
{
    // Direct Form I
    double output = m_b0 * input + m_b1 * m_x1 + m_b2 * m_x2
                  - m_a1 * m_y1 - m_a2 * m_y2;

    // Update state
    m_x2 = m_x1;
    m_x1 = input;
    m_y2 = m_y1;
    m_y1 = output;

    return output;
}

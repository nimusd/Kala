#include "combfilter.h"
#include <algorithm>
#include <cmath>

CombFilter::CombFilter(double sampleRate)
    : m_sampleRate(sampleRate)
    , m_mode(Feedforward)
    , m_delayTimeMs(5.0)
    , m_feedback(0.5)
    , m_damping(0.0)
    , m_writeIndex(0)
    , m_dampState(0.0)
{
    // Max delay = 100ms at current sample rate
    m_maxDelaySamples = static_cast<int>(m_sampleRate * 0.1) + 1;
    m_buffer.resize(m_maxDelaySamples, 0.0);
}

void CombFilter::setSampleRate(double rate)
{
    m_sampleRate = rate;
    m_maxDelaySamples = static_cast<int>(m_sampleRate * 0.1) + 1;
    m_buffer.assign(m_maxDelaySamples, 0.0);
    m_writeIndex = 0;
    m_dampState = 0.0;
}

void CombFilter::reset()
{
    std::fill(m_buffer.begin(), m_buffer.end(), 0.0);
    m_writeIndex = 0;
    m_dampState = 0.0;
}

void CombFilter::setMode(int mode)
{
    m_mode = (mode == Feedback) ? Feedback : Feedforward;
}

void CombFilter::setDelayTime(double ms)
{
    m_delayTimeMs = std::clamp(ms, 0.1, 100.0);
}

void CombFilter::setFeedback(double fb)
{
    m_feedback = std::clamp(fb, -0.99, 0.99);
}

void CombFilter::setDamping(double damp)
{
    m_damping = std::clamp(damp, 0.0, 1.0);
}

double CombFilter::readDelay(double delaySamples) const
{
    // Linear interpolation for fractional delay
    double readPos = m_writeIndex - delaySamples;
    if (readPos < 0.0) readPos += m_maxDelaySamples;

    int idx0 = static_cast<int>(readPos);
    int idx1 = idx0 + 1;
    double frac = readPos - idx0;

    if (idx0 >= m_maxDelaySamples) idx0 -= m_maxDelaySamples;
    if (idx1 >= m_maxDelaySamples) idx1 -= m_maxDelaySamples;

    return m_buffer[idx0] + frac * (m_buffer[idx1] - m_buffer[idx0]);
}

double CombFilter::processSample(double input)
{
    double delaySamples = m_delayTimeMs * 0.001 * m_sampleRate;
    delaySamples = std::clamp(delaySamples, 1.0, static_cast<double>(m_maxDelaySamples - 1));

    double delayed = readDelay(delaySamples);

    // Apply one-pole lowpass damping to the delayed signal
    // filtered = prev + (1 - damping) * (input - prev)
    double filtered = m_dampState + (1.0 - m_damping) * (delayed - m_dampState);
    m_dampState = filtered;

    double output;
    if (m_mode == Feedforward) {
        // FIR: y[n] = x[n] + feedback * filtered_x[n-D]
        output = input + m_feedback * filtered;
        m_buffer[m_writeIndex] = input;
    } else {
        // IIR: y[n] = x[n] + feedback * filtered_y[n-D]
        output = input + m_feedback * filtered;
        m_buffer[m_writeIndex] = output;
    }

    m_writeIndex++;
    if (m_writeIndex >= m_maxDelaySamples) {
        m_writeIndex = 0;
    }

    return output;
}

#ifndef COMBFILTER_H
#define COMBFILTER_H

#include <vector>

/**
 * CombFilter - Feedforward (FIR) and Feedback (IIR) comb filter
 *
 * Feedforward: y[n] = x[n] + feedback * x[n-D]  (evenly-spaced notches)
 * Feedback:    y[n] = x[n] + feedback * y[n-D]  (resonant peaks / pitched comb)
 *
 * Damping applies a one-pole lowpass in the delay/feedback path to tame highs.
 * Delay time is modulatable for flanging effects.
 */
class CombFilter
{
public:
    enum Mode { Feedforward = 0, Feedback = 1 };

    CombFilter(double sampleRate = 44100.0);

    double processSample(double input);
    void reset();
    void setSampleRate(double rate);

    void setMode(int mode);
    void setDelayTime(double ms);       // 0.1 – 100 ms
    void setFeedback(double fb);        // -0.99 – 0.99
    void setDamping(double damp);       // 0.0 – 1.0

    int getMode() const { return m_mode; }
    double getDelayTime() const { return m_delayTimeMs; }
    double getFeedback() const { return m_feedback; }
    double getDamping() const { return m_damping; }

private:
    double m_sampleRate;
    int m_mode;
    double m_delayTimeMs;
    double m_feedback;
    double m_damping;

    // Circular delay buffer
    std::vector<double> m_buffer;
    int m_writeIndex;
    int m_maxDelaySamples;

    // One-pole lowpass state for damping
    double m_dampState;

    double readDelay(double delaySamples) const;
};

#endif // COMBFILTER_H

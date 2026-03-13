#ifndef LOWHIGHPASSFILTER_H
#define LOWHIGHPASSFILTER_H

/**
 * LowHighPassFilter - Biquad low-pass / high-pass filter
 *
 * Uses Audio EQ Cookbook coefficients for lowpass and highpass.
 * Mode selector switches between LP and HP.
 * Cutoff and resonance are modulatable via input ports.
 */
class LowHighPassFilter
{
public:
    enum Mode { LowPass = 0, HighPass = 1 };

    LowHighPassFilter(double sampleRate = 44100.0);

    double processSample(double input);
    void reset();
    void setSampleRate(double rate);

    void setMode(int mode);
    void setCutoff(double hz);      // 20–20000
    void setResonance(double q);    // 0.5–20

    int getMode() const { return m_mode; }
    double getCutoff() const { return m_cutoff; }
    double getResonance() const { return m_resonance; }

private:
    double m_sampleRate;
    int m_mode;
    double m_cutoff;
    double m_resonance;

    // Biquad coefficients
    double m_b0, m_b1, m_b2;
    double m_a1, m_a2;

    // Biquad state (Direct Form II Transposed)
    double m_x1, m_x2;
    double m_y1, m_y2;

    void updateCoefficients();
};

#endif // LOWHIGHPASSFILTER_H

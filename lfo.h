#ifndef LFO_H
#define LFO_H

#include <cmath>

/**
 * LFO - Low Frequency Oscillator
 *
 * Generates periodic waveforms at low frequencies for modulation purposes.
 * Supports sine, triangle, square, and sawtooth waveforms.
 */
class LFO
{
public:
    enum WaveType {
        Sine = 0,
        Triangle = 1,
        Square = 2,
        Sawtooth = 3
    };

    LFO(double sampleRate = 44100.0);

    // Parameters
    void setFrequency(double freq);  // Hz (typically 0.01 - 20 Hz for LFO)
    void setAmplitude(double amp);   // 0.0 - 1.0
    void setWaveType(WaveType type);
    void setWaveType(int type);      // Convenience overload

    double getFrequency() const { return frequency; }
    double getAmplitude() const { return amplitude; }
    WaveType getWaveType() const { return waveType; }

    // Generate next sample (call once per audio sample)
    double process();

    // Reset phase to 0
    void reset();

private:
    double sampleRate;
    double frequency;
    double amplitude;
    WaveType waveType;
    double phase;  // 0.0 to 1.0

    double generateWave(double phase) const;
};

#endif // LFO_H

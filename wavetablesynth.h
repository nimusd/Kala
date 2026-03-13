#ifndef WAVETABLESYNTH_H
#define WAVETABLESYNTH_H

#include <vector>
#include <complex>
#include <cmath>
#include <QString>
#include "spectrum.h"

class WavetableSynth
{
public:
    enum Preset { Saw, Square, Triangle, Pulse, SuperSaw, Formant };

    // Output mode: which ports are active
    enum OutputMode { Both = 0, SignalOnly = 1, SpectrumOnly = 2 };

    WavetableSynth(double sampleRate = 44100.0);

    // Generate a single audio sample at given pitch (Hz)
    double generateSample(double pitch);

    // Get FFT spectrum of current interpolated frame
    Spectrum getCurrentSpectrum() const { return cachedSpectrum; }

    // Reset phase accumulator (call on new note)
    void reset();

    // Load a built-in preset wavetable
    void loadPreset(Preset preset);

    // Load wavetable from raw sample data
    // totalSamples = total number of float samples
    // cycleLength = samples per single cycle/frame
    void loadFromWav(const float *data, int totalSamples, int cycleLength);

    // Static helper: load WAV file into float vector (mono, normalized)
    // Returns true on success
    static bool loadWavFile(const QString &path, std::vector<float> &out);

    // Setters
    void setPosition(double pos) { position = std::max(0.0, std::min(1.0, pos)); recomputeSpectrum(); }
    void setCycleLength(int len) { cycleLength = len; }
    void setOutputMode(OutputMode mode) { outputMode = mode; }
    void setSampleRate(double rate) { sampleRate = rate; }

    // Getters
    double getPosition() const { return position; }
    int getCycleLength() const { return cycleLength; }
    OutputMode getOutputMode() const { return outputMode; }
    int getFrameCount() const { return static_cast<int>(frames.size()); }

private:
    double sampleRate;
    double phase = 0.0;          // Phase accumulator [0, cycleLength)
    double position = 0.0;       // Frame morph position [0, 1]
    int cycleLength = 2048;      // Samples per frame
    OutputMode outputMode = Both;

    // Wavetable data: each frame is one cycle
    std::vector<std::vector<float>> frames;

    // Mipmap levels: band-limited copies at decreasing bandwidth
    // mipmap[level][frame][sample]
    std::vector<std::vector<std::vector<float>>> mipmaps;

    // Cached spectrum (recomputed when position changes)
    Spectrum cachedSpectrum;
    double lastSpectrumPosition = -1.0;

    // Build mipmap chain from current frames
    void buildMipmaps();

    // Recompute cached FFT spectrum from current position
    void recomputeSpectrum();

    // Simple Cooley-Tukey FFT (radix-2, in-place)
    static void fft(std::vector<std::complex<double>> &x);

    // Generate band-limited preset tables using additive synthesis
    void generateSawTable(int tableSize);
    void generateSquareTable(int tableSize);
    void generateTriangleTable(int tableSize);
    void generatePulseTable(int tableSize, double dutyCycle = 0.25);
    void generateSuperSawTable(int tableSize);
    void generateFormantTable(int tableSize);

    // Interpolate between two frames
    void interpolateFrame(const std::vector<float> &a, const std::vector<float> &b,
                          float t, std::vector<float> &out) const;

    // Select mipmap level based on pitch
    int selectMipmapLevel(double pitch) const;
};

#endif // WAVETABLESYNTH_H

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include "wavetablesynth.h"
#include <algorithm>
#include <numeric>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

WavetableSynth::WavetableSynth(double sampleRate)
    : sampleRate(sampleRate)
{
    loadPreset(Saw);
}

void WavetableSynth::reset()
{
    phase = 0.0;
}

// ---------------------------------------------------------------------------
// Playback
// ---------------------------------------------------------------------------

double WavetableSynth::generateSample(double pitch)
{
    if (frames.empty() || cycleLength <= 0) return 0.0;

    int numFrames = static_cast<int>(frames.size());

    // Select mipmap level based on pitch to avoid aliasing
    int mipLevel = selectMipmapLevel(pitch);

    // Frame index from position
    double framePos = position * (numFrames - 1);
    int frameA = static_cast<int>(framePos);
    int frameB = std::min(frameA + 1, numFrames - 1);
    float frameFrac = static_cast<float>(framePos - frameA);

    // Get the right mipmap level frames
    const auto &mipFrames = (mipLevel < static_cast<int>(mipmaps.size()))
                            ? mipmaps[mipLevel] : frames;

    const auto &fA = mipFrames[frameA];
    const auto &fB = mipFrames[frameB];
    int len = static_cast<int>(fA.size());
    if (len == 0) return 0.0;

    // Linear interpolation within cycle (phase -> sample)
    double samplePos = phase;
    int idxA = static_cast<int>(samplePos) % len;
    int idxB = (idxA + 1) % len;
    double frac = samplePos - std::floor(samplePos);

    // Bilinear interp: frame × sample
    double sA = fA[idxA] * (1.0 - frac) + fA[idxB] * frac;
    double sB = fB[idxA] * (1.0 - frac) + fB[idxB] * frac;
    double sample = sA * (1.0 - frameFrac) + sB * frameFrac;

    // Advance phase
    double phaseInc = pitch * len / sampleRate;
    phase += phaseInc;
    while (phase >= len) phase -= len;

    return sample;
}

// ---------------------------------------------------------------------------
// Preset generation (band-limited additive synthesis)
// ---------------------------------------------------------------------------

void WavetableSynth::loadPreset(Preset preset)
{
    switch (preset) {
    case Saw:      generateSawTable(cycleLength);      break;
    case Square:   generateSquareTable(cycleLength);    break;
    case Triangle: generateTriangleTable(cycleLength);  break;
    case Pulse:    generatePulseTable(cycleLength);     break;
    case SuperSaw: generateSuperSawTable(cycleLength);  break;
    case Formant:  generateFormantTable(cycleLength);   break;
    }
    buildMipmaps();
    recomputeSpectrum();
}

void WavetableSynth::generateSawTable(int tableSize)
{
    // Single frame saw via additive synthesis
    frames.clear();
    frames.resize(1);
    auto &frame = frames[0];
    frame.resize(tableSize, 0.0f);

    int maxHarm = tableSize / 2;
    for (int h = 1; h <= maxHarm; h++) {
        double amp = 1.0 / h;
        for (int i = 0; i < tableSize; i++) {
            double t = 2.0 * M_PI * h * i / tableSize;
            frame[i] += static_cast<float>(amp * std::sin(t));
        }
    }
    // Normalize
    float peak = *std::max_element(frame.begin(), frame.end(),
                                    [](float a, float b){ return std::abs(a) < std::abs(b); });
    if (peak > 0.0f) {
        peak = std::abs(peak);
        for (auto &s : frame) s /= peak;
    }
}

void WavetableSynth::generateSquareTable(int tableSize)
{
    frames.clear();
    frames.resize(1);
    auto &frame = frames[0];
    frame.resize(tableSize, 0.0f);

    int maxHarm = tableSize / 2;
    for (int h = 1; h <= maxHarm; h += 2) {
        double amp = 1.0 / h;
        for (int i = 0; i < tableSize; i++) {
            double t = 2.0 * M_PI * h * i / tableSize;
            frame[i] += static_cast<float>(amp * std::sin(t));
        }
    }
    float peak = *std::max_element(frame.begin(), frame.end(),
                                    [](float a, float b){ return std::abs(a) < std::abs(b); });
    if (peak > 0.0f) {
        peak = std::abs(peak);
        for (auto &s : frame) s /= peak;
    }
}

void WavetableSynth::generateTriangleTable(int tableSize)
{
    frames.clear();
    frames.resize(1);
    auto &frame = frames[0];
    frame.resize(tableSize, 0.0f);

    int maxHarm = tableSize / 2;
    for (int h = 1; h <= maxHarm; h += 2) {
        double amp = 1.0 / (h * h);
        double sign = ((h / 2) % 2 == 0) ? 1.0 : -1.0;
        for (int i = 0; i < tableSize; i++) {
            double t = 2.0 * M_PI * h * i / tableSize;
            frame[i] += static_cast<float>(sign * amp * std::sin(t));
        }
    }
    float peak = *std::max_element(frame.begin(), frame.end(),
                                    [](float a, float b){ return std::abs(a) < std::abs(b); });
    if (peak > 0.0f) {
        peak = std::abs(peak);
        for (auto &s : frame) s /= peak;
    }
}

void WavetableSynth::generatePulseTable(int tableSize, double dutyCycle)
{
    frames.clear();
    frames.resize(1);
    auto &frame = frames[0];
    frame.resize(tableSize, 0.0f);

    int maxHarm = tableSize / 2;
    for (int h = 1; h <= maxHarm; h++) {
        double amp = (2.0 / (h * M_PI)) * std::sin(h * M_PI * dutyCycle);
        for (int i = 0; i < tableSize; i++) {
            double t = 2.0 * M_PI * h * i / tableSize;
            frame[i] += static_cast<float>(amp * std::sin(t));
        }
    }
    float peak = *std::max_element(frame.begin(), frame.end(),
                                    [](float a, float b){ return std::abs(a) < std::abs(b); });
    if (peak > 0.0f) {
        peak = std::abs(peak);
        for (auto &s : frame) s /= peak;
    }
}

void WavetableSynth::generateSuperSawTable(int tableSize)
{
    // 7 detuned saws (classic supersaw)
    frames.clear();
    frames.resize(1);
    auto &frame = frames[0];
    frame.resize(tableSize, 0.0f);

    double detunes[] = { -0.11, -0.06, -0.02, 0.0, 0.02, 0.06, 0.11 };
    int maxHarm = tableSize / 4; // fewer harmonics since we have 7 stacked

    for (int d = 0; d < 7; d++) {
        double ratio = 1.0 + detunes[d];
        for (int h = 1; h <= maxHarm; h++) {
            double amp = 1.0 / (7.0 * h);
            for (int i = 0; i < tableSize; i++) {
                double t = 2.0 * M_PI * h * ratio * i / tableSize;
                frame[i] += static_cast<float>(amp * std::sin(t));
            }
        }
    }
    float peak = *std::max_element(frame.begin(), frame.end(),
                                    [](float a, float b){ return std::abs(a) < std::abs(b); });
    if (peak > 0.0f) {
        peak = std::abs(peak);
        for (auto &s : frame) s /= peak;
    }
}

void WavetableSynth::generateFormantTable(int tableSize)
{
    // Formant-like waveform: harmonics weighted by vowel formant curves
    frames.clear();
    frames.resize(1);
    auto &frame = frames[0];
    frame.resize(tableSize, 0.0f);

    // Approximate "ah" vowel: F1~730 Hz, F2~1090 Hz (relative to fundamental)
    // We just shape the harmonic amplitudes with gaussian bumps
    int maxHarm = tableSize / 2;
    double f1Center = 5.0;  // 5th harmonic ~ F1
    double f2Center = 9.0;  // 9th harmonic ~ F2
    double bw = 2.0;        // bandwidth in harmonics

    for (int h = 1; h <= maxHarm; h++) {
        double baseAmp = 1.0 / h;
        double f1Gain = std::exp(-0.5 * std::pow((h - f1Center) / bw, 2));
        double f2Gain = std::exp(-0.5 * std::pow((h - f2Center) / bw, 2));
        double amp = baseAmp * (1.0 + 3.0 * f1Gain + 2.0 * f2Gain);
        for (int i = 0; i < tableSize; i++) {
            double t = 2.0 * M_PI * h * i / tableSize;
            frame[i] += static_cast<float>(amp * std::sin(t));
        }
    }
    float peak = *std::max_element(frame.begin(), frame.end(),
                                    [](float a, float b){ return std::abs(a) < std::abs(b); });
    if (peak > 0.0f) {
        peak = std::abs(peak);
        for (auto &s : frame) s /= peak;
    }
}

// ---------------------------------------------------------------------------
// WAV loading
// ---------------------------------------------------------------------------

void WavetableSynth::loadFromWav(const float *data, int totalSamples, int cycleLenIn)
{
    if (!data || totalSamples <= 0 || cycleLenIn <= 0) return;

    cycleLength = cycleLenIn;
    frames.clear();

    int numFrames = totalSamples / cycleLength;
    if (numFrames < 1) numFrames = 1;

    frames.resize(numFrames);
    for (int f = 0; f < numFrames; f++) {
        frames[f].resize(cycleLength, 0.0f);
        int offset = f * cycleLength;
        int count = std::min(cycleLength, totalSamples - offset);
        std::memcpy(frames[f].data(), data + offset, count * sizeof(float));
    }

    // Normalize each frame
    for (auto &frame : frames) {
        float peak = 0.0f;
        for (float s : frame) peak = std::max(peak, std::abs(s));
        if (peak > 0.0f) {
            for (float &s : frame) s /= peak;
        }
    }

    buildMipmaps();
    recomputeSpectrum();
}

bool WavetableSynth::loadWavFile(const QString &path, std::vector<float> &out)
{
    drwav wav;
    if (!drwav_init_file(&wav, path.toUtf8().constData(), nullptr)) {
        return false;
    }

    drwav_uint64 totalFrames = wav.totalPCMFrameCount;
    unsigned int channels = wav.channels;

    // Read all frames as float
    std::vector<float> interleaved(totalFrames * channels);
    drwav_uint64 framesRead = drwav_read_pcm_frames_f32(&wav, totalFrames, interleaved.data());
    drwav_uninit(&wav);

    if (framesRead == 0) return false;

    // Convert to mono if stereo
    out.resize(static_cast<size_t>(framesRead));
    if (channels == 1) {
        std::memcpy(out.data(), interleaved.data(), framesRead * sizeof(float));
    } else {
        for (size_t i = 0; i < static_cast<size_t>(framesRead); i++) {
            float sum = 0.0f;
            for (unsigned int ch = 0; ch < channels; ch++) {
                sum += interleaved[i * channels + ch];
            }
            out[i] = sum / channels;
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// Mipmap anti-aliasing
// ---------------------------------------------------------------------------

void WavetableSynth::buildMipmaps()
{
    mipmaps.clear();
    if (frames.empty()) return;

    int tableSize = static_cast<int>(frames[0].size());
    if (tableSize < 2) return;

    // Level 0 = full bandwidth (copy of original frames)
    mipmaps.push_back(frames);

    // Build successive levels, each with half the harmonics
    int maxHarm = tableSize / 2;
    int level = 1;

    while (maxHarm > 1) {
        maxHarm /= 2;
        std::vector<std::vector<float>> levelFrames;

        for (const auto &frame : frames) {
            // FFT the frame
            std::vector<std::complex<double>> spectrum(tableSize);
            for (int i = 0; i < tableSize; i++) {
                spectrum[i] = std::complex<double>(frame[i], 0.0);
            }
            fft(spectrum);

            // Zero out harmonics above maxHarm
            for (int i = maxHarm + 1; i < tableSize - maxHarm; i++) {
                spectrum[i] = {0.0, 0.0};
            }

            // Inverse FFT (conjugate trick)
            for (auto &c : spectrum) c = std::conj(c);
            fft(spectrum);
            double invN = 1.0 / tableSize;
            for (auto &c : spectrum) c = std::conj(c) * invN;

            std::vector<float> filtered(tableSize);
            for (int i = 0; i < tableSize; i++) {
                filtered[i] = static_cast<float>(spectrum[i].real());
            }
            levelFrames.push_back(std::move(filtered));
        }

        mipmaps.push_back(std::move(levelFrames));
        level++;
    }
}

int WavetableSynth::selectMipmapLevel(double pitch) const
{
    if (mipmaps.empty()) return 0;

    // Nyquist frequency
    double nyquist = sampleRate / 2.0;

    // How many harmonics can fit below Nyquist at this pitch?
    int safeHarmonics = static_cast<int>(nyquist / pitch);
    if (safeHarmonics < 1) safeHarmonics = 1;

    int tableSize = cycleLength;
    int maxHarm = tableSize / 2;

    // Find the mipmap level where maxHarm <= safeHarmonics
    int level = 0;
    while (maxHarm > safeHarmonics && level < static_cast<int>(mipmaps.size()) - 1) {
        maxHarm /= 2;
        level++;
    }

    return level;
}

// ---------------------------------------------------------------------------
// FFT (simple radix-2 Cooley-Tukey)
// ---------------------------------------------------------------------------

void WavetableSynth::fft(std::vector<std::complex<double>> &x)
{
    int N = static_cast<int>(x.size());
    if (N <= 1) return;

    // Bit-reversal permutation
    for (int i = 1, j = 0; i < N; i++) {
        int bit = N >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(x[i], x[j]);
    }

    // Butterfly
    for (int len = 2; len <= N; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        std::complex<double> wlen(std::cos(angle), std::sin(angle));
        for (int i = 0; i < N; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (int j = 0; j < len / 2; j++) {
                auto u = x[i + j];
                auto v = x[i + j + len / 2] * w;
                x[i + j] = u + v;
                x[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Spectrum computation
// ---------------------------------------------------------------------------

void WavetableSynth::recomputeSpectrum()
{
    if (frames.empty()) return;

    // Only recompute if position actually changed
    if (std::abs(position - lastSpectrumPosition) < 1e-6) return;
    lastSpectrumPosition = position;

    int numFrames = static_cast<int>(frames.size());
    int tableSize = static_cast<int>(frames[0].size());

    // Get interpolated frame
    std::vector<float> interpFrame(tableSize);
    if (numFrames == 1) {
        interpFrame = frames[0];
    } else {
        double framePos = position * (numFrames - 1);
        int fA = static_cast<int>(framePos);
        int fB = std::min(fA + 1, numFrames - 1);
        float t = static_cast<float>(framePos - fA);
        interpolateFrame(frames[fA], frames[fB], t, interpFrame);
    }

    // FFT
    std::vector<std::complex<double>> spectrum(tableSize);
    for (int i = 0; i < tableSize; i++) {
        spectrum[i] = std::complex<double>(interpFrame[i], 0.0);
    }
    fft(spectrum);

    // Convert to Spectrum object (magnitudes of first N harmonics)
    int numHarmonics = std::min(64, tableSize / 2);
    cachedSpectrum.resize(numHarmonics);
    double invN = 1.0 / tableSize;

    // Find max magnitude for normalization
    double maxMag = 0.0;
    for (int h = 0; h < numHarmonics; h++) {
        double mag = std::abs(spectrum[h + 1]) * invN * 2.0; // h+1 because bin 0 = DC
        if (mag > maxMag) maxMag = mag;
    }

    if (maxMag > 0.0) {
        for (int h = 0; h < numHarmonics; h++) {
            double mag = std::abs(spectrum[h + 1]) * invN * 2.0;
            cachedSpectrum.setAmplitude(h, mag / maxMag);
        }
    }
}

void WavetableSynth::interpolateFrame(const std::vector<float> &a, const std::vector<float> &b,
                                       float t, std::vector<float> &out) const
{
    int len = static_cast<int>(a.size());
    out.resize(len);
    float oneMinusT = 1.0f - t;
    for (int i = 0; i < len; i++) {
        out[i] = a[i] * oneMinusT + b[i] * t;
    }
}

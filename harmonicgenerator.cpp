#include "harmonicgenerator.h"
#include "containersettings.h"
#include <fftw3.h>
#include <algorithm>
#include <cstring>
#include <QVector>
#include <QDebug>

HarmonicGenerator::HarmonicGenerator(double sampleRate)
    : sampleRate(sampleRate)
    , numHarmonics(32)
    , fundamentalHz(131.0)
    , rolloffPower(1.82)  // Lab default: 91/50.0
    , dnaPreset(-1)  // Default: custom (uses rolloffPower)
    , purity(0.0)  // Default: pure DNA preset, no blending
    , drift(0.0)   // Default: no drift
    , phase(0.0)
    , currentDigitOffset(-1)
    , padEnabled(false)
    , padBandwidth(40.0)
    , padBandwidthScale(1.0)
    , padProfileShape(0)
    , padFftSize(262144)
{
    int maxHarmonics = ContainerSettings::instance().harmonicGenerator.maxHarmonics;
    harmonicPhases.resize(maxHarmonics, 0.0);
    harmonicAmplitudes.resize(maxHarmonics, 0.0);
    driftOffsets.resize(maxHarmonics, 0.0);
    updateHarmonicAmplitudes();
}

void HarmonicGenerator::reset()
{
    phase = 0.0;
    std::fill(harmonicPhases.begin(), harmonicPhases.end(), 0.0);
}

void HarmonicGenerator::setNumHarmonics(int count)
{
    const auto &settings = ContainerSettings::instance().harmonicGenerator;
    numHarmonics = std::clamp(count, 1, settings.maxHarmonics);

    // Resize arrays if needed
    if (harmonicPhases.size() < static_cast<size_t>(numHarmonics)) {
        harmonicPhases.resize(numHarmonics, 0.0);
        harmonicAmplitudes.resize(numHarmonics, 0.0);
        driftOffsets.resize(numHarmonics, 0.0);
    }
    updateHarmonicAmplitudes();
}

void HarmonicGenerator::setFundamentalHz(double hz)
{
    const auto &settings = ContainerSettings::instance().harmonicGenerator;
    fundamentalHz = std::clamp(hz, settings.fundamentalHzMin, settings.fundamentalHzMax);
}

void HarmonicGenerator::setRolloffPower(double power)
{
    const auto &settings = ContainerSettings::instance().harmonicGenerator;
    rolloffPower = std::clamp(power, settings.rolloffPowerMin, settings.rolloffPowerMax);
    updateHarmonicAmplitudes();
}

void HarmonicGenerator::setSampleRate(double rate)
{
    sampleRate = rate;
}

void HarmonicGenerator::setDnaPreset(int preset)
{
    qDebug() << "    [HarmonicGenerator] setDnaPreset called with:" << preset;
    dnaPreset = preset;
    updateHarmonicAmplitudes();

    // Debug: Show first 8 harmonic amplitudes to verify DNA differences
    QString ampStr;
    for (int h = 0; h < std::min(8, numHarmonics); h++) {
        ampStr += QString(" H%1=").arg(h+1) + QString::number(harmonicAmplitudes[h], 'f', 4);
    }
    qDebug().noquote() << "    [HarmonicGenerator] First 8 amplitudes:" << ampStr;
}

void HarmonicGenerator::setPurity(double p)
{
    purity = std::clamp(p, 0.0, 1.0);
    updateHarmonicAmplitudes();  // Recalculate amplitudes with new purity
}

void HarmonicGenerator::setDrift(double d)
{
    const auto &settings = ContainerSettings::instance().harmonicGenerator;
    drift = std::clamp(d, 0.0, settings.driftMax);
    // Note: drift affects phase, not amplitudes, so no need to update amplitudes
}

void HarmonicGenerator::setDigitString(const QString &digits, int initialOffset)
{
    digitString = digits;
    currentDigitOffset = -1;   // force recalc on first setDigitWindowOffset
    setDigitWindowOffset(initialOffset);
}

void HarmonicGenerator::setDigitWindowOffset(int offset)
{
    if (digitString.isEmpty()) return;
    offset = std::max(0, offset);
    if (offset == currentDigitOffset) return;  // nothing changed
    currentDigitOffset = offset;

    int available = digitString.length();
    customDnaPattern.resize(numHarmonics, 0.0);
    std::fill(customDnaPattern.begin(), customDnaPattern.end(), 0.0);

    for (int i = 0; i < numHarmonics; i++) {
        int pos = offset + i;
        if (pos < available)
            customDnaPattern[i] = digitString[pos].digitValue() / 9.0;
    }

    dnaPreset = -1;
    updateHarmonicAmplitudes();
}

void HarmonicGenerator::setCustomAmplitudes(const std::vector<double> &amplitudes)
{
    // Set custom harmonic amplitudes and store the pattern
    int maxHarmonics = ContainerSettings::instance().harmonicGenerator.maxHarmonics;
    int count = std::min(static_cast<int>(amplitudes.size()), maxHarmonics);

    // Store the custom DNA pattern so it can be used as base for purity blending
    customDnaPattern.resize(maxHarmonics, 0.0);
    for (int i = 0; i < count; i++) {
        customDnaPattern[i] = amplitudes[i];
    }

    // Update numHarmonics to match
    numHarmonics = count;

    // Set dnaPreset to -1 to indicate custom mode
    dnaPreset = -1;

    // Regenerate harmonicAmplitudes with purity blend applied
    updateHarmonicAmplitudes();
}

void HarmonicGenerator::updateHarmonicAmplitudes()
{
    // First, generate the base DNA pattern
    QVector<double> basePattern(numHarmonics, 0.0);
    double totalAmp = 0.0;

    switch (dnaPreset) {
    case 0: // Vocal (Bright)
        for (int h = 0; h < numHarmonics; h++) {
            int harmonicNum = h + 1;
            double amp = 1.0 / std::pow(harmonicNum, 1.2);
            // Boost formant regions
            if ((harmonicNum >= 3 && harmonicNum <= 5) || (harmonicNum >= 8 && harmonicNum <= 12)) {
                amp *= 1.4;
            }
            basePattern[h] = amp;
            totalAmp += amp;
        }
        break;

    case 1: // Vocal (Warm)
        for (int h = 0; h < numHarmonics; h++) {
            int harmonicNum = h + 1;
            double amp = 1.0 / std::pow(harmonicNum, 1.6);
            // Boost low formant region
            if (harmonicNum >= 2 && harmonicNum <= 4) {
                amp *= 1.5;
            }
            basePattern[h] = amp;
            totalAmp += amp;
        }
        break;

    case 2: // Brass (Trumpet-like)
        for (int h = 0; h < numHarmonics; h++) {
            int harmonicNum = h + 1;
            double amp = 1.0 / std::pow(harmonicNum, 1.5);
            // Emphasize odd harmonics
            if (harmonicNum % 2 == 1) {
                amp *= 1.3;
            }
            basePattern[h] = amp;
            totalAmp += amp;
        }
        break;

    case 3: // Reed (Clarinet-like)
        for (int h = 0; h < numHarmonics; h++) {
            int harmonicNum = h + 1;
            if (harmonicNum % 2 == 1) {
                // Odd harmonics - strong
                double amp = 1.0 / std::pow(harmonicNum, 1.4);
                basePattern[h] = amp;
                totalAmp += amp;
            } else {
                // Even harmonics - very weak
                double amp = 0.1 / std::pow(harmonicNum, 2.0);
                basePattern[h] = amp;
                totalAmp += amp;
            }
        }
        break;

    case 4: // String (Violin-like)
        for (int h = 0; h < numHarmonics; h++) {
            int harmonicNum = h + 1;
            double amp = 1.0 / std::pow(harmonicNum, 1.3);
            // Emphasis on harmonics 5-10
            if (harmonicNum >= 5 && harmonicNum <= 10) {
                amp *= 1.2;
            }
            basePattern[h] = amp;
            totalAmp += amp;
        }
        break;

    case 5: // Pure Sine (Test) - Only fundamental, no harmonics
        basePattern[0] = 1.0;
        totalAmp = 1.0;
        // All other harmonics remain 0.0
        break;

    default: // Custom (-1) - use stored custom DNA pattern if available
        if (!customDnaPattern.empty() && dnaPreset == -1) {
            // Use the stored custom DNA pattern as the base
            for (int h = 0; h < numHarmonics; h++) {
                basePattern[h] = customDnaPattern[h];
                totalAmp += basePattern[h];
            }
        } else {
            // Fallback to rolloffPower-based generation
            for (int h = 0; h < numHarmonics; h++) {
                int harmonicNum = h + 1;
                double amp = 1.0 / std::pow(harmonicNum, rolloffPower);
                basePattern[h] = amp;
                totalAmp += amp;
            }
        }
        break;
    }

    // Normalize base pattern
    if (totalAmp > 0.0) {
        for (int h = 0; h < numHarmonics; h++) {
            basePattern[h] /= totalAmp;
        }
    }

    // Apply purity blend: mix DNA pattern with flat spectrum
    // purity 0.0 = pure DNA pattern
    // purity 1.0 = all harmonics equal (flat with gentle rolloff)
    totalAmp = 0.0;
    for (int h = 0; h < numHarmonics; h++) {
        int harmonicNum = h + 1;
        double dnaAmp = basePattern[h];
        double flatAmp = 1.0 / harmonicNum;  // Flat spectrum with gentle rolloff
        harmonicAmplitudes[h] = dnaAmp * (1.0 - purity) + flatAmp * purity;
        totalAmp += harmonicAmplitudes[h];
    }

    // Final normalization
    if (totalAmp > 0.0) {
        for (int h = 0; h < numHarmonics; h++) {
            harmonicAmplitudes[h] /= totalAmp;
        }
    }
}

// --- PADsynth implementation ---

void HarmonicGenerator::setPadEnabled(bool enabled)
{
    padEnabled = enabled;
}

void HarmonicGenerator::setPadBandwidth(double cents)
{
    padBandwidth = std::clamp(cents, 1.0, 200.0);
}

void HarmonicGenerator::setPadBandwidthScale(double scale)
{
    padBandwidthScale = std::clamp(scale, 0.0, 2.0);
}

void HarmonicGenerator::setPadProfileShape(int shape)
{
    padProfileShape = std::clamp(shape, 0, 2);
}

void HarmonicGenerator::setPadFftSize(int size)
{
    // Only accept powers of 2 between 2^16 and 2^20
    if (size >= 65536 && size <= 1048576 && (size & (size - 1)) == 0) {
        padFftSize = size;
    }
}

double HarmonicGenerator::getBandwidthProfile(double fi, double bwi, int shape) const
{
    // fi = distance from center in Hz, bwi = bandwidth in Hz
    if (bwi <= 0.0) return 0.0;
    double x = fi / bwi;
    switch (shape) {
    case 0: // Gaussian
        return std::exp(-x * x * 2.772588722); // ln(16) ≈ 2.7726 → -6dB at edges
    case 1: // Raised Cosine
        if (std::abs(x) > 1.0) return 0.0;
        return 0.5 * (1.0 + std::cos(M_PI * x));
    case 2: // Square
        return (std::abs(x) <= 1.0) ? 1.0 : 0.0;
    default:
        return std::exp(-x * x * 2.772588722);
    }
}

void HarmonicGenerator::generatePadWavetable(double fundHz, double sr)
{
    int N = padFftSize;
    double freqPerBin = sr / static_cast<double>(N);

    // Allocate FFTW-aligned frequency-domain buffer (complex interleaved: [0]=real, [1]=imag)
    fftwf_complex* freqBuf = fftwf_alloc_complex(N);
    std::memset(freqBuf, 0, N * sizeof(fftwf_complex));

    // For each harmonic, spread its amplitude across bins using the bandwidth profile
    for (int h = 0; h < numHarmonics; h++) {
        double amp = harmonicAmplitudes[h];
        if (amp <= 0.0) continue;

        int harmonicNum = h + 1;
        double harmonicFreq = fundHz * harmonicNum;

        // Bandwidth in Hz: bw_hz = (2^(bw_cents/1200) - 1) * fund * h^bwscale
        double bwHz = (std::pow(2.0, padBandwidth / 1200.0) - 1.0)
                      * fundHz * std::pow(static_cast<double>(harmonicNum), padBandwidthScale);

        if (bwHz < freqPerBin) bwHz = freqPerBin; // minimum 1 bin width

        // Determine bin range to spread across
        int centerBin = static_cast<int>(std::round(harmonicFreq / freqPerBin));
        int spreadBins = static_cast<int>(std::ceil(bwHz * 3.0 / freqPerBin)); // 3x bandwidth for Gaussian tails
        if (padProfileShape == 2) spreadBins = static_cast<int>(std::ceil(bwHz / freqPerBin));

        int binStart = std::max(1, centerBin - spreadBins);
        int binEnd = std::min(N / 2 - 1, centerBin + spreadBins);

        for (int bin = binStart; bin <= binEnd; bin++) {
            double binFreq = bin * freqPerBin;
            double dist = binFreq - harmonicFreq;
            double profile = getBandwidthProfile(dist, bwHz, padProfileShape);
            freqBuf[bin][0] += static_cast<float>(amp * profile);
        }
    }

    // Assign random phases with fixed seed for reproducibility
    uint32_t seed = 42;
    for (int bin = 1; bin < N / 2; bin++) {
        double magnitude = freqBuf[bin][0];
        if (magnitude <= 0.0f) continue;

        // Simple LCG for deterministic pseudo-random phases
        seed = seed * 1664525u + 1013904223u;
        double phase = (static_cast<double>(seed) / 4294967296.0) * 2.0 * M_PI;

        freqBuf[bin][0] = static_cast<float>(magnitude * std::cos(phase));
        freqBuf[bin][1] = static_cast<float>(magnitude * std::sin(phase));

        // Hermitian symmetry for real output
        freqBuf[N - bin][0] =  freqBuf[bin][0];
        freqBuf[N - bin][1] = -freqBuf[bin][1];
    }
    // DC and Nyquist bins are real (zero imaginary)
    freqBuf[0][0] = 0.0f; freqBuf[0][1] = 0.0f;
    freqBuf[N / 2][1] = 0.0f;

    // Inverse FFT via FFTW (complex → complex BACKWARD, output is real-valued
    // due to Hermitian symmetry above; we extract only the real part)
    fftwf_complex* timeBuf = fftwf_alloc_complex(N);
    fftwf_plan plan = fftwf_plan_dft_1d(N, freqBuf, timeBuf, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftwf_execute(plan);
    fftwf_destroy_plan(plan);
    fftwf_free(freqBuf);

    // Extract real part and normalize
    padWavetable.resize(N);
    float maxVal = 0.0f;
    for (int i = 0; i < N; i++) {
        padWavetable[i] = timeBuf[i][0];
        float absVal = std::abs(padWavetable[i]);
        if (absVal > maxVal) maxVal = absVal;
    }
    fftwf_free(timeBuf);

    // Normalize to peak 1.0
    if (maxVal > 0.0f) {
        float scale = 1.0f / maxVal;
        for (int i = 0; i < N; i++) {
            padWavetable[i] *= scale;
        }
    }

    qDebug() << "[PADsynth] Generated wavetable: N=" << N
             << "fund=" << fundHz << "Hz, bw=" << padBandwidth << "cents";
}

double HarmonicGenerator::generateSample()
{
    // Generate harmonics using pre-calculated amplitudes (includes purity blend)
    double voiceSample = 0.0;

    for (int h = 0; h < numHarmonics; h++)
    {
        int harmonicNum = h + 1;

        // Use pre-calculated amplitude (already normalized and purity-blended)
        double amp = harmonicAmplitudes[h];

        // Calculate phase for this harmonic with drift
        double harmonicPhase = phase * harmonicNum;

        // Apply drift: slowly varying frequency offsets for organic sound
        if (drift > 0.0) {
            // Update drift offset very slowly using low-frequency oscillation
            driftOffsets[h] += drift * 0.0001 * std::sin(phase * 0.023 + h * 1.3);
            // Clamp drift offset
            driftOffsets[h] = std::clamp(driftOffsets[h], -drift, drift);
            // Apply drift to phase
            harmonicPhase += driftOffsets[h] * 2.0 * M_PI;
        }

        // Generate sine wave
        voiceSample += amp * std::sin(harmonicPhase);
    }

    // Advance phase
    phase += 2.0 * M_PI * fundamentalHz / sampleRate;

    // Wrap phase to prevent overflow
    if (phase > 2.0 * M_PI * 1000.0)
        phase -= 2.0 * M_PI * 1000.0;

    return voiceSample;
}

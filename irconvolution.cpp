#include "irconvolution.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <mutex>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// FFTW's planner uses global state and is not thread-safe.
// All plan creation/destruction must be serialized with this mutex.
static std::mutex s_fftwPlanMutex;

IRConvolution::IRConvolution(double sampleRate)
    : m_sampleRate(sampleRate)
    , m_wetDry(0.5)
    , m_predelayMs(0.0)
    , m_predelaySamples(0)
    , m_delayWritePos(0)
    , m_fadeInPct(10.0)
    , m_fadeInSamples(0)
    , m_fadeInCounter(-1)
    , m_decay(1.0)
    , m_highDampFreq(20000.0)
    , m_dampCoeff(0.0)
    , m_dampState(0.0)
    , m_lowCutFreq(20.0)
    , m_lowCutCoeff(0.0)
    , m_lowCutState(0.0)
    , m_blockSize(512)
    , m_fftSize(1024)
    , m_specSize(0)
    , m_numPartitions(0)
    , m_planForward(nullptr)
    , m_planInverse(nullptr)
    , m_realBuf(nullptr)
    , m_complexBuf(nullptr)
    , m_accumBuf(nullptr)
    , m_fdlPos(0)
    , m_inputPos(0)
    , m_peakInputRms(0.0f)
    , m_lastInputRms(0.0f)
    , m_lastSignificantBlockRms(0.0f)
    , m_blocksBelowThreshold(0)
    , m_staticWetDry(0.5)
    , m_tailModeActive(false)
    , m_tailInjectionActive(false)
    , m_tailInjectionFactor(0.0)
    , m_tailInjectionDecayPerBlock(0.0)
    , m_irCapturePos(0)
    , m_irCapturedCount(0)
    , m_irCaptureLengthSamples(0)
    , m_irCaptureCommitted(false)
{
    m_inputBuffer.resize(m_blockSize, 0.0f);
    m_outputBuffer.resize(m_blockSize, 0.0f);
    m_overlapBuffer.resize(m_blockSize, 0.0f);

    // Pre-delay buffer: max 500ms
    int maxDelaySamples = static_cast<int>(0.5 * m_sampleRate) + 1;
    m_delayBuffer.resize(maxDelaySamples, 0.0f);

    // High damp coefficient (20kHz = essentially no damping)
    m_dampCoeff = std::exp(-2.0 * M_PI * m_highDampFreq / m_sampleRate);

    // Low cut coefficient (20Hz = essentially no cut)
    m_lowCutCoeff = std::exp(-2.0 * M_PI * m_lowCutFreq / m_sampleRate);

    // Live capture ring buffer: pre-allocate for max 4 seconds
    int maxCapSamples = static_cast<int>(20.0 * m_sampleRate);
    if (maxCapSamples < 1) maxCapSamples = 882000;
    m_irCaptureBuffer.resize(maxCapSamples, 0.0f);
    m_irCaptureLengthSamples = static_cast<int>(1.0 * m_sampleRate);  // default 1 s
}

IRConvolution::IRConvolution(const IRConvolution& other)
    : m_sampleRate(other.m_sampleRate)
    , m_wetDry(other.m_wetDry)
    , m_ir(other.m_ir)
    , m_predelayMs(other.m_predelayMs)
    , m_predelaySamples(other.m_predelaySamples)
    , m_delayBuffer(other.m_delayBuffer)
    , m_delayWritePos(other.m_delayWritePos)
    , m_fadeInPct(other.m_fadeInPct)
    , m_fadeInSamples(other.m_fadeInSamples)
    , m_fadeInCounter(other.m_fadeInCounter)
    , m_decay(other.m_decay)
    , m_highDampFreq(other.m_highDampFreq)
    , m_dampCoeff(other.m_dampCoeff)
    , m_dampState(other.m_dampState)
    , m_lowCutFreq(other.m_lowCutFreq)
    , m_lowCutCoeff(other.m_lowCutCoeff)
    , m_lowCutState(other.m_lowCutState)
    , m_blockSize(other.m_blockSize)
    , m_fftSize(other.m_fftSize)
    , m_specSize(0)
    , m_numPartitions(0)
    , m_planForward(nullptr)
    , m_planInverse(nullptr)
    , m_realBuf(nullptr)
    , m_complexBuf(nullptr)
    , m_accumBuf(nullptr)
    , m_fdlPos(0)
    , m_inputPos(other.m_inputPos)
    , m_inputBuffer(other.m_inputBuffer)
    , m_outputBuffer(other.m_outputBuffer)
    , m_overlapBuffer(other.m_overlapBuffer)
    , m_peakInputRms(0.0f)
    , m_lastInputRms(0.0f)
    , m_lastSignificantBlockRms(0.0f)
    , m_blocksBelowThreshold(0)
    , m_staticWetDry(other.m_staticWetDry)
    , m_tailModeActive(false)
    , m_tailInjectionActive(false)
    , m_tailInjectionFactor(0.0)
    , m_tailInjectionDecayPerBlock(0.0)
    , m_irCaptureBuffer(other.m_irCaptureBuffer)
    , m_irCapturePos(other.m_irCapturePos)
    , m_irCapturedCount(other.m_irCapturedCount)
    , m_irCaptureLengthSamples(other.m_irCaptureLengthSamples)
    , m_irCaptureCommitted(other.m_irCaptureCommitted)
{
    // Rebuild independent FFTW plans and buffers from the stored IR.
    // FFTW state (FDL) starts zeroed — correct for a freshly cloned note.
    if (!m_ir.empty()) {
        prepareIR();
    }
}

IRConvolution::~IRConvolution()
{
    freeFftwBuffers();
}

// ---------------------------------------------------------------------------
// FFTW buffer management
// ---------------------------------------------------------------------------

void IRConvolution::allocateFftwBuffers()
{
    m_specSize   = m_fftSize / 2 + 1;
    m_realBuf    = fftwf_alloc_real(m_fftSize);
    m_complexBuf = fftwf_alloc_complex(m_specSize);
    m_accumBuf   = fftwf_alloc_complex(m_specSize);

    // Plan creation touches FFTW global state — must be serialized across threads.
    // FFTW_ESTIMATE skips benchmarking so the lock is held for only microseconds.
    {
        std::lock_guard<std::mutex> lock(s_fftwPlanMutex);
        m_planForward = fftwf_plan_dft_r2c_1d(m_fftSize, m_realBuf, m_complexBuf, FFTW_ESTIMATE);
        m_planInverse = fftwf_plan_dft_c2r_1d(m_fftSize, m_accumBuf, m_realBuf,   FFTW_ESTIMATE);
    }
}

void IRConvolution::freeFftwBuffers()
{
    {
        std::lock_guard<std::mutex> lock(s_fftwPlanMutex);
        if (m_planForward) { fftwf_destroy_plan(m_planForward); m_planForward = nullptr; }
        if (m_planInverse) { fftwf_destroy_plan(m_planInverse); m_planInverse = nullptr; }
    }
    if (m_realBuf)    { fftwf_free(m_realBuf);    m_realBuf    = nullptr; }
    if (m_complexBuf) { fftwf_free(m_complexBuf); m_complexBuf = nullptr; }
    if (m_accumBuf)   { fftwf_free(m_accumBuf);   m_accumBuf   = nullptr; }

    for (auto* p : m_irPartitions) if (p) fftwf_free(p);
    m_irPartitions.clear();

    for (auto* p : m_fdl) if (p) fftwf_free(p);
    m_fdl.clear();
}

// ---------------------------------------------------------------------------
// Parameter setters
// ---------------------------------------------------------------------------

void IRConvolution::setSampleRate(double rate)
{
    m_sampleRate = rate;

    int maxDelaySamples = static_cast<int>(0.5 * m_sampleRate) + 1;
    m_delayBuffer.assign(maxDelaySamples, 0.0f);
    m_delayWritePos = 0;
    m_predelaySamples = static_cast<int>(m_predelayMs * m_sampleRate / 1000.0);
    if (m_predelaySamples > 0 && m_fadeInPct > 0.0) {
        double fadeMs = m_predelayMs * m_fadeInPct / 100.0;
        m_fadeInSamples = std::max(1, static_cast<int>(fadeMs * m_sampleRate / 1000.0));
    } else {
        m_fadeInSamples = 0;
    }
    m_fadeInCounter = -1;

    m_dampCoeff   = std::exp(-2.0 * M_PI * m_highDampFreq / m_sampleRate);
    m_lowCutCoeff = std::exp(-2.0 * M_PI * m_lowCutFreq   / m_sampleRate);

    if (!m_ir.empty()) {
        prepareIR();
    }
}

void IRConvolution::setWetDry(double wetDry)
{
    m_wetDry = std::max(0.0, std::min(1.0, wetDry));
}

void IRConvolution::setPredelay(double ms)
{
    m_predelayMs = std::max(0.0, std::min(500.0, ms));
    m_predelaySamples = static_cast<int>(m_predelayMs * m_sampleRate / 1000.0);
    if (m_predelaySamples > 0 && m_fadeInPct > 0.0) {
        double fadeMs = m_predelayMs * m_fadeInPct / 100.0;
        m_fadeInSamples = std::max(1, static_cast<int>(fadeMs * m_sampleRate / 1000.0));
    } else {
        m_fadeInSamples = 0;
    }
}

void IRConvolution::setFadeInPct(double pct)
{
    m_fadeInPct = std::max(0.0, std::min(100.0, pct));
    if (m_predelaySamples > 0 && m_fadeInPct > 0.0) {
        double fadeMs = m_predelayMs * m_fadeInPct / 100.0;
        m_fadeInSamples = std::max(1, static_cast<int>(fadeMs * m_sampleRate / 1000.0));
    } else {
        m_fadeInSamples = 0;
    }
}

void IRConvolution::setDecay(double decay)
{
    double newDecay = std::max(0.0, std::min(1.0, decay));
    if (newDecay != m_decay) {
        m_decay = newDecay;
        if (!m_ir.empty()) {
            prepareIR();
        }
    }
}

void IRConvolution::setHighDamp(double freqHz)
{
    m_highDampFreq = std::max(200.0, std::min(20000.0, freqHz));
    m_dampCoeff = std::exp(-2.0 * M_PI * m_highDampFreq / m_sampleRate);
}

void IRConvolution::setLowCut(double freqHz)
{
    m_lowCutFreq = std::max(20.0, std::min(2000.0, freqHz));
    m_lowCutCoeff = std::exp(-2.0 * M_PI * m_lowCutFreq / m_sampleRate);
}

void IRConvolution::setCaptureLengthSeconds(double secs)
{
    int maxCap = static_cast<int>(m_irCaptureBuffer.size());
    m_irCaptureLengthSamples = std::max(1, std::min(
        static_cast<int>(secs * m_sampleRate), maxCap));
}

void IRConvolution::pushIRSample(double sample)
{
    if (m_irCaptureCommitted) return;
    int maxCap = static_cast<int>(m_irCaptureBuffer.size());
    if (maxCap == 0) return;
    m_irCaptureBuffer[m_irCapturePos] = static_cast<float>(sample);
    m_irCapturePos = (m_irCapturePos + 1) % maxCap;
    if (m_irCapturedCount < maxCap) m_irCapturedCount++;

    // Auto-commit once we have captured enough samples
    if (m_irCaptureLengthSamples > 0 && m_irCapturedCount >= m_irCaptureLengthSamples) {
        commitCapturedIR();  // sets m_irCaptureCommitted = true
    }
}

void IRConvolution::commitCapturedIR()
{
    int maxCap = static_cast<int>(m_irCaptureBuffer.size());
    if (maxCap == 0 || m_irCapturedCount == 0) return;

    int available = std::min(m_irCapturedCount, maxCap);
    int useLen = std::min(m_irCaptureLengthSamples, available);
    if (useLen <= 0) return;

    // Extract the captured samples in chronological order from the ring buffer
    std::vector<float> newIR(useLen);
    int startPos = (m_irCapturePos - useLen + maxCap) % maxCap;
    for (int i = 0; i < useLen; i++) {
        newIR[i] = m_irCaptureBuffer[(startPos + i) % maxCap];
    }

    setImpulseResponse(newIR);
    m_irCaptureCommitted = true;
}

void IRConvolution::setImpulseResponse(const std::vector<float>& ir)
{
    m_ir = ir;
    if (!m_ir.empty()) {
        prepareIR();
    } else {
        m_numPartitions = 0;
        freeFftwBuffers();
    }
    resetConvolutionState();
}

// ---------------------------------------------------------------------------
// IR preparation
// ---------------------------------------------------------------------------

void IRConvolution::prepareIR()
{
    m_fftSize       = m_blockSize * 2;
    m_numPartitions = (static_cast<int>(m_ir.size()) + m_blockSize - 1) / m_blockSize;

    // Normalize IR by its energy so output stays at input signal level
    double sumSq = 0.0;
    for (float s : m_ir) sumSq += static_cast<double>(s) * s;
    double rms = std::sqrt(sumSq);
    float normScale = (rms > 1e-8) ? static_cast<float>(1.0 / rms) : 1.0f;

    // Exponential decay envelope
    double decayRate = 0.0;
    if (m_decay < 1.0) {
        double irLen     = static_cast<double>(m_ir.size());
        double fraction  = 0.1 + 0.9 * m_decay;
        double targetLen = irLen * fraction;
        if (targetLen > 1.0) {
            decayRate = 4.6 / targetLen; // -40 dB
        }
    }

    // (Re)allocate FFTW plans and work buffers
    freeFftwBuffers();
    allocateFftwBuffers();

    // Allocate and populate IR partitions
    m_irPartitions.resize(m_numPartitions, nullptr);
    for (int p = 0; p < m_numPartitions; p++) {
        m_irPartitions[p] = fftwf_alloc_complex(m_specSize);

        // Fill real buffer: IR segment with decay envelope, zero-padded
        std::fill(m_realBuf, m_realBuf + m_fftSize, 0.0f);
        int offset = p * m_blockSize;
        int count  = std::min(m_blockSize, static_cast<int>(m_ir.size()) - offset);
        for (int i = 0; i < count; i++) {
            float envelope = (decayRate > 0.0)
                ? static_cast<float>(std::exp(-decayRate * (offset + i)))
                : 1.0f;
            m_realBuf[i] = m_ir[offset + i] * normScale * envelope;
        }

        // Forward FFT → store directly into IR partition slot
        fftwf_execute_dft_r2c(m_planForward, m_realBuf, m_irPartitions[p]);
    }

    // Allocate and zero frequency-domain delay line
    m_fdl.resize(m_numPartitions, nullptr);
    for (int p = 0; p < m_numPartitions; p++) {
        m_fdl[p] = fftwf_alloc_complex(m_specSize);
        std::memset(m_fdl[p], 0, m_specSize * sizeof(fftwf_complex));
    }
    m_fdlPos = 0;
}

// ---------------------------------------------------------------------------
// reset
// ---------------------------------------------------------------------------

void IRConvolution::resetConvolutionState()
{
    m_inputPos = 0;
    std::fill(m_inputBuffer.begin(),  m_inputBuffer.end(),  0.0f);
    std::fill(m_outputBuffer.begin(), m_outputBuffer.end(), 0.0f);
    std::fill(m_overlapBuffer.begin(),m_overlapBuffer.end(),0.0f);

    for (auto* p : m_fdl) {
        if (p) std::memset(p, 0, m_specSize * sizeof(fftwf_complex));
    }
    m_fdlPos = 0;

    std::fill(m_delayBuffer.begin(), m_delayBuffer.end(), 0.0f);
    m_delayWritePos = 0;
    m_fadeInCounter = -1;

    m_dampState   = 0.0;
    m_lowCutState = 0.0;

    m_peakInputRms             = 0.0f;
    m_lastInputRms             = 0.0f;
    m_lastSignificantBlockRms  = 0.0f;
    m_lastSignificantBlock.clear();
    m_blocksBelowThreshold     = 0;
    m_tailModeActive           = false;
    m_tailInjectionActive      = false;
    m_tailInjectionFactor      = 0.0;
    m_tailInjectionDecayPerBlock = 0.0;
}

void IRConvolution::reset()
{
    resetConvolutionState();

    // Also reset capture state so the next note captures fresh
    m_irCapturedCount    = 0;
    m_irCapturePos       = 0;
    m_irCaptureCommitted = false;
}

// ---------------------------------------------------------------------------
// processSample
// ---------------------------------------------------------------------------

double IRConvolution::processSample(double input)
{
    if (m_numPartitions == 0) {
        return input;
    }

    // Tail injection: replace zero input with a decaying replay of the last
    // peak-amplitude block so the FDL has energy to produce a natural ring-out
    // when the note ended with a soft (near-zero) dynamics fade.
    if (m_tailInjectionActive && !m_lastSignificantBlock.empty()) {
        input = static_cast<double>(m_lastSignificantBlock[m_inputPos]) * m_tailInjectionFactor;
    }

    m_inputBuffer[m_inputPos] = static_cast<float>(input);
    double convOut = static_cast<double>(m_outputBuffer[m_inputPos]);
    m_inputPos++;

    if (m_inputPos >= m_blockSize) {
        processBlock();
        m_inputPos = 0;
    }

    // Pre-delay
    double wet;
    if (m_predelaySamples > 0 && !m_delayBuffer.empty()) {
        int bufSize = static_cast<int>(m_delayBuffer.size());
        m_delayBuffer[m_delayWritePos] = static_cast<float>(convOut);
        int readPos = (m_delayWritePos - m_predelaySamples + bufSize) % bufSize;
        wet = static_cast<double>(m_delayBuffer[readPos]);
        m_delayWritePos = (m_delayWritePos + 1) % bufSize;

        if (m_fadeInSamples > 0) {
            if (m_fadeInCounter < 0 && wet != 0.0) {
                m_fadeInCounter = 0;
            }
            if (m_fadeInCounter >= 0 && m_fadeInCounter < m_fadeInSamples) {
                double t = static_cast<double>(m_fadeInCounter) / m_fadeInSamples;
                wet *= 0.5 * (1.0 - std::cos(M_PI * t));
                m_fadeInCounter++;
            }
        }
    } else {
        wet = convOut;
    }

    // Low-cut
    if (m_lowCutFreq > 21.0) {
        m_lowCutState = m_lowCutState * m_lowCutCoeff + wet * (1.0 - m_lowCutCoeff);
        wet = wet - m_lowCutState;
    }

    // High-frequency damping
    if (m_highDampFreq < 19999.0) {
        m_dampState = m_dampState * m_dampCoeff + wet * (1.0 - m_dampCoeff);
        wet = m_dampState;
    }

    return input * (1.0 - m_wetDry) + wet * m_wetDry;
}

// ---------------------------------------------------------------------------
// processSampleWetOnly — same signal path as processSample but returns only
// the convolved+filtered wet signal (no dry mix, no m_wetDry scaling).
// The caller (SounitGraph::processPostIR) handles the wet/dry blend using a
// separate dry source and the current getWetDry() value.
// ---------------------------------------------------------------------------

double IRConvolution::processSampleWetOnly(double input)
{
    if (m_numPartitions == 0) {
        return 0.0;  // No IR loaded — no wet signal
    }

    // Tail injection: replace zero input with a decaying replay of the peak block
    if (m_tailInjectionActive && !m_lastSignificantBlock.empty()) {
        input = static_cast<double>(m_lastSignificantBlock[m_inputPos]) * m_tailInjectionFactor;
    }

    m_inputBuffer[m_inputPos] = static_cast<float>(input);
    double convOut = static_cast<double>(m_outputBuffer[m_inputPos]);
    m_inputPos++;

    if (m_inputPos >= m_blockSize) {
        processBlock();
        m_inputPos = 0;
    }

    // Pre-delay
    double wet;
    if (m_predelaySamples > 0 && !m_delayBuffer.empty()) {
        int bufSize = static_cast<int>(m_delayBuffer.size());
        m_delayBuffer[m_delayWritePos] = static_cast<float>(convOut);
        int readPos = (m_delayWritePos - m_predelaySamples + bufSize) % bufSize;
        wet = static_cast<double>(m_delayBuffer[readPos]);
        m_delayWritePos = (m_delayWritePos + 1) % bufSize;

        if (m_fadeInSamples > 0) {
            if (m_fadeInCounter < 0 && wet != 0.0) {
                m_fadeInCounter = 0;
            }
            if (m_fadeInCounter >= 0 && m_fadeInCounter < m_fadeInSamples) {
                double t = static_cast<double>(m_fadeInCounter) / m_fadeInSamples;
                wet *= 0.5 * (1.0 - std::cos(M_PI * t));
                m_fadeInCounter++;
            }
        }
    } else {
        wet = convOut;
    }

    // Low-cut
    if (m_lowCutFreq > 21.0) {
        m_lowCutState = m_lowCutState * m_lowCutCoeff + wet * (1.0 - m_lowCutCoeff);
        wet = wet - m_lowCutState;
    }

    // High-frequency damping
    if (m_highDampFreq < 19999.0) {
        m_dampState = m_dampState * m_dampCoeff + wet * (1.0 - m_dampCoeff);
        wet = m_dampState;
    }

    return wet;  // Caller mixes with dry using getWetDry()
}

// ---------------------------------------------------------------------------
// processBlock — FFTW r2c / c2r partitioned overlap-add
// ---------------------------------------------------------------------------

void IRConvolution::processBlock()
{
    // Compute per-block RMS for peak tracking and soft-ending detection.
    {
        float sumSq = 0.0f;
        for (int i = 0; i < m_blockSize; i++) {
            sumSq += m_inputBuffer[i] * m_inputBuffer[i];
        }
        m_lastInputRms = std::sqrt(sumSq / static_cast<float>(m_blockSize));
    }

    if (m_lastInputRms > m_peakInputRms) {
        m_peakInputRms = m_lastInputRms;
    }

    // Save this block when it carries meaningful energy (> 1 % of peak).
    // This gives prepareForTailMode() a reference block with the note's spectral content.
    // Also track how many consecutive sub-threshold blocks have elapsed (taper depth):
    // prepareForTailMode() uses this to scale injection only for the portion of the FDL
    // that has actually been replaced by near-zero data.
    if (m_peakInputRms > 0.0f && m_lastInputRms > 0.01f * m_peakInputRms) {
        if (static_cast<int>(m_lastSignificantBlock.size()) != m_blockSize) {
            m_lastSignificantBlock.resize(m_blockSize);
        }
        std::copy(m_inputBuffer.begin(), m_inputBuffer.end(), m_lastSignificantBlock.begin());
        m_lastSignificantBlockRms = m_lastInputRms;
        m_blocksBelowThreshold = 0;
    } else if (!m_tailModeActive) {
        m_blocksBelowThreshold++;
    }

    // Copy input block into real buffer, zero-pad upper half
    std::memcpy(m_realBuf, m_inputBuffer.data(), m_blockSize * sizeof(float));
    std::fill(m_realBuf + m_blockSize, m_realBuf + m_fftSize, 0.0f);

    // Forward FFT: write result directly into current FDL slot
    fftwf_execute_dft_r2c(m_planForward, m_realBuf, m_fdl[m_fdlPos]);

    // Zero accumulation buffer
    std::memset(m_accumBuf, 0, m_specSize * sizeof(fftwf_complex));

    // Accumulate: complex multiply each FDL entry with its IR partition
    for (int p = 0; p < m_numPartitions; p++) {
        int fdlIdx = (m_fdlPos - p + m_numPartitions) % m_numPartitions;
        const fftwf_complex* x = m_fdl[fdlIdx];
        const fftwf_complex* h = m_irPartitions[p];

        for (int k = 0; k < m_specSize; k++) {
            m_accumBuf[k][0] += x[k][0] * h[k][0] - x[k][1] * h[k][1];
            m_accumBuf[k][1] += x[k][1] * h[k][0] + x[k][0] * h[k][1];
        }
    }

    // Inverse FFT: c2r destroys accumBuf (fine — it's zeroed next block)
    fftwf_execute_dft_c2r(m_planInverse, m_accumBuf, m_realBuf);

    // FFTW c2r does not normalize — scale by 1/fftSize
    const float scale = 1.0f / static_cast<float>(m_fftSize);
    for (int i = 0; i < m_fftSize; i++) {
        m_realBuf[i] *= scale;
    }

    // Overlap-add output
    for (int i = 0; i < m_blockSize; i++) {
        m_outputBuffer[i] = m_realBuf[i] + m_overlapBuffer[i];
    }
    for (int i = 0; i < m_blockSize; i++) {
        m_overlapBuffer[i] = m_realBuf[m_blockSize + i];
    }

    m_fdlPos = (m_fdlPos + 1) % m_numPartitions;

    // Decay the tail injection factor each block until it reaches silence.
    if (m_tailInjectionActive) {
        m_tailInjectionFactor *= m_tailInjectionDecayPerBlock;
        if (m_tailInjectionFactor < 1e-5) {
            m_tailInjectionActive = false;
            m_tailInjectionFactor = 0.0;
        }
    }
}

// ---------------------------------------------------------------------------
// setStaticWetDry / prepareForTailMode
// ---------------------------------------------------------------------------

void IRConvolution::setStaticWetDry(double wetDry)
{
    m_staticWetDry = wetDry;
    m_wetDry       = wetDry;
}

void IRConvolution::prepareForTailMode(double endDynamics)
{
    if (m_tailModeActive) return;   // Idempotent — run only once per note
    m_tailModeActive = true;

    // Restore wetDry to the static (graph-build) value.  If an Envelope Engine
    // with followDynamics was connected to the wetDry port it may have driven
    // m_wetDry to near-zero by the note's last sample, which would silence the
    // convolution tail entirely.
    m_wetDry = m_staticWetDry;

    // Soft-ending injection: only when the NOTE'S DYNAMICS (not the signal's own
    // natural decay) caused the silence.  endDynamics is the dynamics curve value
    // at the last non-tail sample.  For pen notes the curve returns to ~0; for
    // flat-dynamics notes (mouse input, Karplus Strong "let ring", physical models
    // whose signal decays on its own) it stays near 1.0.
    // This prevents injection from wrongly shortening the natural ring-out of
    // decaying instruments while still fixing pen-note reverb tails.
    if (endDynamics < 0.05
        && m_numPartitions > 0
        && m_peakInputRms > 1e-6f
        && m_lastSignificantBlockRms > 0.0f
        && !m_lastSignificantBlock.empty())
    {
        // Scale the replayed block so its effective RMS equals peakInputRms —
        // then modulate by how much of the FDL has actually been replaced by the taper.
        //
        // When the taper is SHORT relative to the IR length, the older FDL slots
        // still hold peak-level data and the convolution will ring out naturally at
        // the correct level — no injection needed (fdlFillFraction ≈ 0).
        // When the taper is LONG (> IR length), the FDL is fully overwritten with
        // near-zero data, so the convolution would ring out too quietly — full
        // injection needed (fdlFillFraction ≈ 1).
        // Scaling by this fraction prevents the double-energy overshoot that caused
        // distortion on notes with a brief dynamic fade at the end.
        double fdlFillFraction = (m_numPartitions > 0)
            ? std::min(static_cast<double>(m_blocksBelowThreshold) / m_numPartitions, 1.0)
            : 1.0;

        double factor = static_cast<double>(m_peakInputRms)
                      / static_cast<double>(m_lastSignificantBlockRms);
        factor = std::min(factor, 100.0) * fdlFillFraction;  // Scale by taper depth in FDL

        m_tailInjectionFactor = factor;

        // Decay the injection over one IR-length (numPartitions blocks) so it
        // reaches -60 dB and the tail sounds natural, not like a second note.
        int decayBlocks = std::max(m_numPartitions, 4);
        m_tailInjectionDecayPerBlock = std::exp(-std::log(1000.0) / decayBlocks);

        m_tailInjectionActive = true;
    }
}

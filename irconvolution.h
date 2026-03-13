#ifndef IRCONVOLUTION_H
#define IRCONVOLUTION_H

#include <fftw3.h>
#include <vector>

/**
 * IRConvolution - Impulse Response convolution processor
 *
 * Convolves an input signal with a loaded impulse response (IR) for
 * realistic reverb, cabinet simulation, and spatial effects.
 *
 * Uses partitioned overlap-add FFT convolution for efficient processing
 * of both short and long IRs.
 *
 * FFT backend: FFTW3 single-precision (fftwf), r2c/c2r plans.
 */
class IRConvolution
{
public:
    IRConvolution(double sampleRate = 44100.0);
    IRConvolution(const IRConvolution& other);   // deep copy — owns independent FFTW state
    IRConvolution& operator=(const IRConvolution&) = delete;
    ~IRConvolution();

    double processSample(double input);
    // Like processSample but returns only the wet (convolved+filtered) signal — no dry mix.
    // Used by SounitGraph::processPostIR() so the caller controls the wet/dry blend.
    double processSampleWetOnly(double input);
    void reset();
    void setSampleRate(double rate);

    void setImpulseResponse(const std::vector<float>& ir);
    void pushIRSample(double sample);     // Feed live irIn signal into capture ring buffer
    void commitCapturedIR();             // At note onset: freeze ring buffer → setImpulseResponse()
    void setCaptureLengthSeconds(double secs);  // How much of ring buffer to use as IR
    bool hasCaptureData() const { return m_irCapturedCount > 0; }
    const std::vector<float>& getIR() const { return m_ir; }
    void freezeCapture() { m_irCaptureCommitted = true; }  // Prevent any further capture/commit
    void setWetDry(double wetDry);        // 0.0 = fully dry, 1.0 = fully wet
    void setStaticWetDry(double wetDry);  // Set once at graph-build time; restored in tail mode
    double getWetDry() const { return m_wetDry; }

    // Call once when the note transitions into tail mode.
    // endDynamics: the note's dynamics value at the last non-tail sample
    //   (pass m_lastNormalDynamics from SounitGraph).
    // Resets wetDry to the static (non-modulated) value and, when endDynamics
    // is near-zero (pen note whose dynamics faded to silence), injects a decaying
    // replay of the last peak-amplitude block so the FDL has energy to ring out
    // naturally — without triggering on instruments that simply decay on their own
    // (Karplus Strong "let ring", physical models, etc.).
    void prepareForTailMode(double endDynamics = 1.0);
    bool hasIR() const { return !m_ir.empty(); }

    void setPredelay(double ms);        // 0–500 ms
    double getPredelay() const { return m_predelayMs; }

    void setFadeInPct(double pct);      // 0–100 % of pre-delay used as fade-in ramp
    double getFadeInPct() const { return m_fadeInPct; }

    void setDecay(double decay);        // 0.0–1.0 (triggers IR re-preparation)
    double getDecay() const { return m_decay; }

    void setHighDamp(double freqHz);    // 200–20000 Hz
    double getHighDamp() const { return m_highDampFreq; }

    void setLowCut(double freqHz);     // 20–2000 Hz
    double getLowCut() const { return m_lowCutFreq; }

private:
    double m_sampleRate;
    double m_wetDry;
    std::vector<float> m_ir;

    // Pre-delay: circular buffer to delay wet signal
    double m_predelayMs;
    int m_predelaySamples;
    std::vector<float> m_delayBuffer;
    int m_delayWritePos;

    // Fade-in ramp after pre-delay to avoid onset artifact
    double m_fadeInPct;        // user-controlled: % of pre-delay used as ramp
    int m_fadeInSamples;       // ramp length in samples
    int m_fadeInCounter;       // counts up from 0 during ramp

    // Decay trim: exponential fade applied to IR in prepareIR()
    double m_decay;

    // High-frequency damping: one-pole lowpass on wet signal
    double m_highDampFreq;
    double m_dampCoeff;
    double m_dampState;

    // Low-cut: one-pole highpass on wet signal (subtract lowpass)
    double m_lowCutFreq;
    double m_lowCutCoeff;
    double m_lowCutState;

    // Partitioned overlap-add FFT convolution
    int m_blockSize;         // processing block size (e.g. 512)
    int m_fftSize;           // blockSize * 2
    int m_specSize;          // fftSize/2 + 1  (r2c output size)
    int m_numPartitions;

    // FFTW plans (r2c / c2r)
    fftwf_plan m_planForward;   // float[fftSize] → fftwf_complex[specSize]
    fftwf_plan m_planInverse;   // fftwf_complex[specSize] → float[fftSize]

    // FFTW-aligned work buffers (reused every block)
    float*         m_realBuf;    // size: m_fftSize
    fftwf_complex* m_complexBuf; // size: m_specSize  (scratch for plan creation)
    fftwf_complex* m_accumBuf;   // size: m_specSize  (convolution accumulator)

    // IR partitions in frequency domain [numPartitions][specSize]
    std::vector<fftwf_complex*> m_irPartitions;

    // Frequency-domain delay line [numPartitions][specSize]
    std::vector<fftwf_complex*> m_fdl;
    int m_fdlPos;

    // Input accumulation buffer
    std::vector<float> m_inputBuffer;
    int m_inputPos;

    // Output buffer (valid output for current block)
    std::vector<float> m_outputBuffer;
    // Overlap-add tail from previous block
    std::vector<float> m_overlapBuffer;

    // Soft-ending tail support:
    // Track peak input energy during the note so tail mode can inject enough
    // energy for a natural reverb ring-out when dynamics faded the note to silence.
    float  m_peakInputRms;              // Highest per-block RMS seen this note
    float  m_lastInputRms;              // RMS of the most recently processed block
    std::vector<float> m_lastSignificantBlock;  // Copy of last block with RMS > 1% of peak
    float  m_lastSignificantBlockRms;   // RMS of that saved block
    int    m_blocksBelowThreshold;      // Consecutive blocks with RMS < 1% of peak (taper depth)
    double m_staticWetDry;              // wetDry set at graph-build time; restored in tail mode
    bool   m_tailModeActive;            // True once prepareForTailMode() has run
    bool   m_tailInjectionActive;       // True while replaying the peak block
    double m_tailInjectionFactor;       // Current amplitude scale for injected signal
    double m_tailInjectionDecayPerBlock;// Multiply factor applied each processBlock() in tail

    // Live signal capture (ring buffer, max 4 s at current sample rate)
    std::vector<float> m_irCaptureBuffer;
    int                m_irCapturePos;
    int                m_irCapturedCount;
    int                m_irCaptureLengthSamples;
    bool               m_irCaptureCommitted;  // true once auto-commit has fired this note

    void prepareIR();
    void processBlock();
    void resetConvolutionState();  // resets convolution buffers only (not capture state)

    void allocateFftwBuffers();
    void freeFftwBuffers();
};

#endif // IRCONVOLUTION_H

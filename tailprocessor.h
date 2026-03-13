#ifndef TAILPROCESSOR_H
#define TAILPROCESSOR_H

#include <cstddef>

/**
 * TailProcessor - Smooth fade-out tail for note endings
 *
 * Prevents click artifacts by extending the rendered note with a cosine
 * fade-out over a configurable duration.  During the note the signal
 * passes through unchanged.  When tail mode begins the processor applies
 * a cosine fade to the live incoming signal over tailLength milliseconds,
 * allowing the upstream source to continue generating (let-ring behaviour).
 *
 * Parameters:
 *   tailLength : Fade duration in milliseconds (1–9999)
 */
class TailProcessor
{
public:
    explicit TailProcessor(double sampleRate = 44100.0);

    void setTailLength(double ms);
    double getTailLength() const { return tailLengthMs; }

    void setSampleRate(double rate);

    /**
     * Process one sample.
     * @param inputSignal  Upstream signal value
     * @param tailMode     True when the note has ended (tail rendering phase)
     * @return             Processed sample
     */
    double processSample(double inputSignal, bool tailMode);

    /** Reset all state (call at note start) */
    void reset();

private:
    double sampleRate;
    double tailLengthMs;

    // Internal state
    bool   inTail;                // True once tail mode has started
    size_t tailTotalSamples;      // Total fade duration in samples
    size_t tailSamplesRemaining;  // Samples left in the fade
};

#endif // TAILPROCESSOR_H

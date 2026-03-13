#include "tailprocessor.h"
#include <cmath>
#include <algorithm>

static constexpr double PI = 3.14159265358979323846;

TailProcessor::TailProcessor(double sampleRate)
    : sampleRate(sampleRate)
    , tailLengthMs(200.0)
    , inTail(false)
    , tailTotalSamples(0)
    , tailSamplesRemaining(0)
{
}

void TailProcessor::setSampleRate(double rate)
{
    if (rate > 0.0)
        sampleRate = rate;
}

void TailProcessor::setTailLength(double ms)
{
    tailLengthMs = std::clamp(ms, 1.0, 9999.0);
}

double TailProcessor::processSample(double inputSignal, bool tailMode)
{
    if (!tailMode) {
        // Normal note: pass through unchanged
        return inputSignal;
    }

    // ── Tail mode ──────────────────────────────────────────────────────────
    // Signal sources keep running when a TailProcessor is present, so
    // inputSignal is the live upstream output.  We apply a cosine fade to it.

    // First sample of tail: initialise the cosine fade
    if (!inTail) {
        inTail = true;
        tailTotalSamples = static_cast<size_t>(tailLengthMs / 1000.0 * sampleRate);
        if (tailTotalSamples < 1) tailTotalSamples = 1;
        tailSamplesRemaining = tailTotalSamples;
    }

    if (tailSamplesRemaining == 0) {
        return 0.0;
    }

    // Cosine fade: 1.0 at the start of the tail, 0.0 at the end
    // progress goes 0 → 1 as tailSamplesRemaining counts down
    double progress = 1.0 - static_cast<double>(tailSamplesRemaining)
                              / static_cast<double>(tailTotalSamples);
    double fadeFactor = 0.5 * (1.0 + std::cos(PI * progress));

    tailSamplesRemaining--;

    return inputSignal * fadeFactor;
}

void TailProcessor::reset()
{
    inTail = false;
    tailTotalSamples = 0;
    tailSamplesRemaining = 0;
}

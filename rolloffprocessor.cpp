#include "rolloffprocessor.h"
#include <cmath>
#include <algorithm>

RolloffProcessor::RolloffProcessor()
    : rolloffPower(0.6)
    , lowRolloff(0.3)
    , highRolloff(1.0)
    , crossoverHarmonic(8)
    , transitionWidth(4)
{
}

void RolloffProcessor::processSpectrum(const Spectrum &input, Spectrum &output, double rolloffPower)
{
    int numHarmonics = input.getNumHarmonics();
    output.resize(numHarmonics);

    // Apply rolloff curve: amplitudeOut[h] = amplitudeIn[h] / pow(h, rolloff)
    for (int h = 0; h < numHarmonics; h++) {
        int harmonicNum = h + 1;  // h=0 is fundamental (harmonic #1)
        double inputAmp = input.getAmplitude(h);
        double outputAmp = inputAmp / std::pow(harmonicNum, rolloffPower);
        output.setAmplitude(h, outputAmp);
    }
}

void RolloffProcessor::processSpectrumDualBand(const Spectrum &input, Spectrum &output,
                                                double lowRolloff, double highRolloff,
                                                int crossoverHarmonic, int transitionWidth)
{
    int numHarmonics = input.getNumHarmonics();
    output.resize(numHarmonics);

    // Ensure valid parameters
    crossoverHarmonic = std::max(1, crossoverHarmonic);
    transitionWidth = std::max(0, transitionWidth);

    // Calculate transition boundaries
    double transitionStart = crossoverHarmonic - transitionWidth / 2.0;
    double transitionEnd = crossoverHarmonic + transitionWidth / 2.0;

    for (int h = 0; h < numHarmonics; h++) {
        int harmonicNum = h + 1;  // h=0 is fundamental (harmonic #1)
        double inputAmp = input.getAmplitude(h);

        // Determine effective rolloff power based on harmonic position
        double effectiveRolloff;

        if (transitionWidth == 0 || harmonicNum <= transitionStart) {
            // Below crossover: use low rolloff
            effectiveRolloff = lowRolloff;
        } else if (harmonicNum >= transitionEnd) {
            // Above crossover: use high rolloff
            effectiveRolloff = highRolloff;
        } else {
            // In transition zone: smooth blend using cosine interpolation
            double t = (harmonicNum - transitionStart) / (transitionEnd - transitionStart);
            // Cosine interpolation for smooth transition (no discontinuity in derivative)
            double blend = 0.5 * (1.0 - std::cos(t * M_PI));
            effectiveRolloff = lowRolloff + (highRolloff - lowRolloff) * blend;
        }

        // Apply rolloff: amplitudeOut[h] = amplitudeIn[h] / pow(h, effectiveRolloff)
        double outputAmp = inputAmp / std::pow(harmonicNum, effectiveRolloff);
        output.setAmplitude(h, outputAmp);
    }
}

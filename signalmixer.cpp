#include "signalmixer.h"

SignalMixer::SignalMixer() {}

double SignalMixer::process(double signalA, double signalB, double gainA, double gainB) {
    return signalA * gainA + signalB * gainB;
}

void SignalMixer::setGainA(double g) { gainA = g; }
void SignalMixer::setGainB(double g) { gainB = g; }
double SignalMixer::getGainA() const { return gainA; }
double SignalMixer::getGainB() const { return gainB; }

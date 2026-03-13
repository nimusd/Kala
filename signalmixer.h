#ifndef SIGNALMIXER_H
#define SIGNALMIXER_H

class SignalMixer {
public:
    SignalMixer();
    double process(double signalA, double signalB, double gainA, double gainB);
    void setGainA(double g);
    void setGainB(double g);
    double getGainA() const;
    double getGainB() const;
private:
    double gainA = 1.0;
    double gainB = 1.0;
};

#endif // SIGNALMIXER_H

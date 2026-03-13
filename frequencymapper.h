#ifndef FREQUENCYMAPPER_H
#define FREQUENCYMAPPER_H

#include <cmath>
#include <algorithm>

class FrequencyMapper
{
public:
    enum CurveType {
        Linear = 0,
        Logarithmic = 1,
        Exponential = 2,
        SCurve = 3
    };

    void setLowFreq(double freq)    { lowFreq = std::max(1.0, freq); }
    void setHighFreq(double freq)   { highFreq = std::max(lowFreq + 1.0, freq); }
    void setOutputMin(double val)   { outputMin = val; }
    void setOutputMax(double val)   { outputMax = val; }
    void setCurveType(CurveType t)  { curveType = t; }
    void setCurveType(int t)        { if (t >= 0 && t <= 3) curveType = static_cast<CurveType>(t); }
    void setInvert(bool inv)        { invert = inv; }

    // Map pitch (Hz) to output range using built-in curve
    double map(double pitchHz) const;

    // Map pitch (Hz) using external curve value to reshape the transfer
    double map(double pitchHz, double externalCurve) const;

    void reset() {} // Stateless — included for pattern consistency

private:
    double lowFreq = 25.0;
    double highFreq = 8000.0;
    double outputMin = 0.0;
    double outputMax = 1.0;
    CurveType curveType = Logarithmic;
    bool invert = false;

    double normalize(double pitchHz) const;
    double applyCurve(double t) const;
};

#endif // FREQUENCYMAPPER_H

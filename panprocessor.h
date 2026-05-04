#ifndef PANPROCESSOR_H
#define PANPROCESSOR_H

#include <algorithm>

/**
 * PanProcessor - Stereo pan position controller
 *
 * Outputs a pan value in the range [-1.0, +1.0] where:
 *   -1.0 = full left
 *    0.0 = center
 *   +1.0 = full right
 *
 * The base pan parameter sets a static position. When driven by
 * a controlIn connection (e.g. from a Frequency Mapper), the
 * external value is added to the base pan, then clamped.
 */
class PanProcessor
{
public:
    void setPan(double p) { pan = std::clamp(p, -1.0, 1.0); }

    double process(double controlIn) const {
        return std::clamp(pan + controlIn, -1.0, 1.0);
    }

    double process() const {
        return pan;
    }

    void reset() {} // Stateless

private:
    double pan = 0.0;
};

#endif // PANPROCESSOR_H

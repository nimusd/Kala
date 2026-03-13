#ifndef EASINGAPPLICATOR_H
#define EASINGAPPLICATOR_H

#include "easing.h"

/**
 * EasingApplicator - Shapes parameter transitions using easing functions
 *
 * Prescribed rather than emergent movement. Applies mathematical easing
 * functions to create smooth, stylized transitions.
 *
 * Parameters:
 * - startValue: Beginning of transition
 * - endValue: End of transition
 * - progress: Position in transition (0-1)
 * - easingSelect: Which easing function to use
 *
 * Output:
 * - easedValue: Result after applying easing function
 *
 * Processing:
 * - t_eased = easingFunction(progress)
 * - output = startValue + (endValue - startValue) × t_eased
 *
 * Use cases:
 * - Pitch glide between notes
 * - Formant sweeps for consonants
 * - Brightness bloom on attack
 *
 * Note: This class wraps the Easing class for backward compatibility.
 * New code should consider using Easing directly.
 */
class EasingApplicator
{
public:
    // Legacy enum kept for backward compatibility
    enum class EasingType {
        Linear = 0,      // No easing, constant speed
        QuadIn,          // Accelerating from zero velocity
        QuadOut,         // Decelerating to zero velocity
        QuadInOut,       // Acceleration until halfway, then deceleration
        CubicIn,         // Cubic acceleration
        CubicOut,        // Cubic deceleration
        CubicInOut,      // Cubic acceleration/deceleration
        SineIn,          // Sine wave acceleration
        SineOut,         // Sine wave deceleration
        SineInOut,       // Sine wave acceleration/deceleration
        Elastic,         // Elastic bounce (uses mode)
        Bounce,          // Bouncing ball effect (uses mode)
        Back             // Overshoot and come back (uses mode)
    };

    enum class EasingMode {
        In,      // Ease in (slow start)
        Out,     // Ease out (slow end)
        InOut    // Ease in and out
    };

    EasingApplicator();

    // Apply easing to interpolate between start and end
    double process(double startValue, double endValue, double progress);

    // Parameter setters
    void setEasingType(EasingType type);
    void setEasingSelect(int index);  // Select by index
    void setEasingMode(EasingMode mode);

    // New: Set easing directly using Easing class
    void setEasing(const Easing &easing);

    // Parameter getters
    EasingType getEasingType() const { return easingType; }
    EasingMode getEasingMode() const { return easingMode; }

    // New: Get the underlying Easing object
    const Easing& getEasing() const { return easing; }

    // Parameter setters (forwarded to internal Easing)
    void setOvershoot(double value) { easing.setOvershoot(value); }
    void setAmplitude(double value) { easing.setAmplitude(value); }
    void setPeriod(double value) { easing.setPeriod(value); }
    void setFrequency(double value) { easing.setFrequency(value); }
    void setDecay(double value) { easing.setDecay(value); }

    // Parameter getters
    double getOvershoot() const { return easing.getOvershoot(); }
    double getAmplitude() const { return easing.getAmplitude(); }
    double getPeriod() const { return easing.getPeriod(); }
    double getFrequency() const { return easing.getFrequency(); }
    double getDecay() const { return easing.getDecay(); }

private:
    void updateEasing();  // Updates internal Easing based on type+mode

    EasingType easingType;
    EasingMode easingMode;
    Easing easing;  // The actual easing function
};

#endif // EASINGAPPLICATOR_H

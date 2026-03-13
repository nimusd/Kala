#ifndef ENVELOPEENGINE_H
#define ENVELOPEENGINE_H

#include <algorithm>
#include <cmath>
#include <QVector>
#include "envelopelibraryDialog.h"

/**
 * EnvelopeEngine - Applies arbitrary envelope shapes to parameters over note lifetime
 *
 * Modulates parameters based on position in the note (0-1).
 * Use cases:
 * - Ben Webster's breathy release (breath blend increases as note fades)
 * - Brightness that evolves over sustained notes
 * - Vibrato that develops over time
 *
 * Parameters:
 * - noteProgress: Position in note (0-1)
 * - envelopeSelect: Which envelope curve to use
 * - timeScale: Stretch/compress duration
 * - valueScale: Multiply output
 * - valueOffset: Add to output
 *
 * Loop modes:
 * - None: Hold at final value after completion
 * - Loop: Restart from beginning
 * - Pingpong: Reverse direction and bounce
 */
class EnvelopeEngine
{
public:
    enum class EnvelopeType {
        Linear,           // Straight line 0->1
        ExponentialRise,  // Slow start, fast end (e^x curve)
        ExponentialFall,  // Fast start, slow end (1-e^x curve)
        Sine,             // Smooth S-curve (sin-based)
        AttackDecay,      // Rise to 1 then fall to 0 (triangle-like)
        SustainRelease,   // Stay at 1, then fall at end
        LogarithmicRise,  // Fast start, slow end — log10(1 + 9x)
        LogarithmicFall,  // Mirror: starts high, drops quickly then flattens
        Custom            // User-defined multi-point envelope
    };

    enum class LoopMode {
        None,       // Hold at final value
        Loop,       // Restart from beginning
        Pingpong    // Reverse direction and bounce
    };

    EnvelopeEngine();

    // Process and return envelope value for given note progress
    // isLegato: if true, skip attack phase and start at sustain/post-attack level
    double process(double noteProgress, bool isLegato = false);

    // Parameter setters
    void setEnvelopeType(EnvelopeType type);
    void setEnvelopeSelect(int index);  // Select by index (0-5)
    void setTimeScale(double scale);
    void setValueScale(double scale);
    void setValueOffset(double offset);
    void setLoopMode(LoopMode mode);

    // Envelope-specific parameters
    void setAttackTime(double time);
    void setDecayTime(double time);
    void setSustainLevel(double level);
    void setReleaseTime(double time);
    void setFadeTime(double time);

    // Custom envelope management
    void setCustomEnvelope(const QVector<EnvelopePoint> &points);
    QVector<EnvelopePoint> getCustomEnvelope() const { return customEnvelopePoints; }
    bool hasCustomEnvelope() const { return !customEnvelopePoints.isEmpty(); }

    // Parameter getters
    EnvelopeType getEnvelopeType() const { return envelopeType; }
    double getTimeScale() const { return timeScale; }
    double getValueScale() const { return valueScale; }
    double getValueOffset() const { return valueOffset; }
    LoopMode getLoopMode() const { return loopMode; }

private:
    double evaluateEnvelope(double position) const;
    double evaluateCustomEnvelope(double position) const;
    double applyLoopMode(double position) const;

    EnvelopeType envelopeType;
    double timeScale;
    double valueScale;
    double valueOffset;
    LoopMode loopMode;

    // Envelope-specific timing parameters
    double attackTime;
    double decayTime;
    double sustainLevel;
    double releaseTime;
    double fadeTime;

    // Custom envelope data
    QVector<EnvelopePoint> customEnvelopePoints;
};

#endif // ENVELOPEENGINE_H

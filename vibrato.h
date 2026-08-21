#ifndef VIBRATO_H
#define VIBRATO_H

#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <cmath>
#include <algorithm>
#include "envelopelibraryDialog.h"  // For EnvelopePoint

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * Vibrato - Per-note vibrato settings
 *
 * Vibrato is applied to the note after the note is built so that
 * if the note has an irregular dynamic evolution it will be preserved.
 */
struct Vibrato
{
    bool active = false;            // Whether vibrato is enabled for this note

    double rate = 5.0;              // Speed of oscillation in Hz (typical range: 3-8 Hz)
    double pitchDepth = 0.02;       // How much pitch varies (fraction, e.g., 0.02 = 2%)
    double amplitudeDepth = 0.3;    // How much loudness varies (0.0-1.0)
    double onset = 0.2;             // When vibrato starts (0.0-1.0, fraction into note)
    double regularity = 0.5;        // Physics variation (0 = mechanical, 1 = organic)

    // Envelope: how vibrato intensity grows over note lifetime
    // Default: linear ramp from 0 to 1
    QVector<EnvelopePoint> envelope;

    // Rate envelope: scales the rate over note lifetime (0.0 = 0 Hz, 1.0 = full rate)
    // Default: linear ramp from 0 to 1 (rate grows from 0 to max)
    QVector<EnvelopePoint> rateEnvelope;

    Vibrato()
    {
        // Default envelope: linear rise from onset to full intensity
        envelope.append(EnvelopePoint(0.0, 0.0, 0));  // Start at 0
        envelope.append(EnvelopePoint(1.0, 1.0, 0));  // End at full intensity

        // Default rate envelope: ramp from 0 to full rate
        rateEnvelope.append(EnvelopePoint(0.0, 0.0, 0));
        rateEnvelope.append(EnvelopePoint(1.0, 1.0, 0));
    }

    // Evaluate envelope at normalized time (0.0-1.0)
    double envelopeAt(double normalizedTime) const
    {
        if (envelope.isEmpty()) return 1.0;
        if (normalizedTime <= envelope.first().time) return envelope.first().value;
        if (normalizedTime >= envelope.last().time) return envelope.last().value;

        // Find the two points we're between
        for (int i = 0; i < envelope.size() - 1; ++i) {
            if (normalizedTime >= envelope[i].time && normalizedTime <= envelope[i + 1].time) {
                const EnvelopePoint &p1 = envelope[i];
                const EnvelopePoint &p2 = envelope[i + 1];

                double t = (normalizedTime - p1.time) / (p2.time - p1.time);

                if (p1.curveType == 0) {
                    // Linear
                    return p1.value + t * (p2.value - p1.value);
                } else if (p1.curveType == 1) {
                    // Smooth (cosine)
                    double smoothT = (1.0 - std::cos(t * M_PI)) * 0.5;
                    return p1.value + smoothT * (p2.value - p1.value);
                } else {
                    // Step
                    return p1.value;
                }
            }
        }
        return 1.0;
    }

    // Evaluate rate envelope at normalized time (0.0-1.0)
    // Returns a multiplier (0.0-1.0) that scales the base rate
    double rateAt(double normalizedTime) const
    {
        if (rateEnvelope.isEmpty()) return 1.0;
        if (normalizedTime <= rateEnvelope.first().time) return rateEnvelope.first().value;
        if (normalizedTime >= rateEnvelope.last().time) return rateEnvelope.last().value;

        for (int i = 0; i < rateEnvelope.size() - 1; ++i) {
            if (normalizedTime >= rateEnvelope[i].time && normalizedTime <= rateEnvelope[i + 1].time) {
                const EnvelopePoint &p1 = rateEnvelope[i];
                const EnvelopePoint &p2 = rateEnvelope[i + 1];

                double t = (normalizedTime - p1.time) / (p2.time - p1.time);

                if (p1.curveType == 0) {
                    return p1.value + t * (p2.value - p1.value);
                } else if (p1.curveType == 1) {
                    double smoothT = (1.0 - std::cos(t * M_PI)) * 0.5;
                    return p1.value + smoothT * (p2.value - p1.value);
                } else {
                    return p1.value;
                }
            }
        }
        return 1.0;
    }

    // Serialization
    QJsonObject toJson() const
    {
        QJsonObject json;
        json["active"] = active;
        json["rate"] = rate;
        json["pitchDepth"] = pitchDepth;
        json["amplitudeDepth"] = amplitudeDepth;
        json["onset"] = onset;
        json["regularity"] = regularity;

        QJsonArray envArray;
        for (const EnvelopePoint &pt : envelope) {
            QJsonObject ptObj;
            ptObj["time"] = pt.time;
            ptObj["value"] = pt.value;
            ptObj["curveType"] = pt.curveType;
            envArray.append(ptObj);
        }
        json["envelope"] = envArray;

        QJsonArray rateEnvArray;
        for (const EnvelopePoint &pt : rateEnvelope) {
            QJsonObject ptObj;
            ptObj["time"] = pt.time;
            ptObj["value"] = pt.value;
            ptObj["curveType"] = pt.curveType;
            rateEnvArray.append(ptObj);
        }
        json["rateEnvelope"] = rateEnvArray;

        return json;
    }

    static Vibrato fromJson(const QJsonObject &json)
    {
        Vibrato v;
        v.active = json["active"].toBool(false);
        v.rate = json["rate"].toDouble(5.0);
        v.pitchDepth = json["pitchDepth"].toDouble(0.02);
        v.amplitudeDepth = json["amplitudeDepth"].toDouble(0.3);
        v.onset = json["onset"].toDouble(0.2);
        v.regularity = json["regularity"].toDouble(0.5);

        v.envelope.clear();
        QJsonArray envArray = json["envelope"].toArray();
        for (const QJsonValue &val : envArray) {
            QJsonObject ptObj = val.toObject();
            EnvelopePoint pt;
            pt.time = ptObj["time"].toDouble();
            pt.value = ptObj["value"].toDouble();
            pt.curveType = ptObj["curveType"].toInt(0);
            v.envelope.append(pt);
        }

        // Ensure at least default envelope if empty
        if (v.envelope.isEmpty()) {
            v.envelope.append(EnvelopePoint(0.0, 0.0, 0));
            v.envelope.append(EnvelopePoint(1.0, 1.0, 0));
        }

        // Rate envelope — if missing (old projects), default to constant at max
        v.rateEnvelope.clear();
        QJsonArray rateEnvArray = json["rateEnvelope"].toArray();
        for (const QJsonValue &val : rateEnvArray) {
            QJsonObject ptObj = val.toObject();
            EnvelopePoint pt;
            pt.time = ptObj["time"].toDouble();
            pt.value = ptObj["value"].toDouble();
            pt.curveType = ptObj["curveType"].toInt(0);
            v.rateEnvelope.append(pt);
        }
        if (v.rateEnvelope.isEmpty()) {
            // Backward compat: old notes had constant rate, so use flat-at-max
            v.rateEnvelope.append(EnvelopePoint(0.0, 1.0, 0));
            v.rateEnvelope.append(EnvelopePoint(1.0, 1.0, 0));
        }

        return v;
    }
};

/**
 * Apply note vibrato to pitch and dynamics at normalized progress.
 *
 * Shared by the audio renderer (dtSeconds = 1/sampleRate per sample) and the
 * MIDI bake (dtSeconds = update interval per tick) so both paths produce the
 * same continuous vibrato. Phase accumulators advance only while vibrato is
 * active and past its onset, matching the original per-sample rendering.
 */
inline void applyVibrato(const Vibrato &v, double progress, double dtSeconds,
                         double &pitch, double &dynamics,
                         double &phaseAccum, double &organicAccum)
{
    if (v.active && progress >= v.onset) {
        double vibratoProgress = (progress - v.onset) / (1.0 - v.onset);
        double envelopeIntensity = v.envelopeAt(vibratoProgress);

        // Time-varying rate via rate envelope (0.0 = 0 Hz, 1.0 = full rate)
        double effectiveRate = v.rate * v.rateAt(vibratoProgress);

        // Advance phase accumulators (dt integration)
        double phaseInc = effectiveRate * 2.0 * M_PI * dtSeconds;
        phaseAccum += phaseInc;

        double organicMod = 1.0;
        if (v.regularity > 0.0) {
            double slowPhaseInc = effectiveRate * 0.23 * 2.0 * M_PI * dtSeconds;
            organicAccum += slowPhaseInc;
            organicMod = 1.0 + v.regularity * 0.3 * std::sin(organicAccum);
        }

        double vibratoPhase = phaseAccum
                            + v.regularity * 0.5 * std::sin(organicAccum * 0.7);

        double lfoValue = std::sin(vibratoPhase) * organicMod;
        pitch *= 1.0 + (lfoValue * v.pitchDepth * envelopeIntensity);
        dynamics *= std::max(0.0, 1.0 + (lfoValue * v.amplitudeDepth * envelopeIntensity));
    }
}

#endif // VIBRATO_H

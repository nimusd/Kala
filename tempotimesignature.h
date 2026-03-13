#ifndef TEMPOTIMESIGNATURE_H
#define TEMPOTIMESIGNATURE_H

#include <QJsonObject>

struct TempoTimeSignature {
    double bpm = 120.0;              // 20-300
    int timeSigNumerator = 4;        // 1-99
    int timeSigDenominator = 4;      // 1-64 (or 0 for simple mode)
    bool gradualTransition = false;  // Interpolate tempo to next marker

    // Serialization
    QJsonObject toJson() const;
    static TempoTimeSignature fromJson(const QJsonObject &json);

    // Comparison
    bool operator==(const TempoTimeSignature &other) const;
    bool operator!=(const TempoTimeSignature &other) const;
};

#endif // TEMPOTIMESIGNATURE_H

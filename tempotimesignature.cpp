#include "tempotimesignature.h"

QJsonObject TempoTimeSignature::toJson() const
{
    QJsonObject json;
    json["bpm"] = bpm;
    json["timeSigNumerator"] = timeSigNumerator;
    json["timeSigDenominator"] = timeSigDenominator;
    json["gradualTransition"] = gradualTransition;
    return json;
}

TempoTimeSignature TempoTimeSignature::fromJson(const QJsonObject &json)
{
    TempoTimeSignature tts;
    tts.bpm = json["bpm"].toDouble(120.0);
    tts.timeSigNumerator = json["timeSigNumerator"].toInt(4);
    tts.timeSigDenominator = json["timeSigDenominator"].toInt(4);
    tts.gradualTransition = json["gradualTransition"].toBool(false);
    return tts;
}

bool TempoTimeSignature::operator==(const TempoTimeSignature &other) const
{
    return bpm == other.bpm &&
           timeSigNumerator == other.timeSigNumerator &&
           timeSigDenominator == other.timeSigDenominator &&
           gradualTransition == other.gradualTransition;
}

bool TempoTimeSignature::operator!=(const TempoTimeSignature &other) const
{
    return !(*this == other);
}

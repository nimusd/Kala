#include "compositionsettings.h"
#include <cmath>

CompositionSettings::CompositionSettings()
    : compositionName("Untitled")
    , timeMode(Musical)
    , tempo(100)
    , referenceTempo(100)  // Initially matches tempo
    , timeSigTop(5)
    , timeSigBottom(4)
    , lengthMs(300000.0)
    , lengthBars(20)
    , sampleRate(48000)
    , bitDepth(24)
{
}

CompositionSettings CompositionSettings::defaults()
{
    return CompositionSettings();  // Use the constructor
}

double CompositionSettings::calculateBarDuration() const
{
    if (timeSigBottom == 0) {
        // Simple mode: beat = tempo pulse
        return (60000.0 / tempo) * timeSigTop;
    } else {
        // Scaled mode: adjust for note value
        return (60000.0 / tempo) * (static_cast<double>(timeSigTop) / timeSigBottom);
    }
}

void CompositionSettings::syncLengthFromBars()
{
    lengthMs = calculateBarDuration() * lengthBars;
}

void CompositionSettings::syncLengthFromMs()
{
    double barDuration = calculateBarDuration();
    if (barDuration > 0.0) {
        lengthBars = static_cast<int>(std::ceil(lengthMs / barDuration));
    }
}

QJsonObject CompositionSettings::toJson() const
{
    QJsonObject json;
    json["name"] = compositionName;
    json["timeMode"] = (timeMode == Musical) ? "Musical" : "Absolute";
    json["tempo"] = tempo;
    json["referenceTempo"] = referenceTempo;
    json["timeSigTop"] = timeSigTop;
    json["timeSigBottom"] = timeSigBottom;
    json["lengthMs"] = lengthMs;
    json["lengthBars"] = lengthBars;
    json["sampleRate"] = sampleRate;
    json["bitDepth"] = bitDepth;
    return json;
}

CompositionSettings CompositionSettings::fromJson(const QJsonObject &json)
{
    CompositionSettings s;
    s.compositionName = json["name"].toString("Untitled");
    s.timeMode = (json["timeMode"].toString() == "Musical") ? Musical : Absolute;
    s.tempo = json["tempo"].toInt(100);
    // Default referenceTempo to tempo if not present (backwards compatibility)
    s.referenceTempo = json["referenceTempo"].toInt(s.tempo);
    s.timeSigTop = json["timeSigTop"].toInt(5);
    s.timeSigBottom = json["timeSigBottom"].toInt(4);
    s.lengthMs = json["lengthMs"].toDouble(300000.0);
    s.lengthBars = json["lengthBars"].toInt(20);
    s.sampleRate = json["sampleRate"].toInt(48000);
    s.bitDepth = json["bitDepth"].toInt(24);
    return s;
}

bool CompositionSettings::isValid() const
{
    return tempo >= 20 && tempo <= 300 &&
           referenceTempo >= 20 && referenceTempo <= 300 &&
           timeSigTop >= 1 && timeSigTop <= 99 &&
           timeSigBottom >= 0 && timeSigBottom <= 99 &&
           lengthMs > 0.0 &&
           lengthBars > 0;
}

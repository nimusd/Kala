#include "compositionsettings.h"
#include <cmath>

CompositionSettings::CompositionSettings()
    : compositionName("Untitled")
    , timeMode(Musical)
    , tempo(120)
    , referenceTempo(120)  // Initially matches tempo
    , timeSigTop(5)
    , timeSigBottom(0)
    , lengthMs(300000.0)
    , lengthBars(20)
    , sampleRate(48000)
    , bitDepth(24)
    , preRenderDelayMs(400)
    , autoRender(true)
{
}

CompositionSettings CompositionSettings::defaults()
{
    return CompositionSettings();  // Use the constructor
}

double CompositionSettings::calculateBarDuration() const
{
    // Bar duration = beat duration × beats per bar.
    // Tempo is always "beats per minute" where a beat is one pulse (Kala)
    // or one denominator-note (Western). Denominator is informational for
    // display/notation and does not scale bar duration.
    return (60000.0 / tempo) * timeSigTop;
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
    json["preRenderDelayMs"] = preRenderDelayMs;
    json["autoRender"] = autoRender;
    return json;
}

CompositionSettings CompositionSettings::fromJson(const QJsonObject &json)
{
    CompositionSettings s;
    s.compositionName = json["name"].toString("Untitled");
    s.timeMode = (json["timeMode"].toString() == "Musical") ? Musical : Absolute;
    s.tempo = json["tempo"].toInt(60);
    // Default referenceTempo to tempo if not present (backwards compatibility)
    s.referenceTempo = json["referenceTempo"].toInt(s.tempo);
    s.timeSigTop = json["timeSigTop"].toInt(5);
    s.timeSigBottom = json["timeSigBottom"].toInt(4);
    s.lengthMs = json["lengthMs"].toDouble(300000.0);
    s.lengthBars = json["lengthBars"].toInt(20);
    s.sampleRate = json["sampleRate"].toInt(48000);
    s.bitDepth = json["bitDepth"].toInt(24);
    s.preRenderDelayMs = json["preRenderDelayMs"].toInt(400);
    s.autoRender = json["autoRender"].toBool(true);
    return s;
}

bool CompositionSettings::isValid() const
{
    return tempo >= 1 && tempo <= 300 &&
           referenceTempo >= 1 && referenceTempo <= 300 &&
           timeSigTop >= 1 && timeSigTop <= 99 &&
           timeSigBottom >= 0 && timeSigBottom <= 99 &&
           lengthMs > 0.0 &&
           lengthBars > 0;
}

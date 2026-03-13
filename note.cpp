#include "note.h"
#include <cstring>  // For std::memcpy in computeHash()
#include <QJsonObject>

Note::Note()
    : id(QUuid::createUuid().toString())
    , startTime(0.0)
    , duration(1000.0)  // Default 1 second
    , pitchHz(440.0)    // Default A4
    , trackIndex(0)       // Default to track 0
    , variationIndex(0)   // Default to base sounit
    , quantized(false)          // Not quantized by default
    , renderDirty(true)         // New notes need rendering
    , legato(false)             // Not legato by default
    , linkedAsLegato(false)     // Not linked-as-legato by default
    , dynamicsCurve(0.5)  // Default medium dynamics (constant curve)
    , bottomCurve(0.6)    // Default bottom curve value (spectrum placeholder)
{
}

Note::Note(double startTime, double duration, double pitchHz, double dynamics)
    : id(QUuid::createUuid().toString())
    , startTime(startTime)
    , duration(duration)
    , pitchHz(pitchHz)
    , trackIndex(0)           // Default to track 0
    , variationIndex(0)       // Default to base sounit
    , quantized(false)          // Not quantized by default
    , renderDirty(true)         // New notes need rendering
    , legato(false)             // Not legato by default
    , linkedAsLegato(false)     // Not linked-as-legato by default
    , dynamicsCurve(dynamics)  // Create constant dynamics curve
    , bottomCurve(0.6)         // Default bottom curve value
{
}

double Note::getDynamics() const
{
    // Return average dynamics across the curve
    if (dynamicsCurve.isEmpty()) {
        return 0.5;  // Default
    }

    // Sample at several points and average
    const int numSamples = 10;
    double sum = 0.0;
    for (int i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / (numSamples - 1);
        sum += dynamicsCurve.valueAt(t);
    }
    return sum / numSamples;
}

double Note::getDynamicsAt(double normalizedTime) const
{
    return dynamicsCurve.valueAt(normalizedTime);
}

double Note::getPitchAt(double normalizedTime) const
{
    // If pitch curve exists, use it; otherwise return constant pitch
    if (pitchCurve.getPoints().size() > 0) {
        return pitchCurve.valueAt(normalizedTime);
    }
    return pitchHz;  // Constant pitch
}

void Note::setDynamics(double dyn)
{
    // Set constant dynamics (flat curve)
    dynamicsCurve.clearPoints();
    dynamicsCurve.addPoint(0.0, dyn);
    dynamicsCurve.addPoint(1.0, dyn);
}

double Note::getBottomCurveAt(double normalizedTime) const
{
    return bottomCurve.valueAt(normalizedTime);
}

QString Note::getExpressiveCurveName(int index) const
{
    if (index == 0) return "Dynamics";
    if (index >= 1 && index <= m_additionalExpressiveCurves.size())
        return m_additionalExpressiveCurves[index - 1].name;
    return QString();
}

const Curve& Note::getExpressiveCurve(int index) const
{
    if (index == 0) return dynamicsCurve;
    return m_additionalExpressiveCurves[index - 1].curve;
}

Curve& Note::getExpressiveCurve(int index)
{
    if (index == 0) return dynamicsCurve;
    return m_additionalExpressiveCurves[index - 1].curve;
}

void Note::addExpressiveCurve(const QString &name, const Curve &curve)
{
    NamedCurve nc;
    nc.name = name;
    nc.curve = curve;
    m_additionalExpressiveCurves.append(nc);
    renderDirty = true;
}

void Note::removeExpressiveCurve(int index)
{
    if (index >= 1 && index <= m_additionalExpressiveCurves.size()) {
        m_additionalExpressiveCurves.removeAt(index - 1);
        renderDirty = true;
    }
}

void Note::renameExpressiveCurve(int index, const QString &name)
{
    if (index >= 1 && index <= m_additionalExpressiveCurves.size()) {
        m_additionalExpressiveCurves[index - 1].name = name;
    }
}

void Note::detectSegments()
{
    segments.clear();
    if (!hasPitchCurve() || !quantized) return;

    const auto& points = pitchCurve.getPoints();
    if (points.isEmpty()) return;

    Segment currentSeg;
    currentSeg.startTime = 0.0;
    currentSeg.pitchHz = points[0].value;
    currentSeg.useNoteVibrato = true;
    currentSeg.variationIndex = -1;

    const double pitchTolerance = 0.5;  // Hz tolerance for detecting pitch changes

    for (int i = 1; i < points.size(); ++i) {
        // Detect pitch change (step transition)
        if (std::abs(points[i].value - currentSeg.pitchHz) > pitchTolerance) {
            // End current segment
            currentSeg.endTime = points[i].time;
            segments.append(currentSeg);

            // Start new segment
            currentSeg.startTime = points[i].time;
            currentSeg.pitchHz = points[i].value;
            currentSeg.useNoteVibrato = true;
            currentSeg.variationIndex = -1;
        }
    }

    // Add final segment
    currentSeg.endTime = 1.0;
    segments.append(currentSeg);
}

void Note::updateSegmentPitch(int segmentIndex, double newPitchHz)
{
    if (segmentIndex < 0 || segmentIndex >= segments.size()) return;
    if (!hasPitchCurve()) return;

    Segment& seg = segments[segmentIndex];
    double oldPitchHz = seg.pitchHz;
    seg.pitchHz = newPitchHz;

    // Update the pitch curve to reflect the new pitch for this segment
    Curve newPitchCurve;
    const auto& points = pitchCurve.getPoints();

    for (const auto& point : points) {
        double t = point.time;
        double pitch = point.value;

        // If this point is within the segment being edited, use the new pitch
        if (t >= seg.startTime && t <= seg.endTime) {
            // Check if this point was at the old segment pitch
            if (std::abs(pitch - oldPitchHz) < 1.0) {
                pitch = newPitchHz;
            }
        }

        newPitchCurve.addPoint(t, pitch);
    }

    pitchCurve = newPitchCurve;
    renderDirty = true;
}

int Note::findSegmentAtTime(double normalizedTime) const
{
    for (int i = 0; i < segments.size(); ++i) {
        if (normalizedTime >= segments[i].startTime && normalizedTime <= segments[i].endTime) {
            return i;
        }
    }
    return -1;
}

uint64_t Note::computeHash() const
{
    // Hash note properties that affect rendered audio
    // NOTE: startTime is NOT included - moving a note doesn't change its audio
    //
    // We use a simple FNV-1a inspired hash combining:
    // - duration (affects render length)
    // - pitchHz (base pitch)
    // - pitch curve points (for glissando/portamento)
    // - dynamics curve points (for expression)
    // - bottom curve points (for timbre)

    uint64_t hash = 14695981039346656037ULL;  // FNV offset basis
    const uint64_t prime = 1099511628211ULL;  // FNV prime

    // Helper lambda to mix a double into the hash
    auto mixDouble = [&hash, prime](double value) {
        uint64_t bits;
        std::memcpy(&bits, &value, sizeof(bits));
        hash ^= bits;
        hash *= prime;
    };

    // Hash duration and base pitch
    mixDouble(duration);
    mixDouble(pitchHz);

    // Hash variation index (different variation = different sound)
    hash ^= static_cast<uint64_t>(variationIndex);
    hash *= prime;

    // Hash pitch curve
    const auto& pitchPoints = pitchCurve.getPoints();
    hash ^= static_cast<uint64_t>(pitchPoints.size());
    hash *= prime;
    for (const auto& point : pitchPoints) {
        mixDouble(point.time);
        mixDouble(point.value);
    }

    // Hash dynamics curve
    const auto& dynPoints = dynamicsCurve.getPoints();
    hash ^= static_cast<uint64_t>(dynPoints.size());
    hash *= prime;
    for (const auto& point : dynPoints) {
        mixDouble(point.time);
        mixDouble(point.value);
    }

    // Hash bottom curve
    const auto& bottomPoints = bottomCurve.getPoints();
    hash ^= static_cast<uint64_t>(bottomPoints.size());
    hash *= prime;
    for (const auto& point : bottomPoints) {
        mixDouble(point.time);
        mixDouble(point.value);
    }

    // Hash vibrato settings
    hash ^= static_cast<uint64_t>(vibrato.active ? 1 : 0);
    hash *= prime;
    if (vibrato.active) {
        mixDouble(vibrato.rate);
        mixDouble(vibrato.pitchDepth);
        mixDouble(vibrato.amplitudeDepth);
        mixDouble(vibrato.onset);
        mixDouble(vibrato.regularity);
        hash ^= static_cast<uint64_t>(vibrato.envelope.size());
        hash *= prime;
        for (const auto& pt : vibrato.envelope) {
            mixDouble(pt.time);
            mixDouble(pt.value);
            hash ^= static_cast<uint64_t>(pt.curveType);
            hash *= prime;
        }
    }

    // Hash legato flag (affects attack/release rendering)
    hash ^= static_cast<uint64_t>(legato ? 1 : 0);
    hash *= prime;

    return hash;
}

QJsonObject Note::toJson() const
{
    QJsonObject json;

    json["id"] = id;
    json["startTime"] = startTime;
    json["duration"] = duration;
    json["pitchHz"] = pitchHz;
    json["trackIndex"] = trackIndex;
    json["variationIndex"] = variationIndex;
    json["quantized"] = quantized;
    json["legato"] = legato;
    json["linkedAsLegato"] = linkedAsLegato;

    // Serialize curves
    json["pitchCurve"] = pitchCurve.toJson();
    json["dynamicsCurve"] = dynamicsCurve.toJson();
    json["bottomCurve"] = bottomCurve.toJson();

    // Serialize vibrato
    json["vibrato"] = vibrato.toJson();

    // Serialize segments if quantized and segments exist
    if (quantized && !segments.isEmpty()) {
        QJsonArray segArray;
        for (const Segment& seg : segments) {
            segArray.append(seg.toJson());
        }
        json["segments"] = segArray;
    }

    // Serialize additional expressive curves
    if (!m_additionalExpressiveCurves.isEmpty()) {
        QJsonArray ecArray;
        for (const NamedCurve &nc : m_additionalExpressiveCurves) {
            QJsonObject ecObj;
            ecObj["name"] = nc.name;
            ecObj["curve"] = nc.curve.toJson();
            ecArray.append(ecObj);
        }
        json["expressiveCurves"] = ecArray;
    }

    return json;
}

Note Note::fromJson(const QJsonObject &json)
{
    Note note;

    // Restore ID (preserve original ID for references)
    note.id = json["id"].toString(QUuid::createUuid().toString());

    note.startTime = json["startTime"].toDouble(0.0);
    note.duration = json["duration"].toDouble(1000.0);
    note.pitchHz = json["pitchHz"].toDouble(440.0);
    note.trackIndex = json["trackIndex"].toInt(0);
    note.variationIndex = json["variationIndex"].toInt(0);
    note.quantized = json["quantized"].toBool(false);
    note.legato = json["legato"].toBool(false);
    note.linkedAsLegato = json["linkedAsLegato"].toBool(false);

    // Deserialize curves
    note.pitchCurve = Curve::fromJson(json["pitchCurve"].toObject());
    note.dynamicsCurve = Curve::fromJson(json["dynamicsCurve"].toObject());
    note.bottomCurve = Curve::fromJson(json["bottomCurve"].toObject());

    // Deserialize vibrato
    if (json.contains("vibrato")) {
        note.vibrato = Vibrato::fromJson(json["vibrato"].toObject());
    }

    // Deserialize segments
    if (json.contains("segments")) {
        QJsonArray segArray = json["segments"].toArray();
        for (const QJsonValue& val : segArray) {
            note.segments.append(Segment::fromJson(val.toObject()));
        }
    }

    // Deserialize additional expressive curves (backward-compatible)
    if (json.contains("expressiveCurves")) {
        QJsonArray ecArray = json["expressiveCurves"].toArray();
        for (const QJsonValue &val : ecArray) {
            QJsonObject ecObj = val.toObject();
            NamedCurve nc;
            nc.name = ecObj["name"].toString();
            nc.curve = Curve::fromJson(ecObj["curve"].toObject());
            note.m_additionalExpressiveCurves.append(nc);
        }
    }

    // Mark as needing render (freshly loaded)
    note.renderDirty = true;

    return note;
}

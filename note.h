#ifndef NOTE_H
#define NOTE_H

#include "curve.h"
#include "vibrato.h"
#include <QString>
#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>

/**
 * Segment - A constant-pitch section within a quantized continuous note
 *
 * When a continuous note is "snapped to scale", it creates a stepped pitch curve.
 * Each horizontal step is a segment that can be individually edited.
 */
struct Segment {
    double startTime;       // Normalized time (0.0-1.0) where segment starts
    double endTime;         // Normalized time where segment ends
    double pitchHz;         // Pitch for this segment
    int variationIndex;     // Per-segment sounit variation (-1 = use note default)
    Vibrato vibrato;        // Per-segment vibrato settings
    bool useNoteVibrato;    // If true, inherit from note's vibrato

    Segment() : startTime(0), endTime(1), pitchHz(0),
                variationIndex(-1), useNoteVibrato(true) {}

    QJsonObject toJson() const {
        QJsonObject json;
        json["startTime"] = startTime;
        json["endTime"] = endTime;
        json["pitchHz"] = pitchHz;
        json["variationIndex"] = variationIndex;
        json["useNoteVibrato"] = useNoteVibrato;
        if (!useNoteVibrato) {
            json["vibrato"] = vibrato.toJson();
        }
        return json;
    }

    static Segment fromJson(const QJsonObject &json) {
        Segment seg;
        seg.startTime = json["startTime"].toDouble(0.0);
        seg.endTime = json["endTime"].toDouble(1.0);
        seg.pitchHz = json["pitchHz"].toDouble(0.0);
        seg.variationIndex = json["variationIndex"].toInt(-1);
        seg.useNoteVibrato = json["useNoteVibrato"].toBool(true);
        if (!seg.useNoteVibrato && json.contains("vibrato")) {
            seg.vibrato = Vibrato::fromJson(json["vibrato"].toObject());
        }
        return seg;
    }
};

/**
 * Note - The compositional atom
 *
 * What you see on the score canvas. Stores parameter curves captured from pen input.
 */
class Note
{
public:
    Note();
    Note(double startTime, double duration, double pitchHz, double dynamics = 0.5);

    // Getters
    QString getId() const { return id; }
    double getStartTime() const { return startTime; }
    double getDuration() const { return duration; }
    double getPitchHz() const { return pitchHz; }
    double getEndTime() const { return startTime + duration; }
    int getTrackIndex() const { return trackIndex; }
    int getVariationIndex() const { return variationIndex; }

    // Pitch - supports both simple value and curve (for glissando/portamento)
    double getPitchAt(double normalizedTime) const;  // Query pitch curve at specific time
    const Curve& getPitchCurve() const { return pitchCurve; }
    Curve& getPitchCurve() { return pitchCurve; }
    bool hasPitchCurve() const { return pitchCurve.getPoints().size() > 0; }

    // Quantization state (tracks whether continuous note has been snapped to scale)
    bool isQuantized() const { return quantized; }
    void setQuantized(bool q) { quantized = q; }

    // Render dirty flag (tracks whether note needs re-rendering)
    bool isRenderDirty() const { return renderDirty; }
    void setRenderDirty(bool dirty) { renderDirty = dirty; }

    // Compute hash of note properties for render cache invalidation
    // Hash changes when pitch, duration, or curves change (but NOT when startTime changes)
    uint64_t computeHash() const;

    // Serialization
    QJsonObject toJson() const;
    static Note fromJson(const QJsonObject &json);

    // Dynamics - supports both simple value and curve
    double getDynamics() const;  // Returns average dynamics
    double getDynamicsAt(double normalizedTime) const;  // Query curve at specific time
    const Curve& getDynamicsCurve() const { return dynamicsCurve; }
    Curve& getDynamicsCurve() { return dynamicsCurve; }

    // Bottom curve (spectrum/timbre for now, will be configurable later)
    double getBottomCurveAt(double normalizedTime) const;
    const Curve& getBottomCurve() const { return bottomCurve; }
    Curve& getBottomCurve() { return bottomCurve; }

    // Vibrato
    const Vibrato& getVibrato() const { return vibrato; }
    Vibrato& getVibrato() { return vibrato; }
    void setVibrato(const Vibrato &v) { vibrato = v; }

    // Expressive curves (index 0 = dynamics, index 1+ = additional)
    int getExpressiveCurveCount() const { return 1 + m_additionalExpressiveCurves.size(); }
    QString getExpressiveCurveName(int index) const;
    const Curve& getExpressiveCurve(int index) const;
    Curve& getExpressiveCurve(int index);
    void addExpressiveCurve(const QString &name, const Curve &curve);
    void removeExpressiveCurve(int index);  // index >= 1 only
    void renameExpressiveCurve(int index, const QString &name);

    // Setters
    void setStartTime(double time) { startTime = time; }
    void setDuration(double dur) { duration = dur; }
    void setPitchHz(double pitch) { pitchHz = pitch; }
    void setDynamics(double dyn);  // Sets constant dynamics
    void setDynamicsCurve(const Curve &curve) { dynamicsCurve = curve; }
    void setBottomCurve(const Curve &curve) { bottomCurve = curve; }
    void setPitchCurve(const Curve &curve) { pitchCurve = curve; }
    void setTrackIndex(int index) { trackIndex = index; }
    void setVariationIndex(int index) { variationIndex = index; }

    // Generate a new unique ID (used when copying/pasting notes)
    void regenerateId() { id = QUuid::createUuid().toString(); }

    // Legato articulation (note connects smoothly from previous, no re-attack)
    bool isLegato() const { return legato; }
    void setLegato(bool l) { legato = l; }

    // True only when this note was created by "Link as Legato" (not just quantized)
    bool isLinkedAsLegato() const { return linkedAsLegato; }
    void setLinkedAsLegato(bool l) { linkedAsLegato = l; }

    // Segment management (for quantized continuous notes)
    const QVector<Segment>& getSegments() const { return segments; }
    QVector<Segment>& getSegments() { return segments; }
    void detectSegments();  // Analyze pitch curve to populate segments
    void updateSegmentPitch(int segmentIndex, double newPitchHz);
    int getSegmentCount() const { return segments.size(); }
    int findSegmentAtTime(double normalizedTime) const;
    void clearSegments() { segments.clear(); }

private:
    QString id;           // Unique identifier
    double startTime;     // Start time in milliseconds
    double duration;      // Duration in milliseconds
    double pitchHz;       // Base pitch in Hz (used when no pitch curve)
    int trackIndex;       // Which track/sounit this note uses (default 0)
    int variationIndex;   // Which variation to use (0 = base sounit, 1+ = variation)
    bool quantized;       // True if continuous note has been snapped to scale
    bool renderDirty;     // True if note needs re-rendering (new notes start dirty)
    bool legato;          // True if note connects smoothly from previous (no re-attack)
    bool linkedAsLegato;  // True if created by "Link as Legato" (visual flag, not audio)
    Curve pitchCurve;     // Pitch curve over time (Hz values for glissando/portamento)
    Curve dynamicsCurve;  // Dynamics curve over time (0.0-1.0)
    Curve bottomCurve;    // Bottom edge curve (spectrum/timbre placeholder, configurable later)
    Vibrato vibrato;      // Per-note vibrato settings
    QVector<Segment> segments;  // Per-segment properties (populated when quantized)

    // Named additional expressive curves (dynamics is always index 0, handled separately)
    struct NamedCurve {
        QString name;
        Curve curve;
    };
    QVector<NamedCurve> m_additionalExpressiveCurves;
};

#endif // NOTE_H

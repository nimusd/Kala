#include "scorecanvascommands.h"
#include "scorecanvas.h"
#include <QDebug>
#include <cmath>

// ============================================================================
// Add Note Command
// ============================================================================

AddNoteCommand::AddNoteCommand(Phrase *phrase, const Note &note, ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_note(note)
    , m_canvas(canvas)
    , m_firstTime(true)
{
    setText("Add Note");
}

void AddNoteCommand::undo()
{
    // Remove the last note (the one we added)
    QVector<Note> &notes = m_phrase->getNotes();
    if (!notes.isEmpty()) {
        notes.removeLast();
        m_canvas->update();
        qDebug() << "Undo: Note removed";
    }
}

void AddNoteCommand::redo()
{
    // Add the note
    m_phrase->addNote(m_note);
    m_canvas->update();
    qDebug() << "Redo: Note added";
}

// ============================================================================
// Delete Note Command
// ============================================================================

DeleteNoteCommand::DeleteNoteCommand(Phrase *phrase, int noteIndex, ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndex(noteIndex)
    , m_canvas(canvas)
{
    // Store the note before it's deleted
    const QVector<Note> &notes = m_phrase->getNotes();
    if (noteIndex >= 0 && noteIndex < notes.size()) {
        m_note = notes[noteIndex];
    }
    setText("Delete Note");
}

void DeleteNoteCommand::undo()
{
    // Re-insert the note at its original position
    QVector<Note> &notes = m_phrase->getNotes();
    notes.insert(m_noteIndex, m_note);
    m_canvas->update();
    qDebug() << "Undo: Note restored at index" << m_noteIndex;
}

void DeleteNoteCommand::redo()
{
    // Remove the note at the stored index
    m_phrase->removeNoteByIndex(m_noteIndex);
    m_canvas->update();
    qDebug() << "Redo: Note deleted at index" << m_noteIndex;
}

// ============================================================================
// Move Note Command
// ============================================================================

MoveNoteCommand::MoveNoteCommand(Phrase *phrase, int noteIndex,
                                 double oldStartTime, double oldPitch,
                                 double newStartTime, double newPitch,
                                 const Curve &oldPitchCurve, const Curve &newPitchCurve,
                                 bool hasPitchCurve, ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndex(noteIndex)
    , m_oldStartTime(oldStartTime)
    , m_oldPitch(oldPitch)
    , m_newStartTime(newStartTime)
    , m_newPitch(newPitch)
    , m_oldPitchCurve(oldPitchCurve)
    , m_newPitchCurve(newPitchCurve)
    , m_hasPitchCurve(hasPitchCurve)
    , m_canvas(canvas)
{
    setText("Move Note");
}

void MoveNoteCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    if (m_noteIndex >= 0 && m_noteIndex < notes.size()) {
        Note &note = notes[m_noteIndex];
        note.setStartTime(m_oldStartTime);

        if (m_hasPitchCurve) {
            note.setPitchCurve(m_oldPitchCurve);
            if (note.isQuantized()) note.detectSegments();
        } else {
            note.setPitchHz(m_oldPitch);
        }

        m_canvas->update();
        qDebug() << "Undo: Note moved to" << m_oldStartTime << "ms," << m_oldPitch << "Hz";
    }
}

void MoveNoteCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    if (m_noteIndex >= 0 && m_noteIndex < notes.size()) {
        Note &note = notes[m_noteIndex];
        note.setStartTime(m_newStartTime);

        if (m_hasPitchCurve) {
            note.setPitchCurve(m_newPitchCurve);
            if (note.isQuantized()) note.detectSegments();
        } else {
            note.setPitchHz(m_newPitch);
        }

        m_canvas->update();
        qDebug() << "Redo: Note moved to" << m_newStartTime << "ms," << m_newPitch << "Hz";
    }
}

bool MoveNoteCommand::mergeWith(const QUndoCommand *other)
{
    // Merge consecutive move commands for the same note
    if (other->id() != id()) {
        return false;
    }

    const MoveNoteCommand *moveCommand = static_cast<const MoveNoteCommand*>(other);
    if (moveCommand->m_noteIndex != m_noteIndex) {
        return false;
    }

    // Update the new position to the latest
    m_newStartTime = moveCommand->m_newStartTime;
    m_newPitch = moveCommand->m_newPitch;
    m_newPitchCurve = moveCommand->m_newPitchCurve;

    return true;
}

// ============================================================================
// Resize Note Command
// ============================================================================

ResizeNoteCommand::ResizeNoteCommand(Phrase *phrase, int noteIndex,
                                     double oldStartTime, double oldDuration,
                                     double newStartTime, double newDuration,
                                     ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndex(noteIndex)
    , m_oldStartTime(oldStartTime)
    , m_oldDuration(oldDuration)
    , m_newStartTime(newStartTime)
    , m_newDuration(newDuration)
    , m_canvas(canvas)
{
    setText("Resize Note");
}

void ResizeNoteCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    if (m_noteIndex >= 0 && m_noteIndex < notes.size()) {
        Note &note = notes[m_noteIndex];
        note.setStartTime(m_oldStartTime);
        note.setDuration(m_oldDuration);
        m_canvas->update();
        qDebug() << "Undo: Note resized to" << m_oldStartTime << "ms," << m_oldDuration << "ms duration";
    }
}

void ResizeNoteCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    if (m_noteIndex >= 0 && m_noteIndex < notes.size()) {
        Note &note = notes[m_noteIndex];
        note.setStartTime(m_newStartTime);
        note.setDuration(m_newDuration);
        m_canvas->update();
        qDebug() << "Redo: Note resized to" << m_newStartTime << "ms," << m_newDuration << "ms duration";
    }
}

bool ResizeNoteCommand::mergeWith(const QUndoCommand *other)
{
    // Merge consecutive resize commands for the same note
    if (other->id() != id()) {
        return false;
    }

    const ResizeNoteCommand *resizeCommand = static_cast<const ResizeNoteCommand*>(other);
    if (resizeCommand->m_noteIndex != m_noteIndex) {
        return false;
    }

    // Update the new size to the latest
    m_newStartTime = resizeCommand->m_newStartTime;
    m_newDuration = resizeCommand->m_newDuration;

    return true;
}

// ============================================================================
// Edit Curve Command
// ============================================================================

EditCurveCommand::EditCurveCommand(Phrase *phrase, int noteIndex, CurveType curveType,
                                   const Curve &oldCurve, const Curve &newCurve,
                                   ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndex(noteIndex)
    , m_curveType(curveType)
    , m_curveIndex(0)
    , m_oldCurve(oldCurve)
    , m_newCurve(newCurve)
    , m_canvas(canvas)
{
    if (curveType == DynamicsCurve) {
        setText("Edit Dynamics Curve");
    } else if (curveType == PitchCurve) {
        setText("Edit Pitch Curve");
    } else {
        setText("Edit Bottom Curve");
    }
}

EditCurveCommand::EditCurveCommand(Phrase *phrase, int noteIndex, CurveType curveType,
                                   int curveIndex,
                                   const Curve &oldCurve, const Curve &newCurve,
                                   ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndex(noteIndex)
    , m_curveType(curveType)
    , m_curveIndex(curveIndex)
    , m_oldCurve(oldCurve)
    , m_newCurve(newCurve)
    , m_canvas(canvas)
{
    setText(QString("Edit Expressive Curve"));
}

void EditCurveCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    if (m_noteIndex >= 0 && m_noteIndex < notes.size()) {
        Note &note = notes[m_noteIndex];

        if (m_curveType == DynamicsCurve) {
            note.setDynamicsCurve(m_oldCurve);
        } else if (m_curveType == BottomCurve) {
            note.setBottomCurve(m_oldCurve);
        } else if (m_curveType == PitchCurve) {
            note.setPitchCurve(m_oldCurve);
        } else if (m_curveType == ExpressiveCurveN) {
            if (m_curveIndex >= 0 && m_curveIndex < note.getExpressiveCurveCount()) {
                note.getExpressiveCurve(m_curveIndex) = m_oldCurve;
            }
        }

        m_canvas->update();
        qDebug() << "Undo: Curve edited";
    }
}

void EditCurveCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    if (m_noteIndex >= 0 && m_noteIndex < notes.size()) {
        Note &note = notes[m_noteIndex];

        if (m_curveType == DynamicsCurve) {
            note.setDynamicsCurve(m_newCurve);
        } else if (m_curveType == BottomCurve) {
            note.setBottomCurve(m_newCurve);
        } else if (m_curveType == PitchCurve) {
            note.setPitchCurve(m_newCurve);
        } else if (m_curveType == ExpressiveCurveN) {
            if (m_curveIndex >= 0 && m_curveIndex < note.getExpressiveCurveCount()) {
                note.getExpressiveCurve(m_curveIndex) = m_newCurve;
            }
        }

        m_canvas->update();
        qDebug() << "Redo: Curve edited";
    }
}

bool EditCurveCommand::mergeWith(const QUndoCommand *other)
{
    // Merge consecutive curve edits for the same note and curve type
    if (other->id() != id()) {
        return false;
    }

    const EditCurveCommand *editCommand = static_cast<const EditCurveCommand*>(other);
    if (editCommand->m_noteIndex != m_noteIndex || editCommand->m_curveType != m_curveType) {
        return false;
    }
    if (m_curveType == ExpressiveCurveN && editCommand->m_curveIndex != m_curveIndex) {
        return false;
    }

    // Update the new curve to the latest
    m_newCurve = editCommand->m_newCurve;

    return true;
}

// ============================================================================
// Delete Multiple Notes Command
// ============================================================================

DeleteMultipleNotesCommand::DeleteMultipleNotesCommand(Phrase *phrase, const QVector<int> &noteIndices, ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_canvas(canvas)
{
    // Store notes with their indices before deletion (in descending order for proper restoration)
    QVector<int> sortedIndices = noteIndices;
    std::sort(sortedIndices.begin(), sortedIndices.end(), std::greater<int>());

    const QVector<Note> &notes = m_phrase->getNotes();
    for (int index : sortedIndices) {
        if (index >= 0 && index < notes.size()) {
            m_notesWithIndices.append(qMakePair(index, notes[index]));
        }
    }

    if (noteIndices.size() == 1) {
        setText("Delete Note");
    } else {
        setText(QString("Delete %1 Notes").arg(noteIndices.size()));
    }
}

void DeleteMultipleNotesCommand::undo()
{
    // Re-insert notes at their original positions (in reverse order since we stored descending)
    for (int i = m_notesWithIndices.size() - 1; i >= 0; --i) {
        int index = m_notesWithIndices[i].first;
        const Note &note = m_notesWithIndices[i].second;

        QVector<Note> &notes = m_phrase->getNotes();
        notes.insert(index, note);
    }

    m_canvas->update();
    qDebug() << "Undo: Restored" << m_notesWithIndices.size() << "notes";
}

void DeleteMultipleNotesCommand::redo()
{
    // Delete notes in descending order to avoid index shifting
    for (const auto &pair : m_notesWithIndices) {
        int index = pair.first;
        m_phrase->removeNoteByIndex(index);
    }

    m_canvas->update();
    qDebug() << "Redo: Deleted" << m_notesWithIndices.size() << "notes";
}

// ============================================================================
// Move Multiple Notes Command
// ============================================================================

MoveMultipleNotesCommand::MoveMultipleNotesCommand(Phrase *phrase, const QVector<int> &noteIndices,
                                                   const QVector<NoteState> &oldStates,
                                                   const QVector<NoteState> &newStates,
                                                   ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_oldStates(oldStates)
    , m_newStates(newStates)
    , m_canvas(canvas)
{
    if (noteIndices.size() == 1) {
        setText("Move Note");
    } else {
        setText(QString("Move %1 Notes").arg(noteIndices.size()));
    }
}

void MoveMultipleNotesCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    // Restore old states
    for (const NoteState &state : m_oldStates) {
        if (state.index >= 0 && state.index < notes.size()) {
            Note &note = notes[state.index];
            note.setStartTime(state.startTime);

            if (state.hasPitchCurve) {
                note.setPitchCurve(state.pitchCurve);
                if (note.isQuantized()) note.detectSegments();
            } else {
                note.setPitchHz(state.pitch);
            }
        }
    }

    m_canvas->update();
    qDebug() << "Undo: Restored" << m_oldStates.size() << "notes to original positions";
}

void MoveMultipleNotesCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    // Apply new states
    for (const NoteState &state : m_newStates) {
        if (state.index >= 0 && state.index < notes.size()) {
            Note &note = notes[state.index];
            note.setStartTime(state.startTime);

            if (state.hasPitchCurve) {
                note.setPitchCurve(state.pitchCurve);
                if (note.isQuantized()) note.detectSegments();
            } else {
                note.setPitchHz(state.pitch);
            }
        }
    }

    m_canvas->update();
    qDebug() << "Redo: Moved" << m_newStates.size() << "notes";
}

// ============================================================================
// Resize Multiple Notes Command
// ============================================================================

ResizeMultipleNotesCommand::ResizeMultipleNotesCommand(Phrase *phrase,
                                                       const QVector<NoteState> &oldStates,
                                                       const QVector<NoteState> &newStates,
                                                       ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_oldStates(oldStates)
    , m_newStates(newStates)
    , m_canvas(canvas)
{
    setText("Resize Multiple Notes");
}

void ResizeMultipleNotesCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    for (const NoteState &state : m_oldStates) {
        if (state.index >= 0 && state.index < notes.size()) {
            Note &note = notes[state.index];
            note.setStartTime(state.startTime);
            note.setDuration(state.duration);
        }
    }

    m_canvas->update();
    qDebug() << "Undo: Restored" << m_oldStates.size() << "notes to original sizes";
}

void ResizeMultipleNotesCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    for (const NoteState &state : m_newStates) {
        if (state.index >= 0 && state.index < notes.size()) {
            Note &note = notes[state.index];
            note.setStartTime(state.startTime);
            note.setDuration(state.duration);
        }
    }

    m_canvas->update();
    qDebug() << "Redo: Resized" << m_newStates.size() << "notes";
}

// ============================================================================
// Apply Dynamics Curve Command
// ============================================================================

ApplyDynamicsCurveCommand::ApplyDynamicsCurveCommand(Phrase *phrase,
                                                       const QVector<int> &noteIndices,
                                                       const QVector<EnvelopePoint> &curve,
                                                       double weight,
                                                       bool perNote,
                                                       ScoreCanvas *canvas,
                                                       QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndices(noteIndices)
    , m_curve(curve)
    , m_weight(weight)
    , m_perNote(perNote)
    , m_canvas(canvas)
{
    setText("Apply Dynamics Curve");

    // Store original dynamics curves and envelope control points for undo
    QVector<Note> &notes = m_phrase->getNotes();
    for (int idx : m_noteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            m_oldDynamicsCurves.append(notes[idx].getDynamicsCurve());
            m_oldEnvelopeControlPoints.append(
                notes[idx].getEnvelopeControlPoints(QStringLiteral("Dynamics")));
        }
    }
}

void ApplyDynamicsCurveCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    // Restore original dynamics curves and envelope control points
    for (int i = 0; i < m_noteIndices.size() && i < m_oldDynamicsCurves.size(); ++i) {
        int idx = m_noteIndices[i];
        if (idx >= 0 && idx < notes.size()) {
            notes[idx].setDynamicsCurve(m_oldDynamicsCurves[i]);
            if (i < m_oldEnvelopeControlPoints.size() && !m_oldEnvelopeControlPoints[i].isEmpty())
                notes[idx].setEnvelopeControlPoints(QStringLiteral("Dynamics"),
                                                     m_oldEnvelopeControlPoints[i]);
            else
                notes[idx].removeEnvelopeControlPoints(QStringLiteral("Dynamics"));
            notes[idx].setRenderDirty(true);
        }
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Undo: Restored original dynamics curves for" << m_noteIndices.size() << "notes";
}

void ApplyDynamicsCurveCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    if (m_noteIndices.isEmpty() || m_curve.isEmpty()) return;

    // Evaluate the applied curve at normalized time t ∈ [0, 1].
    // Matches EnvelopeCurveCanvas::evaluateCurve exactly:
    //   curveType 0 = linear, 1 = cosine smooth, 2 = step
    auto evaluateCurve = [this](double t) -> double {
        if (m_curve.isEmpty()) return 1.0;
        if (m_curve.size() == 1) return m_curve[0].value;
        if (t <= m_curve.first().time) return m_curve.first().value;
        if (t >= m_curve.last().time)  return m_curve.last().value;
        for (int i = 0; i < m_curve.size() - 1; ++i) {
            if (t >= m_curve[i].time && t <= m_curve[i + 1].time) {
                double segT = (t - m_curve[i].time) / (m_curve[i + 1].time - m_curve[i].time);
                if (m_curve[i].curveType == 1) {
                    // Smooth – cosine interpolation (same as EnvelopeCurveCanvas)
                    double smoothT = (1.0 - std::cos(segT * M_PI)) * 0.5;
                    return m_curve[i].value + smoothT * (m_curve[i + 1].value - m_curve[i].value);
                } else if (m_curve[i].curveType == 2) {
                    // Step – hold value until next point
                    return m_curve[i].value;
                } else {
                    // Linear (default)
                    return m_curve[i].value + segT * (m_curve[i + 1].value - m_curve[i].value);
                }
            }
        }
        return m_curve.last().value;
    };

    // Helper: detect flat curves (e.g., from MIDI velocity) and compute baseline.
    // Used to scale applied curve proportionally to preserve relative loudness.
    auto curveBaselineIfFlat = [](const Curve &curve, double threshold = 0.05) -> std::pair<bool, double> {
        if (curve.isEmpty()) return {true, 0.5};  // Empty curve defaults to 0.5
        const int SAMPLE_COUNT = 5;
        double minVal = 1.0;
        double maxVal = 0.0;
        double sum = 0.0;
        for (int i = 0; i <= SAMPLE_COUNT; ++i) {
            double t = static_cast<double>(i) / SAMPLE_COUNT;
            double val = curve.valueAt(t);
            minVal = qMin(minVal, val);
            maxVal = qMax(maxVal, val);
            sum += val;
        }
        bool flat = (maxVal - minVal) <= threshold;
        return {flat, sum / (SAMPLE_COUNT + 1)};
    };

    if (m_perNote) {
        // Per-note mode: the chosen curve becomes each note's dynamics shape.
        // Sample at 20 evenly-spaced points so that smooth (cosine) segments in
        // the applied curve are faithfully captured — the note's Curve only does
        // linear interpolation between stored points, so enough density is needed.
        // At weight=1 the note gets exactly the smooth curve values.
        const int SAMPLE_COUNT = 20;
        for (int idx : m_noteIndices) {
            if (idx < 0 || idx >= notes.size()) continue;
            Note &note = notes[idx];
            const Curve &existing = note.getDynamicsCurve();

            // Detect flat curves (e.g., MIDI velocity) to scale envelope proportionally
            auto flatResult = curveBaselineIfFlat(existing);
            bool isFlat = flatResult.first;
            double baseline = flatResult.second;
            double scale = baseline;  // Multiply envelope by baseline when flat

            Curve newCurve;
            for (int s = 0; s <= SAMPLE_COUNT; ++s) {
                double t = static_cast<double>(s) / SAMPLE_COUNT;
                double curveVal   = evaluateCurve(t);
                double existingVal = existing.isEmpty() ? 0.5 : existing.valueAt(t);

                // Scale curve value if existing curve is flat
                double scaledCurveVal = isFlat ? (scale * curveVal) : curveVal;
                double newVal = existingVal + (scaledCurveVal - existingVal) * m_weight;
                newCurve.addPoint(t, qBound(0.0, newVal, 1.0), 1.0);
            }

            note.setDynamicsCurve(newCurve);
            note.setEnvelopeControlPoints(QStringLiteral("Dynamics"), m_curve);
            note.setRenderDirty(true);
        }
    } else if (m_noteIndices.size() == 1) {
        // Single note in selection mode: apply the curve across the note's own duration.
        // The curve value is the target dynamics level (top of canvas = 1.0, bottom = 0.0).
        // Blending formula: newVal = existing + (target - existing) * weight
        // At weight=1.0 this is idempotent — applying the same curve again yields no change.
        const int SAMPLE_COUNT = 20;
        int idx = m_noteIndices[0];
        if (idx >= 0 && idx < notes.size()) {
            Note &note = notes[idx];
            const Curve &existingCurve = note.getDynamicsCurve();

            // Detect flat curves (e.g., MIDI velocity) to scale envelope proportionally
            auto flatResult = curveBaselineIfFlat(existingCurve);
            bool isFlat = flatResult.first;
            double baseline = flatResult.second;
            double scale = baseline;  // Multiply envelope by baseline when flat

            Curve newCurve;
            for (int s = 0; s <= SAMPLE_COUNT; ++s) {
                double t  = static_cast<double>(s) / SAMPLE_COUNT;
                double cv = evaluateCurve(t);
                double existingVal = existingCurve.isEmpty() ? 0.5 : existingCurve.valueAt(t);

                // Scale curve value if existing curve is flat
                double scaledCurveVal = isFlat ? (scale * cv) : cv;
                double newVal = existingVal + (scaledCurveVal - existingVal) * m_weight;
                newCurve.addPoint(t, qBound(0.0, newVal, 1.0), 1.0);
            }
            note.setDynamicsCurve(newCurve);
            note.setEnvelopeControlPoints(QStringLiteral("Dynamics"), m_curve);
            note.setRenderDirty(true);
        }
    } else {
        // Multi-note selection mode: curve spans the whole selection; each note gets
        // a target level determined by its center's position in the selection.
        // The curve value is absolute (top = 1.0, bottom = 0.0); applying the same
        // curve repeatedly yields no change at weight=1.0 (idempotent).
        double startTime = std::numeric_limits<double>::max();
        double endTime   = std::numeric_limits<double>::lowest();

        for (int idx : m_noteIndices) {
            if (idx >= 0 && idx < notes.size()) {
                startTime = qMin(startTime, notes[idx].getStartTime());
                endTime   = qMax(endTime,   notes[idx].getStartTime() + notes[idx].getDuration());
            }
        }

        double selectionDuration = endTime - startTime;
        if (selectionDuration <= 0) return;

        for (int idx : m_noteIndices) {
            if (idx < 0 || idx >= notes.size()) continue;
            Note &note = notes[idx];

            double noteCenter    = note.getStartTime() + note.getDuration() / 2.0;
            double normalizedPos = (noteCenter - startTime) / selectionDuration;
            double curveValue    = evaluateCurve(normalizedPos);

            Curve dynamicsCurve = note.getDynamicsCurve();
            const QVector<Curve::Point> &points = dynamicsCurve.getPoints();
            Curve newCurve;

            // Scale factor: lerp from 1.0 (no change) to curveValue as weight goes 0→1.
            // This multiplies the note's existing dynamics proportionally, preserving
            // the internal shape while modulating the overall level across the selection.
            double scale = 1.0 + m_weight * (curveValue - 1.0);

            if (points.isEmpty()) {
                double existingDefault = 0.7;
                double defaultValue = qBound(0.0, existingDefault * scale, 1.0);
                newCurve.addPoint(0.0, defaultValue, 1.0);
                newCurve.addPoint(1.0, defaultValue, 1.0);
            } else {
                for (const Curve::Point &pt : points) {
                    double newVal = pt.value * scale;
                    newCurve.addPoint(pt.time, qBound(0.0, newVal, 1.0), pt.pressure);
                }
            }

            note.setDynamicsCurve(newCurve);
            note.setRenderDirty(true);
        }
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Redo: Applied dynamics curve (weight=" << m_weight
             << ", perNote=" << m_perNote << ") to" << m_noteIndices.size() << "notes";
}

// ============================================================================
// Paste Notes Command
// ============================================================================

PasteNotesCommand::PasteNotesCommand(Phrase *phrase, const QVector<Note> &notes,
                                   double targetTime, int targetTrackIndex,
                                   ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_notes(notes)
    , m_targetTime(targetTime)
    , m_targetTrackIndex(targetTrackIndex)
    , m_canvas(canvas)
    , m_firstTime(true)
{
    setText("Paste Notes");
}

void PasteNotesCommand::undo()
{
    // Remove the pasted notes (in reverse order to maintain indices)
    QVector<Note> &notes = m_phrase->getNotes();
    for (int i = m_pastedIndices.size() - 1; i >= 0; --i) {
        int index = m_pastedIndices[i];
        if (index >= 0 && index < notes.size()) {
            notes.removeAt(index);
        }
    }
    m_canvas->update();
    qDebug() << "Undo: Pasted notes removed";
}

void PasteNotesCommand::redo()
{
    if (m_notes.isEmpty()) return;

    // Calculate time offset: earliest note should start at targetTime
    double minStartTime = m_notes[0].getStartTime();
    for (const Note &note : m_notes) {
        if (note.getStartTime() < minStartTime) {
            minStartTime = note.getStartTime();
        }
    }
    double timeOffset = m_targetTime - minStartTime;

    // Add notes with time offset
    QVector<Note> &phraseNotes = m_phrase->getNotes();
    m_pastedIndices.clear();

    for (const Note &note : m_notes) {
        Note pastedNote = note;
        pastedNote.regenerateId();  // Generate new unique ID to avoid sharing state with original
        pastedNote.setStartTime(note.getStartTime() + timeOffset);
        // Always assign to the active track at paste time — this is what makes
        // cross-track paste work.  Variation indices are per-track, so only
        // reset to base on a cross-track paste (where the source's index may
        // not exist on the destination); within-track paste keeps it.
        if (note.getTrackIndex() != m_targetTrackIndex) {
            pastedNote.setVariationIndex(0);
        }
        pastedNote.setTrackIndex(m_targetTrackIndex);
        pastedNote.setRenderDirty(true);

        // Add note and track its index
        int insertIndex = phraseNotes.size();
        m_phrase->addNote(pastedNote);
        m_pastedIndices.append(insertIndex);
    }

    m_canvas->update();
    qDebug() << "Redo: Pasted" << m_notes.size() << "notes at time" << m_targetTime;
}

// ============================================================================
// Apply Rhythmic Easing Command
// ============================================================================

ApplyRhythmicEasingCommand::ApplyRhythmicEasingCommand(Phrase *phrase,
                                                       const QVector<int> &noteIndices,
                                                       const Easing &easing,
                                                       AnchorMode anchorMode,
                                                       double weight,
                                                       ScoreCanvas *canvas,
                                                       QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_easing(easing)
    , m_anchorMode(anchorMode)
    , m_weight(weight)
    , m_canvas(canvas)
{
    setText(QString("Apply %1 Easing").arg(easing.getName()));
    calculateNewTimes(noteIndices);
}

void ApplyRhythmicEasingCommand::calculateNewTimes(const QVector<int> &noteIndices)
{
    const QVector<Note> &notes = m_phrase->getNotes();

    // Gather valid indices and find time window
    // Window spans from earliest note start to latest note END (start + duration)
    QVector<int> validIndices;
    double windowStart = std::numeric_limits<double>::max();
    double windowEnd = std::numeric_limits<double>::lowest();
    double firstNoteStart = std::numeric_limits<double>::max();
    double lastNoteStart = std::numeric_limits<double>::lowest();

    for (int idx : noteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            validIndices.append(idx);
            double startTime = notes[idx].getStartTime();
            double endTime = startTime + notes[idx].getDuration();
            windowStart = qMin(windowStart, startTime);
            windowEnd = qMax(windowEnd, endTime);
            firstNoteStart = qMin(firstNoteStart, startTime);
            lastNoteStart = qMax(lastNoteStart, startTime);
        }
    }

    // Need at least 2 notes
    if (validIndices.size() < 2) {
        return;
    }

    double windowSpan = windowEnd - windowStart;
    if (windowSpan <= 0) {
        return;
    }

    // Sort indices by start time for index-based positioning
    std::sort(validIndices.begin(), validIndices.end(), [&notes](int a, int b) {
        return notes[a].getStartTime() < notes[b].getStartTime();
    });

    int N = validIndices.size();

    // Calculate old and new times for each note based on anchor mode
    for (int i = 0; i < N; ++i) {
        int idx = validIndices[i];
        double oldTime = notes[idx].getStartTime();
        double t;
        double easedT;
        double newTime;

        switch (m_anchorMode) {
        case AnchorNone:
            // Use index-based positioning: t = (i + 1) / (N + 1)
            // This places all notes in the interior of (0, 1), so all can move
            t = static_cast<double>(i + 1) / static_cast<double>(N + 1);
            easedT = m_easing.calculate(t);
            newTime = windowStart + easedT * windowSpan;
            break;

        case AnchorFirst:
            // First note at t=0 stays fixed, others redistribute
            // Use time-based positioning within [firstNoteStart, windowEnd]
            if (qFuzzyCompare(oldTime, firstNoteStart)) {
                newTime = oldTime;  // First note stays put
            } else {
                t = (oldTime - firstNoteStart) / (windowEnd - firstNoteStart);
                easedT = m_easing.calculate(t);
                newTime = firstNoteStart + easedT * (windowEnd - firstNoteStart);
            }
            break;

        case AnchorLast:
            // Last note stays fixed, others redistribute
            // Use time-based positioning within [windowStart, lastNoteStart]
            if (qFuzzyCompare(oldTime, lastNoteStart)) {
                newTime = oldTime;  // Last note stays put
            } else {
                double span = lastNoteStart - windowStart;
                if (span > 0) {
                    t = (oldTime - windowStart) / span;
                    easedT = m_easing.calculate(t);
                    newTime = windowStart + easedT * span;
                } else {
                    newTime = oldTime;
                }
            }
            break;

        case AnchorBoth:
            // Both first and last notes stay fixed
            // Use time-based positioning within [firstNoteStart, lastNoteStart]
            if (qFuzzyCompare(oldTime, firstNoteStart) || qFuzzyCompare(oldTime, lastNoteStart)) {
                newTime = oldTime;  // Anchored notes stay put
            } else {
                double span = lastNoteStart - firstNoteStart;
                if (span > 0) {
                    t = (oldTime - firstNoteStart) / span;
                    easedT = m_easing.calculate(t);
                    newTime = firstNoteStart + easedT * span;
                } else {
                    newTime = oldTime;
                }
            }
            break;
        }

        // Apply weight: interpolate between original and eased position
        // weight=0: no change, weight=1: full easing, weight=2: double effect
        double weightedNewTime = oldTime + m_weight * (newTime - oldTime);

        NoteTimeState state;
        state.index = idx;
        state.oldStartTime = oldTime;
        state.newStartTime = weightedNewTime;
        m_noteStates.append(state);
    }
}

void ApplyRhythmicEasingCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    for (const NoteTimeState &state : m_noteStates) {
        if (state.index >= 0 && state.index < notes.size()) {
            notes[state.index].setStartTime(state.oldStartTime);
            notes[state.index].setRenderDirty(true);
        }
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Undo: Restored" << m_noteStates.size() << "notes to original times";
}

void ApplyRhythmicEasingCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    for (const NoteTimeState &state : m_noteStates) {
        if (state.index >= 0 && state.index < notes.size()) {
            notes[state.index].setStartTime(state.newStartTime);
            notes[state.index].setRenderDirty(true);
        }
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Redo: Applied" << m_easing.getName() << "easing to" << m_noteStates.size() << "notes";
}

// ============================================================================
// Split Note At Segment Command
// ============================================================================

SplitNoteAtSegmentCommand::SplitNoteAtSegmentCommand(Phrase *phrase, int noteIndex,
                                                       int segmentIndex, ScoreCanvas *canvas,
                                                       QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndex(noteIndex)
    , m_segmentIndex(segmentIndex)
    , m_hasBefore(false)
    , m_hasAfter(false)
    , m_canvas(canvas)
{
    setText("Delete Segment");

    const QVector<Note> &notes = m_phrase->getNotes();
    if (noteIndex < 0 || noteIndex >= notes.size()) return;

    m_originalNote = notes[noteIndex];
    const auto &segments = m_originalNote.getSegments();

    if (segmentIndex < 0 || segmentIndex >= segments.size()) return;

    // Determine if we have segments before and after
    m_hasBefore = (segmentIndex > 0);
    m_hasAfter = (segmentIndex < segments.size() - 1);

    const Segment &deletedSeg = segments[segmentIndex];

    // Create note for segments before the deleted one
    if (m_hasBefore) {
        m_noteBefore = m_originalNote;
        m_noteBefore.regenerateId();

        // Calculate duration to end at the deleted segment's start
        double beforeEndTime = deletedSeg.startTime;
        double newDuration = m_originalNote.getDuration() * beforeEndTime;
        m_noteBefore.setDuration(newDuration);

        // Rebuild pitch curve for the "before" portion
        if (m_originalNote.hasPitchCurve()) {
            Curve newPitchCurve;
            const auto &originalPoints = m_originalNote.getPitchCurve().getPoints();
            for (const auto &point : originalPoints) {
                if (point.time <= beforeEndTime) {
                    // Rescale time to 0.0-1.0 range for the new duration
                    double rescaledTime = point.time / beforeEndTime;
                    newPitchCurve.addPoint(rescaledTime, point.value);
                }
            }
            // Ensure curve ends at 1.0
            if (!newPitchCurve.getPoints().isEmpty()) {
                double lastTime = newPitchCurve.getPoints().last().time;
                if (lastTime < 1.0) {
                    double lastValue = m_originalNote.getPitchCurve().valueAt(beforeEndTime);
                    newPitchCurve.addPoint(1.0, lastValue);
                }
            }
            m_noteBefore.setPitchCurve(newPitchCurve);
        }

        // Rebuild dynamics curve similarly
        Curve newDynamicsCurve;
        const auto &dynPoints = m_originalNote.getDynamicsCurve().getPoints();
        for (const auto &point : dynPoints) {
            if (point.time <= beforeEndTime) {
                double rescaledTime = point.time / beforeEndTime;
                newDynamicsCurve.addPoint(rescaledTime, point.value);
            }
        }
        if (!newDynamicsCurve.getPoints().isEmpty()) {
            double lastTime = newDynamicsCurve.getPoints().last().time;
            if (lastTime < 1.0) {
                double lastValue = m_originalNote.getDynamicsCurve().valueAt(beforeEndTime);
                newDynamicsCurve.addPoint(1.0, lastValue);
            }
        }
        m_noteBefore.setDynamicsCurve(newDynamicsCurve);

        // Clear and rebuild segments for the before note
        m_noteBefore.clearSegments();
        m_noteBefore.detectSegments();
    }

    // Create note for segments after the deleted one
    if (m_hasAfter) {
        m_noteAfter = m_originalNote;
        m_noteAfter.regenerateId();

        // Calculate new start time and duration
        double afterStartTime = deletedSeg.endTime;
        double newStartTimeMs = m_originalNote.getStartTime() + m_originalNote.getDuration() * afterStartTime;
        double newDuration = m_originalNote.getDuration() * (1.0 - afterStartTime);

        m_noteAfter.setStartTime(newStartTimeMs);
        m_noteAfter.setDuration(newDuration);

        // Rebuild pitch curve for the "after" portion
        if (m_originalNote.hasPitchCurve()) {
            Curve newPitchCurve;
            const auto &originalPoints = m_originalNote.getPitchCurve().getPoints();
            for (const auto &point : originalPoints) {
                if (point.time >= afterStartTime) {
                    // Rescale time to 0.0-1.0 range for the new duration
                    double rescaledTime = (point.time - afterStartTime) / (1.0 - afterStartTime);
                    newPitchCurve.addPoint(rescaledTime, point.value);
                }
            }
            // Ensure curve starts at 0.0
            if (newPitchCurve.getPoints().isEmpty() || newPitchCurve.getPoints().first().time > 0.0) {
                double firstValue = m_originalNote.getPitchCurve().valueAt(afterStartTime);
                Curve tempCurve;
                tempCurve.addPoint(0.0, firstValue);
                for (const auto &pt : newPitchCurve.getPoints()) {
                    tempCurve.addPoint(pt.time, pt.value);
                }
                newPitchCurve = tempCurve;
            }
            m_noteAfter.setPitchCurve(newPitchCurve);
        }

        // Rebuild dynamics curve similarly
        Curve newDynamicsCurve;
        const auto &dynPoints = m_originalNote.getDynamicsCurve().getPoints();
        for (const auto &point : dynPoints) {
            if (point.time >= afterStartTime) {
                double rescaledTime = (point.time - afterStartTime) / (1.0 - afterStartTime);
                newDynamicsCurve.addPoint(rescaledTime, point.value);
            }
        }
        if (newDynamicsCurve.getPoints().isEmpty() || newDynamicsCurve.getPoints().first().time > 0.0) {
            double firstValue = m_originalNote.getDynamicsCurve().valueAt(afterStartTime);
            Curve tempCurve;
            tempCurve.addPoint(0.0, firstValue);
            for (const auto &pt : newDynamicsCurve.getPoints()) {
                tempCurve.addPoint(pt.time, pt.value);
            }
            newDynamicsCurve = tempCurve;
        }
        m_noteAfter.setDynamicsCurve(newDynamicsCurve);

        // Clear and rebuild segments for the after note
        m_noteAfter.clearSegments();
        m_noteAfter.detectSegments();
    }
}

void SplitNoteAtSegmentCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    // Remove the split notes (in reverse order of insertion)
    int removeCount = 0;
    if (m_hasAfter) removeCount++;
    if (m_hasBefore) removeCount++;

    // Find and remove the split notes by searching from the original position
    // The notes should be near m_noteIndex
    for (int i = 0; i < removeCount && m_noteIndex < notes.size(); ++i) {
        notes.removeAt(m_noteIndex);
    }

    // Re-insert the original note
    notes.insert(m_noteIndex, m_originalNote);

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Undo: Restored original note from split";
}

void SplitNoteAtSegmentCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    // Remove the original note
    if (m_noteIndex >= 0 && m_noteIndex < notes.size()) {
        notes.removeAt(m_noteIndex);
    }

    // Insert the split notes (before first, then after)
    if (m_hasBefore) {
        notes.insert(m_noteIndex, m_noteBefore);
    }
    if (m_hasAfter) {
        int insertIndex = m_hasBefore ? m_noteIndex + 1 : m_noteIndex;
        notes.insert(insertIndex, m_noteAfter);
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Redo: Split note at segment" << m_segmentIndex;
}

// ============================================================================
// Change Segment Pitch Command
// ============================================================================

ChangeSegmentPitchCommand::ChangeSegmentPitchCommand(Phrase *phrase, int noteIndex,
                                                       int segmentIndex, double newPitchHz,
                                                       ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndex(noteIndex)
    , m_segmentIndex(segmentIndex)
    , m_newPitchHz(newPitchHz)
    , m_canvas(canvas)
{
    setText("Change Segment Pitch");

    const QVector<Note> &notes = m_phrase->getNotes();
    if (noteIndex < 0 || noteIndex >= notes.size()) return;

    const Note &note = notes[noteIndex];
    const auto &segments = note.getSegments();

    if (segmentIndex < 0 || segmentIndex >= segments.size()) return;

    m_oldPitchHz = segments[segmentIndex].pitchHz;
    m_oldPitchCurve = note.getPitchCurve();

    // Build the new pitch curve with the changed segment pitch
    m_newPitchCurve = m_oldPitchCurve;
    const Segment &seg = segments[segmentIndex];

    // Update pitch curve points within this segment
    Curve updatedCurve;
    const auto &points = m_oldPitchCurve.getPoints();
    for (const auto &point : points) {
        double t = point.time;
        double pitch = point.value;

        // If this point is within the segment being edited, use the new pitch
        if (t >= seg.startTime && t <= seg.endTime) {
            // Check if this point was at the old segment pitch
            if (std::abs(pitch - m_oldPitchHz) < 1.0) {
                pitch = m_newPitchHz;
            }
        }

        updatedCurve.addPoint(t, pitch);
    }
    m_newPitchCurve = updatedCurve;
}

void ChangeSegmentPitchCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    if (m_noteIndex >= 0 && m_noteIndex < notes.size()) {
        Note &note = notes[m_noteIndex];
        note.setPitchCurve(m_oldPitchCurve);
        note.setRenderDirty(true);

        // Re-detect segments
        note.detectSegments();

        m_canvas->update();
        emit m_canvas->notesChanged();
        qDebug() << "Undo: Restored segment pitch to" << m_oldPitchHz << "Hz";
    }
}

void ChangeSegmentPitchCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    if (m_noteIndex >= 0 && m_noteIndex < notes.size()) {
        Note &note = notes[m_noteIndex];
        note.setPitchCurve(m_newPitchCurve);
        note.setRenderDirty(true);

        // Re-detect segments
        note.detectSegments();

        m_canvas->update();
        emit m_canvas->notesChanged();
        qDebug() << "Redo: Changed segment pitch to" << m_newPitchHz << "Hz";
    }
}

// ============================================================================
// Detach Segment Command
// ============================================================================

DetachSegmentCommand::DetachSegmentCommand(Phrase *phrase, int noteIndex,
                                             int segmentIndex, ScoreCanvas *canvas,
                                             QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndex(noteIndex)
    , m_segmentIndex(segmentIndex)
    , m_hasBefore(false)
    , m_hasAfter(false)
    , m_canvas(canvas)
{
    setText("Detach Segment");

    const QVector<Note> &notes = m_phrase->getNotes();
    if (noteIndex < 0 || noteIndex >= notes.size()) return;

    m_originalNote = notes[noteIndex];
    const auto &segments = m_originalNote.getSegments();

    if (segmentIndex < 0 || segmentIndex >= segments.size()) return;

    // Determine if we have segments before and after
    m_hasBefore = (segmentIndex > 0);
    m_hasAfter = (segmentIndex < segments.size() - 1);

    const Segment &detachedSeg = segments[segmentIndex];

    // Create the detached discrete note
    double segStartMs = m_originalNote.getStartTime() + detachedSeg.startTime * m_originalNote.getDuration();
    double segDuration = (detachedSeg.endTime - detachedSeg.startTime) * m_originalNote.getDuration();

    m_detachedNote = Note(segStartMs, segDuration, detachedSeg.pitchHz);
    m_detachedNote.setTrackIndex(m_originalNote.getTrackIndex());
    m_detachedNote.setQuantized(false);  // It's now a discrete note
    // No pitch curve - it's a simple discrete note

    // Copy dynamics for the detached segment's time range
    Curve detachedDynamics;
    const auto &origDynPoints = m_originalNote.getDynamicsCurve().getPoints();
    for (const auto &point : origDynPoints) {
        if (point.time >= detachedSeg.startTime && point.time <= detachedSeg.endTime) {
            // Rescale time to 0.0-1.0 for the detached note
            double rescaledTime = (point.time - detachedSeg.startTime) / (detachedSeg.endTime - detachedSeg.startTime);
            detachedDynamics.addPoint(rescaledTime, point.value);
        }
    }
    // Ensure we have start and end points
    if (detachedDynamics.getPoints().isEmpty() || detachedDynamics.getPoints().first().time > 0.0) {
        double startVal = m_originalNote.getDynamicsCurve().valueAt(detachedSeg.startTime);
        Curve temp;
        temp.addPoint(0.0, startVal);
        for (const auto &pt : detachedDynamics.getPoints()) temp.addPoint(pt.time, pt.value);
        detachedDynamics = temp;
    }
    if (detachedDynamics.getPoints().last().time < 1.0) {
        double endVal = m_originalNote.getDynamicsCurve().valueAt(detachedSeg.endTime);
        detachedDynamics.addPoint(1.0, endVal);
    }
    m_detachedNote.setDynamicsCurve(detachedDynamics);

    // Use the segment's variation if set, otherwise use note's variation
    if (detachedSeg.variationIndex >= 0) {
        m_detachedNote.setVariationIndex(detachedSeg.variationIndex);
    } else {
        m_detachedNote.setVariationIndex(m_originalNote.getVariationIndex());
    }

    // Use the segment's vibrato if not using note's vibrato
    if (!detachedSeg.useNoteVibrato) {
        m_detachedNote.setVibrato(detachedSeg.vibrato);
    } else {
        m_detachedNote.setVibrato(m_originalNote.getVibrato());
    }

    // Create note for segments before the detached one (similar to SplitNoteAtSegmentCommand)
    if (m_hasBefore) {
        m_noteBefore = m_originalNote;
        m_noteBefore.regenerateId();

        double beforeEndTime = detachedSeg.startTime;
        double newDuration = m_originalNote.getDuration() * beforeEndTime;
        m_noteBefore.setDuration(newDuration);

        if (m_originalNote.hasPitchCurve()) {
            Curve newPitchCurve;
            const auto &originalPoints = m_originalNote.getPitchCurve().getPoints();
            for (const auto &point : originalPoints) {
                if (point.time <= beforeEndTime) {
                    double rescaledTime = point.time / beforeEndTime;
                    newPitchCurve.addPoint(rescaledTime, point.value);
                }
            }
            if (!newPitchCurve.getPoints().isEmpty()) {
                double lastTime = newPitchCurve.getPoints().last().time;
                if (lastTime < 1.0) {
                    double lastValue = m_originalNote.getPitchCurve().valueAt(beforeEndTime);
                    newPitchCurve.addPoint(1.0, lastValue);
                }
            }
            m_noteBefore.setPitchCurve(newPitchCurve);
        }

        Curve newDynamicsCurve;
        const auto &dynPoints = m_originalNote.getDynamicsCurve().getPoints();
        for (const auto &point : dynPoints) {
            if (point.time <= beforeEndTime) {
                double rescaledTime = point.time / beforeEndTime;
                newDynamicsCurve.addPoint(rescaledTime, point.value);
            }
        }
        if (!newDynamicsCurve.getPoints().isEmpty()) {
            double lastTime = newDynamicsCurve.getPoints().last().time;
            if (lastTime < 1.0) {
                double lastValue = m_originalNote.getDynamicsCurve().valueAt(beforeEndTime);
                newDynamicsCurve.addPoint(1.0, lastValue);
            }
        }
        m_noteBefore.setDynamicsCurve(newDynamicsCurve);

        m_noteBefore.clearSegments();
        m_noteBefore.detectSegments();
    }

    // Create note for segments after the detached one
    if (m_hasAfter) {
        m_noteAfter = m_originalNote;
        m_noteAfter.regenerateId();

        double afterStartTime = detachedSeg.endTime;
        double newStartTimeMs = m_originalNote.getStartTime() + m_originalNote.getDuration() * afterStartTime;
        double newDuration = m_originalNote.getDuration() * (1.0 - afterStartTime);

        m_noteAfter.setStartTime(newStartTimeMs);
        m_noteAfter.setDuration(newDuration);

        if (m_originalNote.hasPitchCurve()) {
            Curve newPitchCurve;
            const auto &originalPoints = m_originalNote.getPitchCurve().getPoints();
            for (const auto &point : originalPoints) {
                if (point.time >= afterStartTime) {
                    double rescaledTime = (point.time - afterStartTime) / (1.0 - afterStartTime);
                    newPitchCurve.addPoint(rescaledTime, point.value);
                }
            }
            if (newPitchCurve.getPoints().isEmpty() || newPitchCurve.getPoints().first().time > 0.0) {
                double firstValue = m_originalNote.getPitchCurve().valueAt(afterStartTime);
                Curve tempCurve;
                tempCurve.addPoint(0.0, firstValue);
                for (const auto &pt : newPitchCurve.getPoints()) {
                    tempCurve.addPoint(pt.time, pt.value);
                }
                newPitchCurve = tempCurve;
            }
            m_noteAfter.setPitchCurve(newPitchCurve);
        }

        Curve newDynamicsCurve;
        const auto &dynPoints = m_originalNote.getDynamicsCurve().getPoints();
        for (const auto &point : dynPoints) {
            if (point.time >= afterStartTime) {
                double rescaledTime = (point.time - afterStartTime) / (1.0 - afterStartTime);
                newDynamicsCurve.addPoint(rescaledTime, point.value);
            }
        }
        if (newDynamicsCurve.getPoints().isEmpty() || newDynamicsCurve.getPoints().first().time > 0.0) {
            double firstValue = m_originalNote.getDynamicsCurve().valueAt(afterStartTime);
            Curve tempCurve;
            tempCurve.addPoint(0.0, firstValue);
            for (const auto &pt : newDynamicsCurve.getPoints()) {
                tempCurve.addPoint(pt.time, pt.value);
            }
            newDynamicsCurve = tempCurve;
        }
        m_noteAfter.setDynamicsCurve(newDynamicsCurve);

        m_noteAfter.clearSegments();
        m_noteAfter.detectSegments();
    }
}

void DetachSegmentCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    // Count how many notes we inserted
    int removeCount = 1;  // Always have the detached note
    if (m_hasBefore) removeCount++;
    if (m_hasAfter) removeCount++;

    // Remove the notes we created
    for (int i = 0; i < removeCount && m_noteIndex < notes.size(); ++i) {
        notes.removeAt(m_noteIndex);
    }

    // Re-insert the original note
    notes.insert(m_noteIndex, m_originalNote);

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Undo: Restored original note from detach";
}

void DetachSegmentCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    // Remove the original note
    if (m_noteIndex >= 0 && m_noteIndex < notes.size()) {
        notes.removeAt(m_noteIndex);
    }

    // Insert notes in order: before (continuous), detached (discrete), after (continuous)
    int insertIndex = m_noteIndex;

    if (m_hasBefore) {
        notes.insert(insertIndex, m_noteBefore);
        insertIndex++;
    }

    // Always insert the detached discrete note
    notes.insert(insertIndex, m_detachedNote);
    insertIndex++;

    if (m_hasAfter) {
        notes.insert(insertIndex, m_noteAfter);
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Redo: Detached segment" << m_segmentIndex << "as discrete note";
}

// ============================================================================
// Link As Legato Command (merge notes into one continuous note)
// ============================================================================

LinkAsLegatoCommand::LinkAsLegatoCommand(Phrase *phrase, const QVector<int> &noteIndices,
                                           ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_canvas(canvas)
{
    setText("Link as Legato");

    const QVector<Note> &notes = m_phrase->getNotes();

    // Sort selected notes by start time, storing original index and note data
    QVector<QPair<double, int>> sortedIndices;
    for (int idx : noteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            sortedIndices.append(qMakePair(notes[idx].getStartTime(), idx));
        }
    }
    std::sort(sortedIndices.begin(), sortedIndices.end());

    for (const auto &pair : sortedIndices) {
        int idx = pair.second;
        OriginalNote orig;
        orig.index = idx;
        orig.note = notes[idx];
        m_originalNotes.append(orig);
    }

    if (m_originalNotes.size() < 2) return;

    // Build the merged note from the first note as base
    const Note &firstNote = m_originalNotes.first().note;
    const Note &lastNote = m_originalNotes.last().note;

    double totalStart = firstNote.getStartTime();
    double totalEnd = lastNote.getStartTime() + lastNote.getDuration();
    double totalDuration = totalEnd - totalStart;

    m_mergedNote = firstNote;  // Copy first note as base (preserves track, variation, vibrato, id)
    m_mergedNote.setDuration(totalDuration);
    m_mergedNote.setLegato(false);
    m_mergedNote.setLinkedAsLegato(true);

    // If any original note is continuous (has a pitch curve), the merged note must NOT
    // be quantized — otherwise it would snap to scale and lose free pitch movement.
    // For all-discrete merges, quantized=true keeps the stepped scale behavior.
    bool anyContinuous = false;
    for (const auto &orig : m_originalNotes) {
        if (orig.note.hasPitchCurve()) { anyContinuous = true; break; }
    }
    m_mergedNote.setQuantized(!anyContinuous);

    // Build tiling boundaries: each note's region extends to the next note's start
    // so there are no gaps (which would cause pitch interpolation / portamento)
    QVector<double> regionStart(m_originalNotes.size());
    QVector<double> regionEnd(m_originalNotes.size());
    for (int i = 0; i < m_originalNotes.size(); ++i) {
        regionStart[i] = m_originalNotes[i].note.getStartTime();
        if (i < m_originalNotes.size() - 1) {
            // Extend to meet the next note's start
            regionEnd[i] = m_originalNotes[i + 1].note.getStartTime();
        } else {
            // Last note keeps its own end
            regionEnd[i] = m_originalNotes[i].note.getStartTime()
                         + m_originalNotes[i].note.getDuration();
        }
    }

    // Build pitch curve: remap continuous notes' full curves; flat steps for discrete notes
    Curve pitchCurve;
    for (int i = 0; i < m_originalNotes.size(); ++i) {
        double normStart = (regionStart[i] - totalStart) / totalDuration;
        double normEnd   = (regionEnd[i]   - totalStart) / totalDuration;
        double normSpan  = normEnd - normStart;

        const Note &srcNote = m_originalNotes[i].note;
        const auto &srcPoints = srcNote.getPitchCurve().getPoints();

        if (srcNote.hasPitchCurve() && !srcPoints.isEmpty()) {
            // Continuous note: remap its full pitch curve into the merged note's time space
            for (const auto &pt : srcPoints) {
                pitchCurve.addPoint(normStart + pt.time * normSpan, pt.value, pt.pressure);
            }
        } else {
            // Discrete note (or continuous with empty curve): flat step at midpoint pitch
            double pitch = srcNote.getPitchAt(0.5);
            pitchCurve.addPoint(normStart, pitch);
            pitchCurve.addPoint(normEnd, pitch);
        }
    }
    m_mergedNote.setPitchCurve(pitchCurve);
    m_mergedNote.setPitchHz(firstNote.getPitchAt(0.5));  // Base pitch fallback

    // Build concatenated dynamics curve using the same tiled regions
    Curve dynamicsCurve;
    for (int i = 0; i < m_originalNotes.size(); ++i) {
        double normStart = (regionStart[i] - totalStart) / totalDuration;
        double normEnd   = (regionEnd[i]   - totalStart) / totalDuration;
        double normSpan  = normEnd - normStart;

        const Curve &srcDyn = m_originalNotes[i].note.getDynamicsCurve();
        const auto &srcPoints = srcDyn.getPoints();

        if (srcPoints.isEmpty()) {
            // No dynamics curve — use a flat default (0.7)
            dynamicsCurve.addPoint(normStart, 0.7);
            dynamicsCurve.addPoint(normEnd, 0.7);
        } else {
            // Remap each dynamics point into the merged note's time space
            for (const auto &pt : srcPoints) {
                double remappedTime = normStart + pt.time * normSpan;
                dynamicsCurve.addPoint(remappedTime, pt.value, pt.pressure);
            }
        }
    }
    m_mergedNote.setDynamicsCurve(dynamicsCurve);

    // Build segments
    m_mergedNote.clearSegments();
    if (anyContinuous) {
        // Continuous merge: place one boundary segment per original note so that
        // "Unlink (Staccato)" splits back into the same number of notes, not into
        // dozens of tiny scale-step fragments.
        QVector<Segment> &segs = m_mergedNote.getSegments();
        for (int i = 0; i < m_originalNotes.size(); ++i) {
            double normStart = (regionStart[i] - totalStart) / totalDuration;
            double normEnd   = (regionEnd[i]   - totalStart) / totalDuration;
            Segment seg;
            seg.startTime      = normStart;
            seg.endTime        = normEnd;
            seg.pitchHz        = m_originalNotes[i].note.getPitchAt(0.5);
            seg.variationIndex = m_originalNotes[i].note.getVariationIndex();
            seg.useNoteVibrato = true;
            segs.append(seg);
        }
    } else {
        // Discrete merge: detect scale-stepped segments from the flat pitch curve
        m_mergedNote.detectSegments();
    }

    m_mergedNote.setRenderDirty(true);
}

void LinkAsLegatoCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    if (m_originalNotes.size() < 2) return;

    // Build sorted index list: m_originalNotes is sorted by start time,
    // not by index — we need ascending index order for re-insertion.
    QVector<int> sortedIndices;
    sortedIndices.reserve(m_originalNotes.size());
    for (const auto &orig : m_originalNotes)
        sortedIndices.append(orig.index);
    std::sort(sortedIndices.begin(), sortedIndices.end());

    // Remove the merged note
    int mergedIdx = sortedIndices.first();
    if (mergedIdx >= 0 && mergedIdx < notes.size()) {
        notes.removeAt(mergedIdx);
    }

    // Re-insert all original notes in ascending index order
    for (int idx : sortedIndices) {
        int ins = qMin(idx, notes.size());
        // Find the OriginalNote with this index
        for (const auto &orig : m_originalNotes) {
            if (orig.index == idx) {
                notes.insert(ins, orig.note);
                break;
            }
        }
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Undo: Restored" << m_originalNotes.size() << "notes from legato merge";
}

void LinkAsLegatoCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    if (m_originalNotes.size() < 2) return;

    // m_originalNotes is sorted by start time for the pitch-curve builder.
    // Removal must be in descending INDEX order — otherwise earlier
    // removals shift later indices and we delete the wrong notes.
    QVector<int> sortedIndices;
    sortedIndices.reserve(m_originalNotes.size());
    for (const auto &orig : m_originalNotes)
        sortedIndices.append(orig.index);
    std::sort(sortedIndices.begin(), sortedIndices.end(), std::greater<int>());

    for (int idx : sortedIndices) {
        if (idx >= 0 && idx < notes.size()) {
            notes.removeAt(idx);
        }
    }

    // Insert the merged note at the lowest original index
    int insertIdx = qMin(sortedIndices.last(), notes.size());
    notes.insert(insertIdx, m_mergedNote);

    // Update selection to the merged note only — the old selectedNoteIndices
    // are now stale and would point to unrelated notes that shifted into those slots.
    m_canvas->selectNotes({insertIdx});

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Redo: Merged" << m_originalNotes.size() << "notes into legato";
}

// ============================================================================
// Unlink Legato Command (split merged note back into discrete notes)
// ============================================================================

UnlinkLegatoCommand::UnlinkLegatoCommand(Phrase *phrase, const QVector<int> &noteIndices,
                                           ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_canvas(canvas)
{
    setText("Unlink (Staccato)");

    const QVector<Note> &notes = m_phrase->getNotes();

    // Collect splittable notes (segmented), sorted by descending index for safe removal
    QVector<int> validIndices;
    for (int idx : noteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            const Note &note = notes[idx];
            if ((note.isQuantized() || note.isLinkedAsLegato()) && note.getSegmentCount() > 1) {
                validIndices.append(idx);
            }
        }
    }
    std::sort(validIndices.begin(), validIndices.end(), std::greater<int>());

    // Build split entries
    for (int idx : validIndices) {
        const Note &note = notes[idx];
        const auto &segments = note.getSegments();

        SplitEntry entry;
        entry.originalIndex = idx;
        entry.originalNote = note;

        for (int s = 0; s < segments.size(); ++s) {
            const Segment &seg = segments[s];

            double segStartMs = note.getStartTime() + seg.startTime * note.getDuration();
            double segDuration = (seg.endTime - seg.startTime) * note.getDuration();

            // Apply staccato gap: shorten all notes except the last
            if (s < segments.size() - 1) {
                double proportionalGap = segDuration * 0.08;
                double gapMs = std::clamp(proportionalGap, 15.0, 200.0);
                double shortened = segDuration - gapMs;
                if (shortened >= 50.0) {
                    segDuration = shortened;
                }
            }

            Note splitNote(segStartMs, segDuration, seg.pitchHz);
            splitNote.setTrackIndex(note.getTrackIndex());
            splitNote.setQuantized(false);
            splitNote.setLegato(false);
            splitNote.setRenderDirty(true);

            // Use segment's variation, falling back to note's default
            splitNote.setVariationIndex(
                seg.variationIndex >= 0 ? seg.variationIndex : note.getVariationIndex());

            // Use segment's vibrato or note's default
            if (!seg.useNoteVibrato) {
                splitNote.setVibrato(seg.vibrato);
            } else {
                splitNote.setVibrato(note.getVibrato());
            }

            // Extract dynamics for this segment's time range
            Curve splitDyn;
            const Curve &srcDyn = note.getDynamicsCurve();
            double segSpan = seg.endTime - seg.startTime;
            if (segSpan > 0) {
                // Sample source dynamics and remap to 0-1 for the split note
                const int dynSamples = 10;
                for (int d = 0; d <= dynSamples; ++d) {
                    double localT = static_cast<double>(d) / dynSamples;
                    double globalT = seg.startTime + localT * segSpan;
                    double dynVal = srcDyn.valueAt(globalT);
                    splitDyn.addPoint(localT, dynVal);
                }
            }
            splitNote.setDynamicsCurve(splitDyn);

            // For continuous linked notes, carry the pitch curve slice forward so
            // the split note keeps its original glide rather than becoming a flat note.
            if (note.hasPitchCurve() && note.isLinkedAsLegato() && segSpan > 0) {
                const Curve &srcPitch = note.getPitchCurve();
                const auto &srcPts = srcPitch.getPoints();

                // At segment boundaries the merged curve has coincident points
                // (prev-seg end + this-seg start at the same time). For this
                // segment's start we want the RIGHTMOST coincident value; for
                // the end we want the LEFTMOST. valueAt() returns the leftmost,
                // so it's correct for endTime but wrong for startTime.
                auto startAnchor = [&]() {
                    int lastMatch = -1;
                    for (int i = 0; i < srcPts.size(); ++i) {
                        if (std::abs(srcPts[i].time - seg.startTime) < 1e-9) {
                            lastMatch = i;
                        } else if (srcPts[i].time > seg.startTime) {
                            break;
                        }
                    }
                    return lastMatch >= 0 ? srcPts[lastMatch].value
                                          : srcPitch.valueAt(seg.startTime);
                };
                auto endAnchor = [&]() {
                    for (int i = 0; i < srcPts.size(); ++i) {
                        if (std::abs(srcPts[i].time - seg.endTime) < 1e-9) {
                            return srcPts[i].value;
                        }
                        if (srcPts[i].time > seg.endTime) break;
                    }
                    return srcPitch.valueAt(seg.endTime);
                };

                Curve splitPitch;
                splitPitch.addPoint(0.0, startAnchor(), 1.0);
                for (const auto &pt : srcPts) {
                    if (pt.time > seg.startTime && pt.time < seg.endTime) {
                        splitPitch.addPoint((pt.time - seg.startTime) / segSpan,
                                            pt.value, pt.pressure);
                    }
                }
                splitPitch.addPoint(1.0, endAnchor(), 1.0);
                splitNote.setPitchCurve(splitPitch);
            }

            entry.splitNotes.append(splitNote);
        }

        m_splits.append(entry);
    }
}

void UnlinkLegatoCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    // Reverse of redo: remove split notes, re-insert originals
    // m_splits is sorted by descending original index
    // redo() processes in that order (remove original, insert splits)
    // undo() reverses: remove splits, insert original — in ascending index order
    for (int i = m_splits.size() - 1; i >= 0; --i) {
        const SplitEntry &entry = m_splits[i];

        // Remove the split notes at this position
        int removeIdx = entry.originalIndex;
        for (int s = 0; s < entry.splitNotes.size() && removeIdx < notes.size(); ++s) {
            notes.removeAt(removeIdx);
        }

        // Re-insert the original merged note
        int insertIdx = qMin(entry.originalIndex, notes.size());
        notes.insert(insertIdx, entry.originalNote);
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Undo: Restored" << m_splits.size() << "merged note(s) from staccato split";
}

void UnlinkLegatoCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    // Process in descending index order (m_splits is already sorted this way)
    for (const SplitEntry &entry : m_splits) {
        // Remove the original merged note
        if (entry.originalIndex >= 0 && entry.originalIndex < notes.size()) {
            notes.removeAt(entry.originalIndex);
        }

        // Insert split notes at the same position
        int insertIdx = qMin(entry.originalIndex, notes.size());
        for (int s = 0; s < entry.splitNotes.size(); ++s) {
            notes.insert(insertIdx + s, entry.splitNotes[s]);
        }
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Redo: Split" << m_splits.size() << "merged note(s) into discrete staccato notes";
}

// ============================================================================
// Scale Timing Command (for tempo change on selected notes)
// ============================================================================

ScaleTimingCommand::ScaleTimingCommand(Phrase *phrase, const QVector<int> &noteIndices,
                                       double proportion, ScoreCanvas *canvas,
                                       QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_proportion(proportion)
    , m_canvas(canvas)
{
    setText(QString("Scale Timing (%1×)").arg(proportion, 0, 'f', 2));
    calculateNewTimes(noteIndices);
}

void ScaleTimingCommand::calculateNewTimes(const QVector<int> &noteIndices)
{
    const QVector<Note> &notes = m_phrase->getNotes();

    // Gather valid indices and find the selection window
    QVector<int> validIndices;
    double selectionStart = std::numeric_limits<double>::max();
    double selectionEnd = std::numeric_limits<double>::lowest();

    for (int idx : noteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            validIndices.append(idx);
            double noteStart = notes[idx].getStartTime();
            double noteEnd = noteStart + notes[idx].getDuration();
            selectionStart = qMin(selectionStart, noteStart);
            selectionEnd = qMax(selectionEnd, noteEnd);
        }
    }

    if (validIndices.isEmpty()) {
        return;
    }

    double totalDuration = selectionEnd - selectionStart;
    if (totalDuration <= 0) {
        return;
    }

    // Calculate new times for each note
    // Algorithm:
    // 1. For each note, find its relative position within the selection (0.0 to 1.0)
    // 2. Apply the same relative position to the new (scaled) duration
    // 3. Scale the note's duration by the proportion

    double newTotalDuration = totalDuration * m_proportion;

    for (int idx : validIndices) {
        const Note &note = notes[idx];

        // Store old state
        NoteTimingState oldState;
        oldState.index = idx;
        oldState.startTime = note.getStartTime();
        oldState.duration = note.getDuration();
        m_oldStates.append(oldState);

        // Calculate relative position of note start within selection
        double relativeStart = (note.getStartTime() - selectionStart) / totalDuration;

        // Calculate new timing
        NoteTimingState newState;
        newState.index = idx;
        newState.startTime = selectionStart + (relativeStart * newTotalDuration);
        newState.duration = note.getDuration() * m_proportion;
        m_newStates.append(newState);
    }
}

void ScaleTimingCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    for (const NoteTimingState &state : m_oldStates) {
        if (state.index >= 0 && state.index < notes.size()) {
            notes[state.index].setStartTime(state.startTime);
            notes[state.index].setDuration(state.duration);
            notes[state.index].setRenderDirty(true);
        }
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Undo: Restored timing for" << m_oldStates.size() << "notes";
}

void ScaleTimingCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    for (const NoteTimingState &state : m_newStates) {
        if (state.index >= 0 && state.index < notes.size()) {
            notes[state.index].setStartTime(state.startTime);
            notes[state.index].setDuration(state.duration);
            notes[state.index].setRenderDirty(true);
        }
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Redo: Scaled timing by" << m_proportion << "× for" << m_newStates.size() << "notes";
}

// ============================================================================
// Add Scale Change Command
// ============================================================================

AddScaleChangeCommand::AddScaleChangeCommand(ScoreCanvas *canvas, double timeMs,
                                             const Scale &scale, double baseFreq,
                                             QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_canvas(canvas)
    , m_timeMs(timeMs)
    , m_scale(scale)
    , m_baseFreq(baseFreq)
{
    setText("Add Scale Modulation");
}

void AddScaleChangeCommand::undo()
{
    // Revert pitch changes before removing the modulation
    m_canvas->applyNotePitchChanges(m_pitchChanges, false);

    m_canvas->removeScaleChange(m_timeMs);
    m_canvas->update();
    emit m_canvas->scaleSettingsChanged();
    qDebug() << "Undo: Removed scale modulation at" << m_timeMs << "ms";
}

void AddScaleChangeCommand::redo()
{
    // Determine the old scale/baseFreq that was active at this time before adding
    Scale oldScale = m_canvas->getScaleAtTime(m_timeMs);
    double oldBaseFreq = m_canvas->getBaseFrequencyAtTime(m_timeMs);

    m_canvas->addScaleChange(m_timeMs, m_scale, m_baseFreq);

    // Compute and apply pitch remapping
    m_pitchChanges = m_canvas->remapDiscreteNotesForModulation(
        m_timeMs, oldScale, oldBaseFreq, m_scale, m_baseFreq);
    m_canvas->applyNotePitchChanges(m_pitchChanges, true);

    m_canvas->update();
    emit m_canvas->scaleSettingsChanged();
    qDebug() << "Redo: Added scale modulation at" << m_timeMs << "ms:" << m_scale.getName()
             << "remapped" << m_pitchChanges.size() << "notes";
}

// ============================================================================
// Edit Scale Change Command
// ============================================================================

EditScaleChangeCommand::EditScaleChangeCommand(ScoreCanvas *canvas, double timeMs,
                                               const Scale &oldScale, double oldBaseFreq,
                                               const Scale &newScale, double newBaseFreq,
                                               bool isDefault, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_canvas(canvas)
    , m_timeMs(timeMs)
    , m_oldScale(oldScale)
    , m_oldBaseFreq(oldBaseFreq)
    , m_newScale(newScale)
    , m_newBaseFreq(newBaseFreq)
    , m_isDefault(isDefault)
{
    setText(m_isDefault ? "Edit Default Scale" : "Edit Scale Modulation");
}

void EditScaleChangeCommand::undo()
{
    // Revert pitch changes first
    m_canvas->applyNotePitchChanges(m_pitchChanges, false);

    if (m_isDefault) {
        m_canvas->setScale(m_oldScale);
        m_canvas->setBaseFrequency(m_oldBaseFreq);
    } else {
        m_canvas->addScaleChange(m_timeMs, m_oldScale, m_oldBaseFreq);
    }
    m_canvas->update();
    emit m_canvas->scaleSettingsChanged();
    qDebug() << "Undo: Restored scale at" << m_timeMs << "ms to" << m_oldScale.getName();
}

void EditScaleChangeCommand::redo()
{
    if (m_isDefault) {
        m_canvas->setScale(m_newScale);
        m_canvas->setBaseFrequency(m_newBaseFreq);
    } else {
        m_canvas->addScaleChange(m_timeMs, m_newScale, m_newBaseFreq);
    }

    // Compute and apply pitch remapping from old to new scale
    m_pitchChanges = m_canvas->remapDiscreteNotesForModulation(
        m_timeMs, m_oldScale, m_oldBaseFreq, m_newScale, m_newBaseFreq);
    m_canvas->applyNotePitchChanges(m_pitchChanges, true);

    m_canvas->update();
    emit m_canvas->scaleSettingsChanged();
    qDebug() << "Redo: Changed scale at" << m_timeMs << "ms to" << m_newScale.getName()
             << "remapped" << m_pitchChanges.size() << "notes";
}

// ============================================================================
// Make Notes Continuous Command
// ============================================================================

MakeNotesContinuousCommand::MakeNotesContinuousCommand(Phrase *phrase,
                                                       const QVector<int> &noteIndices,
                                                       ScoreCanvas *canvas,
                                                       QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_canvas(canvas)
{
    setText("Make Notes Continuous");

    const QVector<Note> &notes = m_phrase->getNotes();
    for (int idx : noteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            const Note &note = notes[idx];
            // Only affect discrete notes (those without a pitch curve)
            if (!note.hasPitchCurve()) {
                NoteState state;
                state.index = idx;
                state.oldPitchCurve = note.getPitchCurve();
                state.hadPitchCurve = false;
                state.wasQuantized = note.isQuantized();
                state.oldSegments = note.getSegments();
                m_noteStates.append(state);
            }
        }
    }
}

void MakeNotesContinuousCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (const NoteState &state : m_noteStates) {
        if (state.index >= 0 && state.index < notes.size()) {
            Note &note = notes[state.index];
            note.setPitchCurve(state.oldPitchCurve);
            note.setQuantized(state.wasQuantized);
            note.getSegments() = state.oldSegments;
            note.setRenderDirty(true);
        }
    }
    m_canvas->update();
    qDebug() << "Undo: Reverted" << m_noteStates.size() << "notes back to discrete";
}

void MakeNotesContinuousCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (const NoteState &state : m_noteStates) {
        if (state.index >= 0 && state.index < notes.size()) {
            Note &note = notes[state.index];
            // Create a flat pitch curve at the note's current pitch
            Curve flatCurve;
            flatCurve.addPoint(0.0, note.getPitchHz());
            flatCurve.addPoint(1.0, note.getPitchHz());
            note.setPitchCurve(flatCurve);
            note.setQuantized(false);
            note.clearSegments();
            note.setRenderDirty(true);
        }
    }
    m_canvas->update();
    qDebug() << "Made" << m_noteStates.size() << "notes continuous";
}

// ============================================================================
// Make Notes Discrete Command
// ============================================================================

MakeNotesDiscreteCommand::MakeNotesDiscreteCommand(Phrase *phrase,
                                                   const QVector<int> &noteIndices,
                                                   ScoreCanvas *canvas,
                                                   QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_canvas(canvas)
{
    setText("Make Notes Discrete");

    const QVector<Note> &notes = m_phrase->getNotes();
    for (int idx : noteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            const Note &note = notes[idx];
            // Only affect continuous notes (those with a pitch curve)
            if (note.hasPitchCurve()) {
                NoteState state;
                state.index = idx;
                state.oldPitchCurve = note.getPitchCurve();
                state.wasQuantized = note.isQuantized();
                state.oldSegments = note.getSegments();
                state.oldPitchHz = note.getPitchHz();
                // Snap to nearest scale line at note's start time
                state.newPitchHz = canvas->snapToNearestScaleLineAtTime(note.getPitchHz(), note.getStartTime());
                m_noteStates.append(state);
            }
        }
    }
}

void MakeNotesDiscreteCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (const NoteState &state : m_noteStates) {
        if (state.index >= 0 && state.index < notes.size()) {
            Note &note = notes[state.index];
            note.setPitchCurve(state.oldPitchCurve);
            note.setQuantized(state.wasQuantized);
            note.getSegments() = state.oldSegments;
            note.setPitchHz(state.oldPitchHz);
            note.setRenderDirty(true);
        }
    }
    m_canvas->update();
    qDebug() << "Undo: Reverted" << m_noteStates.size() << "notes back to continuous";
}

void MakeNotesDiscreteCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (const NoteState &state : m_noteStates) {
        if (state.index >= 0 && state.index < notes.size()) {
            Note &note = notes[state.index];
            note.setPitchCurve(Curve{});
            note.setQuantized(false);
            note.clearSegments();
            note.setPitchHz(state.newPitchHz);
            note.setRenderDirty(true);
        }
    }
    m_canvas->update();
    qDebug() << "Made" << m_noteStates.size() << "notes discrete";
}

// ============================================================================
// Delete Scale Change Command
// ============================================================================

DeleteScaleChangeCommand::DeleteScaleChangeCommand(ScoreCanvas *canvas, double timeMs,
                                                   const Scale &oldScale, double oldBaseFreq,
                                                   QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_canvas(canvas)
    , m_timeMs(timeMs)
    , m_oldScale(oldScale)
    , m_oldBaseFreq(oldBaseFreq)
{
    setText("Delete Scale Modulation");
}

void DeleteScaleChangeCommand::undo()
{
    m_canvas->addScaleChange(m_timeMs, m_oldScale, m_oldBaseFreq);
    m_canvas->update();
    emit m_canvas->scaleSettingsChanged();
    qDebug() << "Undo: Restored scale modulation at" << m_timeMs << "ms:" << m_oldScale.getName();
}

void DeleteScaleChangeCommand::redo()
{
    m_canvas->removeScaleChange(m_timeMs);
    m_canvas->update();
    emit m_canvas->scaleSettingsChanged();
    qDebug() << "Redo: Deleted scale modulation at" << m_timeMs << "ms";
}

// ============================================================================
// Remove Expressive Curve Command
// ============================================================================

RemoveExpressiveCurveCommand::RemoveExpressiveCurveCommand(Phrase *phrase,
                                                            const QVector<int> &noteIndices,
                                                            int curveIndex,
                                                            ScoreCanvas *canvas,
                                                            QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndices(noteIndices)
    , m_curveIndex(curveIndex)
    , m_canvas(canvas)
{
    setText("Remove Expressive Curve");

    // Save curves for undo
    QVector<Note> &notes = m_phrase->getNotes();
    for (int idx : m_noteIndices) {
        if (idx >= 0 && idx < notes.size() &&
            m_curveIndex >= 1 && m_curveIndex < notes[idx].getExpressiveCurveCount()) {
            SavedCurve sc;
            sc.name  = notes[idx].getExpressiveCurveName(m_curveIndex);
            sc.curve = notes[idx].getExpressiveCurve(m_curveIndex);
            m_savedCurves.append(sc);
        } else {
            m_savedCurves.append(SavedCurve{});  // placeholder
        }
    }
}

void RemoveExpressiveCurveCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (int i = 0; i < m_noteIndices.size(); ++i) {
        int idx = m_noteIndices[i];
        if (idx >= 0 && idx < notes.size() && i < m_savedCurves.size()) {
            // Re-insert the saved curve at the correct position.
            // addExpressiveCurve always appends; if curve count is already at the right index, it lands correctly.
            // For simplicity (notes in selection should be consistent), append then shift if needed.
            Note &note = notes[idx];
            // First bring count up to m_curveIndex - 1 if needed (shouldn't happen in normal use)
            note.addExpressiveCurve(m_savedCurves[i].name, m_savedCurves[i].curve);
        }
    }
    m_canvas->update();
    qDebug() << "Undo: Restored expressive curve at index" << m_curveIndex;
}

void RemoveExpressiveCurveCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (int idx : m_noteIndices) {
        if (idx >= 0 && idx < notes.size() &&
            m_curveIndex >= 1 && m_curveIndex < notes[idx].getExpressiveCurveCount()) {
            notes[idx].removeExpressiveCurve(m_curveIndex);
        }
    }
    m_canvas->update();
    qDebug() << "Redo: Removed expressive curve at index" << m_curveIndex;
}

// ============================================================================
// Remove Named Expressive Curve Command
// ============================================================================

RemoveNamedExpressiveCurveCommand::RemoveNamedExpressiveCurveCommand(
    Phrase *phrase, const QString &curveName, const QVector<int> &noteIndices,
    ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_curveName(curveName)
    , m_canvas(canvas)
{
    const bool scoped = !noteIndices.isEmpty();
    setText(scoped
        ? QString("Delete Curve \"%1\" from Selection").arg(curveName)
        : QString("Delete Curve \"%1\" from All Notes").arg(curveName));

    // Save state for notes that will actually have the curve removed.
    const QVector<Note> &notes = m_phrase->getNotes();
    auto captureNote = [&](int i) {
        if (i < 0 || i >= notes.size()) return;
        for (int j = 1; j < notes[i].getExpressiveCurveCount(); ++j) {
            if (notes[i].getExpressiveCurveName(j) == curveName) {
                m_saved.append({i, notes[i].getExpressiveCurve(j)});
                break;
            }
        }
    };
    if (scoped) {
        for (int i : noteIndices) captureNote(i);
    } else {
        for (int i = 0; i < notes.size(); ++i) captureNote(i);
    }
}

void RemoveNamedExpressiveCurveCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (const SavedState &s : m_saved) {
        if (s.noteIdx >= 0 && s.noteIdx < notes.size())
            notes[s.noteIdx].addExpressiveCurve(m_curveName, s.curve);
    }
    m_canvas->update();
    qDebug() << "Undo: Restored curve '" << m_curveName << "' to" << m_saved.size() << "notes";
}

void RemoveNamedExpressiveCurveCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (const SavedState &s : m_saved) {
        if (s.noteIdx < 0 || s.noteIdx >= notes.size()) continue;
        Note &note = notes[s.noteIdx];
        for (int j = 1; j < note.getExpressiveCurveCount(); ++j) {
            if (note.getExpressiveCurveName(j) == m_curveName) {
                note.removeExpressiveCurve(j);
                break;
            }
        }
    }
    m_canvas->update();
    qDebug() << "Redo: Removed curve '" << m_curveName << "' from" << m_saved.size() << "notes";
}

// ============================================================================
// Apply Named Curve Command
// ============================================================================

ApplyNamedCurveCommand::ApplyNamedCurveCommand(Phrase *phrase,
                                                const QVector<int> &noteIndices,
                                                const QString &name,
                                                const Curve &curve,
                                                ScoreCanvas *canvas,
                                                QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndices(noteIndices)
    , m_name(name)
    , m_curve(curve)
    , m_canvas(canvas)
    , m_firstTime(true)
{
    setText("Apply Named Curve");
}

void ApplyNamedCurveCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    if (m_firstTime) {
        m_noteStates.clear();
        for (int idx : m_noteIndices) {
            if (idx < 0 || idx >= notes.size()) continue;
            const Note &note = notes[idx];
            NoteState state;
            state.index = idx;
            state.hadCurve = false;
            state.existingCurveIdx = -1;
            for (int i = 1; i < note.getExpressiveCurveCount(); ++i) {
                if (note.getExpressiveCurveName(i) == m_name) {
                    state.hadCurve = true;
                    state.existingCurveIdx = i;
                    state.oldCurve = note.getExpressiveCurve(i);
                    break;
                }
            }
            m_noteStates.append(state);
        }
        m_firstTime = false;
    }

    for (const NoteState &state : m_noteStates) {
        if (state.index < 0 || state.index >= notes.size()) continue;
        Note &note = notes[state.index];
        if (state.hadCurve) {
            note.getExpressiveCurve(state.existingCurveIdx) = m_curve;
        } else {
            note.addExpressiveCurve(m_name, m_curve);
        }
        note.setRenderDirty(true);
    }

    m_canvas->update();
    qDebug() << "Redo: Applied named curve '" << m_name << "' to" << m_noteStates.size() << "notes";
}

void ApplyNamedCurveCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    for (const NoteState &state : m_noteStates) {
        if (state.index < 0 || state.index >= notes.size()) continue;
        Note &note = notes[state.index];
        if (state.hadCurve) {
            note.getExpressiveCurve(state.existingCurveIdx) = state.oldCurve;
        } else {
            // Remove the curve we added - find it by name
            for (int i = 1; i < note.getExpressiveCurveCount(); ++i) {
                if (note.getExpressiveCurveName(i) == m_name) {
                    note.removeExpressiveCurve(i);
                    break;
                }
            }
        }
        note.setRenderDirty(true);
    }

    m_canvas->update();
    qDebug() << "Undo: Reverted named curve '" << m_name << "' on" << m_noteStates.size() << "notes";
}

// ============================================================================
// Apply Expressive Curve To Selection Command
// ============================================================================

ApplyExpressiveCurveToSelectionCommand::ApplyExpressiveCurveToSelectionCommand(
    Phrase *phrase,
    const QVector<int> &noteIndices,
    const QString &curveName,
    const QVector<EnvelopePoint> &envelope,
    double weight,
    bool perNote,
    ScoreCanvas *canvas,
    QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndices(noteIndices)
    , m_curveName(curveName)
    , m_envelope(envelope)
    , m_weight(weight)
    , m_perNote(perNote)
    , m_firstTime(true)
    , m_canvas(canvas)
{
    setText("Apply Expressive Curve");
}

void ApplyExpressiveCurveToSelectionCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (const PriorState &state : m_priorStates) {
        int idx = state.noteIdx;
        if (idx < 0 || idx >= notes.size()) continue;
        Note &note = notes[idx];
        if (state.hadCurve) {
            note.upsertExpressiveCurve(m_curveName, state.oldCurve);
            if (!state.oldControlPoints.isEmpty())
                note.setEnvelopeControlPoints(m_curveName, state.oldControlPoints);
            else
                note.removeEnvelopeControlPoints(m_curveName);
        } else {
            note.removeExpressiveCurveByName(m_curveName);
            note.removeEnvelopeControlPoints(m_curveName);
        }
        note.setRenderDirty(true);
    }
    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Undo: Apply Expressive Curve '" << m_curveName << "' on"
             << m_priorStates.size() << "notes";
}

void ApplyExpressiveCurveToSelectionCommand::redo()
{
    if (m_noteIndices.isEmpty() || m_envelope.isEmpty()) return;

    // Evaluate the envelope shape at normalized time t in [0,1].
    auto evalEnvelope = [this](double t) -> double {
        if (m_envelope.size() == 1) return m_envelope[0].value;
        if (t <= m_envelope.first().time) return m_envelope.first().value;
        if (t >= m_envelope.last().time)  return m_envelope.last().value;
        for (int i = 0; i < m_envelope.size() - 1; ++i) {
            if (t >= m_envelope[i].time && t <= m_envelope[i + 1].time) {
                double segT = (t - m_envelope[i].time)
                              / (m_envelope[i + 1].time - m_envelope[i].time);
                if (m_envelope[i].curveType == 1) {
                    double smoothT = (1.0 - std::cos(segT * M_PI)) * 0.5;
                    return m_envelope[i].value
                           + smoothT * (m_envelope[i + 1].value - m_envelope[i].value);
                } else if (m_envelope[i].curveType == 2) {
                    return m_envelope[i].value;
                } else {
                    return m_envelope[i].value
                           + segT * (m_envelope[i + 1].value - m_envelope[i].value);
                }
            }
        }
        return m_envelope.last().value;
    };

    // Sample the envelope into a Curve (used by per-note / single-note / "add
    // as-is" branches).  Weight linearly scales every value; when the target
    // note had no prior curve, this is the curve that gets laid down.
    const int SAMPLE_COUNT = 30;
    auto buildShapedCurve = [&](double extraScale) {
        Curve c;
        for (int s = 0; s <= SAMPLE_COUNT; ++s) {
            double t = static_cast<double>(s) / SAMPLE_COUNT;
            double v = qBound(0.0, evalEnvelope(t) * m_weight * extraScale, 1.0);
            c.addPoint(t, v, 1.0);
        }
        return c;
    };

    QVector<Note> &notes = m_phrase->getNotes();

    if (m_firstTime) {
        m_priorStates.clear();
        for (int idx : m_noteIndices) {
            if (idx < 0 || idx >= notes.size()) continue;
            PriorState ps;
            ps.noteIdx = idx;
            int existingIdx = notes[idx].findExpressiveCurveIndexByName(m_curveName);
            if (existingIdx >= 1) {
                ps.hadCurve = true;
                ps.oldCurve = notes[idx].getExpressiveCurve(existingIdx);
                ps.oldControlPoints = notes[idx].getEnvelopeControlPoints(m_curveName);
            } else {
                ps.hadCurve = false;
            }
            m_priorStates.append(ps);
        }
        m_firstTime = false;
    }

    // Per-note mode, or single-note selection → copy the drawn shape to each
    // note's own duration. Same behaviour as the legacy path.
    if (m_perNote || m_priorStates.size() == 1) {
        Curve shapedCurve = buildShapedCurve(1.0);
        for (const PriorState &ps : m_priorStates) {
            int idx = ps.noteIdx;
            if (idx < 0 || idx >= notes.size()) continue;
            notes[idx].upsertExpressiveCurve(m_curveName, shapedCurve);
            notes[idx].setEnvelopeControlPoints(m_curveName, m_envelope);
            notes[idx].setRenderDirty(true);
        }

        m_canvas->update();
        emit m_canvas->notesChanged();
        qDebug() << "Redo: Apply Expressive Curve '" << m_curveName
                 << "' per-note on" << m_priorStates.size()
                 << "notes (weight=" << m_weight << ")";
        return;
    }

    // Selection-scaling mode: envelope spans the whole selection.  Notes that
    // already carry the named curve keep their per-note shape, scaled by
    // evalEnvelope at their centre's position.  Notes without the curve get
    // the drawn shape laid down as-is.
    double selStart = std::numeric_limits<double>::max();
    double selEnd   = std::numeric_limits<double>::lowest();
    for (const PriorState &ps : m_priorStates) {
        int idx = ps.noteIdx;
        if (idx >= 0 && idx < notes.size()) {
            selStart = qMin(selStart, notes[idx].getStartTime());
            selEnd   = qMax(selEnd,   notes[idx].getStartTime() + notes[idx].getDuration());
        }
    }
    double selDuration = selEnd - selStart;
    if (selDuration <= 0.0) return;

    for (const PriorState &ps : m_priorStates) {
        int idx = ps.noteIdx;
        if (idx < 0 || idx >= notes.size()) continue;
        Note &note = notes[idx];

        if (ps.hadCurve) {
            // Scale the pre-existing per-note shape by the envelope at the
            // note's centre in the selection — preserves internal shape.
            double noteCenter    = note.getStartTime() + note.getDuration() / 2.0;
            double normalizedPos = qBound(0.0, (noteCenter - selStart) / selDuration, 1.0);
            double curveValue    = evalEnvelope(normalizedPos);
            double scale         = 1.0 + m_weight * (curveValue - 1.0);

            const QVector<Curve::Point> &pts = ps.oldCurve.getPoints();
            Curve newCurve;
            if (pts.isEmpty()) {
                double v = qBound(0.0, 0.7 * scale, 1.0);
                newCurve.addPoint(0.0, v, 1.0);
                newCurve.addPoint(1.0, v, 1.0);
            } else {
                for (const Curve::Point &pt : pts) {
                    newCurve.addPoint(pt.time, qBound(0.0, pt.value * scale, 1.0),
                                      pt.pressure);
                }
            }
            note.upsertExpressiveCurve(m_curveName, newCurve);
            // Preserve prior control points — the per-note shape is what was
            // drawn (or pre-existing); dialog envelope belongs to the selection
            // spread, not to this note's own shape.
            if (!ps.oldControlPoints.isEmpty())
                note.setEnvelopeControlPoints(m_curveName, ps.oldControlPoints);
        } else {
            // No prior curve → slice the drawn envelope across this note's
            // time span within the selection so the envelope reads
            // continuously across the whole selection rather than being
            // repeated per-note.
            double tNoteStart = qBound(0.0,
                (note.getStartTime() - selStart) / selDuration, 1.0);
            double tNoteEnd   = qBound(0.0,
                (note.getStartTime() + note.getDuration() - selStart) / selDuration, 1.0);
            Curve sliceCurve;
            for (int s = 0; s <= SAMPLE_COUNT; ++s) {
                double localT = static_cast<double>(s) / SAMPLE_COUNT;
                double selT   = tNoteStart + localT * (tNoteEnd - tNoteStart);
                double v      = qBound(0.0, evalEnvelope(selT) * m_weight, 1.0);
                sliceCurve.addPoint(localT, v, 1.0);
            }
            note.upsertExpressiveCurve(m_curveName, sliceCurve);
            note.setEnvelopeControlPoints(m_curveName, m_envelope);
        }
        note.setRenderDirty(true);
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Redo: Apply Expressive Curve '" << m_curveName
             << "' selection-scaled on" << m_priorStates.size()
             << "notes (weight=" << m_weight << ")";
}

// ============================================================================
// Apply EQ Curve Command
// ============================================================================

ApplyEqCurveCommand::ApplyEqCurveCommand(Phrase *phrase,
                                         const QVector<int> &noteIndices,
                                         const QStringList &bandNames,
                                         const QVector<EnvelopePoint> &shape,
                                         const QVector<EnvelopePoint> &intensity,
                                         double weight,
                                         bool perNote,
                                         ScoreCanvas *canvas,
                                         QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndices(noteIndices)
    , m_bandNames(bandNames)
    , m_shape(shape)
    , m_intensity(intensity)
    , m_weight(weight)
    , m_perNote(perNote)
    , m_firstTime(true)
    , m_canvas(canvas)
{
    setText("Apply EQ Curve");
}

void ApplyEqCurveCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (const PriorNoteState &pns : m_priorStates) {
        int idx = pns.noteIdx;
        if (idx < 0 || idx >= notes.size()) continue;
        Note &note = notes[idx];
        for (int b = 0; b < m_bandNames.size() && b < pns.bands.size(); ++b) {
            const QString &name = m_bandNames[b];
            const PriorBandState &ps = pns.bands[b];
            if (ps.hadCurve) {
                note.upsertExpressiveCurve(name, ps.oldCurve);
                if (!ps.oldControlPoints.isEmpty())
                    note.setEnvelopeControlPoints(name, ps.oldControlPoints);
                else
                    note.removeEnvelopeControlPoints(name);
            } else {
                note.removeExpressiveCurveByName(name);
                note.removeEnvelopeControlPoints(name);
            }
        }
        if (pns.hadShape)
            note.setEnvelopeControlPoints(QStringLiteral("EQ Shape"), pns.oldShape);
        else
            note.removeEnvelopeControlPoints(QStringLiteral("EQ Shape"));
        if (pns.hadIntensity)
            note.setEnvelopeControlPoints(QStringLiteral("EQ Intensity"), pns.oldIntensity);
        else
            note.removeEnvelopeControlPoints(QStringLiteral("EQ Intensity"));
        note.setRenderDirty(true);
    }
    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Undo: Apply EQ Curve on" << m_priorStates.size() << "notes";
}

void ApplyEqCurveCommand::redo()
{
    if (m_noteIndices.isEmpty() || m_shape.isEmpty() || m_intensity.isEmpty()) return;
    if (m_bandNames.size() != 10) return;

    // --- Envelope evaluators (mirrors ApplyExpressiveCurveToSelectionCommand) ---
    auto evalEnvelope = [](const QVector<EnvelopePoint> &env, double t) -> double {
        if (env.isEmpty()) return 0.5;
        if (env.size() == 1) return env[0].value;
        if (t <= env.first().time) return env.first().value;
        if (t >= env.last().time)  return env.last().value;
        for (int i = 0; i < env.size() - 1; ++i) {
            if (t >= env[i].time && t <= env[i + 1].time) {
                double dt = env[i + 1].time - env[i].time;
                if (dt <= 0.0) return env[i + 1].value;
                double segT = (t - env[i].time) / dt;
                if (env[i].curveType == 1) {
                    double smoothT = (1.0 - std::cos(segT * M_PI)) * 0.5;
                    return env[i].value + smoothT * (env[i + 1].value - env[i].value);
                } else if (env[i].curveType == 2) {
                    return env[i].value;
                } else {
                    return env[i].value + segT * (env[i + 1].value - env[i].value);
                }
            }
        }
        return env.last().value;
    };

    // --- Sample shape into 10 band values by averaging over each 1/10th slice ---
    double bandValues[10];
    const int SUB = 8;  // sub-samples per band for a smooth average
    for (int b = 0; b < 10; ++b) {
        double t0 = b / 10.0;
        double t1 = (b + 1) / 10.0;
        double sum = 0.0;
        for (int s = 0; s < SUB; ++s) {
            double t = t0 + (t1 - t0) * (s + 0.5) / SUB;
            sum += evalEnvelope(m_shape, t);
        }
        bandValues[b] = sum / SUB;
    }

    QVector<Note> &notes = m_phrase->getNotes();

    // --- On first redo, snapshot prior state for undo ---
    if (m_firstTime) {
        m_priorStates.clear();
        for (int idx : m_noteIndices) {
            if (idx < 0 || idx >= notes.size()) continue;
            PriorNoteState pns;
            pns.noteIdx = idx;
            pns.bands.reserve(10);
            for (int b = 0; b < 10; ++b) {
                const QString &name = m_bandNames[b];
                PriorBandState ps;
                int existingIdx = notes[idx].findExpressiveCurveIndexByName(name);
                if (existingIdx >= 1) {
                    ps.hadCurve = true;
                    ps.oldCurve = notes[idx].getExpressiveCurve(existingIdx);
                    ps.oldControlPoints = notes[idx].getEnvelopeControlPoints(name);
                } else {
                    ps.hadCurve = false;
                }
                pns.bands.append(ps);
            }
            pns.hadShape = notes[idx].hasEnvelopeControlPoints(QStringLiteral("EQ Shape"));
            if (pns.hadShape)
                pns.oldShape = notes[idx].getEnvelopeControlPoints(QStringLiteral("EQ Shape"));
            pns.hadIntensity = notes[idx].hasEnvelopeControlPoints(QStringLiteral("EQ Intensity"));
            if (pns.hadIntensity)
                pns.oldIntensity = notes[idx].getEnvelopeControlPoints(QStringLiteral("EQ Intensity"));
            m_priorStates.append(pns);
        }
        m_firstTime = false;
    }

    const bool perNoteMode = m_perNote || (m_priorStates.size() == 1);

    // --- Selection time span (only needed for phrase mode) ---
    double selStart = 0.0, selEnd = 0.0, selDuration = 0.0;
    if (!perNoteMode) {
        selStart = std::numeric_limits<double>::max();
        selEnd   = std::numeric_limits<double>::lowest();
        for (const PriorNoteState &pns : m_priorStates) {
            int idx = pns.noteIdx;
            if (idx < 0 || idx >= notes.size()) continue;
            selStart = qMin(selStart, notes[idx].getStartTime());
            selEnd   = qMax(selEnd,   notes[idx].getStartTime() + notes[idx].getDuration());
        }
        selDuration = selEnd - selStart;
        if (selDuration <= 0.0) return;
    }

    const int SAMPLE_COUNT = 30;

    for (const PriorNoteState &pns : m_priorStates) {
        int idx = pns.noteIdx;
        if (idx < 0 || idx >= notes.size()) continue;
        Note &note = notes[idx];

        // Determine per-note scalar intensity (used only in phrase mode)
        double noteIntensity = 1.0;
        if (!perNoteMode) {
            double noteCenter = note.getStartTime() + note.getDuration() / 2.0;
            double nPos = qBound(0.0, (noteCenter - selStart) / selDuration, 1.0);
            noteIntensity = evalEnvelope(m_intensity, nPos);
        }

        for (int b = 0; b < 10; ++b) {
            const QString &name = m_bandNames[b];
            const double delta = bandValues[b] - 0.5;  // band offset from flat

            Curve c;
            QVector<EnvelopePoint> eps;

            if (perNoteMode) {
                // Intensity curve spans this note's duration; sample it.
                for (int s = 0; s <= SAMPLE_COUNT; ++s) {
                    double t = static_cast<double>(s) / SAMPLE_COUNT;
                    double intensity_t = evalEnvelope(m_intensity, t);
                    double v = 0.5 + delta * intensity_t * m_weight;
                    v = qBound(0.0, v, 1.0);
                    c.addPoint(t, v, 1.0);
                    eps.append(EnvelopePoint(t, v, 0));
                }
            } else {
                // Static EQ per note, scaled by this note's position on intensity curve.
                double v = qBound(0.0, 0.5 + delta * noteIntensity * m_weight, 1.0);
                c.addPoint(0.0, v, 1.0);
                c.addPoint(1.0, v, 1.0);
                eps.append(EnvelopePoint(0.0, v, 0));
                eps.append(EnvelopePoint(1.0, v, 0));
            }

            note.upsertExpressiveCurve(name, c);
            note.setEnvelopeControlPoints(name, eps);
        }
        // Persist the original author curves so re-editing restores them
        // verbatim instead of reconstructing from the dense band samples.
        note.setEnvelopeControlPoints(QStringLiteral("EQ Shape"), m_shape);
        note.setEnvelopeControlPoints(QStringLiteral("EQ Intensity"), m_intensity);
        note.setRenderDirty(true);
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Redo: Apply EQ Curve on" << m_priorStates.size()
             << "notes (weight=" << m_weight
             << ", perNote=" << perNoteMode << ")";
}

// ============================================================================
// Scale Dynamics Command
// ============================================================================

ScaleDynamicsCommand::ScaleDynamicsCommand(Phrase *phrase,
                                           const QVector<int> &noteIndices,
                                           double factor,
                                           ScoreCanvas *canvas,
                                           QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndices(noteIndices)
    , m_factor(factor)
    , m_firstTime(true)
    , m_canvas(canvas)
{
    setText("Scale Dynamics");
}

void ScaleDynamicsCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (int i = 0; i < m_noteIndices.size() && i < m_oldCurves.size(); ++i) {
        int idx = m_noteIndices[i];
        if (idx >= 0 && idx < notes.size())
            notes[idx].setDynamicsCurve(m_oldCurves[i]);
    }
    m_canvas->update();
    emit m_canvas->notesChanged();
}

void ScaleDynamicsCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    if (m_firstTime) {
        m_oldCurves.clear();
        for (int idx : m_noteIndices)
            m_oldCurves.append((idx >= 0 && idx < notes.size())
                               ? notes[idx].getDynamicsCurve()
                               : Curve{});
        m_firstTime = false;
    }

    for (int i = 0; i < m_noteIndices.size(); ++i) {
        int idx = m_noteIndices[i];
        if (idx < 0 || idx >= notes.size()) continue;
        Note &note = notes[idx];

        const Curve &old = m_oldCurves[i];
        if (old.isEmpty()) {
            // No existing curve — treat as flat 0.5, scale it
            Curve newCurve;
            double v = qBound(0.0, 0.5 * m_factor, 1.0);
            newCurve.addPoint(0.0, v, 1.0);
            newCurve.addPoint(1.0, v, 1.0);
            note.setDynamicsCurve(newCurve);
        } else {
            Curve newCurve;
            for (const Curve::Point &p : old.getPoints())
                newCurve.addPoint(p.time, qBound(0.0, p.value * m_factor, 1.0), p.pressure);
            note.setDynamicsCurve(newCurve);
        }
        note.setRenderDirty(true);
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "ScaleDynamicsCommand: scaled" << m_noteIndices.size()
             << "notes by factor" << m_factor;
}

// ============================================================================
// Set Beat Dynamics Command
// ============================================================================

SetBeatDynamicsCommand::SetBeatDynamicsCommand(Phrase *phrase,
                                               const QVector<int> &noteIndices,
                                               const QVector<double> &newDynamics,
                                               ScoreCanvas *canvas,
                                               QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndices(noteIndices)
    , m_newDynamics(newDynamics)
    , m_firstTime(true)
    , m_canvas(canvas)
{
    setText("Apply Beat Dynamics");
}

void SetBeatDynamicsCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (int i = 0; i < m_noteIndices.size(); ++i) {
        int idx = m_noteIndices[i];
        if (idx >= 0 && idx < notes.size())
            notes[idx].setDynamicsCurve(m_oldCurves[i]);
    }
    m_canvas->update();
    emit m_canvas->notesChanged();
}

void SetBeatDynamicsCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    if (m_firstTime) {
        m_oldCurves.clear();
        for (int idx : m_noteIndices)
            m_oldCurves.append((idx >= 0 && idx < notes.size())
                                   ? notes[idx].getDynamicsCurve()
                                   : Curve{});
        m_firstTime = false;
    }
    for (int i = 0; i < m_noteIndices.size(); ++i) {
        int idx = m_noteIndices[i];
        if (idx < 0 || idx >= notes.size()) continue;
        const double factor = m_newDynamics[i];
        const Curve &old = m_oldCurves[i];
        if (old.isEmpty()) {
            notes[idx].setDynamics(factor);
        } else {
            Curve scaled;
            for (const Curve::Point &p : old.getPoints())
                scaled.addPoint(p.time, qBound(0.0, p.value * factor, 1.0), p.pressure);
            notes[idx].setDynamicsCurve(scaled);
        }
    }
    m_canvas->update();
    emit m_canvas->notesChanged();
}

// ============================================================================
// Set Vibrato Command
// ============================================================================

SetVibratoCommand::SetVibratoCommand(Phrase *phrase,
                                     const QVector<int> &noteIndices,
                                     const Vibrato &newVibrato,
                                     ScoreCanvas *canvas,
                                     QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndices(noteIndices)
    , m_newVibrato(newVibrato)
    , m_firstTime(true)
    , m_canvas(canvas)
{
    setText("Set Vibrato");
}

void SetVibratoCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (int i = 0; i < m_noteIndices.size(); ++i) {
        int idx = m_noteIndices[i];
        if (idx >= 0 && idx < notes.size()) {
            notes[idx].setVibrato(m_oldVibratos[i]);
            notes[idx].setRenderDirty(true);
        }
    }
    m_canvas->update();
    emit m_canvas->notesChanged();
}

void SetVibratoCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    if (m_firstTime) {
        m_oldVibratos.clear();
        for (int idx : m_noteIndices)
            m_oldVibratos.append((idx >= 0 && idx < notes.size())
                                     ? notes[idx].getVibrato()
                                     : Vibrato{});
        m_firstTime = false;
    }
    for (int idx : m_noteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            notes[idx].setVibrato(m_newVibrato);
            notes[idx].setRenderDirty(true);
        }
    }
    m_canvas->update();
    emit m_canvas->notesChanged();
}

// ============================================================================
// Set Default Tempo Command
// ============================================================================

SetDefaultTempoCommand::SetDefaultTempoCommand(ScoreCanvas *canvas,
                                               double oldTempo, int oldTimeSigNum, int oldTimeSigDenom,
                                               double newTempo, int newTimeSigNum, int newTimeSigDenom,
                                               QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_canvas(canvas)
    , m_oldTempo(oldTempo), m_newTempo(newTempo)
    , m_oldTimeSigNum(oldTimeSigNum), m_oldTimeSigDenom(oldTimeSigDenom)
    , m_newTimeSigNum(newTimeSigNum), m_newTimeSigDenom(newTimeSigDenom)
    , m_firstTime(true)
{
    setText("Set Default Tempo");

    // Capture current (old) note states
    const QVector<Note> &notes = canvas->getPhrase().getNotes();
    for (int i = 0; i < notes.size(); ++i)
        m_oldNoteStates.append({i, notes[i].getStartTime(), notes[i].getDuration()});

    // Capture current scale-change and tempo-change markers
    m_oldScaleChanges = canvas->getScaleChanges();
    m_oldTempoChanges = canvas->getTempoChanges();

    // Pre-compute new states (scaled by oldTempo/newTempo if tempo actually changed)
    double factor = (newTempo > 0.0 && oldTempo > 0.0 && newTempo != oldTempo)
                    ? oldTempo / newTempo : 1.0;

    for (const NoteState &s : m_oldNoteStates)
        m_newNoteStates.append({s.index, s.startTime * factor, s.duration * factor});

    for (auto it = m_oldScaleChanges.constBegin(); it != m_oldScaleChanges.constEnd(); ++it)
        m_newScaleChanges[it.key() * factor] = it.value();

    for (auto it = m_oldTempoChanges.constBegin(); it != m_oldTempoChanges.constEnd(); ++it)
        m_newTempoChanges[it.key() * factor] = it.value();
}

void SetDefaultTempoCommand::applyState(bool useNew)
{
    const QVector<NoteState> &noteStates = useNew ? m_newNoteStates : m_oldNoteStates;
    const QMap<double, QPair<Scale, double>> &scaleChanges = useNew ? m_newScaleChanges : m_oldScaleChanges;
    const QMap<double, TempoTimeSignature> &tempoChanges = useNew ? m_newTempoChanges : m_oldTempoChanges;
    double tempo    = useNew ? m_newTempo    : m_oldTempo;
    int timeSigNum  = useNew ? m_newTimeSigNum  : m_oldTimeSigNum;
    int timeSigDenom= useNew ? m_newTimeSigDenom : m_oldTimeSigDenom;

    // Apply default tempo / timesig
    m_canvas->setDefaultTempo(tempo);
    m_canvas->setDefaultTimeSignature(timeSigNum, timeSigDenom);

    // Apply note states
    QVector<Note> &notes = m_canvas->getPhrase().getNotes();
    for (const NoteState &s : noteStates) {
        if (s.index >= 0 && s.index < notes.size()) {
            notes[s.index].setStartTime(s.startTime);
            notes[s.index].setDuration(s.duration);
            notes[s.index].setRenderDirty(true);
        }
    }

    // Apply scale-change markers
    m_canvas->clearScaleChanges();
    for (auto it = scaleChanges.constBegin(); it != scaleChanges.constEnd(); ++it)
        m_canvas->addScaleChange(it.key(), it.value().first, it.value().second);

    // Apply tempo-change markers
    m_canvas->clearTempoChanges();
    for (auto it = tempoChanges.constBegin(); it != tempoChanges.constEnd(); ++it)
        m_canvas->addTempoChange(it.key(), it.value());

    m_canvas->getPhrase().markDirty();
    m_canvas->update();
}

void SetDefaultTempoCommand::undo()
{
    applyState(false);
    qDebug() << "Undo: Default tempo restored to" << m_oldTempo;
}

void SetDefaultTempoCommand::redo()
{
    applyState(true);
    m_firstTime = false;
    qDebug() << "Redo: Default tempo set to" << m_newTempo;
}

// ============================================================================
// Add Tempo Change Command
// ============================================================================

AddTempoChangeCommand::AddTempoChangeCommand(ScoreCanvas *canvas, double timeMs,
                                             const TempoTimeSignature &newTts,
                                             QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_canvas(canvas)
    , m_timeMs(timeMs)
    , m_newTts(newTts)
    , m_hadPrevious(false)
{
    setText("Add Time Signature Change");
    // Check if a marker already exists at this position
    const QMap<double, TempoTimeSignature> &changes = canvas->getTempoChanges();
    auto it = changes.find(timeMs);
    if (it != changes.end()) {
        m_hadPrevious = true;
        m_previousTts = it.value();
    }
}

void AddTempoChangeCommand::undo()
{
    if (m_hadPrevious)
        m_canvas->addTempoChange(m_timeMs, m_previousTts);
    else
        m_canvas->removeTempoChange(m_timeMs);
}

void AddTempoChangeCommand::redo()
{
    m_canvas->addTempoChange(m_timeMs, m_newTts);
}

// ============================================================================
// Remove Tempo Change Command
// ============================================================================

RemoveTempoChangeCommand::RemoveTempoChangeCommand(ScoreCanvas *canvas, double timeMs,
                                                   QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_canvas(canvas)
    , m_timeMs(timeMs)
    , m_valid(false)
{
    setText("Remove Time Signature Change");
    const QMap<double, TempoTimeSignature> &changes = canvas->getTempoChanges();
    auto it = changes.find(timeMs);
    if (it != changes.end()) {
        m_valid = true;
        m_savedTts = it.value();
    }
}

void RemoveTempoChangeCommand::undo()
{
    if (m_valid)
        m_canvas->addTempoChange(m_timeMs, m_savedTts);
}

void RemoveTempoChangeCommand::redo()
{
    m_canvas->removeTempoChange(m_timeMs);
}

// ============================================================================
// FadeOutNotesCommand
// ============================================================================

FadeOutNotesCommand::FadeOutNotesCommand(Phrase *phrase,
                                         const QVector<int> &noteIndices,
                                         double startTime,
                                         double endValue,
                                         ScoreCanvas *canvas,
                                         QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndices(noteIndices)
    , m_startTime(qBound(0.0, startTime, 1.0))
    , m_endValue(qBound(0.0, endValue, 1.0))
    , m_canvas(canvas)
{
    setText("Fade Out Notes");
    QVector<Note> &notes = m_phrase->getNotes();
    for (int idx : m_noteIndices) {
        if (idx >= 0 && idx < notes.size())
            m_oldCurves.append(notes[idx].getDynamicsCurve());
        else
            m_oldCurves.append(Curve{});
    }
}

void FadeOutNotesCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (int i = 0; i < m_noteIndices.size() && i < m_oldCurves.size(); ++i) {
        int idx = m_noteIndices[i];
        if (idx >= 0 && idx < notes.size()) {
            notes[idx].setDynamicsCurve(m_oldCurves[i]);
            notes[idx].setRenderDirty(true);
        }
    }
    m_canvas->update();
    emit m_canvas->notesChanged();
}

void FadeOutNotesCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (int idx : m_noteIndices) {
        if (idx < 0 || idx >= notes.size()) continue;
        Note &note = notes[idx];
        const Curve &old = note.getDynamicsCurve();

        Curve newCurve;
        // Copy all existing points strictly before startTime
        for (const Curve::Point &pt : old.getPoints()) {
            if (pt.time < m_startTime)
                newCurve.addPoint(pt.time, pt.value, 1.0);
        }
        // Splice point at startTime using the existing curve's value there
        double valueAtStart = old.isEmpty() ? 1.0 : old.valueAt(m_startTime);
        newCurve.addPoint(m_startTime, qBound(0.0, valueAtStart, 1.0), 1.0);
        // Final fade-out point
        newCurve.addPoint(1.0, m_endValue, 1.0);

        note.setDynamicsCurve(newCurve);
        note.setRenderDirty(true);
    }
    m_canvas->update();
    emit m_canvas->notesChanged();
}

// ============================================================================
// Snap To Scale Command
// ============================================================================

SnapToScaleCommand::SnapToScaleCommand(Phrase *phrase,
                                       const QVector<int> &noteIndices,
                                       ScoreCanvas *canvas,
                                       QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_canvas(canvas)
    , m_firstTime(true)
{
    setText("Snap to Scale");

    const QVector<Note> &notes = m_phrase->getNotes();
    for (int idx : noteIndices) {
        if (idx < 0 || idx >= notes.size()) continue;
        const Note &note = notes[idx];
        if (!note.hasPitchCurve()) continue;  // Only continuous notes

        NoteState state;
        state.index         = idx;
        state.oldPitchCurve = note.getPitchCurve();
        state.wasQuantized  = note.isQuantized();
        state.oldSegments   = note.getSegments();
        state.newPitchCurve = canvas->quantizePitchCurveToScale(note.getPitchCurve(), note);
        m_noteStates.append(state);
    }
}

void SnapToScaleCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (const NoteState &state : m_noteStates) {
        if (state.index < 0 || state.index >= notes.size()) continue;
        Note &note = notes[state.index];
        note.setPitchCurve(state.oldPitchCurve);
        note.setQuantized(state.wasQuantized);
        note.getSegments() = state.oldSegments;
        note.setRenderDirty(true);
    }
    m_canvas->update();
    emit m_canvas->notesChanged();
}

void SnapToScaleCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (const NoteState &state : m_noteStates) {
        if (state.index < 0 || state.index >= notes.size()) continue;
        Note &note = notes[state.index];
        note.setPitchCurve(state.newPitchCurve);
        note.setQuantized(true);
        note.detectSegments();
        note.setRenderDirty(true);
    }
    m_canvas->update();
    if (!m_firstTime)
        emit m_canvas->notesChanged();
    m_firstTime = false;
}

// ============================================================================
// Edit Note Property Command
// ============================================================================

EditNotePropertyCommand::EditNotePropertyCommand(Phrase *phrase,
                                                  const QVector<NoteChange> &changes,
                                                  Property property,
                                                  ScoreCanvas *canvas,
                                                  QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_changes(changes)
    , m_property(property)
    , m_canvas(canvas)
{
    static const char* names[] = { "Start", "Duration", "Pitch", "Variation" };
    setText(QString("Edit Note %1").arg(names[property]));
}

void EditNotePropertyCommand::applyValues(bool useNew)
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (const NoteChange &c : m_changes) {
        if (c.index < 0 || c.index >= notes.size()) continue;
        double v = useNew ? c.newValue : c.oldValue;
        switch (m_property) {
        case StartTime:     notes[c.index].setStartTime(v);  break;
        case Duration:      notes[c.index].setDuration(v);   break;
        case Pitch:         notes[c.index].setPitchHz(v);    break;
        case VariationIndex: notes[c.index].setVariationIndex(static_cast<int>(v)); break;
        }
        notes[c.index].setRenderDirty(true);
    }
    m_canvas->update();
}

void EditNotePropertyCommand::undo()
{
    applyValues(false);
    qDebug() << "Undo: EditNoteProperty" << id();
}

void EditNotePropertyCommand::redo()
{
    applyValues(true);
    qDebug() << "Redo: EditNoteProperty" << id();
}

bool EditNotePropertyCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id())
        return false;
    const EditNotePropertyCommand *cmd = static_cast<const EditNotePropertyCommand*>(other);
    if (cmd->m_property != m_property)
        return false;
    // Adopt new values from the incoming command; preserve old values from this one.
    // Build a map from note index to old value.
    QMap<int, double> oldValMap;
    for (const NoteChange &c : m_changes)
        oldValMap[c.index] = c.oldValue;
    m_changes = cmd->m_changes;
    for (NoteChange &c : m_changes) {
        if (oldValMap.contains(c.index))
            c.oldValue = oldValMap[c.index];
    }
    return true;
}

// ============================================================================
// Retrograde Notes Command
// ============================================================================

RetrogradeNotesCommand::RetrogradeNotesCommand(Phrase *phrase, const QVector<Note> &selectedNotes,
                                               double targetTime, int targetTrackIndex,
                                               ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_targetTime(targetTime)
    , m_targetTrackIndex(targetTrackIndex)
    , m_canvas(canvas)
{
    setText("Retrograde");

    if (selectedNotes.isEmpty()) return;

    // Find the selection span: earliest start and latest end
    double selectionStart = selectedNotes[0].getStartTime();
    double selectionEnd = selectedNotes[0].getStartTime() + selectedNotes[0].getDuration();
    for (const Note &note : selectedNotes) {
        double ns = note.getStartTime();
        double ne = ns + note.getDuration();
        if (ns < selectionStart) selectionStart = ns;
        if (ne > selectionEnd) selectionEnd = ne;
    }
    double selectionDuration = selectionEnd - selectionStart;

    // Build retrograded notes: mirror each note's position within the span
    for (const Note &note : selectedNotes) {
        Note retNote = note;
        retNote.regenerateId();

        // Mirror: note that ended at selectionEnd now starts at targetTime
        // newStart = targetTime + (selectionEnd - (noteStart + noteDuration))
        double distFromEnd = selectionEnd - (note.getStartTime() + note.getDuration());
        retNote.setStartTime(m_targetTime + distFromEnd);
        if (note.getTrackIndex() != m_targetTrackIndex) {
            retNote.setVariationIndex(0);
        }
        retNote.setTrackIndex(m_targetTrackIndex);
        retNote.setRenderDirty(true);

        // Reverse internal curves
        retNote.getDynamicsCurve() = reverseCurve(note.getDynamicsCurve());
        if (note.hasPitchCurve()) {
            retNote.getPitchCurve() = reverseCurve(note.getPitchCurve());
        }
        // Reverse additional expressive curves (index 0 is dynamics, already handled)
        for (int i = 1; i < retNote.getExpressiveCurveCount(); ++i) {
            retNote.getExpressiveCurve(i) = reverseCurve(note.getExpressiveCurve(i));
        }

        m_retrogradeNotes.append(retNote);
    }
}

Curve RetrogradeNotesCommand::reverseCurve(const Curve &curve)
{
    const QVector<Curve::Point> &pts = curve.getPoints();
    if (pts.size() <= 1) return curve;

    Curve reversed;
    // Add points in reverse order with mirrored time
    for (int i = pts.size() - 1; i >= 0; --i) {
        reversed.addPoint(1.0 - pts[i].time, pts[i].value, pts[i].pressure);
    }
    return reversed;
}

void RetrogradeNotesCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (int i = m_insertedIndices.size() - 1; i >= 0; --i) {
        int index = m_insertedIndices[i];
        if (index >= 0 && index < notes.size()) {
            notes.removeAt(index);
        }
    }
    m_canvas->update();
}

void RetrogradeNotesCommand::redo()
{
    if (m_retrogradeNotes.isEmpty()) return;

    QVector<Note> &phraseNotes = m_phrase->getNotes();
    m_insertedIndices.clear();

    for (const Note &note : m_retrogradeNotes) {
        int insertIndex = phraseNotes.size();
        m_phrase->addNote(note);
        m_insertedIndices.append(insertIndex);
    }

    m_canvas->update();
}

// ============================================================================
// Set Note Curves Batch Command
// ============================================================================

namespace {
// Sample a points array (time/value/curveType per point) into a dense Curve.
// Mirrors the evaluation used by ApplyDynamicsCurveCommand and
// ApplyExpressiveCurveToSelectionCommand so the shape matches existing tools.
static Curve sampleEnvelopeToCurve(const QVector<EnvelopePoint> &env, double weight)
{
    Curve out;
    if (env.isEmpty()) return out;

    auto eval = [&](double t) -> double {
        if (env.size() == 1) return env[0].value;
        if (t <= env.first().time) return env.first().value;
        if (t >= env.last().time)  return env.last().value;
        for (int i = 0; i < env.size() - 1; ++i) {
            if (t >= env[i].time && t <= env[i + 1].time) {
                double segT = (t - env[i].time)
                            / (env[i + 1].time - env[i].time);
                if (env[i].curveType == 1) {
                    double smoothT = (1.0 - std::cos(segT * M_PI)) * 0.5;
                    return env[i].value
                         + smoothT * (env[i + 1].value - env[i].value);
                } else if (env[i].curveType == 2) {
                    return env[i].value;
                } else {
                    return env[i].value
                         + segT * (env[i + 1].value - env[i].value);
                }
            }
        }
        return env.last().value;
    };

    const int SAMPLE_COUNT = 30;
    for (int s = 0; s <= SAMPLE_COUNT; ++s) {
        double t = static_cast<double>(s) / SAMPLE_COUNT;
        double v = qBound(0.0, eval(t) * weight, 1.0);
        out.addPoint(t, v, 1.0);
    }
    return out;
}
} // anonymous namespace

SetNoteCurvesBatchCommand::SetNoteCurvesBatchCommand(
    Phrase *phrase,
    const QVector<int> &noteIndices,
    const QVector<QVector<EnvelopePoint>> &pointsPerNote,
    const QString &name,
    double weight,
    ScoreCanvas *canvas,
    QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndices(noteIndices)
    , m_pointsPerNote(pointsPerNote)
    , m_name(name.isEmpty() ? QStringLiteral("Dynamics") : name)
    , m_weight(weight)
    , m_isDynamics(m_name == QStringLiteral("Dynamics"))
    , m_firstTime(true)
    , m_canvas(canvas)
{
    setText(m_isDynamics ? "Set Dynamics Curves (batch)"
                         : QString("Set '%1' Curves (batch)").arg(m_name));
}

void SetNoteCurvesBatchCommand::redo()
{
    if (m_noteIndices.isEmpty() || m_pointsPerNote.isEmpty()) return;
    if (m_noteIndices.size() != m_pointsPerNote.size()) return;

    QVector<Note> &notes = m_phrase->getNotes();

    if (m_firstTime) {
        m_priorStates.clear();
        m_priorStates.reserve(m_noteIndices.size());
        for (int idx : m_noteIndices) {
            PriorState ps;
            ps.noteIdx = idx;
            ps.hadCurve = false;
            if (idx < 0 || idx >= notes.size()) {
                m_priorStates.append(ps);
                continue;
            }
            if (m_isDynamics) {
                ps.oldCurve = notes[idx].getDynamicsCurve();
                ps.hadCurve = true;  // dynamics always present
            } else {
                int existingIdx = notes[idx].findExpressiveCurveIndexByName(m_name);
                if (existingIdx >= 1) {
                    ps.hadCurve = true;
                    ps.oldCurve = notes[idx].getExpressiveCurve(existingIdx);
                }
            }
            m_priorStates.append(ps);
        }
        m_firstTime = false;
    }

    for (int i = 0; i < m_noteIndices.size(); ++i) {
        int idx = m_noteIndices[i];
        if (idx < 0 || idx >= notes.size()) continue;
        const QVector<EnvelopePoint> &env = m_pointsPerNote[i];
        if (env.isEmpty()) continue;
        Curve shaped = sampleEnvelopeToCurve(env, m_weight);
        if (m_isDynamics) {
            notes[idx].setDynamicsCurve(shaped);
        } else {
            notes[idx].upsertExpressiveCurve(m_name, shaped);
        }
        notes[idx].setRenderDirty(true);
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Redo: Set Note Curves Batch '" << m_name << "' on"
             << m_noteIndices.size() << "notes";
}

void SetNoteCurvesBatchCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (const PriorState &ps : m_priorStates) {
        int idx = ps.noteIdx;
        if (idx < 0 || idx >= notes.size()) continue;
        if (m_isDynamics) {
            notes[idx].setDynamicsCurve(ps.oldCurve);
        } else if (ps.hadCurve) {
            notes[idx].upsertExpressiveCurve(m_name, ps.oldCurve);
        } else {
            notes[idx].removeExpressiveCurveByName(m_name);
        }
        notes[idx].setRenderDirty(true);
    }
    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Undo: Set Note Curves Batch '" << m_name << "' on"
             << m_priorStates.size() << "notes";
}

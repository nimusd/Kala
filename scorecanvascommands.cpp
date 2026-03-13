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

    // Store original dynamics curves for undo
    QVector<Note> &notes = m_phrase->getNotes();
    for (int idx : m_noteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            m_oldDynamicsCurves.append(notes[idx].getDynamicsCurve());
        }
    }
}

void ApplyDynamicsCurveCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    // Restore original dynamics curves
    for (int i = 0; i < m_noteIndices.size() && i < m_oldDynamicsCurves.size(); ++i) {
        int idx = m_noteIndices[i];
        if (idx >= 0 && idx < notes.size()) {
            notes[idx].setDynamicsCurve(m_oldDynamicsCurves[i]);
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

            Curve newCurve;
            for (int s = 0; s <= SAMPLE_COUNT; ++s) {
                double t = static_cast<double>(s) / SAMPLE_COUNT;
                double curveVal   = evaluateCurve(t);
                double existingVal = existing.isEmpty() ? 0.5 : existing.valueAt(t);
                double newVal = existingVal + (curveVal - existingVal) * m_weight;
                newCurve.addPoint(t, qBound(0.0, newVal, 1.0), 1.0);
            }

            note.setDynamicsCurve(newCurve);
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

            Curve newCurve;
            for (int s = 0; s <= SAMPLE_COUNT; ++s) {
                double t  = static_cast<double>(s) / SAMPLE_COUNT;
                double cv = evaluateCurve(t);
                double existingVal = existingCurve.isEmpty() ? 0.5 : existingCurve.valueAt(t);
                double newVal = existingVal + (cv - existingVal) * m_weight;
                newCurve.addPoint(t, qBound(0.0, newVal, 1.0), 1.0);
            }
            note.setDynamicsCurve(newCurve);
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
        // cross-track paste work.  Variation indices are per-track, so reset to
        // base sounit (0) to avoid an invalid index on the destination track.
        pastedNote.setTrackIndex(m_targetTrackIndex);
        pastedNote.setVariationIndex(0);
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

    // Remove the merged note (it's at the first note's original index)
    int mergedIdx = m_originalNotes.first().index;
    if (mergedIdx >= 0 && mergedIdx < notes.size()) {
        notes.removeAt(mergedIdx);
    }

    // Re-insert all original notes in ascending index order
    for (const OriginalNote &orig : m_originalNotes) {
        int idx = qMin(orig.index, notes.size());
        notes.insert(idx, orig.note);
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Undo: Restored" << m_originalNotes.size() << "notes from legato merge";
}

void LinkAsLegatoCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    if (m_originalNotes.size() < 2) return;

    // Remove all original notes in descending index order (to preserve indices)
    for (int i = m_originalNotes.size() - 1; i >= 0; --i) {
        int idx = m_originalNotes[i].index;
        if (idx >= 0 && idx < notes.size()) {
            notes.removeAt(idx);
        }
    }

    // Insert the merged note at the first note's original index
    int insertIdx = qMin(m_originalNotes.first().index, notes.size());
    notes.insert(insertIdx, m_mergedNote);

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
                Curve splitPitch;
                // Anchor start and end so the curve always covers [0, 1]
                splitPitch.addPoint(0.0, srcPitch.valueAt(seg.startTime), 1.0);
                for (const auto &pt : srcPitch.getPoints()) {
                    if (pt.time > seg.startTime && pt.time < seg.endTime) {
                        splitPitch.addPoint((pt.time - seg.startTime) / segSpan,
                                            pt.value, pt.pressure);
                    }
                }
                splitPitch.addPoint(1.0, srcPitch.valueAt(seg.endTime), 1.0);
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
// Add Expressive Curve Command
// ============================================================================

AddExpressiveCurveCommand::AddExpressiveCurveCommand(Phrase *phrase,
                                                      const QVector<int> &noteIndices,
                                                      const QString &name,
                                                      const QVector<EnvelopePoint> &envelopeCurve,
                                                      double weight,
                                                      bool perNote,
                                                      ScoreCanvas *canvas,
                                                      QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndices(noteIndices)
    , m_name(name)
    , m_envelopeCurve(envelopeCurve)
    , m_weight(weight)
    , m_perNote(perNote)
    , m_canvas(canvas)
{
    setText("Add Expressive Curve");
}

Curve AddExpressiveCurveCommand::evaluateToCurve(double noteStart, double noteDuration,
                                                  double selStart, double selDuration) const
{
    // Reuse the same evaluation logic as ApplyDynamicsCurveCommand
    auto evaluateCurve = [this](double t) -> double {
        if (m_envelopeCurve.isEmpty()) return 1.0;
        if (m_envelopeCurve.size() == 1) return m_envelopeCurve[0].value;
        if (t <= m_envelopeCurve.first().time) return m_envelopeCurve.first().value;
        if (t >= m_envelopeCurve.last().time)  return m_envelopeCurve.last().value;
        for (int i = 0; i < m_envelopeCurve.size() - 1; ++i) {
            if (t >= m_envelopeCurve[i].time && t <= m_envelopeCurve[i + 1].time) {
                double segT = (t - m_envelopeCurve[i].time) / (m_envelopeCurve[i + 1].time - m_envelopeCurve[i].time);
                if (m_envelopeCurve[i].curveType == 1) {
                    double smoothT = (1.0 - std::cos(segT * M_PI)) * 0.5;
                    return m_envelopeCurve[i].value + smoothT * (m_envelopeCurve[i + 1].value - m_envelopeCurve[i].value);
                } else if (m_envelopeCurve[i].curveType == 2) {
                    return m_envelopeCurve[i].value;
                } else {
                    return m_envelopeCurve[i].value + segT * (m_envelopeCurve[i + 1].value - m_envelopeCurve[i].value);
                }
            }
        }
        return m_envelopeCurve.last().value;
    };

    const int SAMPLE_COUNT = 20;
    Curve result;

    if (m_perNote || m_noteIndices.size() == 1 || selDuration <= 0.0) {
        // Per-note (or single note): curve applied across entire note duration
        for (int s = 0; s <= SAMPLE_COUNT; ++s) {
            double t = static_cast<double>(s) / SAMPLE_COUNT;
            double val = qBound(0.0, evaluateCurve(t) * m_weight, 1.0);
            result.addPoint(t, val, 1.0);
        }
    } else {
        // Multi-note: curve spans whole selection, note gets value at its center
        double noteCenter = noteStart + noteDuration / 2.0;
        double normalizedPos = qBound(0.0, (noteCenter - selStart) / selDuration, 1.0);
        double targetVal = qBound(0.0, evaluateCurve(normalizedPos) * m_weight, 1.0);
        result.addPoint(0.0, targetVal, 1.0);
        result.addPoint(1.0, targetVal, 1.0);
    }

    return result;
}

void AddExpressiveCurveCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    for (int i = 0; i < m_noteIndices.size(); ++i) {
        int idx = m_noteIndices[i];
        if (idx >= 0 && idx < notes.size() && i < m_addedAtIndex.size()) {
            notes[idx].removeExpressiveCurve(m_addedAtIndex[i]);
        }
    }
    m_canvas->update();
    qDebug() << "Undo: Removed expressive curve from" << m_noteIndices.size() << "notes";
}

void AddExpressiveCurveCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();
    m_addedAtIndex.clear();

    // Calculate selection time span for multi-note mode
    double selStart = std::numeric_limits<double>::max();
    double selEnd   = std::numeric_limits<double>::lowest();
    for (int idx : m_noteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            selStart = qMin(selStart, notes[idx].getStartTime());
            selEnd   = qMax(selEnd,   notes[idx].getStartTime() + notes[idx].getDuration());
        }
    }
    double selDuration = (selEnd > selStart) ? (selEnd - selStart) : 0.0;

    for (int idx : m_noteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            Note &note = notes[idx];
            int addedAt = note.getExpressiveCurveCount();
            Curve c = evaluateToCurve(note.getStartTime(), note.getDuration(), selStart, selDuration);
            note.addExpressiveCurve(m_name, c);
            m_addedAtIndex.append(addedAt);
        }
    }

    m_canvas->update();
    qDebug() << "Redo: Added expressive curve '" << m_name << "' to" << m_noteIndices.size() << "notes";
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
    Phrase *phrase, const QString &curveName, ScoreCanvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_curveName(curveName)
    , m_canvas(canvas)
{
    setText(QString("Delete Curve \"%1\" from All Notes").arg(curveName));

    // Scan all notes and save those that carry this curve
    const QVector<Note> &notes = m_phrase->getNotes();
    for (int i = 0; i < notes.size(); ++i) {
        for (int j = 1; j < notes[i].getExpressiveCurveCount(); ++j) {
            if (notes[i].getExpressiveCurveName(j) == curveName) {
                m_saved.append({i, notes[i].getExpressiveCurve(j)});
                break;
            }
        }
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
// Apply Expressive Curve Shape Command
// ============================================================================

ApplyExpressiveCurveShapeCommand::ApplyExpressiveCurveShapeCommand(
    Phrase *phrase,
    const QVector<int> &noteIndices,
    const QString &curveName,
    const QVector<EnvelopePoint> &envelope,
    double weight,
    ScoreCanvas *canvas,
    QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_phrase(phrase)
    , m_noteIndices(noteIndices)
    , m_curveName(curveName)
    , m_envelope(envelope)
    , m_weight(weight)
    , m_firstTime(true)
    , m_canvas(canvas)
{
    setText("Apply Expressive Curve Shape");
}

void ApplyExpressiveCurveShapeCommand::undo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    for (const SavedState &state : m_savedStates) {
        if (state.curveIdx < 0) continue;  // note was skipped on redo
        int idx = state.noteIdx;
        if (idx < 0 || idx >= notes.size()) continue;
        Note &note = notes[idx];
        if (state.curveIdx >= 1 && state.curveIdx < note.getExpressiveCurveCount()) {
            note.getExpressiveCurve(state.curveIdx) = state.oldCurve;
            note.setRenderDirty(true);
        }
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Undo: Restored expressive curve '" << m_curveName
             << "' on" << m_savedStates.size() << "notes";
}

void ApplyExpressiveCurveShapeCommand::redo()
{
    QVector<Note> &notes = m_phrase->getNotes();

    if (m_noteIndices.isEmpty() || m_envelope.isEmpty()) return;

    // Evaluate the envelope shape at normalized time t in [0,1]
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

    // Calculate selection time span
    double selStart = std::numeric_limits<double>::max();
    double selEnd   = std::numeric_limits<double>::lowest();
    for (int idx : m_noteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            selStart = qMin(selStart, notes[idx].getStartTime());
            selEnd   = qMax(selEnd,   notes[idx].getStartTime() + notes[idx].getDuration());
        }
    }
    double selDuration = selEnd - selStart;
    if (selDuration <= 0.0) return;

    if (m_firstTime) {
        m_savedStates.clear();

        for (int idx : m_noteIndices) {
            if (idx < 0 || idx >= notes.size()) continue;
            const Note &note = notes[idx];

            SavedState state;
            state.noteIdx  = idx;
            state.curveIdx = -1;

            // Find the named curve
            for (int i = 1; i < note.getExpressiveCurveCount(); ++i) {
                if (note.getExpressiveCurveName(i) == m_curveName) {
                    state.curveIdx = i;
                    state.oldCurve = note.getExpressiveCurve(i);
                    break;
                }
            }

            m_savedStates.append(state);
        }

        m_firstTime = false;
    }

    // Apply the scaled curve to each note that has the named curve
    for (const SavedState &state : m_savedStates) {
        if (state.curveIdx < 0) continue;  // note doesn't carry this curve
        int idx = state.noteIdx;
        if (idx < 0 || idx >= notes.size()) continue;
        Note &note = notes[idx];
        if (state.curveIdx < 1 || state.curveIdx >= note.getExpressiveCurveCount()) continue;

        // Normalised position of this note's centre within the selection
        double noteCenter    = note.getStartTime() + note.getDuration() / 2.0;
        double normalizedPos = qBound(0.0, (noteCenter - selStart) / selDuration, 1.0);
        double curveValue    = evalEnvelope(normalizedPos);

        // scale = 1.0 when curveValue=1 (no change), blended by weight
        double scale = 1.0 + m_weight * (curveValue - 1.0);

        const Curve &existingCurve = note.getExpressiveCurve(state.curveIdx);
        const QVector<Curve::Point> &pts = existingCurve.getPoints();
        Curve newCurve;

        if (pts.isEmpty()) {
            double val = qBound(0.0, 0.7 * scale, 1.0);
            newCurve.addPoint(0.0, val, 1.0);
            newCurve.addPoint(1.0, val, 1.0);
        } else {
            for (const Curve::Point &pt : pts) {
                newCurve.addPoint(pt.time, qBound(0.0, pt.value * scale, 1.0), pt.pressure);
            }
        }

        note.getExpressiveCurve(state.curveIdx) = newCurve;
        note.setRenderDirty(true);
    }

    m_canvas->update();
    emit m_canvas->notesChanged();
    qDebug() << "Redo: Applied expressive curve shape to '" << m_curveName
             << "' (weight=" << m_weight << ") across" << m_noteIndices.size() << "notes";
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
        if (idx >= 0 && idx < notes.size())
            notes[idx].setDynamics(m_newDynamics[i]);
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

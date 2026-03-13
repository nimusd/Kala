#ifndef SCORECANVASCOMMANDS_H
#define SCORECANVASCOMMANDS_H

#include <QUndoCommand>
#include "note.h"
#include "curve.h"
#include "phrase.h"
#include "easing.h"
#include "scale.h"
#include "envelopelibraryDialog.h"

#include "scorecanvas.h"
#include "vibrato.h"
#include "tempotimesignature.h"

// ============================================================================
// Add Note Command
// ============================================================================
class AddNoteCommand : public QUndoCommand
{
public:
    AddNoteCommand(Phrase *phrase, const Note &note, ScoreCanvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    Note m_note;
    ScoreCanvas *m_canvas;
    bool m_firstTime;
};

// ============================================================================
// Delete Note Command
// ============================================================================
class DeleteNoteCommand : public QUndoCommand
{
public:
    DeleteNoteCommand(Phrase *phrase, int noteIndex, ScoreCanvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    Note m_note;
    int m_noteIndex;
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Move Note Command
// ============================================================================
class MoveNoteCommand : public QUndoCommand
{
public:
    MoveNoteCommand(Phrase *phrase, int noteIndex, double oldStartTime, double oldPitch,
                    double newStartTime, double newPitch,
                    const Curve &oldPitchCurve, const Curve &newPitchCurve,
                    bool hasPitchCurve, ScoreCanvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    int id() const override { return 1; }  // For command merging
    bool mergeWith(const QUndoCommand *other) override;

private:
    Phrase *m_phrase;
    int m_noteIndex;
    double m_oldStartTime;
    double m_oldPitch;
    double m_newStartTime;
    double m_newPitch;
    Curve m_oldPitchCurve;
    Curve m_newPitchCurve;
    bool m_hasPitchCurve;
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Resize Note Command
// ============================================================================
class ResizeNoteCommand : public QUndoCommand
{
public:
    ResizeNoteCommand(Phrase *phrase, int noteIndex,
                     double oldStartTime, double oldDuration,
                     double newStartTime, double newDuration,
                     ScoreCanvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    int id() const override { return 2; }  // For command merging
    bool mergeWith(const QUndoCommand *other) override;

private:
    Phrase *m_phrase;
    int m_noteIndex;
    double m_oldStartTime;
    double m_oldDuration;
    double m_newStartTime;
    double m_newDuration;
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Edit Curve Command
// ============================================================================
class EditCurveCommand : public QUndoCommand
{
public:
    enum CurveType {
        DynamicsCurve,
        BottomCurve,
        ExpressiveCurveN  // Additional expressive curve at m_curveIndex
    };

    EditCurveCommand(Phrase *phrase, int noteIndex, CurveType curveType,
                    const Curve &oldCurve, const Curve &newCurve,
                    ScoreCanvas *canvas, QUndoCommand *parent = nullptr);

    // Overload for ExpressiveCurveN (additional curve at curveIndex >= 1)
    EditCurveCommand(Phrase *phrase, int noteIndex, CurveType curveType,
                    int curveIndex,
                    const Curve &oldCurve, const Curve &newCurve,
                    ScoreCanvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    int id() const override { return 3; }  // For command merging
    bool mergeWith(const QUndoCommand *other) override;

private:
    Phrase *m_phrase;
    int m_noteIndex;
    CurveType m_curveType;
    int m_curveIndex = 0;  // Used when m_curveType == ExpressiveCurveN
    Curve m_oldCurve;
    Curve m_newCurve;
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Delete Multiple Notes Command (for multi-selection delete)
// ============================================================================
class DeleteMultipleNotesCommand : public QUndoCommand
{
public:
    DeleteMultipleNotesCommand(Phrase *phrase, const QVector<int> &noteIndices, ScoreCanvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    QVector<QPair<int, Note>> m_notesWithIndices;  // Store notes with their original indices
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Move Multiple Notes Command (for multi-selection drag)
// ============================================================================
class MoveMultipleNotesCommand : public QUndoCommand
{
public:
    struct NoteState {
        int index;
        double startTime;
        double pitch;
        Curve pitchCurve;
        bool hasPitchCurve;
    };

    MoveMultipleNotesCommand(Phrase *phrase, const QVector<int> &noteIndices,
                            const QVector<NoteState> &oldStates,
                            const QVector<NoteState> &newStates,
                            ScoreCanvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    QVector<NoteState> m_oldStates;
    QVector<NoteState> m_newStates;
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Apply Dynamics Curve Command
// ============================================================================
class ApplyDynamicsCurveCommand : public QUndoCommand
{
public:
    ApplyDynamicsCurveCommand(Phrase *phrase,
                               const QVector<int> &noteIndices,
                               const QVector<EnvelopePoint> &curve,
                               double weight,
                               bool perNote,
                               ScoreCanvas *canvas,
                               QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    QVector<int> m_noteIndices;
    QVector<EnvelopePoint> m_curve;
    double m_weight;  // 0.0-2.0, where 1.0 is neutral
    bool m_perNote;   // true: curve applied per-note; false: across whole selection
    QVector<Curve> m_oldDynamicsCurves;  // For undo
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Paste Notes Command
// ============================================================================
class PasteNotesCommand : public QUndoCommand
{
public:
    PasteNotesCommand(Phrase *phrase, const QVector<Note> &notes,
                     double targetTime, int targetTrackIndex, ScoreCanvas *canvas,
                     QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    // Returns indices of pasted notes (for selection)
    const QVector<int>& getPastedIndices() const { return m_pastedIndices; }

private:
    Phrase *m_phrase;
    QVector<Note> m_notes;          // Notes to paste
    double m_targetTime;            // Time position to paste at
    int m_targetTrackIndex;         // Track to paste into (active track at paste time)
    QVector<int> m_pastedIndices;   // Indices where notes were inserted
    ScoreCanvas *m_canvas;
    bool m_firstTime;
};

// ============================================================================
// Apply Rhythmic Easing Command
// ============================================================================
class ApplyRhythmicEasingCommand : public QUndoCommand
{
public:
    struct NoteTimeState {
        int index;
        double oldStartTime;
        double newStartTime;
    };

    // Anchor mode determines which notes stay fixed
    enum AnchorMode {
        AnchorNone,   // All notes can move
        AnchorFirst,  // First note stays fixed
        AnchorLast,   // Last note stays fixed
        AnchorBoth    // First and last notes stay fixed
    };

    ApplyRhythmicEasingCommand(Phrase *phrase, const QVector<int> &noteIndices,
                               const Easing &easing, AnchorMode anchorMode,
                               double weight, ScoreCanvas *canvas,
                               QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    QVector<NoteTimeState> m_noteStates;
    Easing m_easing;
    AnchorMode m_anchorMode;
    double m_weight;
    ScoreCanvas *m_canvas;

    void calculateNewTimes(const QVector<int> &noteIndices);
};

// ============================================================================
// Split Note At Segment Command (for deleting a segment)
// ============================================================================
class SplitNoteAtSegmentCommand : public QUndoCommand
{
public:
    SplitNoteAtSegmentCommand(Phrase *phrase, int noteIndex, int segmentIndex,
                               ScoreCanvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    int m_noteIndex;
    int m_segmentIndex;
    Note m_originalNote;
    Note m_noteBefore;   // Segments before deleted one
    Note m_noteAfter;    // Segments after deleted one
    bool m_hasBefore;    // Whether there are segments before
    bool m_hasAfter;     // Whether there are segments after
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Detach Segment Command (extract segment as discrete note)
// ============================================================================
class DetachSegmentCommand : public QUndoCommand
{
public:
    DetachSegmentCommand(Phrase *phrase, int noteIndex, int segmentIndex,
                          ScoreCanvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    int m_noteIndex;
    int m_segmentIndex;
    Note m_originalNote;
    Note m_noteBefore;      // Continuous note with segments before
    Note m_detachedNote;    // Discrete note (the detached segment)
    Note m_noteAfter;       // Continuous note with segments after
    bool m_hasBefore;
    bool m_hasAfter;
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Change Segment Pitch Command
// ============================================================================
class ChangeSegmentPitchCommand : public QUndoCommand
{
public:
    ChangeSegmentPitchCommand(Phrase *phrase, int noteIndex, int segmentIndex,
                               double newPitchHz, ScoreCanvas *canvas,
                               QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    int m_noteIndex;
    int m_segmentIndex;
    double m_oldPitchHz;
    double m_newPitchHz;
    Curve m_oldPitchCurve;
    Curve m_newPitchCurve;
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Link As Legato Command (merge notes into one continuous note)
// ============================================================================
class LinkAsLegatoCommand : public QUndoCommand
{
public:
    LinkAsLegatoCommand(Phrase *phrase, const QVector<int> &noteIndices,
                        ScoreCanvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    struct OriginalNote {
        int index;
        Note note;
    };

    Phrase *m_phrase;
    QVector<OriginalNote> m_originalNotes;  // All selected notes sorted by start time
    Note m_mergedNote;                      // The resulting merged note
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Unlink Legato Command (split merged note back into discrete notes)
// ============================================================================
class UnlinkLegatoCommand : public QUndoCommand
{
public:
    UnlinkLegatoCommand(Phrase *phrase, const QVector<int> &noteIndices,
                        ScoreCanvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    struct SplitEntry {
        int originalIndex;
        Note originalNote;
        QVector<Note> splitNotes;
    };

    Phrase *m_phrase;
    QVector<SplitEntry> m_splits;  // Sorted by descending original index
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Scale Timing Command (for tempo change on selected notes)
// ============================================================================
class ScaleTimingCommand : public QUndoCommand
{
public:
    struct NoteTimingState {
        int index;
        double startTime;
        double duration;
    };

    ScaleTimingCommand(Phrase *phrase, const QVector<int> &noteIndices,
                       double proportion, ScoreCanvas *canvas,
                       QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    QVector<NoteTimingState> m_oldStates;
    QVector<NoteTimingState> m_newStates;
    double m_proportion;
    ScoreCanvas *m_canvas;

    void calculateNewTimes(const QVector<int> &noteIndices);
};

// ============================================================================
// Make Notes Continuous Command (convert discrete notes to continuous)
// ============================================================================
class MakeNotesContinuousCommand : public QUndoCommand
{
public:
    MakeNotesContinuousCommand(Phrase *phrase, const QVector<int> &noteIndices,
                               ScoreCanvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    struct NoteState {
        int index;
        Curve oldPitchCurve;
        bool hadPitchCurve;
        bool wasQuantized;
        QVector<Segment> oldSegments;
    };

    Phrase *m_phrase;
    QVector<NoteState> m_noteStates;
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Make Notes Discrete Command (convert continuous notes back to discrete)
// ============================================================================
class MakeNotesDiscreteCommand : public QUndoCommand
{
public:
    MakeNotesDiscreteCommand(Phrase *phrase, const QVector<int> &noteIndices,
                             ScoreCanvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    struct NoteState {
        int index;
        Curve oldPitchCurve;
        bool wasQuantized;
        QVector<Segment> oldSegments;
        double oldPitchHz;
        double newPitchHz;  // snapped pitch to restore on redo
    };

    Phrase *m_phrase;
    QVector<NoteState> m_noteStates;
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Add Scale Change Command
// ============================================================================
class AddScaleChangeCommand : public QUndoCommand
{
public:
    AddScaleChangeCommand(ScoreCanvas *canvas, double timeMs,
                          const Scale &scale, double baseFreq,
                          QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    ScoreCanvas *m_canvas;
    double m_timeMs;
    Scale m_scale;
    double m_baseFreq;
    QVector<ScoreCanvas::NotePitchChange> m_pitchChanges;
};

// ============================================================================
// Edit Scale Change Command
// ============================================================================
class EditScaleChangeCommand : public QUndoCommand
{
public:
    EditScaleChangeCommand(ScoreCanvas *canvas, double timeMs,
                           const Scale &oldScale, double oldBaseFreq,
                           const Scale &newScale, double newBaseFreq,
                           bool isDefault, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    ScoreCanvas *m_canvas;
    double m_timeMs;
    Scale m_oldScale;
    double m_oldBaseFreq;
    Scale m_newScale;
    double m_newBaseFreq;
    bool m_isDefault;
    QVector<ScoreCanvas::NotePitchChange> m_pitchChanges;
};

// ============================================================================
// Delete Scale Change Command
// ============================================================================
class DeleteScaleChangeCommand : public QUndoCommand
{
public:
    DeleteScaleChangeCommand(ScoreCanvas *canvas, double timeMs,
                             const Scale &oldScale, double oldBaseFreq,
                             QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    ScoreCanvas *m_canvas;
    double m_timeMs;
    Scale m_oldScale;
    double m_oldBaseFreq;
};

// ============================================================================
// Add Expressive Curve Command
// ============================================================================
class AddExpressiveCurveCommand : public QUndoCommand
{
public:
    AddExpressiveCurveCommand(Phrase *phrase,
                               const QVector<int> &noteIndices,
                               const QString &name,
                               const QVector<EnvelopePoint> &envelopeCurve,
                               double weight,
                               bool perNote,
                               ScoreCanvas *canvas,
                               QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    QVector<int> m_noteIndices;
    QString m_name;
    QVector<EnvelopePoint> m_envelopeCurve;
    double m_weight;
    bool m_perNote;
    QVector<int> m_addedAtIndex;  // Index where curve was added per note (for undo)
    ScoreCanvas *m_canvas;

    Curve evaluateToCurve(double noteStart, double noteDuration,
                          double selStart, double selDuration) const;
};

// ============================================================================
// Apply Named Curve Command (reuse an existing named curve on a selection)
// ============================================================================
class ApplyNamedCurveCommand : public QUndoCommand
{
public:
    ApplyNamedCurveCommand(Phrase *phrase,
                            const QVector<int> &noteIndices,
                            const QString &name,
                            const Curve &curve,
                            ScoreCanvas *canvas,
                            QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    struct NoteState {
        int index;
        bool hadCurve;         // Did this note already have a curve with this name?
        int existingCurveIdx;  // If hadCurve, which index held it
        Curve oldCurve;        // Previous curve data (for undo when hadCurve is true)
    };

    Phrase *m_phrase;
    QVector<int> m_noteIndices;
    QString m_name;
    Curve m_curve;
    QVector<NoteState> m_noteStates;  // Populated on first redo
    ScoreCanvas *m_canvas;
    bool m_firstTime;
};

// ============================================================================
// Remove Expressive Curve Command
// ============================================================================
class RemoveExpressiveCurveCommand : public QUndoCommand
{
public:
    RemoveExpressiveCurveCommand(Phrase *phrase,
                                  const QVector<int> &noteIndices,
                                  int curveIndex,
                                  ScoreCanvas *canvas,
                                  QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    QVector<int> m_noteIndices;
    int m_curveIndex;
    struct SavedCurve { QString name; Curve curve; };
    QVector<SavedCurve> m_savedCurves;  // Per-note saved data
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Remove Named Expressive Curve Command
// Removes a curve by name from every note in the phrase that has it.
// ============================================================================
class RemoveNamedExpressiveCurveCommand : public QUndoCommand
{
public:
    RemoveNamedExpressiveCurveCommand(Phrase *phrase,
                                      const QString &curveName,
                                      ScoreCanvas *canvas,
                                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    QString m_curveName;
    struct SavedState { int noteIdx; Curve curve; };
    QVector<SavedState> m_saved;
    ScoreCanvas *m_canvas;
};

// ============================================================================
// Apply Expressive Curve Shape Command
// Applies a dynamics-style envelope shape progressively across a selection,
// scaling the values of a named expressive curve (same multiplier algorithm
// as ApplyDynamicsCurveCommand multi-note mode).  No per-note option.
// ============================================================================
class ApplyExpressiveCurveShapeCommand : public QUndoCommand
{
public:
    ApplyExpressiveCurveShapeCommand(Phrase *phrase,
                                      const QVector<int> &noteIndices,
                                      const QString &curveName,
                                      const QVector<EnvelopePoint> &envelope,
                                      double weight,
                                      ScoreCanvas *canvas,
                                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    QVector<int> m_noteIndices;
    QString m_curveName;
    QVector<EnvelopePoint> m_envelope;
    double m_weight;

    struct SavedState {
        int noteIdx;
        int curveIdx;   // index in note's expressive curve list (-1 = note skipped)
        Curve oldCurve;
    };
    QVector<SavedState> m_savedStates;  // Populated on first redo
    bool m_firstTime;

    ScoreCanvas *m_canvas;
};

// ============================================================================
// Set Beat Dynamics Command
// Sets constant dynamics on a per-note basis (used by apply_beat_dynamics tool).
// Stores old dynamics curves for undo.
// ============================================================================
class SetBeatDynamicsCommand : public QUndoCommand
{
public:
    SetBeatDynamicsCommand(Phrase *phrase,
                           const QVector<int> &noteIndices,
                           const QVector<double> &newDynamics,
                           ScoreCanvas *canvas,
                           QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    QVector<int>    m_noteIndices;
    QVector<double> m_newDynamics;
    QVector<Curve>  m_oldCurves;   // Saved on first redo for undo
    bool            m_firstTime;
    ScoreCanvas    *m_canvas;
};

// ============================================================================
// Set Vibrato Command
// Sets vibrato on a per-note basis. Stores old vibrato per note for undo.
// ============================================================================
class SetVibratoCommand : public QUndoCommand
{
public:
    SetVibratoCommand(Phrase *phrase,
                      const QVector<int> &noteIndices,
                      const Vibrato &newVibrato,
                      ScoreCanvas *canvas,
                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    QVector<int>    m_noteIndices;
    Vibrato         m_newVibrato;
    QVector<Vibrato> m_oldVibratos;   // Saved on first redo for undo
    bool            m_firstTime;
    ScoreCanvas    *m_canvas;
};

// ============================================================================
// Add / Edit Tempo+TimeSig Change Command
// Adds (or replaces) a TempoTimeSignature marker at a given time position.
// Undo restores the previous marker (or removes it if none existed).
// ============================================================================
class AddTempoChangeCommand : public QUndoCommand
{
public:
    AddTempoChangeCommand(ScoreCanvas *canvas, double timeMs,
                          const TempoTimeSignature &newTts,
                          QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ScoreCanvas        *m_canvas;
    double              m_timeMs;
    TempoTimeSignature  m_newTts;
    bool                m_hadPrevious;
    TempoTimeSignature  m_previousTts;
};

// ============================================================================
// Remove Tempo+TimeSig Change Command
// Removes a marker at a given time. Undo re-inserts it.
// ============================================================================
class RemoveTempoChangeCommand : public QUndoCommand
{
public:
    RemoveTempoChangeCommand(ScoreCanvas *canvas, double timeMs,
                             QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    ScoreCanvas        *m_canvas;
    double              m_timeMs;
    TempoTimeSignature  m_savedTts;
    bool                m_valid;   // false if no marker existed at timeMs
};

#endif // SCORECANVASCOMMANDS_H

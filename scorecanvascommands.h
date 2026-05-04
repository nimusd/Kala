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
        PitchCurve,
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
// Resize Multiple Notes Command (for multi-selection resize)
// ============================================================================
class ResizeMultipleNotesCommand : public QUndoCommand
{
public:
    struct NoteState {
        int index;
        double startTime;
        double duration;
    };

    ResizeMultipleNotesCommand(Phrase *phrase,
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
    QVector<QVector<EnvelopePoint>> m_oldEnvelopeControlPoints;  // For undo
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
    // Removes the named curve from the specified notes only. Pass an empty
    // noteIndices list to scope the removal to every note in the phrase that
    // carries this curve (legacy behaviour).
    RemoveNamedExpressiveCurveCommand(Phrase *phrase,
                                      const QString &curveName,
                                      const QVector<int> &noteIndices,
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
// Apply Expressive Curve To Selection Command
// Applies a named expressive curve shape with three modes, mirroring
// ApplyDynamicsCurveCommand semantics:
//   * perNote=true            → the drawn shape becomes each note's curve
//     (independent per-note application over the note's own duration).
//   * perNote=false, single   → the shape is applied across the single note.
//   * perNote=false, multi    → the shape spans the whole selection.
//       - notes that already have this named curve keep their per-note shape,
//         scaled by evalEnvelope(noteCenter-in-selection).
//       - notes without the curve receive the drawn shape as-is.
// Undo restores each note's prior state (either the old shape + control points,
// or removes the curve if the note didn't have one).
// ============================================================================
class ApplyExpressiveCurveToSelectionCommand : public QUndoCommand
{
public:
    ApplyExpressiveCurveToSelectionCommand(Phrase *phrase,
                                            const QVector<int> &noteIndices,
                                            const QString &curveName,
                                            const QVector<EnvelopePoint> &envelope,
                                            double weight,
                                            bool perNote,
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
    bool m_perNote;

    struct PriorState {
        int noteIdx;
        bool hadCurve;
        Curve oldCurve;
        QVector<EnvelopePoint> oldControlPoints;
    };
    QVector<PriorState> m_priorStates;
    bool m_firstTime;

    ScoreCanvas *m_canvas;
};

// ============================================================================
// Apply EQ Curve Command
// Writes per-band envelopes for the 10-Band EQ to each selected note.
//   - shape envelope (X = band 1..10, Y = band gain with 0.5 = flat) is
//     averaged over each band's 1/10th slice of X to get 10 band values.
//   - intensity envelope (X = time, Y = 0..1) scales the shape toward flat:
//     0 = flat EQ, 1 = full shape.
//   - per-band value at time t:
//     v(t) = 0.5 + (bandShape - 0.5) * intensity(t) * weight
//   - perNote=true  → intensity spans each note's duration (EQ morphs within note)
//     perNote=false → intensity spans the whole selection; each note gets a
//                     static EQ scaled by intensity at its centre (morph over phrase)
// m_bandNames is the list of 10 actual curve names (in band order) found on
// the selection's variations — e.g. ["band 1", "band 2", ...] or the
// user's actual casing.  Undo restores prior per-band state per note.
// ============================================================================
class ApplyEqCurveCommand : public QUndoCommand
{
public:
    ApplyEqCurveCommand(Phrase *phrase,
                        const QVector<int> &noteIndices,
                        const QStringList &bandNames,        // size == 10
                        const QVector<EnvelopePoint> &shape,
                        const QVector<EnvelopePoint> &intensity,
                        double weight,
                        bool perNote,
                        ScoreCanvas *canvas,
                        QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase *m_phrase;
    QVector<int> m_noteIndices;
    QStringList m_bandNames;
    QVector<EnvelopePoint> m_shape;
    QVector<EnvelopePoint> m_intensity;
    double m_weight;
    bool m_perNote;

    struct PriorBandState {
        bool hadCurve;
        Curve oldCurve;
        QVector<EnvelopePoint> oldControlPoints;
    };
    struct PriorNoteState {
        int noteIdx;
        QVector<PriorBandState> bands;  // size == 10, aligned with m_bandNames
        bool hadShape = false;
        bool hadIntensity = false;
        QVector<EnvelopePoint> oldShape;
        QVector<EnvelopePoint> oldIntensity;
    };
    QVector<PriorNoteState> m_priorStates;
    bool m_firstTime;

    ScoreCanvas *m_canvas;
};

// ============================================================================
// Scale Dynamics Command
// Multiplies every point value in each note's dynamics curve by a factor,
// clamped to [0, 1].  factor > 1.0 boosts, factor < 1.0 reduces.
// ============================================================================
class ScaleDynamicsCommand : public QUndoCommand
{
public:
    ScaleDynamicsCommand(Phrase *phrase,
                         const QVector<int> &noteIndices,
                         double factor,
                         ScoreCanvas *canvas,
                         QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Phrase         *m_phrase;
    QVector<int>    m_noteIndices;
    double          m_factor;
    QVector<Curve>  m_oldCurves;   // Saved on first redo for undo
    bool            m_firstTime;
    ScoreCanvas    *m_canvas;
};

// ============================================================================
// Set Beat Dynamics Command
// Scales dynamics curves on a per-note basis (used by apply_beat_dynamics tool).
// Each note's existing curve is multiplied by the corresponding factor.
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
// Set Default Tempo Command
// Handles editing the tempo/time-signature at time 0 (the global default).
// Also rescales all note times, scale-change markers, and tempo-change markers
// so musical positions are preserved — mirroring ScoreCanvasWindow::scaleNoteTimes().
// ============================================================================
class SetDefaultTempoCommand : public QUndoCommand
{
public:
    SetDefaultTempoCommand(ScoreCanvas *canvas,
                           double oldTempo, int oldTimeSigNum, int oldTimeSigDenom,
                           double newTempo, int newTimeSigNum, int newTimeSigDenom,
                           QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    ScoreCanvas *m_canvas;
    double m_oldTempo, m_newTempo;
    int m_oldTimeSigNum, m_oldTimeSigDenom;
    int m_newTimeSigNum, m_newTimeSigDenom;

    struct NoteState { int index; double startTime; double duration; };
    QVector<NoteState> m_oldNoteStates;
    QVector<NoteState> m_newNoteStates;
    QMap<double, QPair<Scale, double>> m_oldScaleChanges;
    QMap<double, QPair<Scale, double>> m_newScaleChanges;
    QMap<double, TempoTimeSignature> m_oldTempoChanges;
    QMap<double, TempoTimeSignature> m_newTempoChanges;

    bool m_firstTime;
    void applyState(bool useNew);
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

// ============================================================================
// Snap To Scale Command
// Quantizes the pitch curves of selected continuous notes to the nearest
// scale degree at each note's start time.  Stores per-note old state for undo.
// ============================================================================
class SnapToScaleCommand : public QUndoCommand
{
public:
    SnapToScaleCommand(Phrase *phrase,
                       const QVector<int> &noteIndices,
                       ScoreCanvas *canvas,
                       QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
    bool isNoop() const { return m_noteStates.isEmpty(); }

private:
    struct NoteState {
        int    index;
        Curve  oldPitchCurve;
        Curve  newPitchCurve;
        bool   wasQuantized;
        QVector<Segment> oldSegments;
    };

    Phrase          *m_phrase;
    QVector<NoteState> m_noteStates;
    ScoreCanvas     *m_canvas;
    bool             m_firstTime;
};

// ============================================================================
// Fade Out Notes Command
// Splices a linear fade-out into the tail of each note's dynamics curve,
// preserving all existing curve points before startTime exactly.
// ============================================================================
class FadeOutNotesCommand : public QUndoCommand
{
public:
    FadeOutNotesCommand(Phrase *phrase,
                        const QVector<int> &noteIndices,
                        double startTime,   // 0–1, where fade begins
                        double endValue,    // target value at t=1 (usually 0)
                        ScoreCanvas *canvas,
                        QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    Phrase            *m_phrase;
    QVector<int>       m_noteIndices;
    double             m_startTime;
    double             m_endValue;
    QVector<Curve>     m_oldCurves;   // For undo
    ScoreCanvas       *m_canvas;
};

// ============================================================================
// Edit Note Property Command
// Used by the note inspector spinboxes (start, duration, pitch, variation).
// Merges consecutive changes to the same property so rapid spinbox changes
// produce a single undo step.
// ============================================================================
class EditNotePropertyCommand : public QUndoCommand
{
public:
    enum Property { StartTime, Duration, Pitch, VariationIndex };

    struct NoteChange {
        int index;
        double oldValue;
        double newValue;
    };

    EditNotePropertyCommand(Phrase *phrase,
                            const QVector<NoteChange> &changes,
                            Property property,
                            ScoreCanvas *canvas,
                            QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    int id() const override { return 20; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    Phrase *m_phrase;
    QVector<NoteChange> m_changes;
    Property m_property;
    ScoreCanvas *m_canvas;

    void applyValues(bool useNew);
};

// ============================================================================
// Retrograde Notes Command
// ============================================================================
// Copies selected notes in reverse temporal order starting at the target time.
// Internal curves (dynamics, pitch, expressive) are also time-reversed.
class RetrogradeNotesCommand : public QUndoCommand
{
public:
    RetrogradeNotesCommand(Phrase *phrase, const QVector<Note> &selectedNotes,
                          double targetTime, int targetTrackIndex, ScoreCanvas *canvas,
                          QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    const QVector<int>& getInsertedIndices() const { return m_insertedIndices; }

private:
    Phrase *m_phrase;
    QVector<Note> m_retrogradeNotes;   // Pre-computed retrograded notes
    double m_targetTime;
    int m_targetTrackIndex;
    QVector<int> m_insertedIndices;
    ScoreCanvas *m_canvas;

    static Curve reverseCurve(const Curve &curve);
};

// ============================================================================
// Set Note Curves Batch Command
// ============================================================================
// Applies a named expressive curve (or the Dynamics curve when name=="Dynamics")
// to a set of notes, with a DISTINCT envelope per note (pointsPerNote[i] goes
// to noteIndices[i]). Single undo entry restores every note's prior curve —
// or removes the named curve if the note didn't carry one before.
class SetNoteCurvesBatchCommand : public QUndoCommand
{
public:
    SetNoteCurvesBatchCommand(Phrase *phrase,
                              const QVector<int> &noteIndices,
                              const QVector<QVector<EnvelopePoint>> &pointsPerNote,
                              const QString &name,
                              double weight,
                              ScoreCanvas *canvas,
                              QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    struct PriorState {
        int   noteIdx;
        bool  hadCurve;    // expressive mode only: whether the named curve existed
        Curve oldCurve;    // dynamics: always the previous dynamics curve
                           // expressive: valid iff hadCurve is true
    };

    Phrase *m_phrase;
    QVector<int> m_noteIndices;
    QVector<QVector<EnvelopePoint>> m_pointsPerNote;
    QString m_name;
    double  m_weight;
    bool    m_isDynamics;
    QVector<PriorState> m_priorStates;
    bool    m_firstTime;
    ScoreCanvas *m_canvas;
};

#endif // SCORECANVASCOMMANDS_H

#ifndef SCORECANVAS_H
#define SCORECANVAS_H

#include <QWidget>
#include <QVector>
#include <QColor>
#include <QString>
#include <QUndoStack>
#include <QTimer>
#include "phrase.h"
#include "note.h"
#include "scale.h"
#include "tempotimesignature.h"
#include "envelopelibraryDialog.h"

class TrackSelector;
class Track;
class DynamicsCurveDialog;
class ExpressiveCurveApplyDialog;

class ScoreCanvas : public QWidget
{
    Q_OBJECT

public:
    // Input modes for drawing tools
    enum InputMode {
        SelectionMode,          // Select notes (no drawing)
        DrawModeDiscrete,       // Draw notes snapped to scale lines
        DrawModeContinuous      // Draw notes with literal pitch (no snapping)
    };

    struct ScaleLine {
        double frequencyHz;
        int scaleDegree;        // 1-7 for major scale
        QColor color;
        QString noteName;
        bool isThicker;         // Tonic and Fifth are thicker
    };

    explicit ScoreCanvas(QWidget *parent = nullptr);

    // Synchronization with track selector
    void setFrequencyRange(double minHz, double maxHz);
    void setPixelsPerHz(double ratio);
    void setVerticalOffset(int offsetPixels);

    // Horizontal zoom and scroll (time axis)
    void setPixelsPerSecond(double pps);
    void setHorizontalScrollOffset(int offset);

    // Scale configuration
    void setScale(const Scale &scale);
    void setBaseFrequency(double hz);
    void addScaleChange(double timeMs, const Scale &scale, double baseFreq);
    void removeScaleChange(double timeMs);
    void clearScaleChanges();
    Scale getScaleAtTime(double timeMs) const;
    double getBaseFrequencyAtTime(double timeMs) const;
    QMap<double, QPair<Scale, double>> getScaleChanges() const { return scaleChanges; }
    void generateScaleLines();
    QVector<ScaleLine> generateScaleLinesForScale(const Scale &scale, double baseFreq) const;
    const Scale& getCurrentScale() const { return currentScale; }
    double getBaseFrequency() const { return baseFrequency; }

    // Coordinate conversion (needed for zoom at cursor)
    double pixelToFrequency(int pixel) const;

    // Coordinate conversion for note input
    double pixelToTime(int x) const;
    int timeToPixel(double timeMs) const;
    double snapToNearestScaleLine(double hz) const;  // Deprecated - use snapToNearestScaleLineAtTime
    double snapToNearestScaleLineAtTime(double hz, double timeMs) const;

    // Note management
    Phrase& getPhrase() { return phrase; }
    const Phrase& getPhrase() const { return phrase; }
    void clearNotes();
    void deleteNotesOnTrack(int trackIndex);  // Delete all notes assigned to a specific track
    void updateTrackIndicesAfterDeletion(int deletedTrackIndex);  // Adjust indices after track deletion

    // Undo/Redo
    QUndoStack* getUndoStack() { return undoStack; }

    // Input mode management
    void setInputMode(InputMode mode);
    InputMode getInputMode() const { return currentInputMode; }

    // Track management
    void setActiveTrack(int trackIndex);
    int getActiveTrack() const { return activeTrackIndex; }
    void setTrackSelector(TrackSelector *selector) { trackSelector = selector; }
    void setCurrentTrack(Track *track) { m_currentTrack = track; }
    Track* getCurrentTrack() const { return m_currentTrack; }

    // Variation assignment
    void applyVariationToSelection(int variationIndex);

    // Musical mode settings (for bar lines)
    void setMusicalMode(bool enabled, int tempo = 120, int timeSigTop = 4, int timeSigBottom = 4);

    // Tempo/time signature map management
    void setDefaultTempo(double bpm);
    void setDefaultTimeSignature(int num, int denom);
    double getDefaultTempo() const { return defaultTempo; }
    int getDefaultTimeSigNum() const { return defaultTimeSigNum; }
    int getDefaultTimeSigDenom() const { return defaultTimeSigDenom; }
    void addTempoChange(double timeMs, const TempoTimeSignature &tts);
    void removeTempoChange(double timeMs);
    void clearTempoChanges();
    TempoTimeSignature getTempoTimeSignatureAtTime(double timeMs) const;
    double getTempoAtTime(double timeMs) const;  // With gradual interpolation
    QPair<int, int> getTimeSignatureAtTime(double timeMs) const;
    QMap<double, TempoTimeSignature> getTempoChanges() const { return tempoChanges; }

    // Clipboard operations
    void setPasteTargetTime(double timeMs);

    // Pitch remapping for scale modulations
    struct NotePitchChange {
        int noteIndex;
        double oldPitchHz;
        double newPitchHz;
    };
    QVector<NotePitchChange> remapDiscreteNotesForModulation(
        double modulationTimeMs,
        const Scale &oldScale, double oldBaseFreq,
        const Scale &newScale, double newBaseFreq);
    void applyNotePitchChanges(const QVector<NotePitchChange> &changes, bool forward);

    // Note editing operations
    void snapSelectedNotesToScale();  // Quantize selected continuous notes to scale degrees
    void makeSelectedNotesContinuous();  // Convert selected discrete notes to continuous
    void makeSelectedNotesDiscrete();    // Convert selected continuous notes back to discrete

    // Edit operations (called from menu)
    void performCut();
    void performCopy();
    void performPaste();
    void performDelete();
    void performSelectAll();
    void performSelectToEnd();   // E: extend selection to last note on active track
    void deselectAll();

    // Dynamics curve dialog
    void showDynamicsCurveDialog();
    void showExpressiveCurveApplyDialog();

    // Expressive curve management
    void setActiveExpressiveCurveIndex(int index, const QString &name = QString());
    int getActiveExpressiveCurveIndex() const { return m_activeExpressiveCurveIndex; }
    QString getActiveExpressiveCurveName() const { return m_activeExpressiveCurveName; }
    void addExpressiveCurveToSelection(const QString &name, const QVector<EnvelopePoint> &curve, double weight, bool perNote);
    void removeExpressiveCurveFromSelection(int curveIndex);
    void removeExpressiveCurveByName(const QString &name);
    void applyNamedCurveToSelection(const QString &name, const Curve &curve);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    // Note selection access (for inspector and tools)
    const QVector<int>& getSelectedNoteIndices() const { return selectedNoteIndices; }
    void selectNotes(const QVector<int> &indices);

    // Segment editing mode
    void enterSegmentEditingMode(int noteIndex);
    void exitSegmentEditingMode();
    bool isInSegmentEditingMode() const { return segmentEditingMode; }
    int getSegmentEditingNoteIndex() const { return segmentEditingNoteIndex; }
    int getSelectedSegmentIndex() const { return selectedSegmentIndex; }

    // Slide mode: constrains note dragging to time-axis only (no pitch change)
    void toggleSlideMode();
    bool isSlideMode() const { return slideMode; }

signals:
    void zoomChanged(double pixelsPerHz);
    void slideModeChanged(bool active);  // Emitted when slide mode is toggled
    void frequencyRangeChanged(double minHz, double maxHz);
    void pressureChanged(double pressure, bool active);  // Emits pressure updates during drawing
    void cursorPositionChanged(double timeMs, double pitchHz);  // Emits cursor position for status bar
    void notesChanged();  // Emitted when notes are added, removed, or modified
    void scaleSettingsChanged();  // Emitted when scale or modulations change
    void noteSelectionChanged();  // Emitted when note selection changes
    void tempoSettingsChanged();  // Emitted when tempo or time signature changes
    void requestAutoScroll(int dx, int dy);  // Ask ScoreCanvasWindow to scroll during lasso

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void tabletEvent(QTabletEvent *event) override;

private:
    QVector<ScaleLine> scaleLines;
    Scale currentScale;     // Default scale at time 0 (Just Intonation, Pythagorean, etc.)
    double baseFrequency;   // Default base frequency at time 0 (default: 25 Hz)
    QMap<double, QPair<Scale, double>> scaleChanges;  // Time (ms) -> (Scale, BaseFreq) for modulation

    // Tempo/time signature map
    double defaultTempo = 120.0;        // Default tempo at time 0
    int defaultTimeSigNum = 4;          // Default time signature numerator
    int defaultTimeSigDenom = 4;        // Default time signature denominator
    QMap<double, TempoTimeSignature> tempoChanges;  // Time (ms) -> tempo/timesig for changes

    Phrase phrase;  // Notes storage
    QUndoStack *undoStack;  // Undo/Redo stack

    // Input mode state
    InputMode currentInputMode;

    // Track state
    int activeTrackIndex;   // Which track new notes will use (default 0)
    TrackSelector *trackSelector;  // Reference to track selector for querying track states
    Track *m_currentTrack = nullptr;  // Current track for variation access

    // Frequency mapping (must match TrackSelector)
    double visibleMinHz;
    double visibleMaxHz;
    double pixelsPerHz;
    int verticalScrollOffset;

    // Time axis mapping
    double pixelsPerSecond;    // Horizontal zoom (pixels per second of time)
    int horizontalScrollOffset;  // Horizontal scroll position

    // Musical mode state (for bar lines)
    bool musicalModeEnabled;
    int musicalTempo;          // BPM
    int musicalTimeSigTop;     // Beats per bar
    int musicalTimeSigBottom;  // Note value (4 = quarter note)

    // Note input interaction state
    bool isDrawingNote;
    QPoint noteStartPos;
    QPoint noteCurrentPos;
    Note* pendingNote;

    // Pen tablet state
    bool usingTablet;        // true if current input is from pen tablet
    double penPressure;      // Current pen pressure (0.0-1.0)
    QVector<QPair<double, double>> pressurePoints;  // (time in ms, pressure) pairs during stroke
    QVector<QPair<double, double>> pitchPoints;     // (time in ms, pitch Hz) pairs during stroke

    // Double-tap detection for pen (since Qt has no tabletDoubleClickEvent)
    qint64 lastTabletPressTime;   // Timestamp of last tablet press (milliseconds)
    QPoint lastTabletPressPos;    // Position of last tablet press
    static constexpr int DOUBLE_TAP_TIME_MS = 400;    // Max time between taps
    static constexpr int DOUBLE_TAP_DISTANCE = 10;    // Max distance between taps (pixels)

    // Context menu debounce (prevent multiple menus from pen events)
    qint64 lastContextMenuTime;   // Timestamp when context menu was last shown
    static constexpr int CONTEXT_MENU_DEBOUNCE_MS = 500;  // Min time between context menus

    // Selection state
    QVector<int> selectedNoteIndices;  // Empty if no selection

    // Clipboard state
    QVector<Note> clipboard;           // Copied notes
    double pasteTargetTime;            // Where to paste (set from now marker)

    // Lasso selection state
    bool isDrawingLasso;
    bool lassoAddToSelection;  // Ctrl held when lasso started
    QPoint lassoStartPos;
    QPoint lassoCurrentPos;

    // Auto-scroll during lasso: asks ScoreCanvasWindow to shift the QScrollArea
    QTimer *m_autoScrollTimer = nullptr;
    QPoint m_lassoMousePos;  // Current mouse pos (widget coords), updated during lasso

    // Drag state
    enum DragMode {
        NoDrag,
        DraggingNote,          // Dragging note body (move time/pitch)
        ResizingLeft,          // Dragging left resize handle
        ResizingRight,         // Dragging right resize handle
        EditingTopCurve,       // Dragging top edge dot to edit dynamics curve
        EditingBottomCurve,    // Dragging bottom edge dot to edit bottom curve
        EditingTopCurveStart,  // Dragging left-top dot to edit dynamics curve start
        EditingTopCurveEnd,    // Dragging right-top dot to edit dynamics curve end
        EditingBottomCurveStart, // Dragging left-bottom dot to edit bottom curve start
        EditingBottomCurveEnd    // Dragging right-bottom dot to edit bottom curve end
    };
    DragMode currentDragMode;
    DragMode pendingDragMode;      // Drag mode waiting for threshold to be exceeded
    QPoint dragStartPos;
    bool dragThresholdExceeded;    // True once pen has moved beyond threshold
    static constexpr int DRAG_THRESHOLD = 5;  // Pixels of movement before drag starts
    double dragStartTime;
    double dragStartPitch;
    double dragStartDuration;
    int editingDotIndex;       // Which dot is being edited (-1 if none)
    double editingDotTimePos;  // Normalized time position of the dot being edited
    Curve dragStartCurve;      // Original curve state when drag started
    QVector<QPair<int, double>> multiDragStartTimes;  // Original times for multi-selection drag
    QVector<QPair<int, double>> multiDragStartPitches;  // Original pitches for multi-selection drag
    QVector<QPair<int, Curve>> multiDragStartCurves;  // Original curves for multi-selection drag

    // Segment editing mode state
    bool segmentEditingMode = false;

    // Active expressive curve (index kept for inspector sync; name drives per-note drawing)
    int m_activeExpressiveCurveIndex = 0;
    QString m_activeExpressiveCurveName;  // empty / "Dynamics" → dynamics; otherwise find by name

    // Slide mode: time-only dragging (Y axis locked)
    bool slideMode = false;
    int segmentEditingNoteIndex = -1;   // Which note is being segment-edited
    int selectedSegmentIndex = -1;      // Which segment is selected (-1 = none)

    // Visual constants
    static constexpr int HZ_LABEL_WIDTH = 60;
    static constexpr int NORMAL_LINE_WIDTH = 1;
    static constexpr int THICK_LINE_WIDTH = 2;
    static constexpr int PIXELS_PER_OCTAVE = 100;       // Fixed vertical size for each octave

    // Extended color system for scale degrees (supports up to 10 notes)
    static const QColor SCALE_COLORS[10];

    // Helper methods
    int frequencyToPixel(double hz) const;
    QColor getScaleDegreeColor(int degree) const;
    void drawScaleLine(QPainter &painter, const ScaleLine &line, int y);
    void drawScaleLineInRegion(QPainter &painter, const ScaleLine &line, int y, int startX, int endX);
    void drawTimeAwareScaleLines(QPainter &painter);
    void drawHzLabel(QPainter &painter, double hz, int y);
    void drawNote(QPainter &painter, const Note &note, bool isSelected);
    void drawPendingNote(QPainter &painter);
    void drawSelectionRectangle(QPainter &painter, const Note &note);
    void drawLassoRectangle(QPainter &painter);
    void drawBarLines(QPainter &painter);

    // Selection helpers
    int findNoteAtPosition(const QPoint &pos) const;
    QVector<int> findNotesInRectangle(const QRect &rect) const;
    void selectNote(int index, bool addToSelection = false);
    bool isNoteSelected(int index) const;

    // Drag helpers
    DragMode detectDragMode(const QPoint &pos, const Note &note) const;
    QRect getNoteRect(const Note &note) const;
    int calculateCurveDotCount(const Note &note) const;  // Adaptive dot count based on note width
    // Returns the correct expressive curve index for a specific note, using name-based lookup.
    // Falls back to 0 (dynamics) when the note does not carry the active named curve.
    int resolveActiveCurveIndex(const Note &note) const;
    QRect getLeftResizeHandle(const Note &note) const;
    QRect getRightResizeHandle(const Note &note) const;

    // Pitch curve quantization
    Curve quantizePitchCurveToScale(const Curve &pitchCurve, const Note &note) const;

    // Context menu helper
    void showNoteContextMenu(const QPoint &globalPos);

    // Segment editing helpers
    int findSegmentAtPosition(const QPoint &pos, int noteIndex) const;
    QRect getSegmentRect(const Note &note, int segmentIndex) const;
    void drawSegmentOverlay(QPainter &painter, const Note &note);
    void deleteSelectedSegment();
    void showSegmentContextMenu(const QPoint &globalPos);
};

#endif // SCORECANVAS_H

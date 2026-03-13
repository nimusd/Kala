#ifndef SCORECANVASWINDOW_H
#define SCORECANVASWINDOW_H

#include "ui_scorecanvas.h"
#include "trackselect.h"
#include "scorecanvas.h"
#include "timeline.h"
#include "frequencylabels.h"
#include "audioengine.h"
#include "compositionsettings.h"
#include <QMainWindow>

// Forward declarations
class TrackManager;
class Track;
class KalaMain;
#include <QLabel>
#include <QActionGroup>
#include <QPushButton>
#include <QSpinBox>
#include <QProgressBar>

namespace Ui {
class ScoreCanvasWindow;
}

class ScoreCanvasWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit ScoreCanvasWindow(AudioEngine *sharedAudioEngine, QWidget *parent = nullptr);
    ~ScoreCanvasWindow();

    /**
     * Set TrackManager to use for rendering
     * ScoreCanvas will use Track's graph for audio generation
     */
    void setTrackManager(TrackManager *manager);

    /**
     * Set main window reference for Tab key toggle functionality
     */
    void setMainWindow(KalaMain *mainWindow) { m_mainWindow = mainWindow; }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    bool event(QEvent *event) override;  // Override to catch Tab key before focus navigation
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

public slots:
    void onZoomIn();
    void onZoomOut();
    void zoomToFit();

private slots:
    void onZoomToggled(bool checked);
    void onPressureChanged(double pressure, bool active);
    void onCursorPositionChanged(double timeMs, double pitchHz);
    void onCompositionSettingsTriggered();
    void onAddTrackTriggered();
    void onTrackSelected(int trackIndex);

    // Render progress slots
    void onRenderStarted();
    void onRenderProgressChanged(int percentage);
    void onRenderCompleted();
    void onRenderCancelled();

    // Auto-scroll request from ScoreCanvas during lasso selection
    void onScoreAutoScroll(int dx, int dy);

private:
    // Helper to connect a track's render progress signals
    void connectTrackProgressSignals(Track *track);
    void updateFrequencyLabels();

    Ui::scorecanvas *ui;
    TrackSelector *trackSelector;
    ScoreCanvas *scoreCanvas;
    Timeline *timeline;
    FrequencyLabels *frequencyLabels;
    QScrollArea *scoreScrollArea;
    QScrollArea *trackScrollArea;
    QScrollArea *frequencyScrollArea;

    // Drawing tool action group
    QActionGroup *drawingToolGroup;

    // Status labels
    QLabel *pressureLabel;
    QLabel *cursorTimeLabel;
    QLabel *cursorPitchLabel;
    QLabel *statusTempoLabel;
    QLabel *statusTimeSigLabel;
    QLabel *navigationModeLabel;

    // Render progress bar
    QProgressBar *renderProgressBar;

    // Composition settings widgets (Phase 3)
    QPushButton *timeModeToggle;      // Absolute ↔ Musical
    QPushButton *slideModeBtn;        // Slide mode toggle (time-only movement)
    QSpinBox *tempoSpinBox;           // BPM
    QSpinBox *timeSigNumerator;       // Top number (beats per bar)
    QSpinBox *timeSigDenominator;     // Bottom number (note value)

    // Composition state
    enum TimeMode {
        AbsoluteTime,   // Min:Sec:Ms
        MusicalTime     // Bars:Beats:Ms
    };
    TimeMode currentTimeMode;
    int currentTempo;           // BPM
    int currentTimeSigTop;      // Beats per bar
    int currentTimeSigBottom;   // Note value (4 = quarter note)

    // Composition settings (new)
    CompositionSettings compositionSettings;
    QString compositionName;

    // Audio engine (Phase 3)
    AudioEngine *audioEngine;
    TrackManager *trackManager;  // NEW: Access to Track objects for rendering
    KalaMain *m_mainWindow = nullptr;  // Reference to main window for Tab toggle
    bool m_isRendering = false;  // True while any track is rendering

    // Playback state
    QTimer *playbackTimer;
    int currentNoteIndex;
    double playbackStartTime;      // Current playback position
    double playbackStartPosition;  // Where playback should always start from (set by double-click)
    bool isPlaying;

    // Zoom state
    bool zoomModeActive;
    double currentMinHz;
    double currentMaxHz;
    double currentPixelsPerSecond;  // Horizontal zoom (time axis)
    bool isDraggingZoom;
    QPoint dragStartPos;
    double dragStartMinHz;
    double dragStartMaxHz;
    double dragStartPixelsPerSecond;
    double zoomCenterHz;  // Frequency at cursor position
    double zoomCenterTime;  // Time at cursor position

    // Pan mode state
    bool panModeActive;
    bool isDraggingPan;
    QPoint panDragStartPos;
    int panStartHorizontalScroll;
    int panStartVerticalScroll;

    // Track color management
    QVector<QColor> trackColorPalette;
    int nextColorIndex;

    void setupToolbarColors();
    void setupDrawingTools();
    void setupCompositionSettings();
    void setupTrackSelector();
    void setupScoreCanvas();
    void setupZoom();
    void setZoomMode(bool active);
    void setPanMode(bool active);
    void applyZoom(double minHz, double maxHz);
    void applyHorizontalZoom(double pixelsPerSecond);
    void onTimeModeToggled();
    void onSlideModeChanged(bool active);
    void onTempoChanged(int bpm);
    void onTimeSignatureChanged();
    void onNowMarkerChanged(double timeMs);  // Sync spinboxes with timeline position

    // Audio playback (Phase 3)
    void startPlayback();
    void onPlaybackTick();

public:
    // Scale all note times by factor (for tempo changes)
    void scaleNoteTimes(double scaleFactor);

    // Composition settings methods
    CompositionSettings getSettings() const;
    void updateFromSettings(const CompositionSettings &settings);

    // Pre-render notes without starting playback (can be called externally)
    void prerenderNotes();

    // Expose ScoreCanvas for sharing notes with SounitBuilder
    ScoreCanvas* getScoreCanvas() const { return scoreCanvas; }

    // Expose TrackSelector for track management
    TrackSelector* getTrackSelector() const { return trackSelector; }

    // Expose Timeline for getting now marker position
    Timeline* getTimeline() const { return timeline; }

    // Update specified track with loaded sounit info
    void updateSounitTrack(int trackIndex, const QString &sounitName, const QColor &sounitColor);

    // Get next available track color from palette
    QColor getNextTrackColor();

public slots:
    void stopPlayback(bool stopAudioEngine = true);
    void play();  // Start score playback (used by AI companion)

signals:
    void playbackStarted();
    void compositionSettingsChanged();  // Emitted when tempo, time sig, or other settings change
};

#endif // SCORECANVASWINDOW_H

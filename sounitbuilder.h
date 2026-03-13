#ifndef SOUNITBUILDER_H
#define SOUNITBUILDER_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QProgressBar>
#include "container.h"
#include "canvas.h"
#include "audioengine.h"
#include "phrase.h"

// Forward declarations
class Track;
class KalaMain;
class ScoreCanvasWindow;

namespace Ui {
class SounitBuilderCanvas;
}

class SounitBuilder : public QMainWindow
{
    Q_OBJECT




public:
    explicit SounitBuilder(AudioEngine *sharedAudioEngine, QWidget *parent = nullptr);
    ~SounitBuilder();

    // Set the ScoreCanvas to read notes from
    void setScoreCanvas(class ScoreCanvas *canvas);

    // Get the canvas for wiring up inspectors
    Canvas* getCanvas() const { return canvas; }

    /**
     * Set which Track's canvas to display in Sound Engine
     * Call this when switching between tracks.
     * @param track Track whose canvas should be displayed
     */
    void setTrackCanvas(Track *track);

    // Set main window reference for Tab key toggle
    void setMainWindow(KalaMain *mainWindow) { m_mainWindow = mainWindow; }

    // Set ScoreCanvasWindow reference for note mode
    void setScoreCanvasWindow(ScoreCanvasWindow *w) { m_scoreCanvasWindow = w; }

    // Note mode
    bool isNoteModeActive() const { return m_noteModeActive; }

    // Reconnect container signals (used after loading)
    void connectContainerSignals(Container *container);

protected:
    bool event(QEvent *event) override;  // Override to catch Tab key before focus navigation
    void keyPressEvent(QKeyEvent *event) override;


private:
    Ui::SounitBuilderCanvas *ui;
    KalaMain *m_mainWindow = nullptr;  // Main window reference for Tab key toggle
    // transport labels
    QLabel *labelPosition ;
    QLabel *labelTempo ;
    QLabel *labelTimeSignature ;
    Container *connectionStartContainer = nullptr;
    QString connectionStartPort;
    bool connectionStartIsOutput = false;
    QVector<Container*> containers;

    Canvas *canvas;
    Track *currentTrack;  // NEW: Track being edited (owns canvas and graph)

    // Audio engine (shared)
    AudioEngine *audioEngine;

    // Playback state (same as ScoreCanvas)
    QTimer *playbackTimer;
    int currentNoteIndex;
    double playbackStartTime;
    double noteDuration;  // Total duration of rendered buffer
    bool isPlaying;
    bool m_isRendering = false;  // True while track is rendering

    // ScoreCanvas reference to get notes from (shared with ScoreCanvasWindow)
    class ScoreCanvas *scoreCanvas;

    // Render progress bar
    QProgressBar *renderProgressBar;

    // Toolbar buttons
    QPushButton *m_selectModeButton = nullptr;
    QPushButton *m_noteModeButton = nullptr;

    // Current editing track (which track's graph to rebuild on parameter changes)
    int currentEditingTrack;

public slots:
    void onAddContainer(const QString &name, const QColor &color,
                        const QStringList &inputs, const QStringList &outputs);
    void triggerPlay();   // Public entry point for programmatic playback (e.g. AI agent)

private slots:
    void onPortClicked(Container *container, const QString &portName, bool isOutput, QPoint globalPos);

    void onPlay();
    void onStop();
    void onPlaybackTick();

    // Render progress slots
    void onRenderStarted();
    void onRenderProgressChanged(int percentage);
    void onRenderCompleted();
    void onRenderCancelled();

public slots:
    void stopPlayback(bool stopAudioEngine = true);

    // Variation management (public so KalaMain can call it)
    void onCreateVariation();
    void rebuildGraph(int trackIndex = 0, bool rebuildTrackGraph = true);  // Rebuild audio graph from canvas for specific track
    void setCurrentEditingTrack(int trackIndex);  // Set which track is being edited

private:
    void startPlayback();

signals:
    void playbackStarted();
    void variationsChanged();  // Emitted when a variation is created
    void noteModeChanged(bool active);

private:
    // Clipboard state (stores "containers" array + "connections" array between selected containers)
    QJsonObject m_containerClipboard;

    // Multi-drag undo: maps each selected container to its position at drag start
    QMap<Container*, QPoint> m_multiDragStartPositions;

    // Clipboard operations
    void performCopy();
    void performCut();
    void performPaste();

    // Note Mode state
    bool m_noteModeActive = false;
    QString m_noteModeNoteId;          // Current note being edited
    int m_noteModeNoteIndex = -1;      // Index in phrase's note vector
    QJsonObject m_preModeCanvasState;  // Canvas state before entering note mode
    ScoreCanvasWindow *m_scoreCanvasWindow = nullptr;
    QList<Note> m_noteModePlaybackSavedNotes;  // Notes saved during single-note playback
    bool m_noteModePlaying = false;            // True while note-mode single-note playback is active

    // Note mode methods
    void enterNoteMode();
    void exitNoteMode();
    void noteModeNavigate(int direction); // -1=prev, +1=next
    void noteModeSaveCurrentNote();
    void noteModeLoadNote(int noteIndex);
    void noteModePlayCurrentNote();
    int findNoteAtOrAfterTime(double timeMs, int trackIndex);
    int findAdjacentNote(int currentIndex, int direction, int trackIndex);



};

#endif // SOUNITBUILDER_H

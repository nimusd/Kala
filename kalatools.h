#pragma once
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QColor>

class SounitBuilder;
class TrackManager;
class ScoreCanvasWindow;
class Canvas;
class Track;
class Container;
class KalaMain;

// KalaTools: bridges between LLM tool calls and Kala's C++ operations.
//
// Two responsibilities:
//   1. getToolSchemas()  — static, returns the JSON Schema array to include
//                          in every API request so the LLM knows what it can call.
//   2. dispatchTool()    — routes a tool call (name + args) from the agent loop
//                          to the appropriate Kala operation and returns a JSON
//                          result object the agent loop feeds back to the LLM.
//
// Threading: all methods must be called on the main (GUI) thread.
// This is guaranteed because QNetworkReply::finished fires on the main thread,
// so KalaAgent's dispatch chain never leaves the main thread.
class KalaTools : public QObject
{
    Q_OBJECT

public:
    explicit KalaTools(SounitBuilder    *builder,
                       TrackManager     *trackManager,
                       ScoreCanvasWindow *scoreCanvasWindow,
                       QObject          *parent = nullptr);

    // Tool schemas in OpenAI function-calling format.
    // Include the returned array in every API request under "tools".
    static QJsonArray getToolSchemas();

    // Route a tool call from the agent loop.
    // Returns { "result": ... } on success or { "error": "..." } on failure.
    QJsonObject dispatchTool(const QString &toolName, const QJsonObject &args);

    // KalaMain must call this whenever the user switches to a different track.
    void setCurrentTrackIndex(int index);

    // Called once from KalaMain constructor to enable project open/save tools.
    void setMainWindow(KalaMain *main) { m_kalaMain = main; }

private:
    // ── Tool implementations ─────────────────────────────────────────────────
    QJsonObject toolGetGraphState();
    QJsonObject toolGetSounitList();
    QJsonObject toolClearGraph();
    QJsonObject toolAddContainer   (const QJsonObject &args);
    QJsonObject toolSetParameter   (const QJsonObject &args);
    QJsonObject toolConnectContainers(const QJsonObject &args);
    QJsonObject toolRemoveContainer(const QJsonObject &args);
    QJsonObject toolLoadSounit     (const QJsonObject &args);
    QJsonObject toolSaveSounit     (const QJsonObject &args);
    QJsonObject toolPlayPreview    ();

    // ── Library browser tools ────────────────────────────────────────────────
    QJsonObject toolGetSpectrumList();
    QJsonObject toolGetProjectList();
    QJsonObject toolGetIRList();
    QJsonObject toolSetLibraryRoot  (const QJsonObject &args);
    QJsonObject toolLoadIR          (const QJsonObject &args);
    QJsonObject toolLoadSpectrum    (const QJsonObject &args);
    QJsonObject toolGetEnvelopeList ();
    QJsonObject toolLoadEnvelope    (const QJsonObject &args);
    QJsonObject toolSetEnvelopeShape(const QJsonObject &args);

    // ── Composition tool implementations ────────────────────────────────────
    QJsonObject toolGetCompositionState(const QJsonObject &args);
    QJsonObject toolGetTrackList();
    QJsonObject toolAddNote            (const QJsonObject &args);
    QJsonObject toolDeleteNote         (const QJsonObject &args);
    QJsonObject toolClearNotes         (const QJsonObject &args);
    QJsonObject toolSetScale           (const QJsonObject &args);
    QJsonObject toolSetTempo           (const QJsonObject &args);

    // ── Score canvas operations (right-click equivalent) ─────────────────────
    QJsonObject toolGetVariationList   ();
    QJsonObject toolApplyVariation     (const QJsonObject &args);
    QJsonObject toolCreateVariation    (const QJsonObject &args);
    QJsonObject toolCreateVariationFromSounit(const QJsonObject &args);
    QJsonObject toolDeleteVariation    (const QJsonObject &args);
    QJsonObject toolRenameVariation    (const QJsonObject &args);
    QJsonObject toolSwitchVariation    (const QJsonObject &args);
    QJsonObject toolApplyDynamicsCurve (const QJsonObject &args);
    QJsonObject toolScaleTiming        (const QJsonObject &args);
    QJsonObject toolApplyRhythmEasing  (const QJsonObject &args);
    QJsonObject toolLinkLegato         (const QJsonObject &args);
    QJsonObject toolUnlinkLegato       (const QJsonObject &args);
    QJsonObject toolQuantizeToScale    (const QJsonObject &args);
    QJsonObject toolMakeContinuous     (const QJsonObject &args);
    QJsonObject toolMakeDiscrete       (const QJsonObject &args);
    QJsonObject toolShiftNotes         (const QJsonObject &args);
    QJsonObject toolApplyBeatDynamics  (const QJsonObject &args);
    QJsonObject toolSetNoteDynamics    (const QJsonObject &args);
    QJsonObject toolSelectNotes        (const QJsonObject &args);
    QJsonObject toolGetSelectedNotes   ();

    // ── Note editing ─────────────────────────────────────────────────────────
    QJsonObject toolSetNotePitch       (const QJsonObject &args);
    QJsonObject toolTransposeNotes     (const QJsonObject &args);
    QJsonObject toolSetNoteDuration    (const QJsonObject &args);
    QJsonObject toolStretchNotes       (const QJsonObject &args);
    QJsonObject toolDuplicateNotes     (const QJsonObject &args);

    // ── Expressive curves ────────────────────────────────────────────────────
    QJsonObject toolGetNoteCurves      (const QJsonObject &args);
    QJsonObject toolSetNoteCurve       (const QJsonObject &args);

    // ── Vibrato ──────────────────────────────────────────────────────────────
    QJsonObject toolGetNoteVibrato     (const QJsonObject &args);
    QJsonObject toolSetNoteVibrato     (const QJsonObject &args);

    // ── Transport ────────────────────────────────────────────────────────────
    QJsonObject toolPlayScore          ();
    QJsonObject toolStopScore          ();

    // ── Undo / Redo ──────────────────────────────────────────────────────────
    QJsonObject toolUndo               ();
    QJsonObject toolRedo               ();

    // ── Time signature / tempo changes ───────────────────────────────────────
    QJsonObject toolSetTimeSignature         (const QJsonObject &args);
    QJsonObject toolGetTimeSignatureChanges  ();
    QJsonObject toolAddTimeSignatureChange   (const QJsonObject &args);
    QJsonObject toolRemoveTimeSignatureChange(const QJsonObject &args);

    // ── Project ──────────────────────────────────────────────────────────────
    QJsonObject toolOpenProject        (const QJsonObject &args);
    QJsonObject toolSaveProject        (const QJsonObject &args);

    // ── Track management ─────────────────────────────────────────────────────
    QJsonObject toolAddTrack           (const QJsonObject &args);
    QJsonObject toolRenameTrack        (const QJsonObject &args);
    QJsonObject toolDeleteTrack        (const QJsonObject &args);

    // ── Engine / project settings ─────────────────────────────────────────────
    QJsonObject toolGetEngineSettings  ();
    QJsonObject toolSetEngineSettings  (const QJsonObject &args);

    // ── Graph editing ─────────────────────────────────────────────────────────
    QJsonObject toolRemoveConnection   (const QJsonObject &args);

    // ── Transport ─────────────────────────────────────────────────────────────
    QJsonObject toolSeek               (const QJsonObject &args);
    QJsonObject toolSetLoop            (const QJsonObject &args);
    QJsonObject toolClearLoop          ();

    // ── Helpers ──────────────────────────────────────────────────────────────
    Track   *currentTrack()  const;
    Canvas  *currentCanvas() const;
    Container *findContainer(const QString &instanceName) const;

    // Return the appropriate container color for a given type name.
    // Must stay in sync with the color assignments in sounitbuilder.cpp.
    static QColor colorForType(const QString &type);

    static QJsonObject ok   (const QString &message);
    static QJsonObject error(const QString &message);

    // ── Members ──────────────────────────────────────────────────────────────
    SounitBuilder     *m_builder;
    TrackManager      *m_trackManager;
    ScoreCanvasWindow *m_scoreCanvasWindow;
    KalaMain          *m_kalaMain          = nullptr;
    int                m_currentTrackIndex = 0;

    // Returns the user's kala library root — from QSettings or ~/Music/kala as default
    static QString libraryRoot();
};

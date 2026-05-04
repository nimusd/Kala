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

enum class ToolMode { Sounit, Composition, Full };

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
    QJsonArray getToolSchemas() const;

    void setToolMode(ToolMode mode) { m_toolMode = mode; }
    ToolMode toolMode() const { return m_toolMode; }

    // Route a tool call from the agent loop.
    // Returns { "result": ... } on success or { "error": "..." } on failure.
    QJsonObject dispatchTool(const QString &toolName, const QJsonObject &args);

    // KalaMain must call this whenever the user switches to a different track.
    void setCurrentTrackIndex(int index);

    // Called once from KalaMain constructor to enable project open/save tools.
    void setMainWindow(KalaMain *main) { m_kalaMain = main; }

private:
    // ── Consolidated tool dispatchers ────────────────────────────────────────
    QJsonObject toolBrowseLibrary      (const QJsonObject &args);  // sounit|spectrum|ir|envelope|project
    QJsonObject toolHistory            (const QJsonObject &args);  // undo|redo
    QJsonObject toolTransport          (const QJsonObject &args);  // play|stop|preview|seek|loop|clear_loop
    QJsonObject toolTrack              (const QJsonObject &args);  // list|add|rename|delete
    QJsonObject toolProject            (const QJsonObject &args);  // open|save
    QJsonObject toolVariation          (const QJsonObject &args);  // list|create|create_from_sounit|delete|rename|switch|apply|copy_to_base
    QJsonObject toolEditGraph          (const QJsonObject &args);  // add|remove|connect|disconnect|clear
    QJsonObject toolTimeSig            (const QJsonObject &args);  // get|set|add|remove
    QJsonObject toolEngineSettings     (const QJsonObject &args);  // get|set
    QJsonObject toolInspectNotes       (const QJsonObject &args);  // selection|curves|vibrato
    QJsonObject toolSelectNotes        (const QJsonObject &args);  // range|flat_dynamics|current
    QJsonObject toolSetNoteMode        (const QJsonObject &args);  // legato|unlegato|continuous|discrete
    QJsonObject toolSetDynamics        (const QJsonObject &args);  // curve|envelope_file|scale|per_note|beat_pattern|expressive
    QJsonObject toolTransformNotes     (const QJsonObject &args);  // transpose|set_pitch|shift|stretch|set_duration|scale_timing|ease_rhythm|quantize

    // ── Standalone tool implementations ──────────────────────────────────────
    QJsonObject toolGetGraphState      ();
    QJsonObject toolGetCompositionState(const QJsonObject &args);
    QJsonObject toolAddContainer       (const QJsonObject &args);
    QJsonObject toolSetParameter       (const QJsonObject &args);
    QJsonObject toolSetParameters      (const QJsonObject &args);  // batch version
    QJsonObject toolConnectContainers  (const QJsonObject &args);
    QJsonObject toolRemoveContainer    (const QJsonObject &args);
    QJsonObject toolRenameContainer    (const QJsonObject &args);
    QJsonObject toolRemoveConnection   (const QJsonObject &args);
    QJsonObject toolLoadSounit         (const QJsonObject &args);
    QJsonObject toolSaveSounit         (const QJsonObject &args);
    QJsonObject toolSetLibraryRoot     (const QJsonObject &args);
    QJsonObject toolLoadIR             (const QJsonObject &args);
    QJsonObject toolLoadSpectrum       (const QJsonObject &args);
    QJsonObject toolLoadEnvelope       (const QJsonObject &args);
    QJsonObject toolSetEnvelopeShape   (const QJsonObject &args);
    QJsonObject toolAddNote            (const QJsonObject &args);
    QJsonObject toolDeleteNote         (const QJsonObject &args);
    QJsonObject toolClearNotes         (const QJsonObject &args);
    QJsonObject toolSetScale           (const QJsonObject &args);
    QJsonObject toolSetTempo           (const QJsonObject &args);
    QJsonObject toolDuplicateNotes     (const QJsonObject &args);
    QJsonObject toolSetNoteVibrato     (const QJsonObject &args);
    QJsonObject toolFadeOutNotes       (const QJsonObject &args);
    QJsonObject toolPlayPreview        ();

    // ── Sub-implementations called by consolidated dispatchers ───────────────
    QJsonObject toolGetSounitList      ();
    QJsonObject toolGetSpectrumList    ();
    QJsonObject toolGetProjectList     ();
    QJsonObject toolGetIRList          ();
    QJsonObject toolGetEnvelopeList    ();
    QJsonObject toolClearGraph         ();
    QJsonObject toolPlayScore          ();
    QJsonObject toolStopScore          ();
    QJsonObject toolSeek               (const QJsonObject &args);
    QJsonObject toolSetLoop            (const QJsonObject &args);
    QJsonObject toolClearLoop          ();
    QJsonObject toolUndo               ();
    QJsonObject toolRedo               ();
    QJsonObject toolGetTrackList       ();
    QJsonObject toolAddTrack           (const QJsonObject &args);
    QJsonObject toolRenameTrack        (const QJsonObject &args);
    QJsonObject toolDeleteTrack        (const QJsonObject &args);
    QJsonObject toolOpenProject        (const QJsonObject &args);
    QJsonObject toolSaveProject        (const QJsonObject &args);
    QJsonObject toolGetVariationList   ();
    QJsonObject toolApplyVariation     (const QJsonObject &args);
    QJsonObject toolCreateVariation    (const QJsonObject &args);
    QJsonObject toolCreateVariationFromSounit(const QJsonObject &args);
    QJsonObject toolDeleteVariation    (const QJsonObject &args);
    QJsonObject toolRenameVariation    (const QJsonObject &args);
    QJsonObject toolSwitchVariation    (const QJsonObject &args);
    QJsonObject toolCopyVariationToBase(const QJsonObject &args);
    QJsonObject toolSetTimeSignature         (const QJsonObject &args);
    QJsonObject toolGetTimeSignatureChanges  ();
    QJsonObject toolAddTimeSignatureChange   (const QJsonObject &args);
    QJsonObject toolRemoveTimeSignatureChange(const QJsonObject &args);
    QJsonObject toolGetEngineSettings  ();
    QJsonObject toolSetEngineSettings  (const QJsonObject &args);
    QJsonObject toolGetSelectedNotes   ();
    QJsonObject toolSelectNotesByRange (const QJsonObject &args);
    QJsonObject toolSelectFlatDynamicsNotes(const QJsonObject &args);
    QJsonObject toolGetNoteCurves      (const QJsonObject &args);
    QJsonObject toolGetNoteVibrato     (const QJsonObject &args);
    QJsonObject toolLinkLegato         (const QJsonObject &args);
    QJsonObject toolUnlinkLegato       (const QJsonObject &args);
    QJsonObject toolMakeContinuous     (const QJsonObject &args);
    QJsonObject toolMakeDiscrete       (const QJsonObject &args);
    QJsonObject toolApplyDynamicsCurve (const QJsonObject &args);
    QJsonObject toolLoadEnvelopeAsDynamics(const QJsonObject &args);
    QJsonObject toolScaleDynamics      (const QJsonObject &args);
    QJsonObject toolSetNoteDynamics    (const QJsonObject &args);
    QJsonObject toolApplyBeatDynamics  (const QJsonObject &args);
    QJsonObject toolSetNoteCurve       (const QJsonObject &args);
    QJsonObject toolSetNoteCurvesBatch (const QJsonObject &args);
    QJsonObject toolTransposeNotes     (const QJsonObject &args);
    QJsonObject toolSetNotePitch       (const QJsonObject &args);
    QJsonObject toolShiftNotes         (const QJsonObject &args);
    QJsonObject toolStretchNotes       (const QJsonObject &args);
    QJsonObject toolSetNoteDuration    (const QJsonObject &args);
    QJsonObject toolScaleTiming        (const QJsonObject &args);
    QJsonObject toolApplyRhythmEasing  (const QJsonObject &args);
    QJsonObject toolQuantizeToScale    (const QJsonObject &args);
    QJsonObject toolStrumNotes         (const QJsonObject &args);

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
    ToolMode           m_toolMode          = ToolMode::Sounit;

    // Anti-loop guard: set when graph state was just provided (via switch_variation
    // or get_graph_state). Cleared by any mutating tool call. If get_graph_state is
    // called while true, return a short "already provided" nudge instead of the full dump.
    bool               m_graphStateJustProvided    = false;
    bool               m_selectedNotesJustProvided = false;

    // Returns the user's kala library root — from QSettings or ~/Music/kala as default
    static QString libraryRoot();
};

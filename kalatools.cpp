#include "kalatools.h"
#include "kalamain.h"
#include "vibrato.h"
#include "containersettings.h"
#include "compositionsettings.h"
#include "timeline.h"
#include "sounitbuilder.h"
#include "trackmanager.h"
#include "track.h"
#include "canvas.h"
#include "container.h"
#include "sounitbuildercommands.h"
#include "scorecanvaswindow.h"
#include "scorecanvas.h"
#include "scorecanvascommands.h"
#include "phrase.h"
#include "note.h"
#include "scale.h"
#include "wavetablesynth.h"
#include "envelopelibraryDialog.h"
#include "easing.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QJsonArray>
#include <QJsonDocument>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

KalaTools::KalaTools(SounitBuilder    *builder,
                     TrackManager     *trackManager,
                     ScoreCanvasWindow *scoreCanvasWindow,
                     QObject          *parent)
    : QObject(parent)
    , m_builder(builder)
    , m_trackManager(trackManager)
    , m_scoreCanvasWindow(scoreCanvasWindow)
{
}

void KalaTools::setCurrentTrackIndex(int index)
{
    m_currentTrackIndex = index;
}

// ─────────────────────────────────────────────────────────────────────────────
// Static helpers
// ─────────────────────────────────────────────────────────────────────────────

QColor KalaTools::colorForType(const QString &type)
{
    // Must stay in sync with sounitbuilder.cpp constructor color assignments.
    static const QHash<QString, QString> map = {
        // Essential (Blue)
        {"Harmonic Generator",  "#3498db"},
        {"Spectrum to Signal",  "#3498db"},
        {"Karplus Strong",      "#3498db"},
        {"Signal Mixer",        "#3498db"},
        {"Note Tail",           "#3498db"},
        {"Attack",              "#3498db"},
        {"Wavetable Synth",     "#3498db"},
        {"Recorder",            "#3498db"},
        {"Bowed",               "#3498db"},
        {"Reed",                "#3498db"},
        // Shaping (Orange)
        {"Rolloff Processor",   "#e67e22"},
        {"Spectrum Blender",    "#e67e22"},
        {"Formant Body",        "#e67e22"},
        {"Breath Turbulence",   "#e67e22"},
        {"Noise Color Filter",  "#e67e22"},
        // Modifiers (Green)
        {"Physics System",      "#27ae60"},
        {"Easing Applicator",   "#27ae60"},
        {"Envelope Engine",     "#27ae60"},
        {"Drift Engine",        "#27ae60"},
        {"LFO",                 "#27ae60"},
        {"Frequency Mapper",    "#27ae60"},
        // Filters / FX (Purple)
        {"10-Band EQ",          "#9b59b6"},
        {"Comb Filter",         "#9b59b6"},
        {"LP/HP Filter",        "#9b59b6"},
        {"IR Convolution",      "#9b59b6"},
    };
    return QColor(map.value(type, "#3498db"));
}

QString KalaTools::libraryRoot()
{
    QSettings s;
    const QString saved = s.value("library/kalaRoot").toString();
    return saved.isEmpty() ? (QDir::homePath() + "/Music/kala") : saved;
}

QJsonObject KalaTools::ok(const QString &message)
{
    return QJsonObject{ {"result", message} };
}

QJsonObject KalaTools::error(const QString &message)
{
    return QJsonObject{ {"error", message} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Dispatch
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::dispatchTool(const QString &toolName, const QJsonObject &args)
{
    // Suppress the "Invalid Graph Connection" modal dialog during AI tool calls.
    // Intermediate graph states are expected to be invalid while building.
    m_builder->setSuppressInvalidGraphWarning(true);
    struct Restore { SounitBuilder *b; ~Restore() { b->setSuppressInvalidGraphWarning(false); } } restore{m_builder};

    // Determine if this is a read-only call (don't clear anti-loop guards).
    const QString action = args["action"].toString();
    static const QSet<QString> kReadOnlyTools = {
        "get_graph_state", "get_composition_state", "inspect_notes"
    };
    static const QSet<QString> kReadOnlyActions = {
        "list", "get", "current"
    };
    const bool isReadOnly = kReadOnlyTools.contains(toolName)
        || (toolName == "browse_library")
        || (toolName == "select_notes" && action == "current")
        || (toolName == "engine_settings" && action == "get")
        || (toolName == "time_signature" && action == "get")
        || (toolName == "track" && action == "list")
        || (toolName == "variation" && action == "list");
    if (!isReadOnly) {
        m_graphStateJustProvided    = false;
        m_selectedNotesJustProvided = false;
    }

    // ── Consolidated tools (action-param pattern) ────────────────────────────
    if (toolName == "browse_library")        return toolBrowseLibrary(args);
    if (toolName == "history")               return toolHistory(args);
    if (toolName == "transport")             return toolTransport(args);
    if (toolName == "track")                 return toolTrack(args);
    if (toolName == "project")               return toolProject(args);
    if (toolName == "variation")             return toolVariation(args);
    if (toolName == "edit_graph")            return toolEditGraph(args);
    if (toolName == "time_signature")        return toolTimeSig(args);
    if (toolName == "engine_settings")       return toolEngineSettings(args);
    if (toolName == "inspect_notes")         return toolInspectNotes(args);
    if (toolName == "select_notes")          return toolSelectNotes(args);
    if (toolName == "set_note_mode")         return toolSetNoteMode(args);
    if (toolName == "set_dynamics")          return toolSetDynamics(args);
    if (toolName == "transform_notes")       return toolTransformNotes(args);

    // ── Standalone tools ─────────────────────────────────────────────────────
    if (toolName == "get_graph_state")       return toolGetGraphState();
    if (toolName == "get_composition_state") return toolGetCompositionState(args);
    if (toolName == "add_container")         return toolAddContainer(args);
    if (toolName == "set_parameter")         return toolSetParameter(args);
    if (toolName == "set_parameters")        return toolSetParameters(args);
    if (toolName == "connect_containers")    return toolConnectContainers(args);
    if (toolName == "load_sounit")           return toolLoadSounit(args);
    if (toolName == "save_sounit")           return toolSaveSounit(args);
    if (toolName == "play_preview")          return toolPlayPreview();
    if (toolName == "set_library_root")      return toolSetLibraryRoot(args);
    if (toolName == "load_ir")               return toolLoadIR(args);
    if (toolName == "load_spectrum")         return toolLoadSpectrum(args);
    if (toolName == "load_envelope")         return toolLoadEnvelope(args);
    if (toolName == "set_envelope_shape")    return toolSetEnvelopeShape(args);
    if (toolName == "add_note")              return toolAddNote(args);
    if (toolName == "delete_note")           return toolDeleteNote(args);
    if (toolName == "clear_notes")           return toolClearNotes(args);
    if (toolName == "set_scale")             return toolSetScale(args);
    if (toolName == "set_tempo")             return toolSetTempo(args);
    if (toolName == "duplicate_notes")       return toolDuplicateNotes(args);
    if (toolName == "set_note_vibrato")      return toolSetNoteVibrato(args);
    if (toolName == "fade_out_notes")        return toolFadeOutNotes(args);
    if (toolName == "assign_guitar_strings") return toolAssignGuitarStrings(args);
    return error("Unknown tool: " + toolName);
}

// ─────────────────────────────────────────────────────────────────────────────
// Consolidated dispatchers
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolBrowseLibrary(const QJsonObject &args)
{
    const QString type = args["type"].toString();
    if (type == "sounit")   return toolGetSounitList();
    if (type == "spectrum") return toolGetSpectrumList();
    if (type == "ir")       return toolGetIRList();
    if (type == "envelope") return toolGetEnvelopeList();
    if (type == "project")  return toolGetProjectList();
    return error("browse_library: unknown type '" + type + "'. Use sounit|spectrum|ir|envelope|project.");
}

QJsonObject KalaTools::toolHistory(const QJsonObject &args)
{
    const QString action = args["action"].toString();
    if (action == "undo") return toolUndo();
    if (action == "redo") return toolRedo();
    return error("history: unknown action '" + action + "'. Use undo|redo.");
}

QJsonObject KalaTools::toolTransport(const QJsonObject &args)
{
    const QString action = args["action"].toString();
    if (action == "play")       return toolPlayScore();
    if (action == "stop")       return toolStopScore();
    if (action == "preview")    return toolPlayPreview();
    if (action == "seek")       return toolSeek(args);
    if (action == "loop")       return toolSetLoop(args);
    if (action == "clear_loop") return toolClearLoop();
    return error("transport: unknown action '" + action + "'. Use play|stop|preview|seek|loop|clear_loop.");
}

QJsonObject KalaTools::toolTrack(const QJsonObject &args)
{
    const QString action = args["action"].toString();
    if (action == "list")   return toolGetTrackList();
    if (action == "add")    return toolAddTrack(args);
    if (action == "rename") return toolRenameTrack(args);
    if (action == "delete") return toolDeleteTrack(args);
    return error("track: unknown action '" + action + "'. Use list|add|rename|delete.");
}

QJsonObject KalaTools::toolProject(const QJsonObject &args)
{
    const QString action = args["action"].toString();
    if (action == "open") return toolOpenProject(args);
    if (action == "save") return toolSaveProject(args);
    return error("project: unknown action '" + action + "'. Use open|save.");
}

QJsonObject KalaTools::toolVariation(const QJsonObject &args)
{
    const QString action = args["action"].toString();
    if (action == "list")              return toolGetVariationList();
    if (action == "create")            return toolCreateVariation(args);
    if (action == "create_from_sounit") return toolCreateVariationFromSounit(args);
    if (action == "delete")            return toolDeleteVariation(args);
    if (action == "rename")            return toolRenameVariation(args);
    if (action == "switch")            return toolSwitchVariation(args);
    if (action == "apply")             return toolApplyVariation(args);
    if (action == "copy_to_base")      return toolCopyVariationToBase(args);
    return error("variation: unknown action '" + action + "'. Use list|create|create_from_sounit|delete|rename|switch|apply|copy_to_base.");
}

QJsonObject KalaTools::toolEditGraph(const QJsonObject &args)
{
    const QString action = args["action"].toString();
    if (action == "add")        return toolAddContainer(args);
    if (action == "remove")     return toolRemoveContainer(args);
    if (action == "rename")     return toolRenameContainer(args);
    if (action == "connect")    return toolConnectContainers(args);
    if (action == "disconnect") return toolRemoveConnection(args);
    if (action == "clear")      return toolClearGraph();
    return error("edit_graph: unknown action '" + action + "'. Use add|remove|rename|connect|disconnect|clear.");
}

QJsonObject KalaTools::toolTimeSig(const QJsonObject &args)
{
    const QString action = args["action"].toString();
    if (action == "get")    return toolGetTimeSignatureChanges();
    if (action == "set")    return toolSetTimeSignature(args);
    if (action == "add")    return toolAddTimeSignatureChange(args);
    if (action == "remove") return toolRemoveTimeSignatureChange(args);
    return error("time_signature: unknown action '" + action + "'. Use get|set|add|remove.");
}

QJsonObject KalaTools::toolEngineSettings(const QJsonObject &args)
{
    const QString action = args["action"].toString();
    if (action == "get") return toolGetEngineSettings();
    if (action == "set") return toolSetEngineSettings(args);
    return error("engine_settings: unknown action '" + action + "'. Use get|set.");
}

QJsonObject KalaTools::toolInspectNotes(const QJsonObject &args)
{
    const QString what = args["what"].toString();
    if (what == "selection") return toolGetSelectedNotes();
    if (what == "curves")   return toolGetNoteCurves(args);
    if (what == "vibrato")  return toolGetNoteVibrato(args);
    return error("inspect_notes: unknown what '" + what + "'. Use selection|curves|vibrato.");
}

QJsonObject KalaTools::toolSelectNotes(const QJsonObject &args)
{
    const QString mode = args["mode"].toString();
    if (mode == "current")       return toolGetSelectedNotes();
    if (mode == "flat_dynamics") return toolSelectFlatDynamicsNotes(args);
    if (mode == "range")         return toolSelectNotesByRange(args);
    // Default to range if no mode specified but has filter params
    if (args.contains("pitchMinHz") || args.contains("pitchMaxHz")
        || args.contains("durationMinMs") || args.contains("durationMaxMs")
        || args.contains("trackIndex") || args.contains("indices")
        || args.contains("noteIds"))
        return toolSelectNotesByRange(args);
    return error("select_notes: provide mode (current|range|flat_dynamics) or filter params.");
}

QJsonObject KalaTools::toolSetNoteMode(const QJsonObject &args)
{
    const QString mode = args["mode"].toString();
    if (mode == "legato")     return toolLinkLegato(args);
    if (mode == "unlegato")   return toolUnlinkLegato(args);
    if (mode == "continuous") return toolMakeContinuous(args);
    if (mode == "discrete")   return toolMakeDiscrete(args);
    return error("set_note_mode: unknown mode '" + mode + "'. Use legato|unlegato|continuous|discrete.");
}

QJsonObject KalaTools::toolSetDynamics(const QJsonObject &args)
{
    const QString mode = args["mode"].toString();
    if (mode == "curve")         return toolApplyDynamicsCurve(args);
    if (mode == "envelope_file") return toolLoadEnvelopeAsDynamics(args);
    if (mode == "scale")         return toolScaleDynamics(args);
    if (mode == "per_note")      return toolSetNoteDynamics(args);
    if (mode == "beat_pattern")  return toolApplyBeatDynamics(args);
    if (mode == "expressive")       return toolSetNoteCurve(args);
    if (mode == "expressive_batch") return toolSetNoteCurvesBatch(args);
    return error("set_dynamics: unknown mode '" + mode + "'. Use curve|envelope_file|scale|per_note|beat_pattern|expressive|expressive_batch.");
}

QJsonObject KalaTools::toolTransformNotes(const QJsonObject &args)
{
    const QString transform = args["transform"].toString();
    if (transform == "transpose")     return toolTransposeNotes(args);
    if (transform == "set_pitch")     return toolSetNotePitch(args);
    if (transform == "shift")         return toolShiftNotes(args);
    if (transform == "stretch")       return toolStretchNotes(args);
    if (transform == "set_duration")  return toolSetNoteDuration(args);
    if (transform == "scale_timing")  return toolScaleTiming(args);
    if (transform == "ease_rhythm")   return toolApplyRhythmEasing(args);
    if (transform == "quantize")      return toolQuantizeToScale(args);
    if (transform == "strum")         return toolStrumNotes(args);
    return error("transform_notes: unknown transform '" + transform + "'. Use transpose|set_pitch|shift|stretch|set_duration|scale_timing|ease_rhythm|quantize|strum.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

Track *KalaTools::currentTrack() const
{
    return m_trackManager ? m_trackManager->getTrack(m_currentTrackIndex) : nullptr;
}

Canvas *KalaTools::currentCanvas() const
{
    Track *t = currentTrack();
    return t ? t->getCanvas() : nullptr;
}

Container *KalaTools::findContainer(const QString &instanceName) const
{
    Canvas *cv = currentCanvas();
    if (!cv) return nullptr;
    const auto list = cv->findChildren<Container*>();
    for (Container *c : list) {
        if (c->getInstanceName() == instanceName)
            return c;
    }
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: get_graph_state
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolGetGraphState()
{
    if (m_graphStateJustProvided)
        return QJsonObject{{"result", "Graph state was already provided in the previous response. Use that data and proceed with your task — do not call get_graph_state again until you have made a change."}};

    Canvas *cv = currentCanvas();
    if (!cv)
        return error("No active track/canvas.");

    // Build a compact representation — skip large binary blobs (wavetable data,
    // IR samples, custom DNA amplitude arrays) that would flood the context window.
    QJsonArray containers;
    for (const Container *c : cv->findChildren<Container*>()) {
        QJsonObject params;
        for (auto it = c->getParameters().cbegin(); it != c->getParameters().cend(); ++it)
            params[it.key()] = it.value();

        QJsonObject cObj;
        cObj["type"]         = c->getName();
        cObj["instanceName"] = c->getInstanceName();
        cObj["position"]     = QJsonObject{{"x", c->pos().x()}, {"y", c->pos().y()}};
        cObj["parameters"]   = params;
        containers.append(cObj);
    }

    QJsonArray connections;
    for (const Canvas::Connection &conn : cv->getConnections()) {
        QJsonObject connObj;
        connObj["from"]     = conn.fromContainer ? conn.fromContainer->getInstanceName() : "";
        connObj["fromPort"] = conn.fromPort;
        connObj["to"]       = conn.toContainer   ? conn.toContainer->getInstanceName()   : "";
        connObj["toPort"]   = conn.toPort;
        connObj["function"] = conn.function;
        connObj["weight"]   = conn.weight;
        connections.append(connObj);
    }

    QJsonObject result;
    result["sounitName"]  = cv->getSounitName();
    result["containers"]  = containers;
    result["connections"] = connections;
    m_graphStateJustProvided = true;
    return QJsonObject{ {"result", result} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: get_sounit_list
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolGetSounitList()
{
    QJsonArray files;
    QSet<QString> seen;

    // Helper: add a .sounit file if not already seen
    auto addFile = [&](const QFileInfo &fi) {
        const QString abs = fi.absoluteFilePath();
        if (seen.contains(abs)) return;
        seen.insert(abs);
        // Build a relative label: category/name or just name
        const QString rootPath = libraryRoot() + "/sounit";
        QString relPath = QDir(rootPath).relativeFilePath(abs);
        // Strip the .sounit extension from the display name
        QString label = relPath;
        if (label.endsWith(".sounit")) label.chop(7);
        files.append(QJsonObject{
            {"name", label},
            {"path", abs}
        });
    };

    // 1. Last-used directory (whatever the user had open)
    QSettings settings;
    const QString lastDir = settings.value("lastDirectory/sounit").toString();
    if (!lastDir.isEmpty()) {
        QDir dir(lastDir);
        if (dir.exists()) {
            for (const QFileInfo &fi : dir.entryInfoList({"*.sounit"}, QDir::Files, QDir::Name))
                addFile(fi);
        }
    }

    // 2. Entire sounit library — recursive scan of <libraryRoot>/sounit
    const QString sounitRoot = libraryRoot() + "/sounit";
    QDirIterator it(sounitRoot, {"*.sounit"}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        addFile(it.fileInfo());
    }

    return QJsonObject{ {"result", files} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: clear_graph
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolClearGraph()
{
    Track *t = currentTrack();
    if (!t) return error("No active track.");

    // newSounit() clears containers + connections and resets sounit metadata.
    t->newSounit();

    // Re-sync SounitBuilder so it points at the freshly cleared canvas.
    m_builder->setTrackCanvas(t);
    m_builder->rebuildGraph(m_currentTrackIndex);

    return ok("Graph cleared.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: add_container
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolAddContainer(const QJsonObject &args)
{
    const QString type = args["type"].toString();
    if (type.isEmpty()) return error("'type' is required.");

    Canvas *cv = currentCanvas();
    if (!cv) return error("No active track/canvas.");

    // Look up ports for this type
    QStringList inputs, outputs;
    Canvas::getPortsForContainerType(type, inputs, outputs);
    if (inputs.isEmpty() && outputs.isEmpty())
        return error("Unknown container type: " + type);

    // Snapshot existing instance names so we can find the new one afterwards
    QSet<QString> beforeNames;
    for (const Container *c : cv->findChildren<Container*>())
        beforeNames.insert(c->getInstanceName());

    // Create via the authoritative path — this sets all defaults and pushes undo
    m_builder->onAddContainer(type, colorForType(type), inputs, outputs);

    // Find the newly created container (the one not in beforeNames)
    Container *newC = nullptr;
    for (Container *c : cv->findChildren<Container*>()) {
        if (!beforeNames.contains(c->getInstanceName())) {
            newC = c;
            break;
        }
    }
    if (!newC) return error("Container was created but could not be located.");

    // Apply any extra parameter overrides from the args.
    // String values route to setStringParameter (e.g. scoreCurveName);
    // numeric values route to setParameter.
    const QJsonObject params = args["params"].toObject();
    if (!params.isEmpty()) {
        newC->beginParameterUpdate();
        for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
            const QJsonValue &v = it.value();
            if (v.isString()) {
                newC->setStringParameter(it.key(), v.toString());
                // scoreCurveName must also be registered on the canvas, otherwise
                // it won't appear in the inspector dropdown or be available to notes.
                if (it.key() == QLatin1String("scoreCurveName") && !v.toString().isEmpty())
                    cv->addExpressiveCurveName(v.toString());
            } else {
                newC->setParameter(it.key(), v.toDouble());
            }
        }
        newC->endParameterUpdate();
    }

    // Apply optional position
    if (args.contains("position")) {
        const QJsonObject pos = args["position"].toObject();
        newC->move(pos["x"].toInt(100), pos["y"].toInt(100));
        cv->update();
    }

    return QJsonObject{ {"result", newC->getInstanceName()} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_parameter
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetParameter(const QJsonObject &args)
{
    const QString instanceName = args["instanceName"].toString();
    const QString param        = args["param"].toString();
    if (instanceName.isEmpty()) return error("'instanceName' is required.");
    if (param.isEmpty())        return error("'param' is required.");
    if (!args.contains("value")) return error("'value' is required.");

    Container *c = findContainer(instanceName);
    if (!c) return error("Container not found: " + instanceName);

    const QJsonValue &v = args["value"];
    if (v.isString()) {
        c->setStringParameter(param, v.toString());
        if (param == QLatin1String("scoreCurveName") && !v.toString().isEmpty())
            if (Canvas *cv = currentCanvas())
                cv->addExpressiveCurveName(v.toString());
    } else {
        c->setParameter(param, v.toDouble());
    }
    // parameterChanged() signal fires automatically → rebuildGraph via existing connection
    return ok("Parameter set.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_parameters (batch)
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetParameters(const QJsonObject &args)
{
    const QJsonArray changes = args["changes"].toArray();
    if (changes.isEmpty()) return error("'changes' array is required and must not be empty.");

    QJsonArray errors;
    int applied = 0;

    for (const QJsonValue &v : changes) {
        const QJsonObject ch = v.toObject();
        const QString instanceName = ch["instanceName"].toString();
        const QString param        = ch["param"].toString();
        if (instanceName.isEmpty() || param.isEmpty() || !ch.contains("value")) {
            errors.append(QString("Skipped malformed entry (missing instanceName/param/value)."));
            continue;
        }
        Container *c = findContainer(instanceName);
        if (!c) {
            errors.append(QString("Container not found: %1").arg(instanceName));
            continue;
        }
        const QJsonValue &val = ch["value"];
        if (val.isString()) {
            c->setStringParameter(param, val.toString());
            if (param == QLatin1String("scoreCurveName") && !val.toString().isEmpty())
                if (Canvas *cv = currentCanvas())
                    cv->addExpressiveCurveName(val.toString());
        } else {
            c->setParameter(param, val.toDouble());
        }
        ++applied;
    }

    QJsonObject result;
    result["applied"] = applied;
    if (!errors.isEmpty()) result["errors"] = errors;
    result["message"] = QString("Set %1 parameter(s).").arg(applied);
    return QJsonObject{{"result", result}};
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: connect_containers
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolConnectContainers(const QJsonObject &args)
{
    const QString fromInst = args["fromInstance"].toString();
    const QString fromPort = args["fromPort"].toString();
    const QString toInst   = args["toInstance"].toString();
    const QString toPort   = args["toPort"].toString();

    if (fromInst.isEmpty() || fromPort.isEmpty() ||
        toInst.isEmpty()   || toPort.isEmpty())
        return error("fromInstance, fromPort, toInstance and toPort are all required.");

    Canvas *cv = currentCanvas();
    if (!cv) return error("No active canvas.");

    Container *from = findContainer(fromInst);
    Container *to   = findContainer(toInst);
    if (!from) return error("Source container not found: "      + fromInst);
    if (!to)   return error("Destination container not found: " + toInst);

    // Validate port names against the authoritative port list
    QStringList fromInputs, fromOutputs, toInputs, toOutputs;
    Canvas::getPortsForContainerType(from->getName(), fromInputs, fromOutputs);
    Canvas::getPortsForContainerType(to->getName(),   toInputs,   toOutputs);

    if (!fromOutputs.contains(fromPort))
        return error(QString("'%1' is not an output port of %2. Valid outputs: %3")
                     .arg(fromPort, from->getName(), fromOutputs.join(", ")));
    if (!toInputs.contains(toPort))
        return error(QString("'%1' is not an input port of %2. Valid inputs: %3")
                     .arg(toPort, to->getName(), toInputs.join(", ")));

    Canvas::Connection conn;
    conn.fromContainer = from;
    conn.fromPort      = fromPort;
    conn.toContainer   = to;
    conn.toPort        = toPort;
    conn.function      = args.value("function").toString("passthrough");
    conn.weight        = args.value("weight").toDouble(1.0);

    // Push through undo stack — redo() is called immediately, adding the connection
    // and emitting graphChanged() which triggers rebuildGraph.
    cv->getUndoStack()->push(new AddConnectionCommand(conn, cv));

    return ok(QString("Connected %1:%2 → %3:%4.")
              .arg(fromInst, fromPort, toInst, toPort));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: remove_container
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolRemoveContainer(const QJsonObject &args)
{
    const QString instanceName = args["instanceName"].toString();
    if (instanceName.isEmpty()) return error("'instanceName' is required.");

    Canvas *cv = currentCanvas();
    if (!cv) return error("No active canvas.");

    Container *c = findContainer(instanceName);
    if (!c) return error("Container not found: " + instanceName);

    cv->getUndoStack()->push(new DeleteContainerCommand(c, cv));
    return ok("Container removed: " + instanceName);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: rename_container (edit_graph action="rename")
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolRenameContainer(const QJsonObject &args)
{
    const QString instanceName = args["instanceName"].toString();
    const QString newName      = args["newName"].toString();
    if (instanceName.isEmpty()) return error("'instanceName' is required.");
    if (newName.isEmpty())      return error("'newName' is required.");
    if (instanceName == newName) return ok("No change: name already matches.");

    Canvas *cv = currentCanvas();
    if (!cv) return error("No active canvas.");

    Container *target = findContainer(instanceName);
    if (!target) return error("Container not found: " + instanceName);

    // Reject collisions — keeps instance names unique within the canvas.
    if (findContainer(newName))
        return error("Name already in use: " + newName);

    target->setInstanceName(newName);
    if (QLabel *label = target->findChild<QLabel*>("labelInstanceName"))
        label->setText(newName);
    target->update();
    cv->update();

    return ok("Renamed: " + instanceName + " → " + newName);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: load_sounit
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolLoadSounit(const QJsonObject &args)
{
    const QString filePath = args["filePath"].toString();
    if (filePath.isEmpty()) return error("'filePath' is required.");

    Track *t = currentTrack();
    if (!t) return error("No active track.");

    if (!t->loadSounit(filePath))
        return error("Failed to load sounit from: " + filePath);

    // Connect container signals for all newly loaded containers
    Canvas *cv = t->getCanvas();
    for (Container *c : cv->findChildren<Container*>())
        m_builder->connectContainerSignals(c);

    m_builder->rebuildGraph(m_currentTrackIndex);
    return ok("Sounit loaded: " + filePath);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: save_sounit
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSaveSounit(const QJsonObject &args)
{
    const QString filePath = args["filePath"].toString();
    const QString name     = args.value("name").toString();
    if (filePath.isEmpty()) return error("'filePath' is required.");

    Track *t = currentTrack();
    if (!t) return error("No active track.");

    if (!name.isEmpty())
        t->getCanvas()->setSounitName(name);

    if (!t->saveSounit(filePath))
        return error("Failed to save sounit to: " + filePath);

    return ok("Sounit saved to: " + filePath);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: play_preview
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolPlayPreview()
{
    m_builder->triggerPlay();
    return ok("Playback started.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: get_spectrum_list
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolGetSpectrumList()
{
    QJsonArray files;
    const QString spectrumDir = libraryRoot() + "/spectrum";
    QDirIterator it(spectrumDir, {"*.dna.json"}, QDir::Files);
    while (it.hasNext()) {
        it.next();
        const QFileInfo &fi = it.fileInfo();
        QString name = fi.fileName();
        if (name.endsWith(".dna.json")) name.chop(9);  // strip .dna.json
        files.append(QJsonObject{
            {"name", name},
            {"path", fi.absoluteFilePath()}
        });
    }
    // Sort by name
    QJsonArray sorted;
    QVector<QJsonObject> vec;
    for (const QJsonValue &v : files) vec.append(v.toObject());
    std::sort(vec.begin(), vec.end(), [](const QJsonObject &a, const QJsonObject &b){
        return a["name"].toString() < b["name"].toString();
    });
    for (const QJsonObject &o : vec) sorted.append(o);
    return QJsonObject{ {"result", sorted} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: get_project_list
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolGetProjectList()
{
    QJsonArray files;
    const QString projectDir = libraryRoot() + "/projects";
    QDir dir(projectDir);
    if (!dir.exists()) return error("Projects directory not found: " + projectDir);

    const auto entries = dir.entryInfoList({"*.kala"}, QDir::Files, QDir::Name);
    for (const QFileInfo &fi : entries) {
        files.append(QJsonObject{
            {"name", fi.completeBaseName()},
            {"path", fi.absoluteFilePath()}
        });
    }
    return QJsonObject{ {"result", files} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: get_ir_list
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolGetIRList()
{
    QJsonArray files;
    const QString irRoot = libraryRoot() + "/impulse responses";
    QDirIterator it(irRoot, {"*.wav"}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        const QFileInfo &fi = it.fileInfo();
        // Build a label: subfolder/filename or just filename
        QString relPath = QDir(irRoot).relativeFilePath(fi.absoluteFilePath());
        files.append(QJsonObject{
            {"name", relPath},
            {"path", fi.absoluteFilePath()}
        });
    }
    QJsonArray sorted;
    QVector<QJsonObject> vec;
    for (const QJsonValue &v : files) vec.append(v.toObject());
    std::sort(vec.begin(), vec.end(), [](const QJsonObject &a, const QJsonObject &b){
        return a["name"].toString() < b["name"].toString();
    });
    for (const QJsonObject &o : vec) sorted.append(o);
    return QJsonObject{ {"result", sorted} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_library_root
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetLibraryRoot(const QJsonObject &args)
{
    const QString path = args["path"].toString().trimmed();
    if (path.isEmpty()) return error("'path' is required.");

    QDir dir(path);
    if (!dir.exists()) return error("Directory does not exist: " + path);

    QSettings s;
    s.setValue("library/kalaRoot", path);
    return ok(QString("Library root set to: %1").arg(path));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: load_ir
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolLoadIR(const QJsonObject &args)
{
    const QString instanceName = args["instanceName"].toString();
    const QString filePath     = args["filePath"].toString();
    if (instanceName.isEmpty()) return error("'instanceName' is required.");
    if (filePath.isEmpty())     return error("'filePath' is required.");

    Container *c = findContainer(instanceName);
    if (!c) return error("Container not found: " + instanceName);
    if (c->getName() != "IR Convolution")
        return error(instanceName + " is not an IR Convolution container.");

    std::vector<float> wavData;
    if (!WavetableSynth::loadWavFile(filePath, wavData))
        return error("Failed to load WAV file: " + filePath);

    c->setIRData(wavData);
    c->setIRFilePath(filePath);
    m_builder->rebuildGraph(m_currentTrackIndex);

    return ok(QString("IR loaded: %1 (%2 samples)")
              .arg(QFileInfo(filePath).fileName())
              .arg(static_cast<int>(wavData.size())));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: load_spectrum
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolLoadSpectrum(const QJsonObject &args)
{
    const QString instanceName = args["instanceName"].toString();
    const QString filePath     = args["filePath"].toString();
    if (instanceName.isEmpty()) return error("'instanceName' is required.");
    if (filePath.isEmpty())     return error("'filePath' is required.");

    Container *c = findContainer(instanceName);
    if (!c) return error("Container not found: " + instanceName);
    if (c->getName() != "Harmonic Generator")
        return error(instanceName + " is not a Harmonic Generator container.");

    // Read the .dna.json file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return error("Cannot open file: " + filePath);

    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &pe);
    file.close();

    if (pe.error != QJsonParseError::NoError)
        return error("JSON parse error: " + pe.errorString());

    const QJsonObject root = doc.object();
    const QJsonArray amplitudes = root["amplitudes"].toArray();
    if (amplitudes.isEmpty())
        return error("No 'amplitudes' array found in file.");

    const int count = amplitudes.size();
    const QString spectrumName = root.value("name").toString(QFileInfo(filePath).completeBaseName());

    // Push all amplitude values as numeric parameters — same format the app uses internally
    c->beginParameterUpdate();
    c->setParameter("dnaSelect",    -1.0);
    c->setParameter("customDnaCount", static_cast<double>(count));
    for (int i = 0; i < count; ++i)
        c->setParameter(QString("customDna_%1").arg(i), amplitudes[i].toDouble());
    c->endParameterUpdate();

    // Store the display name so the inspector shows it correctly
    c->setCustomDnaName(spectrumName);

    return ok(QString("Spectrum '%1' loaded (%2 harmonics).").arg(spectrumName).arg(count));
}

// ─────────────────────────────────────────────────────────────────────────────
// Envelope helpers
// ─────────────────────────────────────────────────────────────────────────────

// Parse a JSON object/array of points into an EnvelopeData and apply it to a container.
static QString applyEnvelopeData(Container *c, const EnvelopeData &env,
                                  SounitBuilder *builder, int trackIndex)
{
    c->setParameter("envelopeSelect", 7.0);   // 7 = Custom
    c->setCustomEnvelopeData(env);
    builder->rebuildGraph(trackIndex);
    return QString("Envelope '%1' applied (%2 points, loopMode=%3).")
               .arg(env.name).arg(env.points.size()).arg(env.loopMode);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: get_envelope_list
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolGetEnvelopeList()
{
    QJsonArray files;
    const QString envelopeDir = libraryRoot() + "/envelopes";
    QDirIterator it(envelopeDir, {"*.env.json"}, QDir::Files);
    while (it.hasNext()) {
        it.next();
        const QFileInfo &fi = it.fileInfo();
        QString name = fi.fileName();
        if (name.endsWith(".env.json")) name.chop(9);
        files.append(QJsonObject{ {"name", name}, {"path", fi.absoluteFilePath()} });
    }
    QVector<QJsonObject> vec;
    for (const QJsonValue &v : files) vec.append(v.toObject());
    std::sort(vec.begin(), vec.end(), [](const QJsonObject &a, const QJsonObject &b){
        return a["name"].toString() < b["name"].toString();
    });
    QJsonArray sorted;
    for (const QJsonObject &o : vec) sorted.append(o);
    return QJsonObject{ {"result", sorted} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: load_envelope
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolLoadEnvelope(const QJsonObject &args)
{
    const QString instanceName = args["instanceName"].toString();
    const QString filePath     = args["filePath"].toString();
    if (instanceName.isEmpty()) return error("'instanceName' is required.");
    if (filePath.isEmpty())     return error("'filePath' is required.");

    Container *c = findContainer(instanceName);
    if (!c) return error("Container not found: " + instanceName);
    if (c->getName() != "Envelope Engine")
        return error(instanceName + " is not an Envelope Engine container.");

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return error("Cannot open file: " + filePath);

    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &pe);
    file.close();
    if (pe.error != QJsonParseError::NoError)
        return error("JSON parse error: " + pe.errorString());

    const QJsonObject root = doc.object();
    const QJsonArray  pointsArr = root["points"].toArray();
    if (pointsArr.isEmpty()) return error("No 'points' array found in file.");

    EnvelopeData env;
    env.name     = root.value("name").toString(QFileInfo(filePath).completeBaseName());
    env.loopMode = root.value("loopMode").toInt(0);
    for (const QJsonValue &pv : pointsArr) {
        const QJsonObject po = pv.toObject();
        env.points.append(EnvelopePoint(
            po["time"].toDouble(),
            po["value"].toDouble(),
            po["curveType"].toInt(1)   // default Smooth
        ));
    }

    return ok(applyEnvelopeData(c, env, m_builder, m_currentTrackIndex));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_envelope_shape
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetEnvelopeShape(const QJsonObject &args)
{
    const QString instanceName = args["instanceName"].toString();
    if (instanceName.isEmpty()) return error("'instanceName' is required.");

    const QJsonArray pointsArr = args["points"].toArray();
    if (pointsArr.isEmpty()) return error("'points' array is required and must not be empty.");

    Container *c = findContainer(instanceName);
    if (!c) return error("Container not found: " + instanceName);
    if (c->getName() != "Envelope Engine")
        return error(instanceName + " is not an Envelope Engine container.");

    EnvelopeData env;
    env.name     = args.value("name").toString("Custom");
    env.loopMode = args.value("loopMode").toInt(0);
    for (const QJsonValue &pv : pointsArr) {
        const QJsonObject po = pv.toObject();
        if (!po.contains("time") || !po.contains("value"))
            return error("Each point must have 'time' and 'value' fields.");
        env.points.append(EnvelopePoint(
            po["time"].toDouble(),
            po["value"].toDouble(),
            po.value("curveType").toInt(1)   // default Smooth
        ));
    }

    // Validate: first point should be at time=0, last at time=1
    if (env.points.first().time != 0.0)
        env.points.prepend(EnvelopePoint(0.0, env.points.first().value, 1));
    if (env.points.last().time != 1.0)
        env.points.append(EnvelopePoint(1.0, env.points.last().value, 1));

    return ok(applyEnvelopeData(c, env, m_builder, m_currentTrackIndex));
}

// ─────────────────────────────────────────────────────────────────────────────
// Composition tool helpers
// ─────────────────────────────────────────────────────────────────────────────

static ScoreCanvas *getScoreCanvas(ScoreCanvasWindow *w) {
    return w ? w->getScoreCanvas() : nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: get_composition_state
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolGetCompositionState(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const int filterTrack = args.value("trackIndex").toInt(-1);
    const Phrase &phrase = sc->getPhrase();
    const QVector<Note> &notes = phrase.getNotes();

    QJsonArray noteArr;
    for (const Note &n : notes) {
        if (filterTrack >= 0 && n.getTrackIndex() != filterTrack)
            continue;
        QJsonObject obj;
        obj["id"]         = n.getId();
        obj["startTime"]  = n.getStartTime();   // ms
        obj["duration"]   = n.getDuration();    // ms
        obj["pitchHz"]    = n.getPitchHz();
        obj["dynamics"]   = n.getDynamics();
        obj["trackIndex"] = n.getTrackIndex();
        noteArr.append(obj);
    }

    QJsonObject result;
    result["noteCount"]   = noteArr.size();
    result["notes"]       = noteArr;
    result["scale"]       = sc->getCurrentScale().getName();
    result["baseFreqHz"]  = sc->getBaseFrequency();
    result["tempo"]       = sc->getDefaultTempo();
    result["timeSigNum"]  = sc->getDefaultTimeSigNum();
    result["timeSigDenom"]= sc->getDefaultTimeSigDenom();
    return QJsonObject{ {"result", result} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: get_track_list
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolGetTrackList()
{
    if (!m_trackManager) return error("Track manager not available.");

    QJsonArray tracks;
    const int count = m_trackManager->getTrackCount();
    for (int i = 0; i < count; ++i) {
        Track *t = m_trackManager->getTrack(i);
        if (!t) continue;
        QJsonObject obj;
        obj["index"]     = i;
        obj["name"]      = t->getName();
        obj["sounitName"] = t->getCanvas() ? t->getCanvas()->getSounitName() : QString();
        tracks.append(obj);
    }
    return QJsonObject{ {"result", tracks} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: add_note
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolAddNote(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    if (!args.contains("startTime") || !args.contains("duration") || !args.contains("pitchHz"))
        return error("startTime, duration and pitchHz are required.");

    const double startTime  = args["startTime"].toDouble();
    const double duration   = args["duration"].toDouble();
    const double pitchHz    = args["pitchHz"].toDouble();
    const double dynamics   = args.value("dynamics").toDouble(0.7);
    const int    trackIndex = args.value("trackIndex").toInt(m_currentTrackIndex);

    if (startTime < 0) return error("startTime must be >= 0.");
    if (duration  <= 0) return error("duration must be > 0.");
    if (pitchHz   <= 0) return error("pitchHz must be > 0.");

    Note note(startTime, duration, pitchHz, dynamics);
    note.setTrackIndex(trackIndex);

    Phrase *phrase = &sc->getPhrase();
    sc->getUndoStack()->push(new AddNoteCommand(phrase, note, sc));

    // The note was appended — return the ID of the last note
    const auto &notes = phrase->getNotes();
    const QString addedId = notes.isEmpty() ? QString() : notes.last().getId();
    return QJsonObject{ {"result", QJsonObject{{"id", addedId}, {"message", "Note added."}}} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: delete_note
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolDeleteNote(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QString noteId = args["id"].toString();
    if (noteId.isEmpty()) return error("'id' is required. Use get_composition_state to find note IDs.");

    Phrase *phrase = &sc->getPhrase();
    const auto &notes = phrase->getNotes();
    int idx = -1;
    for (int i = 0; i < notes.size(); ++i) {
        if (notes[i].getId() == noteId) { idx = i; break; }
    }
    if (idx < 0) return error("Note not found: " + noteId);

    sc->getUndoStack()->push(new DeleteNoteCommand(phrase, idx, sc));
    return ok("Note deleted: " + noteId);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: clear_notes
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolClearNotes(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const int filterTrack = args.value("trackIndex").toInt(-1);

    Phrase *phrase = &sc->getPhrase();

    if (filterTrack < 0) {
        // Clear all notes — collect indices in reverse order and delete
        QVector<int> indices;
        const auto &notes = phrase->getNotes();
        for (int i = 0; i < notes.size(); ++i) indices.prepend(i);

        sc->getUndoStack()->beginMacro("Clear all notes");
        for (int idx : indices)
            sc->getUndoStack()->push(new DeleteNoteCommand(phrase, idx, sc));
        sc->getUndoStack()->endMacro();

        return ok(QString("Cleared all notes."));
    } else {
        // Clear only notes on the given track
        QVector<int> indices;
        const auto &notes = phrase->getNotes();
        for (int i = 0; i < notes.size(); ++i) {
            if (notes[i].getTrackIndex() == filterTrack)
                indices.prepend(i);
        }
        if (indices.isEmpty())
            return ok(QString("No notes found on track %1.").arg(filterTrack));

        sc->getUndoStack()->beginMacro(QString("Clear notes on track %1").arg(filterTrack));
        for (int idx : indices)
            sc->getUndoStack()->push(new DeleteNoteCommand(phrase, idx, sc));
        sc->getUndoStack()->endMacro();

        return ok(QString("Cleared %1 notes on track %2.").arg(indices.size()).arg(filterTrack));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_scale
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetScale(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    // Accept either scaleId (int) or scaleName (string)
    Scale newScale;
    bool found = false;

    if (args.contains("scaleId")) {
        const int id = args["scaleId"].toInt();
        const auto all = Scale::getAllScales();
        for (const Scale &s : all) {
            if (s.getScaleId() == id) { newScale = s; found = true; break; }
        }
        if (!found) return error(QString("No scale with id %1.").arg(id));
    } else if (args.contains("scaleName")) {
        const QString name = args["scaleName"].toString().toLower();
        const auto all = Scale::getAllScales();
        for (const Scale &s : all) {
            if (s.getName().toLower() == name) { newScale = s; found = true; break; }
        }
        if (!found) return error("Scale not found: " + args["scaleName"].toString() +
                                 ". Use get_composition_state to see current scale or try another name.");
    } else {
        return error("Either 'scaleId' or 'scaleName' is required.");
    }

    sc->setScale(newScale);

    if (args.contains("baseFreqHz"))
        sc->setBaseFrequency(args["baseFreqHz"].toDouble());

    sc->generateScaleLines();
    sc->update();
    emit sc->scaleSettingsChanged();

    return ok(QString("Scale set to '%1' (base %.1f Hz).")
              .arg(newScale.getName())
              .arg(sc->getBaseFrequency()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_tempo
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetTempo(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    if (!args.contains("bpm")) return error("'bpm' is required.");
    const double bpm = args["bpm"].toDouble();
    if (bpm < 20 || bpm > 300) return error("bpm must be between 20 and 300.");

    sc->setDefaultTempo(bpm);
    sc->update();
    m_scoreCanvasWindow->refreshToolbar();

    return ok(QString("Tempo set to %1 BPM.").arg(bpm));
}

// ─────────────────────────────────────────────────────────────────────────────
// Score canvas operations — shared helper
// ─────────────────────────────────────────────────────────────────────────────

// Resolve an array of note-ID strings to phrase indices. If noteIds is empty,
// returns ALL indices (so callers can optionally accept an "all notes" shortcut).
static QVector<int> resolveNoteIndices(ScoreCanvas *sc, const QJsonArray &noteIds)
{
    const QVector<Note> &notes = sc->getPhrase().getNotes();
    QVector<int> indices;

    if (noteIds.isEmpty()) {
        for (int i = 0; i < notes.size(); ++i) indices.append(i);
        return indices;
    }

    QSet<QString> idSet;
    for (const QJsonValue &v : noteIds) idSet.insert(v.toString());
    for (int i = 0; i < notes.size(); ++i) {
        if (idSet.contains(notes[i].getId())) indices.append(i);
    }
    return indices;
}

// Build a QVector<EnvelopePoint> from a JSON points array.
// Each point: {time, value, curveType (optional, default 1=Smooth)}.
static QVector<EnvelopePoint> parseEnvelopePoints(const QJsonArray &arr, QString &errorOut)
{
    QVector<EnvelopePoint> pts;
    for (const QJsonValue &pv : arr) {
        const QJsonObject po = pv.toObject();
        if (!po.contains("time") || !po.contains("value")) {
            errorOut = "Each point must have 'time' and 'value'.";
            return {};
        }
        pts.append(EnvelopePoint(po["time"].toDouble(),
                                 po["value"].toDouble(),
                                 po.value("curveType").toInt(1)));
    }
    return pts;
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: get_variation_list
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolGetVariationList()
{
    Track *t = currentTrack();
    if (!t) return error("No active track.");

    QJsonArray vars;
    vars.append(QJsonObject{ {"index", 0}, {"name", "Base Sounit"} });

    const int count = t->getVariationCount();
    for (int i = 1; i <= count; ++i) {
        vars.append(QJsonObject{ {"index", i}, {"name", t->getVariationName(i)} });
    }
    return QJsonObject{ {"result", vars} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: apply_variation
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolApplyVariation(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");
    if (!args.contains("variationIndex")) return error("'variationIndex' is required.");

    const int varIndex = args["variationIndex"].toInt();
    const QJsonArray noteIds = args["noteIds"].toArray();

    QVector<int> indices = resolveNoteIndices(sc, noteIds);
    if (indices.isEmpty()) return error("No matching notes found.");

    QVector<Note> &notes = sc->getPhrase().getNotes();
    for (int idx : indices) {
        if (idx >= 0 && idx < notes.size()) {
            notes[idx].setVariationIndex(varIndex);
            notes[idx].setRenderDirty(true);
        }
    }
    sc->update();
    return ok(QString("Variation %1 applied to %2 note(s).").arg(varIndex).arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: create_variation
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolCreateVariation(const QJsonObject &args)
{
    Track *track = currentTrack();
    if (!track) return error("No current track.");
    if (!args.contains("name")) return error("'name' is required.");

    const QString name = args["name"].toString().trimmed();
    if (name.isEmpty()) return error("Variation name must not be empty.");

    int index = track->createVariation(name);
    if (index <= 0) return error("Failed to create variation '" + name + "'. Graph may be empty or invalid.");

    emit m_builder->variationsChanged();
    return ok(QString("Variation %1 '%2' created from current canvas.").arg(index).arg(name));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: create_variation_from_sounit
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolCreateVariationFromSounit(const QJsonObject &args)
{
    Track *track = currentTrack();
    if (!track) return error("No current track.");
    if (!args.contains("filePath")) return error("'filePath' is required.");
    if (!args.contains("name"))     return error("'name' is required.");

    const QString filePath = args["filePath"].toString();
    const QString name     = args["name"].toString().trimmed();
    if (name.isEmpty()) return error("Variation name must not be empty.");

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return error("Cannot open file: " + filePath);

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull() || !doc.isObject())
        return error("Invalid sounit file: " + filePath);

    QJsonObject root = doc.object();
    QJsonObject graphData;
    graphData["containers"] = root["containers"];
    graphData["connections"] = root["connections"];
    graphData["expressiveCurveNames"] =
        root["sounit"].toObject()["expressiveCurveNames"].toArray();

    int index = track->createVariationFromJson(graphData, name);
    if (index <= 0) return error("Failed to build a valid graph from '" + filePath + "'.");

    emit m_builder->variationsChanged();
    return ok(QString("Variation %1 '%2' created from '%3'.")
              .arg(index).arg(name).arg(QFileInfo(filePath).fileName()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: delete_variation
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolDeleteVariation(const QJsonObject &args)
{
    Track *track = currentTrack();
    if (!track) return error("No current track.");
    if (!args.contains("variationIndex")) return error("'variationIndex' is required.");

    const int index = args["variationIndex"].toInt();
    if (index < 1 || index > track->getVariationCount())
        return error(QString("Variation index %1 out of range (1–%2).").arg(index).arg(track->getVariationCount()));

    const QString name = track->getVariationName(index);
    if (!track->deleteVariation(index))
        return error(QString("Failed to delete variation %1.").arg(index));

    emit m_builder->variationsChanged();
    return ok(QString("Variation %1 '%2' deleted.").arg(index).arg(name));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: rename_variation
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolRenameVariation(const QJsonObject &args)
{
    Track *track = currentTrack();
    if (!track) return error("No current track.");
    if (!args.contains("variationIndex")) return error("'variationIndex' is required.");
    if (!args.contains("name"))           return error("'name' is required.");

    const int index = args["variationIndex"].toInt();
    const QString newName = args["name"].toString().trimmed();
    if (newName.isEmpty()) return error("New name must not be empty.");
    if (index < 1 || index > track->getVariationCount())
        return error(QString("Variation index %1 out of range (1–%2).").arg(index).arg(track->getVariationCount()));

    const QString oldName = track->getVariationName(index);
    if (!track->setVariationName(index, newName))
        return error(QString("Failed to rename variation %1.").arg(index));

    emit m_builder->variationsChanged();
    return ok(QString("Variation %1 renamed from '%2' to '%3'.").arg(index).arg(oldName).arg(newName));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: switch_variation
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSwitchVariation(const QJsonObject &args)
{
    Track *track = currentTrack();
    if (!track) return error("No current track.");
    if (!args.contains("variationIndex")) return error("'variationIndex' is required.");

    const int index = args["variationIndex"].toInt();
    if (index < 0 || (index > 0 && index > track->getVariationCount()))
        return error(QString("Variation index %1 out of range (0–%2).").arg(index).arg(track->getVariationCount()));

    if (!track->loadVariationToCanvas(index))
        return ok("Already showing this variation.");

    m_builder->rebuildGraph(track->getTrackId());

    Canvas *canvas = track->getCanvas();
    canvas->cancelPendingConnection();
    for (Container *container : canvas->findChildren<Container*>())
        m_builder->connectContainerSignals(container);

    const QString label = (index == 0) ? "base sounit" : QString("variation %1 '%2'").arg(index).arg(track->getVariationName(index));

    // Include graph state and variation list in the response so the model
    // doesn't need to call get_graph_state / get_variation_list afterwards.
    QJsonObject graphResult = toolGetGraphState()["result"].toObject();

    QJsonArray vars;
    vars.append(QJsonObject{ {"index", 0}, {"name", "Base Sounit"} });
    const int vcount = track->getVariationCount();
    for (int i = 1; i <= vcount; ++i)
        vars.append(QJsonObject{ {"index", i}, {"name", track->getVariationName(i)} });

    QJsonObject result;
    result["message"]    = QString("Switched to %1. Canvas is ready for editing.").arg(label);
    result["variations"] = vars;
    result["graph"]      = graphResult;
    m_graphStateJustProvided = true;
    return QJsonObject{ {"result", result} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: copy_variation_to_base
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolCopyVariationToBase(const QJsonObject &args)
{
    Track *track = currentTrack();
    if (!track) return error("No current track.");
    if (!args.contains("variationIndex")) return error("'variationIndex' is required.");

    const int index = args["variationIndex"].toInt();
    if (index <= 0 || index > track->getVariationCount())
        return error(QString("Variation index %1 out of range (1–%2).").arg(index).arg(track->getVariationCount()));

    // Load the variation onto the canvas
    if (!track->loadVariationToCanvas(index))
        return error(QString("Failed to load variation %1 onto canvas.").arg(index));

    // Persist it as the new base sounit state
    track->saveBaseCanvasState();

    // Rebuild audio graph and reconnect signals
    m_builder->rebuildGraph(track->getTrackId());
    Canvas *canvas = track->getCanvas();
    canvas->cancelPendingConnection();
    for (Container *container : canvas->findChildren<Container*>())
        m_builder->connectContainerSignals(container);

    const QString varName = track->getVariationName(index);
    return ok(QString("Variation %1 '%2' copied to base sounit. The base sounit now matches this variation.").arg(index).arg(varName));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: apply_dynamics_curve
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolApplyDynamicsCurve(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QJsonArray noteIds = args["noteIds"].toArray();
    const QJsonArray pointsArr = args["points"].toArray();
    if (pointsArr.isEmpty()) return error("'points' array is required.");

    QString err;
    QVector<EnvelopePoint> pts = parseEnvelopePoints(pointsArr, err);
    if (!err.isEmpty()) return error(err);

    const QVector<int> indices = resolveNoteIndices(sc, noteIds);
    if (indices.isEmpty()) return error("No matching notes found.");

    const double weight  = args.value("weight").toDouble(1.0);
    const bool   perNote = args.value("perNote").toBool(false);

    sc->getUndoStack()->push(
        new ApplyDynamicsCurveCommand(&sc->getPhrase(), indices, pts, weight, perNote, sc));

    return ok(QString("Dynamics curve applied to %1 note(s).").arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: load_envelope_as_dynamics
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolLoadEnvelopeAsDynamics(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QString filePath = args["filePath"].toString();
    if (filePath.isEmpty()) return error("'filePath' is required.");

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return error("Cannot open file: " + filePath);

    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &pe);
    file.close();
    if (pe.error != QJsonParseError::NoError)
        return error("JSON parse error: " + pe.errorString());

    const QJsonObject root = doc.object();
    const QJsonArray  pointsArr = root["points"].toArray();
    if (pointsArr.isEmpty()) return error("No 'points' array found in file.");

    QString err;
    QVector<EnvelopePoint> pts = parseEnvelopePoints(pointsArr, err);
    if (!err.isEmpty()) return error(err);

    const QJsonArray noteIds = args["noteIds"].toArray();
    const QVector<int> indices = resolveNoteIndices(sc, noteIds);
    if (indices.isEmpty()) return error("No matching notes found.");

    const double weight  = args.value("weight").toDouble(1.0);
    const bool   perNote = args.value("perNote").toBool(false);

    sc->getUndoStack()->push(
        new ApplyDynamicsCurveCommand(&sc->getPhrase(), indices, pts, weight, perNote, sc));

    return ok(QString("Envelope '%1' applied as dynamics curve to %2 note(s).")
              .arg(QFileInfo(filePath).completeBaseName()).arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: scale_dynamics
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolScaleDynamics(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    if (!args.contains("factor")) return error("'factor' is required.");
    const double factor = args["factor"].toDouble();
    if (factor <= 0) return error("factor must be > 0.");

    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    sc->getUndoStack()->push(
        new ScaleDynamicsCommand(&sc->getPhrase(), indices, factor, sc));

    return ok(QString("Dynamics scaled by ×%1 on %2 note(s).").arg(factor).arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: scale_timing
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolScaleTiming(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");
    if (!args.contains("proportion")) return error("'proportion' is required.");

    const double proportion = args["proportion"].toDouble();
    if (proportion <= 0) return error("proportion must be > 0.");

    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    sc->getUndoStack()->push(
        new ScaleTimingCommand(&sc->getPhrase(), indices, proportion, sc));

    return ok(QString("Timing scaled by ×%1 on %2 note(s).").arg(proportion).arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: apply_rhythm_easing
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolApplyRhythmEasing(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.size() < 2) return error("Need at least 2 notes for rhythmic easing.");

    const int easingId   = args.value("easingType").toInt(0);  // 0=Linear default
    const int anchorMode = args.value("anchorMode").toInt(3);  // 3=AnchorBoth default
    const double weight  = args.value("weight").toDouble(1.0);

    if (easingId < 0 || easingId >= static_cast<int>(Easing::Type::Count))
        return error(QString("Invalid easingType %1. Range: 0–%2.")
                     .arg(easingId).arg(static_cast<int>(Easing::Type::Count) - 1));

    Easing easing(static_cast<Easing::Type>(easingId));
    auto anchor = static_cast<ApplyRhythmicEasingCommand::AnchorMode>(anchorMode);

    sc->getUndoStack()->push(
        new ApplyRhythmicEasingCommand(&sc->getPhrase(), indices, easing, anchor, weight, sc));

    return ok(QString("Rhythmic easing (type %1) applied to %2 notes.")
              .arg(easingId).arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: link_legato
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolLinkLegato(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.size() < 2) return error("Link as Legato requires at least 2 notes.");

    sc->getUndoStack()->push(new LinkAsLegatoCommand(&sc->getPhrase(), indices, sc));
    return ok(QString("Linked %1 notes as legato.").arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: unlink_legato
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolUnlinkLegato(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    sc->getUndoStack()->push(new UnlinkLegatoCommand(&sc->getPhrase(), indices, sc));
    return ok(QString("Unlinked %1 note(s) to staccato.").arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers for operations that use ScoreCanvas::selectedNoteIndices internally
// ─────────────────────────────────────────────────────────────────────────────

static void applyWithSelection(ScoreCanvas *sc, const QVector<int> &indices,
                                std::function<void()> fn)
{
    sc->selectNotes(indices);
    fn();
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: quantize_to_scale
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolQuantizeToScale(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    applyWithSelection(sc, indices, [sc]() { sc->snapSelectedNotesToScale(); });
    return ok(QString("Quantized %1 note(s) to scale.").arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: strum_notes (transform "strum")
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolStrumNotes(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    Phrase &phrase = sc->getPhrase();
    QVector<Note> &notes = phrase.getNotes();
    QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.size() < 2) return error("Strum needs at least 2 notes.");

    const QString direction = args.value("direction").toString("down");
    if (direction != "down" && direction != "up")
        return error("'direction' must be 'down' or 'up'.");

    const double speedMs = args.contains("speedMs") ? args["speedMs"].toDouble() : 30.0;
    if (speedMs < 0.0) return error("'speedMs' must be >= 0.");

    const QString shape = args.value("dynamicsShape").toString();
    if (!shape.isEmpty() && shape != "flat"
        && shape != "accent_low" && shape != "accent_high")
        return error("'dynamicsShape' must be flat|accent_low|accent_high.");

    const bool cycleVariations = args.value("cycleVariations").toBool(false);

    // Pitch order (low → high) drives variation cycling so a given pitch always
    // routes to the same sounit regardless of stroke direction. Stroke order
    // drives timing/dynamics: down = low first, up = high first.
    QVector<int> pitchOrder = indices;
    std::sort(pitchOrder.begin(), pitchOrder.end(),
              [&notes](int a, int b) {
                  return notes[a].getPitchHz() < notes[b].getPitchHz();
              });
    QVector<int> strokeOrder = pitchOrder;
    if (direction == "up") std::reverse(strokeOrder.begin(), strokeOrder.end());

    // Anchor at the earliest current start time so the rake begins where the
    // user placed the chord on the timeline.
    double anchorMs = notes[strokeOrder.first()].getStartTime();
    for (int i : strokeOrder)
        anchorMs = std::min(anchorMs, notes[i].getStartTime());

    const int n = strokeOrder.size();
    const double step = (n > 1) ? speedMs / (n - 1) : 0.0;

    QUndoCommand *macro = new QUndoCommand("Strum");

    for (int k = 0; k < n; ++k) {
        const int idx = strokeOrder[k];
        const Note &note = notes[idx];
        const double newStart = anchorMs + step * k;
        if (newStart != note.getStartTime()) {
            new MoveNoteCommand(&phrase, idx,
                                note.getStartTime(), note.getPitchHz(),
                                newStart, note.getPitchHz(),
                                note.getPitchCurve(), note.getPitchCurve(),
                                note.hasPitchCurve(),
                                sc, macro);
        }
    }

    if (!shape.isEmpty()) {
        QVector<int>    dynIndices;
        QVector<double> dynValues;
        dynIndices.reserve(n);
        dynValues.reserve(n);
        for (int k = 0; k < n; ++k) {
            const double t = (n > 1) ? double(k) / double(n - 1) : 0.0;
            double v;
            if (shape == "flat")            v = 0.70;
            else if (shape == "accent_low") v = 0.85 - 0.30 * t;
            else /* accent_high */          v = 0.55 + 0.30 * t;
            dynIndices.append(strokeOrder[k]);
            dynValues.append(v);
        }
        new SetBeatDynamicsCommand(&phrase, dynIndices, dynValues, sc, macro);
    }

    sc->getUndoStack()->push(macro);

    int varAssigned = 0;
    if (cycleVariations) {
        Track *track = currentTrack();
        if (track) {
            const int total = track->getVariationCount() + 1;  // includes base
            if (total > 1) {
                // Highest pitch → variation 0 (base), descending → 1, 2, ...
                for (int k = 0; k < n; ++k) {
                    notes[pitchOrder[n - 1 - k]].setVariationIndex(k % total);
                    notes[pitchOrder[n - 1 - k]].setRenderDirty(true);
                }
                varAssigned = n;
            }
        }
    }

    sc->update();

    QString msg = QString("Strummed %1 notes %2, %3 ms total")
                      .arg(n).arg(direction).arg(speedMs, 0, 'f', 0);
    if (!shape.isEmpty()) msg += QString(", dynamics %1").arg(shape);
    if (varAssigned > 0)  msg += QString(", cycled variations across %1 notes").arg(varAssigned);
    msg += ".";
    return ok(msg);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: assign_guitar_strings
// ─────────────────────────────────────────────────────────────────────────────
// Uses Viterbi dynamic programming to assign each note to a guitar string,
// then maps each string to a variation index so per-string sounits are used.
//
// Default tuning: EADGBE standard — string 1 (high E) through string 6 (low E).
// When no explicit tuning is passed, open-string frequencies are snapped to the
// active scale at the first note's time (not hardcoded ET), so they adapt
// automatically to Just Intonation, Maqam, Raga, etc.
//
// Strings 4–6 are wound (bright/metallic), strings 1–3 are unwound (mellow).
// Per-note variation assignment:
//   string 1 (high E) → variation 0 (base)
//   string 2 (B)      → variation 1
//   ...
//   string 6 (low E)  → variation 5
//
// The algorithm costs fret-distance + string-skip-penalty, so it naturally
// prefers the most playable fingering. Notes that are out of range on every
// string are assigned to the nearest string.

QJsonObject KalaTools::toolAssignGuitarStrings(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    QVector<Note> &notes = sc->getPhrase().getNotes();
    QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    // ── Sort by start time so the sequence is chronological ──
    QVector<int> sorted = indices;
    std::sort(sorted.begin(), sorted.end(),
              [&notes](int a, int b) { return notes[a].getStartTime() < notes[b].getStartTime(); });

    // ── Tuning: 6 open-string frequencies, string 1 (high E) to string 6 (low E) ──
    const QJsonArray tuningArg = args["tuning"].toArray();
    QVector<double> openHz(6);

    // ET reference MIDI numbers for the default tuning — used ONLY as a lookup
    // target to find the matching scale line in non-ET scales. Not used as actual
    // frequencies unless the scale happens to be Equal Temperament.
    const int defaultMidi[6] = {64, 59, 55, 50, 45, 40};  // high-E, B, G, D, A, low-E

    if (!tuningArg.isEmpty()) {
        // User-provided tuning — detect Hz vs MIDI
        bool looksLikeMidi = true;
        for (int s = 0; s < 6 && s < tuningArg.size(); ++s) {
            if (tuningArg[s].toDouble() < 900.0) { looksLikeMidi = false; break; }
        }
        for (int s = 0; s < 6 && s < tuningArg.size(); ++s) {
            double v = tuningArg[s].toDouble();
            openHz[s] = looksLikeMidi ? (440.0 * std::pow(2.0, (v - 69.0) / 12.0)) : v;
        }
        // Fill missing entries with ET defaults
        for (int s = tuningArg.size(); s < 6; ++s)
            openHz[s] = 440.0 * std::pow(2.0, (defaultMidi[s] - 69.0) / 12.0);
    } else {
        // Scale-aware default: snap each string's ET frequency to the nearest
        // scale line from the scale active at the first note's position.
        const double firstTime = notes[sorted[0]].getStartTime();
        const Scale activeScale = sc->getScaleAtTime(firstTime);
        const double activeBaseFreq = sc->getBaseFrequencyAtTime(firstTime);

        QVector<ScoreCanvas::ScaleLine> scaleLines =
            sc->generateScaleLinesForScale(activeScale, activeBaseFreq);

        for (int s = 0; s < 6; ++s) {
            const double etTarget = 440.0 * std::pow(2.0, (defaultMidi[s] - 69.0) / 12.0);
            double bestHz = etTarget;  // fallback to ET if no scale lines
            double bestDist = 1e9;
            for (const auto &line : scaleLines) {
                const double d = std::abs(line.frequencyHz - etTarget);
                if (d < bestDist) { bestDist = d; bestHz = line.frequencyHz; }
            }
            openHz[s] = bestHz;
        }
    }

    const double maxFret    = args.value("maxFret").toDouble(19.0);
    const double maxStretch = args.value("maxStretch").toDouble(4.0);  // max fret span per hand position
    const bool   preferOpen = args.value("preferOpen").toBool(false);
    const int    woundCount = std::clamp(args.value("woundStrings").toInt(3), 0, 6);  // lowest N strings are wound
    const double stringSkipPenalty = args.value("stringSkipPenalty").toDouble(0.7);

    const int n = indices.size();

    // ── Phase 1: compute valid (string, fret) for each note ──
    struct Candidate { int s; double fret; };
    QVector<QVector<Candidate>> cands(n);
    for (int i = 0; i < n; ++i) {
        const double pitch = notes[sorted[i]].getPitchHz();
        for (int s = 0; s < 6; ++s) {
            if (pitch < openHz[s] * 0.99) continue;  // below open string
            const double fret = 12.0 * std::log2(pitch / openHz[s]);
            if (fret > maxFret + 0.01) continue;
            cands[i].append({s, fret});
        }
        if (cands[i].isEmpty()) {
            // Note is unplayable — force to open string of best-fit string
            // by pitch proximity. This is better than leaving it unassigned.
            int bestS = 0;
            double bestDist = 1e9;
            for (int s = 0; s < 6; ++s) {
                double d = std::abs(pitch - openHz[s]);
                if (d < bestDist) { bestDist = d; bestS = s; }
            }
            double forcedFret = 12.0 * std::log2(pitch / openHz[bestS]);
            if (forcedFret < 0.0) forcedFret = 0.0;
            cands[i].append({bestS, forcedFret});
        }
    }

    // ── Phase 2: Viterbi DP ──
    // dp[i][k] = { totalCost, prevK } for the k-th candidate of note i
    struct State { double cost = 1e18; int prev = -1; };
    QVector<QVector<State>> dp(n);
    for (int i = 0; i < n; ++i) dp[i].resize(cands[i].size());

    // First note: base cost = 0 for open strings, small fret penalty otherwise
    for (int k = 0; k < cands[0].size(); ++k) {
        double cost = cands[0][k].fret * 0.3;  // slight preference for open position
        if (preferOpen && cands[0][k].fret < 0.01) cost -= 1.0;  // bonus for open
        dp[0][k].cost = cost;
    }

    // Forward pass
    for (int i = 1; i < n; ++i) {
        for (int k = 0; k < cands[i].size(); ++k) {
            const Candidate &cur = cands[i][k];
            double bestCost = 1e18;
            int    bestPrev = -1;

            for (int pk = 0; pk < cands[i - 1].size(); ++pk) {
                const Candidate &prev = cands[i - 1][pk];
                // Fret distance + string skip penalty
                const double fretDist  = std::abs(cur.fret - prev.fret);
                const double stringDist = std::abs(cur.s - prev.s);
                double transition = fretDist + stringDist * stringSkipPenalty;

                // Penalise exceeding max stretch (hand position constraint)
                if (fretDist > maxStretch)
                    transition += (fretDist - maxStretch) * 2.0;

                // Soft penalty for crossing wound/unwound boundary
                // when the note is in the mid register (where both are viable)
                const bool curWound  = (cur.s >= 6 - woundCount);   // strings 4-6 with woundCount=3
                const bool prevWound = (prev.s >= 6 - woundCount);
                const double midPitch = notes[sorted[i]].getPitchHz();
                if (curWound != prevWound && midPitch > openHz[2] && midPitch < openHz[0] * 2.0)
                    transition += 0.5;

                const double total = dp[i - 1][pk].cost + transition;
                if (total < bestCost) { bestCost = total; bestPrev = pk; }
            }
            dp[i][k] = {bestCost, bestPrev};
        }
    }

    // ── Phase 3: backtrack ──
    QVector<int> bestCandIdx(n);
    {
        double bestFinal = 1e18;
        int    bestK     = 0;
        for (int k = 0; k < cands[n - 1].size(); ++k) {
            if (dp[n - 1][k].cost < bestFinal) { bestFinal = dp[n - 1][k].cost; bestK = k; }
        }
        bestCandIdx[n - 1] = bestK;
        for (int i = n - 1; i >= 1; --i)
            bestCandIdx[i - 1] = dp[i][bestCandIdx[i]].prev;
    }

    // ── Phase 4: assign variation indices ──
    // string s → variation s (0 = high E, 5 = low E)
    Track *track = currentTrack();
    const int varCount = track ? (track->getVariationCount() + 1) : 6;  // includes base

    QJsonArray assignments;
    int changedCount = 0;
    for (int i = 0; i < n; ++i) {
        const int noteIdx  = sorted[i];
        const int string   = cands[i][bestCandIdx[i]].s;  // 0=high-E, 5=low-E
        const double fret  = cands[i][bestCandIdx[i]].fret;
        const int varIndex = string;  // 0–5, maps to string 1–6

        if (noteIdx >= 0 && noteIdx < notes.size()) {
            const int oldVar = notes[noteIdx].getVariationIndex();
            notes[noteIdx].setVariationIndex(varIndex);
            notes[noteIdx].setRenderDirty(true);
            if (oldVar != varIndex) changedCount++;

            QJsonObject a;
            a["noteId"]    = notes[noteIdx].getId();
            a["pitchHz"]   = notes[noteIdx].getPitchHz();
            a["string"]    = string + 1;  // 1-based for readability
            a["fret"]      = std::round(fret * 10.0) / 10.0;
            a["variation"] = varIndex;
            a["wound"]     = (string >= 6 - woundCount);
            assignments.append(a);
        }
    }

    sc->update();

    QString summary = QString("Assigned %1 notes to guitar strings (%2 changed). "
                              "EADGBE tuning, max fret %3.")
                          .arg(n).arg(changedCount).arg(maxFret, 0, 'f', 0);
    return QJsonObject{ {"result", summary}, {"assignments", assignments} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: make_continuous
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolMakeContinuous(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    applyWithSelection(sc, indices, [sc]() { sc->makeSelectedNotesContinuous(); });
    return ok(QString("Converted %1 note(s) to continuous.").arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: make_discrete
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolMakeDiscrete(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    applyWithSelection(sc, indices, [sc]() { sc->makeSelectedNotesDiscrete(); });
    return ok(QString("Converted %1 note(s) to discrete.").arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: get_selected_notes
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolGetSelectedNotes()
{
    if (m_selectedNotesJustProvided)
        return QJsonObject{{"result", "Selected note IDs were already provided. Use those IDs and proceed — do not call get_selected_notes again until the selection changes."}};

    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QVector<int> &indices = sc->getSelectedNoteIndices();
    const QVector<Note> &notes  = sc->getPhrase().getNotes();

    QJsonArray ids;
    for (int i : indices) {
        if (i >= 0 && i < notes.size())
            ids.append(notes[i].getId());
    }

    QJsonObject result;
    result["selectedCount"] = ids.size();
    result["noteIds"]       = ids;
    result["message"]       = ids.isEmpty()
                                  ? "No notes are currently selected."
                                  : QString("%1 note(s) are selected.").arg(ids.size());
    m_selectedNotesJustProvided = true;
    return QJsonObject{ {"result", result} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_note_dynamics
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetNoteDynamics(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");
    if (!args.contains("noteIds")) return error("'noteIds' is required.");
    if (!args.contains("values"))  return error("'values' is required.");

    const QJsonArray noteIdArr = args["noteIds"].toArray();
    const QJsonArray valArr    = args["values"].toArray();

    if (noteIdArr.isEmpty()) return error("'noteIds' must not be empty.");
    if (valArr.size() != noteIdArr.size())
        return error(QString("'values' length (%1) must match 'noteIds' length (%2).")
                         .arg(valArr.size()).arg(noteIdArr.size()));

    // Validate values
    QVector<double> values;
    for (int i = 0; i < valArr.size(); ++i) {
        double d = valArr[i].toDouble();
        if (d < 0.0 || d > 1.0)
            return error(QString("values[%1] = %2 is out of range (must be 0.0–1.0).").arg(i).arg(d));
        values.append(d);
    }

    // Resolve note IDs to indices
    Phrase &phrase = sc->getPhrase();
    const QVector<Note> &notes = phrase.getNotes();

    QSet<QString> idSet;
    for (const QJsonValue &v : noteIdArr) idSet.insert(v.toString());

    // Build a map id→value so order matches noteIdArr
    QMap<QString, double> idToValue;
    for (int i = 0; i < noteIdArr.size(); ++i)
        idToValue[noteIdArr[i].toString()] = values[i];

    QVector<int>    indices;
    QVector<double> dynValues;
    for (int i = 0; i < notes.size(); ++i) {
        const QString id = notes[i].getId();
        if (idToValue.contains(id)) {
            indices.append(i);
            dynValues.append(idToValue[id]);
        }
    }

    if (indices.isEmpty()) return error("None of the provided note IDs were found.");

    sc->getUndoStack()->push(
        new SetBeatDynamicsCommand(&phrase, indices, dynValues, sc));

    return ok(QString("Dynamics set on %1 note(s).").arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: apply_beat_dynamics
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolApplyBeatDynamics(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");
    if (!args.contains("pattern")) return error("'pattern' is required.");

    const QJsonArray patternArr = args["pattern"].toArray();
    if (patternArr.isEmpty()) return error("'pattern' must not be empty.");

    QVector<double> pattern;
    for (const QJsonValue &v : patternArr) {
        double d = v.toDouble();
        if (d < 0.0 || d > 1.0) return error("All pattern values must be between 0.0 and 1.0.");
        pattern.append(d);
    }

    // Resolve note candidates
    const QJsonArray noteIds = args["noteIds"].toArray();
    QVector<int> candidates = resolveNoteIndices(sc, noteIds);
    if (candidates.isEmpty()) return error("No notes found.");

    Phrase &phrase = sc->getPhrase();
    const QVector<Note> &notes = phrase.getNotes();

    // For each candidate note, determine its beat position using the time
    // signature active at that note's start time.
    QVector<int>    targetIndices;
    QVector<double> targetDynamics;

    for (int i : candidates) {
        if (i < 0 || i >= notes.size()) continue;
        const double startMs = notes[i].getStartTime();

        // Time signature and tempo at this note's position
        QPair<int,int> timeSig = sc->getTimeSignatureAtTime(startMs);
        const int   sigNum    = timeSig.first;
        const int   sigDenom  = timeSig.second;
        const double tempo    = sc->getDefaultTempo();  // BPM

        if (sigNum <= 0 || sigDenom <= 0 || tempo <= 0.0) continue;

        // Beat duration: one beat = one denominator note value
        // e.g. 4/4 or 5/4 → denominator=4 → beat = quarter note = 60000/bpm ms
        const double beatMs    = 60000.0 / tempo * (4.0 / sigDenom);
        const double measureMs = beatMs * sigNum;

        // Position within the current measure (0-based ms offset)
        const double posInMeasure = std::fmod(startMs, measureMs);
        // 0-based beat index within the measure
        const int beatIndex = static_cast<int>(posInMeasure / beatMs);
        // Clamp to pattern length (wrap if pattern shorter than time sig numerator)
        const int patternIndex = beatIndex % pattern.size();

        targetIndices.append(i);
        targetDynamics.append(pattern[patternIndex]);
    }

    if (targetIndices.isEmpty()) return error("No valid notes to process.");

    sc->getUndoStack()->push(
        new SetBeatDynamicsCommand(&phrase, targetIndices, targetDynamics, sc));

    return ok(QString("Beat dynamics pattern applied to %1 note(s).")
              .arg(targetIndices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: select_notes
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSelectNotesByRange(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QVector<Note> &notes = sc->getPhrase().getNotes();

    const bool hasMin    = args.contains("pitchMinHz");
    const bool hasMax    = args.contains("pitchMaxHz");
    const bool hasTrk    = args.contains("trackIndex");
    const bool hasDurMin = args.contains("durationMinMs");
    const bool hasDurMax = args.contains("durationMaxMs");
    const double minHz   = hasMin    ? args["pitchMinHz"].toDouble()    : 0.0;
    const double maxHz   = hasMax    ? args["pitchMaxHz"].toDouble()    : 1e12;
    const int trackIdx   = hasTrk    ? args["trackIndex"].toInt()       : -1;
    const double durMin  = hasDurMin ? args["durationMinMs"].toDouble() : 0.0;
    const double durMax  = hasDurMax ? args["durationMaxMs"].toDouble() : 1e12;

    if (hasMin && hasMax && minHz > maxHz)
        return error("pitchMinHz must be <= pitchMaxHz.");
    if (hasDurMin && hasDurMax && durMin > durMax)
        return error("durationMinMs must be <= durationMaxMs.");

    QVector<int> indices;
    QJsonArray ids;

    const QString indicesStr = args.value("indices").toString();
    const QJsonArray noteIdsArr = args.value("noteIds").toArray();
    if (!indicesStr.isEmpty()) {
        QString s = indicesStr.trimmed();
        if (s.compare("all", Qt::CaseInsensitive) == 0) {
            for (int i = 0; i < notes.size(); ++i) indices.append(i);
        } else {
            QChar sep = s.contains(':') ? ':' : '-';
            QStringList parts = s.split(sep);
            if (parts.size() == 2) {
                bool ok1, ok2;
                int from = parts[0].trimmed().toInt(&ok1);
                int to   = parts[1].trimmed().toInt(&ok2);
                if (ok1 && ok2) {
                    if (from > to) qSwap(from, to);
                    from = qMax(0, from);
                    to   = qMin(notes.size() - 1, to);
                    for (int i = from; i <= to; ++i) indices.append(i);
                }
            }
        }
    } else if (!noteIdsArr.isEmpty()) {
        QSet<QString> wanted;
        for (const QJsonValue &v : noteIdsArr) wanted.insert(v.toString());
        for (int i = 0; i < notes.size(); ++i) {
            const Note &n = notes[i];
            if (!wanted.contains(n.getId())) continue;
            if (hasTrk && n.getTrackIndex() != trackIdx) continue;
            if (n.getPitchHz() < minHz || n.getPitchHz() > maxHz) continue;
            if (n.getDuration() < durMin || n.getDuration() > durMax) continue;
            indices.append(i);
            ids.append(n.getId());
        }
        if (indices.isEmpty())
            return error("No notes match the given noteIds (with filters).");
    } else {
        for (int i = 0; i < notes.size(); ++i) {
            const Note &n = notes[i];
            if (hasTrk && n.getTrackIndex() != trackIdx) continue;
            if (n.getPitchHz() < minHz || n.getPitchHz() > maxHz) continue;
            if (n.getDuration() < durMin || n.getDuration() > durMax) continue;
            indices.append(i);
            ids.append(n.getId());
        }
    }

    if (indices.isEmpty() && !indicesStr.isEmpty())
        return error("No notes match the indices range.");

    // Populate ids from indices if not already filled (indices-string path)
    if (ids.isEmpty()) {
        for (int i : indices) {
            if (i >= 0 && i < notes.size())
                ids.append(notes[i].getId());
        }
    }

    sc->selectNotes(indices);
    sc->update();

    QJsonObject result;
    result["matchedCount"] = indices.size();
    result["noteIds"]      = ids;
    result["message"]      = QString("%1 note(s) selected.").arg(indices.size());
    return QJsonObject{ {"result", result} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: select_flat_dynamics_notes
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSelectFlatDynamicsNotes(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QVector<Note> &notes = sc->getPhrase().getNotes();

    const bool hasTrk = args.contains("trackIndex");
    const int trackIdx = hasTrk ? args["trackIndex"].toInt() : -1;

    auto isFlatCurve = [](const Curve& curve) -> bool {
        const auto& points = curve.getPoints();
        if (points.size() <= 1) return true; // empty or single point is flat
        const double firstValue = points[0].value;
        for (const auto& p : points) {
            if (std::abs(p.value - firstValue) > 0.0001) return false;
        }
        return true;
    };

    QVector<int> indices;
    QJsonArray ids;

    for (int i = 0; i < notes.size(); ++i) {
        const Note &n = notes[i];
        if (hasTrk && n.getTrackIndex() != trackIdx) continue;
        if (!isFlatCurve(n.getDynamicsCurve())) continue;
        indices.append(i);
        ids.append(n.getId());
    }

    sc->selectNotes(indices);
    sc->update();

    QJsonObject result;
    result["matchedCount"] = indices.size();
    result["noteIds"]      = ids.isEmpty() ? QJsonArray() : ids;
    result["message"]      = QString("Selected %1 note(s) with flat dynamics curves.").arg(indices.size());
    return QJsonObject{ {"result", result} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: shift_notes
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolShiftNotes(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");
    if (!args.contains("offsetMs")) return error("'offsetMs' is required.");

    const double offsetMs = args["offsetMs"].toDouble();
    if (offsetMs == 0.0) return ok("Offset is zero — nothing to do.");

    Phrase &phrase = sc->getPhrase();
    QVector<Note> &notes = phrase.getNotes();

    // Build candidate index list — either explicit noteIds or all notes
    const QJsonArray noteIds = args["noteIds"].toArray();
    QVector<int> candidates = resolveNoteIndices(sc, noteIds);

    // Apply optional pitch filter
    QVector<int> indices;
    if (args.contains("pitchHz")) {
        const double targetHz = args["pitchHz"].toDouble();
        const double toleranceCents = args.contains("pitchToleranceCents")
                                          ? args["pitchToleranceCents"].toDouble()
                                          : 50.0;
        for (int i : candidates) {
            if (i < 0 || i >= notes.size()) continue;
            const double noteHz = notes[i].getPitchHz();
            // Convert frequency ratio to cents: 1200 * log2(f1/f2)
            const double cents = (noteHz > 0.0 && targetHz > 0.0)
                                     ? std::abs(1200.0 * std::log2(noteHz / targetHz))
                                     : 1e9;
            if (cents <= toleranceCents)
                indices.append(i);
        }
    } else {
        indices = candidates;
    }

    if (indices.isEmpty()) return error("No notes matched the given criteria.");

    // Clamp: no note may be shifted before time 0
    for (int i : indices) {
        if (notes[i].getStartTime() + offsetMs < 0.0)
            return error(QString("Shifting would move note %1 before time 0. "
                                 "Use a smaller (less negative) offsetMs.").arg(i));
    }

    // Push one macro command so the whole shift undoes in one Ctrl+Z
    QUndoCommand *macro = new QUndoCommand("Shift Notes");
    for (int i : indices) {
        const Note &n = notes[i];
        new MoveNoteCommand(&phrase, i,
                            n.getStartTime(), n.getPitchHz(),
                            n.getStartTime() + offsetMs, n.getPitchHz(),
                            n.getPitchCurve(), n.getPitchCurve(),
                            n.hasPitchCurve(),
                            sc, macro);
    }
    sc->getUndoStack()->push(macro);
    sc->update();

    return ok(QString("Shifted %1 note(s) by %2 ms.")
              .arg(indices.size())
              .arg(offsetMs, 0, 'f', 1));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_note_pitch
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetNotePitch(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    Phrase &phrase = sc->getPhrase();
    const QVector<Note> &notes = phrase.getNotes();
    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    if (args.contains("pitchHz")) {
        const double hz = args["pitchHz"].toDouble();
        if (hz <= 0) return error("pitchHz must be > 0.");
        QUndoCommand *macro = new QUndoCommand("Set Note Pitch");
        for (int i : indices) {
            const Note &n = notes[i];
            new MoveNoteCommand(&phrase, i,
                                n.getStartTime(), n.getPitchHz(),
                                n.getStartTime(), hz,
                                n.getPitchCurve(), n.getPitchCurve(),
                                n.hasPitchCurve(), sc, macro);
        }
        sc->getUndoStack()->push(macro);
        sc->update();
        return ok(QString("Set pitch to %1 Hz on %2 note(s).").arg(hz, 0, 'f', 3).arg(indices.size()));
    }
    if (args.contains("values")) {
        const QJsonArray valArr = args["values"].toArray();
        if (valArr.size() != indices.size())
            return error(QString("noteIds has %1 entries but values has %2.")
                         .arg(indices.size()).arg(valArr.size()));
        QUndoCommand *macro = new QUndoCommand("Set Note Pitches");
        for (int j = 0; j < indices.size(); ++j) {
            const double hz = valArr[j].toDouble();
            if (hz <= 0) { delete macro; return error(QString("values[%1] must be > 0.").arg(j)); }
            const Note &n = notes[indices[j]];
            new MoveNoteCommand(&phrase, indices[j],
                                n.getStartTime(), n.getPitchHz(),
                                n.getStartTime(), hz,
                                n.getPitchCurve(), n.getPitchCurve(),
                                n.hasPitchCurve(), sc, macro);
        }
        sc->getUndoStack()->push(macro);
        sc->update();
        return ok(QString("Set pitches on %1 note(s).").arg(indices.size()));
    }
    return error("Either 'pitchHz' (single value) or 'values' (array) is required.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: stretch_notes
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolStretchNotes(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");
    if (!args.contains("factor")) return error("'factor' is required.");

    const double factor = args["factor"].toDouble();
    if (factor <= 0) return error("factor must be > 0.");
    if (qFuzzyCompare(factor, 1.0)) return ok("Factor is 1.0 — nothing to do.");

    Phrase &phrase = sc->getPhrase();
    const QVector<Note> &notes = phrase.getNotes();
    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    QUndoCommand *macro = new QUndoCommand("Stretch Notes");
    for (int i : indices) {
        const Note &n = notes[i];
        const double newDur = n.getDuration() * factor;
        new ResizeNoteCommand(&phrase, i,
                              n.getStartTime(), n.getDuration(),
                              n.getStartTime(), newDur,
                              sc, macro);
    }
    sc->getUndoStack()->push(macro);
    sc->update();
    return ok(QString("Stretched %1 note(s) by factor %2 (durations only, start times unchanged).")
              .arg(indices.size()).arg(factor, 0, 'f', 3));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_time_signature  (default, applies from time 0)
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetTimeSignature(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");
    if (!args.contains("numerator") || !args.contains("denominator"))
        return error("'numerator' and 'denominator' are required.");

    const int num   = args["numerator"].toInt();
    const int denom = args["denominator"].toInt();
    if (num < 1 || num > 99)   return error("numerator must be 1–99.");
    if (denom < 1 || denom > 64) return error("denominator must be 1–64.");

    sc->setDefaultTimeSignature(num, denom);

    QString msg = QString("Default time signature set to %1/%2.").arg(num).arg(denom);
    if (args.contains("bpm")) {
        const double bpm = args["bpm"].toDouble();
        if (bpm < 20 || bpm > 300) return error("bpm must be 20–300.");
        sc->setDefaultTempo(bpm);
        msg += QString(" Tempo set to %1 BPM.").arg(bpm);
    }
    m_scoreCanvasWindow->refreshToolbar();
    return ok(msg);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: get_time_signature_changes
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolGetTimeSignatureChanges()
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    QJsonArray result;

    // Include default (time 0)
    result.append(QJsonObject{
        {"timeMs",      0.0},
        {"numerator",   sc->getDefaultTimeSigNum()},
        {"denominator", sc->getDefaultTimeSigDenom()},
        {"bpm",         sc->getDefaultTempo()},
        {"isDefault",   true}
    });

    // All position-specific changes
    const QMap<double, TempoTimeSignature> &changes = sc->getTempoChanges();
    for (auto it = changes.begin(); it != changes.end(); ++it) {
        result.append(QJsonObject{
            {"timeMs",         it.key()},
            {"numerator",      it.value().timeSigNumerator},
            {"denominator",    it.value().timeSigDenominator},
            {"bpm",            it.value().bpm},
            {"gradual",        it.value().gradualTransition},
            {"isDefault",      false}
        });
    }

    return QJsonObject{ {"result", QJsonObject{
        {"count",   (int)result.size()},
        {"changes", result}
    }}};
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: add_time_signature_change
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolAddTimeSignatureChange(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");
    if (!args.contains("timeMs"))     return error("'timeMs' is required.");
    if (!args.contains("numerator") || !args.contains("denominator"))
        return error("'numerator' and 'denominator' are required.");

    const double timeMs = args["timeMs"].toDouble();
    if (timeMs <= 0) return error("timeMs must be > 0 (use set_time_signature for the default at time 0).");

    const int num   = args["numerator"].toInt();
    const int denom = args["denominator"].toInt();
    if (num < 1 || num > 99)    return error("numerator must be 1–99.");
    if (denom < 1 || denom > 64) return error("denominator must be 1–64.");

    // Inherit bpm from the active tempo at this position if not specified
    const TempoTimeSignature prev = sc->getTempoTimeSignatureAtTime(timeMs);
    TempoTimeSignature tts;
    tts.timeSigNumerator   = num;
    tts.timeSigDenominator = denom;
    tts.bpm                = args.contains("bpm") ? args["bpm"].toDouble() : prev.bpm;
    tts.gradualTransition  = args.value("gradual").toBool(false);

    sc->getUndoStack()->push(new AddTempoChangeCommand(sc, timeMs, tts));

    return ok(QString("Time signature %1/%2 at %3 ms (tempo %4 BPM).")
              .arg(num).arg(denom).arg(timeMs, 0, 'f', 0).arg(tts.bpm, 0, 'f', 1));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: remove_time_signature_change
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolRemoveTimeSignatureChange(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");
    if (!args.contains("timeMs")) return error("'timeMs' is required.");

    const double timeMs = args["timeMs"].toDouble();

    // Verify a change exists there
    const QMap<double, TempoTimeSignature> &changes = sc->getTempoChanges();
    if (!changes.contains(timeMs))
        return error(QString("No time signature change found at %1 ms. "
                             "Use get_time_signature_changes to list existing markers.")
                     .arg(timeMs, 0, 'f', 0));

    sc->getUndoStack()->push(new RemoveTempoChangeCommand(sc, timeMs));
    return ok(QString("Removed time signature change at %1 ms.").arg(timeMs, 0, 'f', 0));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: get_note_vibrato
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolGetNoteVibrato(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    const QVector<Note> &notes = sc->getPhrase().getNotes();
    const int i = indices.first();
    if (i < 0 || i >= notes.size()) return error("Note index out of range.");
    const Vibrato &v = notes[i].getVibrato();

    QJsonArray env;
    for (const EnvelopePoint &pt : v.envelope)
        env.append(QJsonObject{ {"time", pt.time}, {"value", pt.value}, {"curveType", pt.curveType} });

    return QJsonObject{ {"result", QJsonObject{
        {"noteId",         notes[i].getId()},
        {"active",         v.active},
        {"rate",           v.rate},
        {"pitchDepth",     v.pitchDepth},
        {"amplitudeDepth", v.amplitudeDepth},
        {"onset",          v.onset},
        {"regularity",     v.regularity},
        {"envelope",       env}
    }}};
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_note_vibrato
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetNoteVibrato(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    // Start from the first targeted note's current vibrato so unspecified params are preserved
    const QVector<Note> &notes = sc->getPhrase().getNotes();
    Vibrato v = notes[indices.first()].getVibrato();

    if (args.contains("active"))         v.active         = args["active"].toBool();
    if (args.contains("rate"))           v.rate           = args["rate"].toDouble();
    if (args.contains("pitchDepth"))     v.pitchDepth     = args["pitchDepth"].toDouble();
    if (args.contains("amplitudeDepth")) v.amplitudeDepth = args["amplitudeDepth"].toDouble();
    if (args.contains("onset"))          v.onset          = args["onset"].toDouble();
    if (args.contains("regularity"))     v.regularity     = args["regularity"].toDouble();

    if (args.contains("envelope")) {
        const QJsonArray envArr = args["envelope"].toArray();
        v.envelope.clear();
        for (const QJsonValue &pv : envArr) {
            const QJsonObject po = pv.toObject();
            EnvelopePoint pt;
            pt.time      = po["time"].toDouble();
            pt.value     = po["value"].toDouble();
            pt.curveType = po.value("curveType").toInt(1);
            v.envelope.append(pt);
        }
        if (v.envelope.isEmpty()) {
            v.envelope.append(EnvelopePoint(0.0, 0.0, 0));
            v.envelope.append(EnvelopePoint(1.0, 1.0, 0));
        }
    }

    sc->getUndoStack()->push(new SetVibratoCommand(&sc->getPhrase(), indices, v, sc));

    return ok(QString("Vibrato %1 on %2 note(s).")
              .arg(v.active ? "enabled" : "disabled")
              .arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: undo
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolUndo()
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");
    QUndoStack *stack = sc->getUndoStack();
    if (!stack->canUndo()) return error("Nothing to undo.");
    const QString text = stack->undoText();
    stack->undo();
    return ok(text.isEmpty() ? "Undo performed." : "Undone: " + text);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: redo
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolRedo()
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");
    QUndoStack *stack = sc->getUndoStack();
    if (!stack->canRedo()) return error("Nothing to redo.");
    const QString text = stack->redoText();
    stack->redo();
    return ok(text.isEmpty() ? "Redo performed." : "Redone: " + text);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: play_score
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolPlayScore()
{
    if (!m_scoreCanvasWindow) return error("Score canvas window not available.");
    m_scoreCanvasWindow->play();
    return ok("Score playback started.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: stop_score
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolStopScore()
{
    if (!m_scoreCanvasWindow) return error("Score canvas window not available.");
    m_scoreCanvasWindow->stopPlayback();
    return ok("Score playback stopped.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: transpose_notes
// ─────────────────────────────────────────────────────────────────────────────

// Helper: walk N scale degrees from a pitch, given scale + base frequency.
// Returns the new pitch in Hz, or the original pitch on failure.
static double transposeByDegrees(double pitchHz, int steps,
                                  const Scale &scale, double baseFreq)
{
    const int degCount = scale.getDegreeCount();
    if (degCount <= 0 || baseFreq <= 0.0 || pitchHz <= 0.0) return pitchHz;

    // Find nearest (octave, degree) by minimizing log2 distance
    const double logPitch = std::log2(pitchHz / baseFreq);
    double minDist = 1e9;
    int bestDeg = 0, bestOct = 0;
    for (int oct = -5; oct <= 8; ++oct) {
        for (int d = 0; d < degCount; ++d) {
            double ratio = scale.getRatio(d);
            if (ratio <= 0.0) continue;
            double logRatio = std::log2(ratio) + oct;
            double dist = std::abs(logPitch - logRatio);
            if (dist < minDist) { minDist = dist; bestDeg = d; bestOct = oct; }
        }
    }

    // Walk N steps from (bestDeg, bestOct)
    int total = bestDeg + steps;
    int newOct = bestOct + total / degCount;
    int newDeg = total % degCount;
    if (newDeg < 0) { newDeg += degCount; newOct--; }
    double newRatio = scale.getRatio(newDeg);
    if (newRatio <= 0.0) return pitchHz;
    return baseFreq * newRatio * std::pow(2.0, newOct);
}

QJsonObject KalaTools::toolTransposeNotes(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const bool hasRatio   = args.contains("ratio");
    const bool hasDegrees = args.contains("scaleDegrees");
    if (!hasRatio && !hasDegrees)
        return error("Either 'ratio' or 'scaleDegrees' is required.");

    Phrase &phrase = sc->getPhrase();
    const QVector<Note> &notes = phrase.getNotes();
    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    QUndoCommand *macro = new QUndoCommand("Transpose Notes");
    int count = 0;
    for (int i : indices) {
        if (i < 0 || i >= notes.size()) continue;
        const Note &n = notes[i];
        double newPitch;
        if (hasRatio) {
            const double ratio = args["ratio"].toDouble();
            if (ratio <= 0.0) { delete macro; return error("ratio must be > 0."); }
            newPitch = n.getPitchHz() * ratio;
        } else {
            const int steps = args["scaleDegrees"].toInt();
            const Scale  &scale    = sc->getScaleAtTime(n.getStartTime());
            const double  baseFreq = sc->getBaseFrequencyAtTime(n.getStartTime());
            newPitch = transposeByDegrees(n.getPitchHz(), steps, scale, baseFreq);
        }
        if (newPitch <= 0.0) continue;

        // Scale pitch curve Hz values if present
        Curve newPitchCurve = n.getPitchCurve();
        if (n.hasPitchCurve()) {
            newPitchCurve.clearPoints();
            const double factor = newPitch / n.getPitchHz();
            for (const Curve::Point &pt : n.getPitchCurve().getPoints())
                newPitchCurve.addPoint(pt.time, pt.value * factor, pt.pressure);
        }

        new MoveNoteCommand(&phrase, i,
                            n.getStartTime(), n.getPitchHz(),
                            n.getStartTime(), newPitch,
                            n.getPitchCurve(), newPitchCurve,
                            n.hasPitchCurve(), sc, macro);
        ++count;
    }
    sc->getUndoStack()->push(macro);
    sc->update();

    if (hasRatio)
        return ok(QString("Transposed %1 note(s) by ratio %2.")
                  .arg(count).arg(args["ratio"].toDouble(), 0, 'f', 4));
    else
        return ok(QString("Transposed %1 note(s) by %2 scale degree(s).")
                  .arg(count).arg(args["scaleDegrees"].toInt()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_note_duration
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetNoteDuration(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    Phrase &phrase = sc->getPhrase();
    const QVector<Note> &notes = phrase.getNotes();
    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    if (args.contains("duration")) {
        const double dur = args["duration"].toDouble();
        if (dur <= 0) return error("duration must be > 0.");
        QUndoCommand *macro = new QUndoCommand("Set Note Duration");
        for (int i : indices) {
            const Note &n = notes[i];
            new ResizeNoteCommand(&phrase, i,
                                  n.getStartTime(), n.getDuration(),
                                  n.getStartTime(), dur,
                                  sc, macro);
        }
        sc->getUndoStack()->push(macro);
        sc->update();
        return ok(QString("Set duration to %1 ms for %2 note(s).").arg(dur).arg(indices.size()));
    }
    if (args.contains("values")) {
        const QJsonArray valArr = args["values"].toArray();
        if (valArr.size() != indices.size())
            return error(QString("noteIds has %1 entries but values has %2.")
                         .arg(indices.size()).arg(valArr.size()));
        QUndoCommand *macro = new QUndoCommand("Set Note Durations");
        for (int j = 0; j < indices.size(); ++j) {
            const double dur = valArr[j].toDouble();
            if (dur <= 0) { delete macro; return error(QString("values[%1] must be > 0.").arg(j)); }
            const Note &n = notes[indices[j]];
            new ResizeNoteCommand(&phrase, indices[j],
                                  n.getStartTime(), n.getDuration(),
                                  n.getStartTime(), dur,
                                  sc, macro);
        }
        sc->getUndoStack()->push(macro);
        sc->update();
        return ok(QString("Set durations for %1 note(s).").arg(indices.size()));
    }
    return error("Either 'duration' (single value for all) or 'values' (array, one per note) is required.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: duplicate_notes
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolDuplicateNotes(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");
    if (!args.contains("offsetMs")) return error("'offsetMs' is required.");

    const double offsetMs   = args["offsetMs"].toDouble();
    const double pitchRatio = args.value("pitchRatio").toDouble(1.0);
    const int targetTrack   = args.value("trackIndex").toInt(m_currentTrackIndex);

    Phrase &phrase = sc->getPhrase();
    const QVector<Note> &notes = phrase.getNotes();
    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    // Find earliest start time of source notes
    double minStart = std::numeric_limits<double>::max();
    for (int i : indices) minStart = std::min(minStart, notes[i].getStartTime());
    const double targetTime = minStart + offsetMs;
    if (targetTime < 0.0) return error("offsetMs would place notes before time 0.");

    // Build notes to paste, applying optional pitch ratio
    QVector<Note> notesToPaste;
    for (int i : indices) {
        Note n = notes[i];
        if (!qFuzzyCompare(pitchRatio, 1.0)) {
            n.setPitchHz(n.getPitchHz() * pitchRatio);
            if (n.hasPitchCurve()) {
                Curve scaled;
                for (const Curve::Point &pt : n.getPitchCurve().getPoints())
                    scaled.addPoint(pt.time, pt.value * pitchRatio, pt.pressure);
                n.setPitchCurve(scaled);
            }
        }
        notesToPaste.append(n);
    }

    PasteNotesCommand *cmd = new PasteNotesCommand(&phrase, notesToPaste, targetTime, targetTrack, sc);
    sc->getUndoStack()->push(cmd);

    QJsonArray newIds;
    const QVector<Note> &updated = phrase.getNotes();
    for (int i : cmd->getPastedIndices()) {
        if (i >= 0 && i < updated.size())
            newIds.append(updated[i].getId());
    }

    return QJsonObject{ {"result", QJsonObject{
        {"count",   (int)newIds.size()},
        {"noteIds", newIds},
        {"message", QString("Duplicated %1 note(s) at offset %2 ms.")
                    .arg(newIds.size()).arg(offsetMs, 0, 'f', 1)}
    }}};
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: get_note_curves
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolGetNoteCurves(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    const QVector<Note> &notes = sc->getPhrase().getNotes();
    const int i = indices.first();
    if (i < 0 || i >= notes.size()) return error("Note index out of range.");
    const Note &n = notes[i];

    QJsonArray curves;
    for (int c = 0; c < n.getExpressiveCurveCount(); ++c) {
        const QString curveName = n.getExpressiveCurveName(c);
        const Curve &curve = n.getExpressiveCurve(c);
        QJsonArray pts;
        for (const Curve::Point &pt : curve.getPoints())
            pts.append(QJsonObject{ {"time", pt.time}, {"value", pt.value} });
        curves.append(QJsonObject{
            {"index", c}, {"name", curveName}, {"pointCount", pts.size()}, {"points", pts}
        });
    }

    return QJsonObject{ {"result", QJsonObject{
        {"noteId",     n.getId()},
        {"curveCount", (int)curves.size()},
        {"curves",     curves}
    }}};
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_note_curve
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetNoteCurve(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");
    if (!args.contains("points")) return error("'points' array is required.");

    const QString name = args.value("name").toString("Dynamics");
    const QJsonArray pointsArr = args["points"].toArray();
    QString err;
    QVector<EnvelopePoint> pts = parseEnvelopePoints(pointsArr, err);
    if (!err.isEmpty()) return error(err);

    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    const double weight  = args.value("weight").toDouble(1.0);
    const bool   perNote = args.value("perNote").toBool(false);

    if (name == "Dynamics" || name.isEmpty()) {
        sc->getUndoStack()->push(
            new ApplyDynamicsCurveCommand(&sc->getPhrase(), indices, pts, weight, perNote, sc));
        return ok(QString("Dynamics curve applied to %1 note(s).").arg(indices.size()));
    }
    sc->getUndoStack()->push(
        new ApplyExpressiveCurveToSelectionCommand(&sc->getPhrase(), indices, name, pts,
                                                    weight, perNote, sc));
    return ok(QString("Expressive curve '%1' applied to %2 note(s).").arg(name).arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_dynamics mode="expressive_batch" — per-note curves, single undo
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetNoteCurvesBatch(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");
    if (!args.contains("noteIds"))       return error("'noteIds' array is required.");
    if (!args.contains("pointsPerNote")) return error("'pointsPerNote' array is required.");

    const QJsonArray idsArr    = args["noteIds"].toArray();
    const QJsonArray pointsArr = args["pointsPerNote"].toArray();
    if (idsArr.isEmpty())    return error("'noteIds' must not be empty.");
    if (idsArr.size() != pointsArr.size())
        return error(QString("noteIds (%1) and pointsPerNote (%2) must have the same length.")
                     .arg(idsArr.size()).arg(pointsArr.size()));

    const QString name   = args.value("name").toString("Dynamics");
    const double  weight = args.value("weight").toDouble(1.0);

    // Resolve ids to indices, preserving caller ordering so pointsPerNote[i]
    // maps to noteIds[i]. Duplicates and unknown ids produce an error so the
    // caller can correct rather than silently apply a misaligned curve.
    const QVector<Note> &notes = sc->getPhrase().getNotes();
    QHash<QString, int> idToIdx;
    idToIdx.reserve(notes.size());
    for (int i = 0; i < notes.size(); ++i) idToIdx.insert(notes[i].getId(), i);

    QVector<int> indices;
    QVector<QVector<EnvelopePoint>> pointsPerNote;
    indices.reserve(idsArr.size());
    pointsPerNote.reserve(pointsArr.size());
    QSet<QString> seen;
    for (int i = 0; i < idsArr.size(); ++i) {
        const QString id = idsArr[i].toString();
        if (id.isEmpty())          return error(QString("noteIds[%1] is empty.").arg(i));
        if (!idToIdx.contains(id)) return error(QString("noteIds[%1] ('%2') not found.").arg(i).arg(id));
        if (seen.contains(id))     return error(QString("noteIds[%1] ('%2') is a duplicate.").arg(i).arg(id));
        seen.insert(id);
        indices.append(idToIdx.value(id));

        const QJsonArray ptsJson = pointsArr[i].toArray();
        if (ptsJson.isEmpty())
            return error(QString("pointsPerNote[%1] is empty.").arg(i));
        QString err;
        QVector<EnvelopePoint> pts = parseEnvelopePoints(ptsJson, err);
        if (!err.isEmpty())
            return error(QString("pointsPerNote[%1]: %2").arg(i).arg(err));
        pointsPerNote.append(pts);
    }

    sc->getUndoStack()->push(
        new SetNoteCurvesBatchCommand(&sc->getPhrase(), indices, pointsPerNote,
                                      name, weight, sc));

    return ok(QString("%1 curve '%2' applied per-note to %3 notes (single undo).")
              .arg(name == "Dynamics" ? "Dynamics" : "Expressive")
              .arg(name)
              .arg(indices.size()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: fade_out_notes
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolFadeOutNotes(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const double startTime = args.value("startTime").toDouble(0.85);
    const double endValue  = args.value("endValue").toDouble(0.0);

    if (startTime <= 0.0 || startTime >= 1.0)
        return error("startTime must be between 0 and 1 (exclusive). Typical value: 0.85.");

    const QVector<int> indices = resolveNoteIndices(sc, args["noteIds"].toArray());
    if (indices.isEmpty()) return error("No matching notes found.");

    sc->getUndoStack()->push(
        new FadeOutNotesCommand(&sc->getPhrase(), indices, startTime, endValue, sc));

    return ok(QString("Fade-out applied to %1 note(s): preserves existing curve up to t=%2, then fades to %3.")
              .arg(indices.size()).arg(startTime).arg(endValue));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: open_project
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolOpenProject(const QJsonObject &args)
{
    if (!m_kalaMain) return error("KalaMain not available.");
    if (!args.contains("filePath")) return error("'filePath' is required.");
    const QString path = args["filePath"].toString();
    if (path.isEmpty()) return error("filePath must not be empty.");
    if (!m_kalaMain->loadProjectFile(path))
        return error("Failed to load project: " + path);
    return ok("Project loaded: " + path);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: save_project
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSaveProject(const QJsonObject &args)
{
    if (!m_kalaMain) return error("KalaMain not available.");
    QString path = args.value("filePath").toString();
    if (path.isEmpty()) {
        path = m_kalaMain->getProjectFilePath();
        if (path.isEmpty()) return error("No current project path. Provide 'filePath'.");
    }
    if (!m_kalaMain->saveProjectFile(path))
        return error("Failed to save project: " + path);
    return ok("Project saved: " + path);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: add_track
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolAddTrack(const QJsonObject &args)
{
    if (!m_trackManager) return error("TrackManager not available.");
    if (!m_scoreCanvasWindow) return error("Score canvas window not available.");

    const QString name = args.value("name").toString("New Track");
    QColor color;
    const QString colorStr = args.value("color").toString();
    if (!colorStr.isEmpty() && QColor::isValidColorName(colorStr))
        color = QColor(colorStr);
    else
        color = m_scoreCanvasWindow->getNextTrackColor();

    m_trackManager->addTrack(name, color);
    const int newIndex = m_trackManager->getTrackCount() - 1;

    const double LOWEST_NOTE  = 27.5;
    const double HIGHEST_NOTE = 4186.0;
    m_scoreCanvasWindow->getTrackSelector()->addTrack(name, color, LOWEST_NOTE, HIGHEST_NOTE);

    return QJsonObject{ {"result", QJsonObject{
        {"trackIndex", newIndex},
        {"message", QString("Track '%1' added at index %2.").arg(name).arg(newIndex)}
    }}};
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: rename_track
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolRenameTrack(const QJsonObject &args)
{
    if (!m_trackManager || !m_scoreCanvasWindow) return error("Track system not available.");
    if (!args.contains("trackIndex") || !args.contains("name"))
        return error("'trackIndex' and 'name' are required.");

    const int idx = args["trackIndex"].toInt();
    const QString newName = args["name"].toString().trimmed();
    if (newName.isEmpty()) return error("name must not be empty.");

    Track *t = m_trackManager->getTrack(idx);
    if (!t) return error(QString("Invalid trackIndex %1.").arg(idx));

    t->setName(newName);
    m_scoreCanvasWindow->getTrackSelector()->updateTrack(idx, newName, t->getColor());

    return ok(QString("Track %1 renamed to '%2'.").arg(idx).arg(newName));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: delete_track
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolDeleteTrack(const QJsonObject &args)
{
    if (!m_trackManager || !m_scoreCanvasWindow) return error("Track system not available.");
    if (!args.contains("trackIndex")) return error("'trackIndex' is required.");

    const int idx = args["trackIndex"].toInt();
    if (idx < 0 || idx >= m_trackManager->getTrackCount())
        return error(QString("Invalid trackIndex %1. Valid range: 0–%2.")
                     .arg(idx).arg(m_trackManager->getTrackCount() - 1));
    if (m_trackManager->getTrackCount() <= 1)
        return error("Cannot delete the last track.");

    // Remove notes from score canvas
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (sc) {
        sc->deleteNotesOnTrack(idx);
        sc->updateTrackIndicesAfterDeletion(idx);
    }

    // Remove from visual selector
    m_scoreCanvasWindow->getTrackSelector()->removeTrack(idx);

    // Remove from TrackManager (data)
    m_trackManager->removeTrack(idx);

    return ok(QString("Track %1 deleted.").arg(idx));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: remove_connection
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolRemoveConnection(const QJsonObject &args)
{
    const QString fromInst = args["fromInstance"].toString();
    const QString fromPort = args["fromPort"].toString();
    const QString toInst   = args["toInstance"].toString();
    const QString toPort   = args["toPort"].toString();

    if (fromInst.isEmpty() || fromPort.isEmpty() ||
        toInst.isEmpty()   || toPort.isEmpty())
        return error("fromInstance, fromPort, toInstance and toPort are all required.");

    Canvas *cv = currentCanvas();
    if (!cv) return error("No active canvas.");

    const QVector<Canvas::Connection> &connections = cv->getConnections();
    int foundIndex = -1;
    for (int i = 0; i < connections.size(); ++i) {
        const Canvas::Connection &c = connections[i];
        if (c.fromContainer && c.toContainer
            && c.fromContainer->getInstanceName() == fromInst
            && c.fromPort == fromPort
            && c.toContainer->getInstanceName() == toInst
            && c.toPort == toPort)
        {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex == -1)
        return error(QString("Connection %1:%2 → %3:%4 not found.")
                     .arg(fromInst, fromPort, toInst, toPort));

    cv->getUndoStack()->push(new DeleteConnectionCommand(foundIndex, cv));

    return ok(QString("Removed connection %1:%2 → %3:%4.")
              .arg(fromInst, fromPort, toInst, toPort));
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper: resolve a timeMs / barsAndBeats argument to milliseconds.
// Returns -1.0 on error and sets *errOut.
// ─────────────────────────────────────────────────────────────────────────────

static double resolveTimeArg(const QJsonObject &obj,
                              const QString &msKey,
                              const QString &bbKey,
                              ScoreCanvas *sc,
                              QString *errOut)
{
    if (obj.contains(msKey)) {
        double t = obj[msKey].toDouble();
        return t < 0.0 ? 0.0 : t;
    }
    if (obj.contains(bbKey)) {
        if (!sc) { *errOut = "Score canvas not available."; return -1.0; }
        QJsonObject bb = obj[bbKey].toObject();
        int    bar    = bb.value("bar").toInt(1);
        int    beat   = bb.value("beat").toInt(1);
        double bpm    = sc->getDefaultTempo();
        int    num    = sc->getDefaultTimeSigNum();
        double beatMs = 60000.0 / bpm;
        double t = ((bar - 1) * num + (beat - 1)) * beatMs;
        return t < 0.0 ? 0.0 : t;
    }
    *errOut = QString("Provide '%1' (ms) or '%2' ({bar, beat}).").arg(msKey, bbKey);
    return -1.0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: seek
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSeek(const QJsonObject &args)
{
    if (!m_scoreCanvasWindow) return error("Score canvas not available.");
    Timeline *timeline = m_scoreCanvasWindow->getTimeline();
    if (!timeline) return error("Timeline not available.");

    QString err;
    double timeMs = resolveTimeArg(args, "timeMs", "barsAndBeats",
                                   getScoreCanvas(m_scoreCanvasWindow), &err);
    if (timeMs < 0.0) return error(err);

    timeline->setNowMarker(timeMs);
    emit timeline->nowMarkerChanged(timeMs);
    return ok(QString("Seeked to %1 ms.").arg(timeMs));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_loop
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetLoop(const QJsonObject &args)
{
    if (!m_scoreCanvasWindow) return error("Score canvas not available.");
    Timeline *timeline = m_scoreCanvasWindow->getTimeline();
    if (!timeline) return error("Timeline not available.");
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);

    QString err;
    double startMs = resolveTimeArg(args, "startMs", "start", sc, &err);
    if (startMs < 0.0) return error("start: " + err);

    double endMs = resolveTimeArg(args, "endMs", "end", sc, &err);
    if (endMs < 0.0) return error("end: " + err);

    if (endMs <= startMs)
        return error("Loop end must be after loop start.");

    // Set the loop region: position now-marker at start, activate loop mode
    // (which sets loopStart = nowMarkerTime), then set loop end.
    timeline->setNowMarker(startMs);
    timeline->setLoopModeActive(true);
    timeline->setLoopEnd(endMs);

    // Also seek to loop start so play_score begins there
    emit timeline->nowMarkerChanged(startMs);

    return ok(QString("Loop set: %1 ms → %2 ms. Call play_score to begin.")
              .arg(startMs).arg(endMs));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: clear_loop
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolClearLoop()
{
    if (!m_scoreCanvasWindow) return error("Score canvas not available.");
    Timeline *timeline = m_scoreCanvasWindow->getTimeline();
    if (!timeline) return error("Timeline not available.");

    timeline->clearLoop();
    return ok("Loop cleared.");
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: get_engine_settings
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolGetEngineSettings()
{
    auto &cs = ContainerSettings::instance();
    QJsonObject result;
    result["containerSettings"] = cs.toJson();

    if (m_scoreCanvasWindow) {
        CompositionSettings ps = m_scoreCanvasWindow->getSettings();
        QJsonObject proj;
        proj["compositionName"] = ps.compositionName;
        proj["sampleRate"]      = ps.sampleRate;
        proj["bitDepth"]        = ps.bitDepth;
        proj["lengthMs"]        = ps.lengthMs;
        result["project"] = proj;
    }

    return QJsonObject{ {"result", result} };
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool: set_engine_settings
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject KalaTools::toolSetEngineSettings(const QJsonObject &args)
{
    auto &cs = ContainerSettings::instance();
    int count = 0;

    if (args.contains("containerSettings")) {
        // Merge incoming partial object onto current values so unspecified
        // fields within a group keep their existing value rather than reset
        // to the hardcoded default inside fromJson().
        QJsonObject current  = cs.toJson();
        QJsonObject incoming = args["containerSettings"].toObject();
        for (const QString &groupKey : incoming.keys()) {
            if (current.contains(groupKey) && incoming[groupKey].isObject()) {
                QJsonObject currentGroup  = current[groupKey].toObject();
                QJsonObject incomingGroup = incoming[groupKey].toObject();
                for (const QString &fieldKey : incomingGroup.keys())
                    currentGroup[fieldKey] = incomingGroup[fieldKey];
                current[groupKey] = currentGroup;
            } else {
                current[groupKey] = incoming[groupKey];
            }
        }
        cs.fromJson(current);
        cs.saveSettings();
        ++count;
    }

    if (args.contains("project") && m_scoreCanvasWindow) {
        QJsonObject proj = args["project"].toObject();
        CompositionSettings settings = m_scoreCanvasWindow->getSettings();
        if (proj.contains("compositionName"))
            settings.compositionName = proj["compositionName"].toString();
        if (proj.contains("sampleRate"))
            settings.sampleRate = proj["sampleRate"].toInt();
        if (proj.contains("bitDepth"))
            settings.bitDepth = proj["bitDepth"].toInt();
        if (proj.contains("lengthMs")) {
            settings.lengthMs = proj["lengthMs"].toDouble();
            settings.syncLengthFromMs();
        }
        m_scoreCanvasWindow->updateFromSettings(settings);
        ++count;
    }

    if (count == 0)
        return error("No recognized keys provided. Use 'containerSettings' and/or 'project'.");

    return ok(QString("Applied %1 settings group(s).").arg(count));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tool Schemas (OpenAI function-calling format)
// ─────────────────────────────────────────────────────────────────────────────

// Helper to build a simple string property object
static QJsonObject strProp(const QString &desc)
{
    return QJsonObject{ {"type", "string"}, {"description", desc} };
}
static QJsonObject numProp(const QString &desc)
{
    return QJsonObject{ {"type", "number"}, {"description", desc} };
}
static QJsonObject objProp(const QString &desc, const QJsonObject &props = {})
{
    QJsonObject o{ {"type", "object"}, {"description", desc} };
    if (!props.isEmpty()) o["properties"] = props;
    return o;
}

// ─────────────────────────────────────────────────────────────────────────────
// Private static schema helpers
// ─────────────────────────────────────────────────────────────────────────────

static QJsonObject actionProp(const QString &desc, const QJsonArray &values)
{
    return QJsonObject{{"type","string"},{"description",desc},{"enum",values}};
}

static QJsonArray coreSchemas()
{
    QJsonArray schemas;
    // ── browse_library ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "browse_library"},
            {"description", "Lists files from the library."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"type", actionProp("Library type", {"sounit","spectrum","ir","envelope","project"})}
                }},
                {"required", QJsonArray{"type"}}
            }}
        }}
    });
    // ── history ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "history"},
            {"description", "Undo or redo the last action."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"action", actionProp("Direction", {"undo","redo"})}
                }},
                {"required", QJsonArray{"action"}}
            }}
        }}
    });
    // ── track ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "track"},
            {"description", "Manage tracks. list: returns all tracks. add: creates track (name required). rename: renames (trackIndex+name). delete: removes track+notes."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"action",     actionProp("Operation", {"list","add","rename","delete"})},
                    {"trackIndex", QJsonObject{{"type","integer"},{"description","0-based index"}}},
                    {"name",       strProp("Track name")},
                    {"color",      strProp("Hex color (add only)")}
                }},
                {"required", QJsonArray{"action"}}
            }}
        }}
    });
    // ── project ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "project"},
            {"description", "Open or save a .kala project file. Omit filePath on save to save in place."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"action",   actionProp("Operation", {"open","save"})},
                    {"filePath", strProp("Absolute path")}
                }},
                {"required", QJsonArray{"action"}}
            }}
        }}
    });
    // ── set_library_root (standalone) ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_library_root"},
            {"description", "Sets library root folder (saved permanently)."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{{"path", strProp("Absolute path to library root")}}},
                {"required", QJsonArray{"path"}}
            }}
        }}
    });
    return schemas;
}

static QJsonArray sounitSchemas()
{
    const QJsonArray containerTypes = {
        "Harmonic Generator", "Spectrum to Signal", "Karplus Strong",
        "Signal Mixer", "Note Tail", "Attack", "Wavetable Synth",
        "Recorder", "Bowed", "Reed",
        "Rolloff Processor", "Spectrum Blender", "Formant Body",
        "Breath Turbulence", "Noise Color Filter",
        "Physics System", "Easing Applicator", "Envelope Engine",
        "Drift Engine", "LFO", "Frequency Mapper", "Pan",
        "10-Band EQ", "Comb Filter", "LP/HP Filter", "IR Convolution"
    };
    const QJsonArray connectionFunctions = {
        "passthrough", "add", "subtract", "multiply", "replace", "modulate"
    };
    QJsonArray schemas;
    // ── get_graph_state (standalone) ──
    schemas.append(QJsonObject{{"type","function"},{"function",QJsonObject{{"name","get_graph_state"},{"description","Returns all containers and connections."},{"parameters",QJsonObject{{"type","object"},{"properties",QJsonObject{}},{"required",QJsonArray{}}}}}}});
    // ── edit_graph ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "edit_graph"},
            {"description", "Mutate the sounit graph. add: adds container (type required). remove: removes container (instanceName). rename: rename container (instanceName + newName). connect: connects ports (fromInstance/fromPort/toInstance/toPort, each output→one input only). disconnect: removes connection (same 4 params). clear: removes all."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"action",       actionProp("Operation", {"add","remove","rename","connect","disconnect","clear"})},
                    {"type",         QJsonObject{{"type","string"},{"description","Container type (add)."},{"enum",containerTypes}}},
                    {"params",       QJsonObject{{"type","object"},{"description","Param overrides (add). Numeric values go to numeric params; string values go to string params (e.g. scoreCurveName, customDnaName)."}}},
                    {"position",     QJsonObject{{"type","object"},{"description","{x,y} position (add)."},{"properties",QJsonObject{{"x",numProp("X")},{"y",numProp("Y")}}}}},
                    {"instanceName", strProp("Instance name (remove/rename)")},
                    {"newName",      strProp("New instance name (rename)")},
                    {"fromInstance", strProp("Source instance (connect/disconnect)")},
                    {"fromPort",     strProp("Source port (connect/disconnect)")},
                    {"toInstance",   strProp("Dest instance (connect/disconnect)")},
                    {"toPort",       strProp("Dest port (connect/disconnect)")},
                    {"function",     QJsonObject{{"type","string"},{"description","Combine fn (connect, default passthrough)"},{"enum",connectionFunctions}}},
                    {"weight",       numProp("Scale factor (connect)")}
                }},
                {"required", QJsonArray{"action"}}
            }}
        }}
    });
    // ── set_parameter (standalone, prefer set_parameters) ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_parameter"},
            {"description", "Sets one parameter on a container. Prefer set_parameters for multiple changes. Pass a string value for string params (e.g. scoreCurveName), numeric otherwise."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"instanceName", strProp("Instance name")},
                    {"param",        strProp("Parameter name")},
                    {"value",        QJsonObject{{"type", QJsonArray{"number","string"}},{"description","Numeric for numeric params, string for string params (e.g. scoreCurveName)."}}}
                }},
                {"required", QJsonArray{"instanceName", "param", "value"}}
            }}
        }}
    });
    // ── set_parameters (standalone, batch) ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_parameters"},
            {"description", "Sets multiple parameters across containers. Always use this instead of multiple set_parameter calls. Each value may be number or string (string for params like scoreCurveName)."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"changes", QJsonObject{
                        {"type", "array"},
                        {"description", "[{instanceName, param, value}]"},
                        {"items", QJsonObject{
                            {"type", "object"},
                            {"properties", QJsonObject{
                                {"instanceName", strProp("Container instance name")},
                                {"param",        strProp("Parameter name")},
                                {"value",        QJsonObject{{"type", QJsonArray{"number","string"}},{"description","Numeric for numeric params, string for string params (e.g. scoreCurveName)."}}}
                            }},
                            {"required", QJsonArray{"instanceName", "param", "value"}}
                        }}
                    }}
                }},
                {"required", QJsonArray{"changes"}}
            }}
        }}
    });
    // ── load_sounit / save_sounit (standalone) ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "load_sounit"},
            {"description", "Loads .sounit file, replacing current graph."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{{"filePath", strProp("Absolute path")}}},
                {"required", QJsonArray{"filePath"}}
            }}
        }}
    });
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "save_sounit"},
            {"description", "Saves current graph to .sounit file."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"filePath", strProp("Absolute path")},
                    {"name",     strProp("Display name (optional)")}
                }},
                {"required", QJsonArray{"filePath"}}
            }}
        }}
    });
    // play_preview is accessed via transport(action:"preview")
    // ── load_ir / load_spectrum / load_envelope / set_envelope_shape (standalone) ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "load_ir"},
            {"description", "Loads WAV IR into an IR Convolution container."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"instanceName", strProp("Instance name")},
                    {"filePath",     strProp("Absolute .wav path")}
                }},
                {"required", QJsonArray{"instanceName", "filePath"}}
            }}
        }}
    });
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "load_envelope"},
            {"description", "Loads .env.json into an Envelope Engine."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"instanceName", strProp("Instance name")},
                    {"filePath",     strProp("Absolute .env.json path")}
                }},
                {"required", QJsonArray{"instanceName", "filePath"}}
            }}
        }}
    });
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_envelope_shape"},
            {"description", "Sets custom envelope on Envelope Engine. Points: [{time,value,curveType}] (0–1). loopMode: 0=None,1=Loop,2=PingPong."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"instanceName", strProp("Instance name")},
                    {"points", QJsonObject{
                        {"type","array"},
                        {"description","[{time,value,curveType}]"},
                        {"items", QJsonObject{{"type","object"}}}
                    }},
                    {"name",     strProp("Envelope name (optional)")},
                    {"loopMode", numProp("0=None,1=Loop,2=PingPong")}
                }},
                {"required", QJsonArray{"instanceName", "points"}}
            }}
        }}
    });
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "load_spectrum"},
            {"description", "Loads .dna.json into Harmonic Generator (sets dnaSelect=-1)."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"instanceName", strProp("Instance name")},
                    {"filePath",     strProp("Absolute .dna.json path")}
                }},
                {"required", QJsonArray{"instanceName", "filePath"}}
            }}
        }}
    });
    // ── variation ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "variation"},
            {"description", "Manage sounit variations. list: returns all (0=base). create: snapshot canvas (name required). create_from_sounit: load .sounit as variation (filePath+name). delete: remove (variationIndex 1+). rename: (variationIndex+name). switch: load onto canvas (variationIndex 0=base). apply: assign to notes (variationIndex, noteIds optional). copy_to_base: promote variation to base sounit (variationIndex 1+)."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"action",         actionProp("Operation", {"list","create","create_from_sounit","delete","rename","switch","apply","copy_to_base"})},
                    {"variationIndex", QJsonObject{{"type","integer"},{"description","0=base,1+=named"}}},
                    {"name",           strProp("Variation name (create/rename)")},
                    {"filePath",       strProp("Absolute .sounit path (create_from_sounit)")},
                    {"noteIds",        QJsonObject{{"type","array"},{"description","Note ids (apply). Empty=all."}}}
                }},
                {"required", QJsonArray{"action"}}
            }}
        }}
    });
    // ── engine_settings ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "engine_settings"},
            {"description", "Get or set engine settings. get: returns containerSettings (min/max) + project settings. set: partial update."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"action",            actionProp("Operation", {"get","set"})},
                    {"containerSettings", QJsonObject{{"type","object"},{"description","By group name (set)."}}},
                    {"project",           QJsonObject{{"type","object"},{"description","Project fields (set)."}}}
                }},
                {"required", QJsonArray{"action"}}
            }}
        }}
    });
    return schemas;
}

static QJsonArray compositionSchemas()
{
    QJsonArray schemas;
    // ── get_composition_state (standalone) ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_composition_state"},
            {"description", "Returns notes (id,startTime,duration,pitchHz,dynamics,trackIndex), scale, tempo, time sig. Optional trackIndex filter."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{{"trackIndex", QJsonObject{{"type","integer"},{"description","Track filter"}}}}},
                {"required", QJsonArray{}}
            }}
        }}
    });
    // ── add_note / delete_note / clear_notes (standalone) ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "add_note"},
            {"description", "Adds a note (startTime/duration ms, pitchHz Hz, dynamics 0–1). Returns id."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"startTime",  numProp("Start ms")},
                    {"duration",   numProp("Duration ms")},
                    {"pitchHz",    numProp("Hz (A4=440)")},
                    {"dynamics",   numProp("0–1 (default 0.7)")},
                    {"trackIndex", QJsonObject{{"type","integer"},{"description","0-based track"}}}
                }},
                {"required", QJsonArray{"startTime", "duration", "pitchHz"}}
            }}
        }}
    });
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "delete_note"},
            {"description", "Deletes a note by id."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{{"id", strProp("Note id")}}},
                {"required", QJsonArray{"id"}}
            }}
        }}
    });
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "clear_notes"},
            {"description", "Removes notes. Omit trackIndex to clear all."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{{"trackIndex", QJsonObject{{"type","integer"},{"description","Track filter"}}}}},
                {"required", QJsonArray{}}
            }}
        }}
    });
    // ── set_scale / set_tempo (standalone) ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_scale"},
            {"description", "Sets tuning scale and optional base freq. IDs: 0=JustIntonation,1=Pythagorean,2=EqualTemperament,3=Meantone,4=MaqamRast,5=MaqamBayati,7=MaqamHijaz,10=RagaBhairav,11=RagaYaman,13=RagaBhairavi,16=ChinesePentatonic,17=Hirajoshi,20=WholeTone,30=PersianShur."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"scaleId",    QJsonObject{{"type","integer"},{"description","Scale ID"}}},
                    {"scaleName",  strProp("Scale name")},
                    {"baseFreqHz", numProp("Tonic Hz (default 261.63)")}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_tempo"},
            {"description", "Sets default tempo in BPM."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{{"bpm", numProp("BPM (20–400)")}}},
                {"required", QJsonArray{"bpm"}}
            }}
        }}
    });
    // ── transport ──
    {
        auto bbProp = [](const QString &label) {
            return QJsonObject{{"type","object"},{"description",label},{"properties",QJsonObject{{"bar",QJsonObject{{"type","integer"},{"description","Bar"}}},{"beat",QJsonObject{{"type","integer"},{"description","Beat"}}}}}};
        };
        schemas.append(QJsonObject{
            {"type", "function"},
            {"function", QJsonObject{
                {"name", "transport"},
                {"description", "Playback transport control. play: starts score playback. stop: stops. preview: auditions sounit graph. seek: moves now-marker (timeMs or barsAndBeats {bar,beat}). loop: sets loop region (startMs/endMs or start/end {bar,beat}). clear_loop: removes loop."},
                {"parameters", QJsonObject{
                    {"type", "object"},
                    {"properties", QJsonObject{
                        {"action",        actionProp("Operation", {"play","stop","preview","seek","loop","clear_loop"})},
                        {"timeMs",        numProp("Position ms (seek)")},
                        {"barsAndBeats",  bbProp("Position {bar,beat} (seek)")},
                        {"startMs",       numProp("Loop start ms")},
                        {"endMs",         numProp("Loop end ms")},
                        {"start",         bbProp("Loop start {bar,beat}")},
                        {"end",           bbProp("Loop end {bar,beat}")}
                    }},
                    {"required", QJsonArray{"action"}}
                }}
            }}
        });
    }
    // ── time_signature ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "time_signature"},
            {"description", "Manage tempo/time-sig markers. get: returns all. set: changes opening (time 0) values. add: inserts change at timeMs>0. remove: deletes marker at timeMs."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"action",      actionProp("Operation", {"get","set","add","remove"})},
                    {"timeMs",      QJsonObject{{"type","number"},{"description","Position ms (add/remove)"}}},
                    {"bpm",         QJsonObject{{"type","number"},{"description","BPM (set/add)"}}},
                    {"numerator",   QJsonObject{{"type","integer"},{"description","Numerator (set/add)"}}},
                    {"denominator", QJsonObject{{"type","integer"},{"description","Denominator (set/add)"}}},
                    {"gradual",     QJsonObject{{"type","boolean"},{"description","Gradual transition (set/add)"}}}
                }},
                {"required", QJsonArray{"action"}}
            }}
        }}
    });
    // ── inspect_notes ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "inspect_notes"},
            {"description", "Read-only inspection. selection: returns IDs of user-selected notes. curves: returns expressive curves on first targeted note. vibrato: returns vibrato settings."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"what",    actionProp("What to inspect", {"selection","curves","vibrato"})},
                    {"noteIds", QJsonObject{{"type","array"},{"description","Note ids (curves/vibrato — first inspected)."}}}
                }},
                {"required", QJsonArray{"what"}}
            }}
        }}
    });
    // ── select_notes ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "select_notes"},
            {"description", "Selects notes. range: by pitch/duration/track/indices/noteIds. flat_dynamics: notes with flat dynamics. current: returns selected IDs."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"mode",          actionProp("Selection mode", {"range","flat_dynamics","current"})},
                    {"indices",       QJsonObject{{"type","string"},{"description","Index range like '0-29' or 'all' (range)"}}},
                    {"noteIds",       QJsonObject{{"type","array"},{"description","Select exactly these UUIDs (range). Other filters (pitch/duration/track) further narrow the set if provided."}}},
                    {"pitchMinHz",    numProp("Lower Hz bound (range)")},
                    {"pitchMaxHz",    numProp("Upper Hz bound (range)")},
                    {"durationMinMs", numProp("Minimum duration ms (range)")},
                    {"durationMaxMs", numProp("Maximum duration ms (range)")},
                    {"trackIndex",    QJsonObject{{"type","integer"},{"description","Track filter"}}}
                }},
                {"required", QJsonArray{"mode"}}
            }}
        }}
    });
    // ── set_note_mode ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_note_mode"},
            {"description", "Set note articulation/continuity. legato: link notes (no retrigger, min 2). unlegato: remove links. continuous: extend to next note. discrete: exact duration."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"mode",    actionProp("Mode", {"legato","unlegato","continuous","discrete"})},
                    {"noteIds", QJsonObject{{"type","array"},{"description","Note ids. Empty=all (legato needs min 2)."}}}
                }},
                {"required", QJsonArray{"mode"}}
            }}
        }}
    });
    // ── set_dynamics ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_dynamics"},
            {"description", "Set dynamics/expressive data. curve: apply points [{time,value,curveType}]. envelope_file: load .env.json. scale: multiply by factor. per_note: set individual values (noteIds+values arrays). beat_pattern: accent pattern. expressive: named curve (name+points) — same curve on every note. expressive_batch: per-note curves (name + noteIds[] + pointsPerNote[][]) — one undo entry."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"mode",          actionProp("Mode", {"curve","envelope_file","scale","per_note","beat_pattern","expressive","expressive_batch"})},
                    {"points",        QJsonObject{{"type","array"},{"description","[{time,value,curveType}] (curve/expressive)"},{"items",QJsonObject{{"type","object"}}}}},
                    {"pointsPerNote", QJsonObject{{"type","array"},{"description","Array of points arrays, one per noteId (expressive_batch). Same length/order as noteIds."},{"items",QJsonObject{{"type","array"}}}}},
                    {"filePath", strProp("Path to .env.json (envelope_file)")},
                    {"factor",   numProp("Multiplier (scale)")},
                    {"values",   QJsonObject{{"type","array"},{"description","Dynamics 0–1 per note (per_note)"},{"items",QJsonObject{{"type","number"}}}}},
                    {"pattern",  QJsonObject{{"type","array"},{"description","Dynamics per beat (beat_pattern)"},{"items",QJsonObject{{"type","number"}}}}},
                    {"name",     strProp("Curve name (expressive/expressive_batch, default 'Dynamics')")},
                    {"weight",   numProp("Blend 0–1")},
                    {"perNote",  QJsonObject{{"type","boolean"},{"description","Per-note lifetime"}}},
                    {"noteIds",  QJsonObject{{"type","array"},{"description","Note ids. Empty=all (required for expressive_batch)."}}}
                }},
                {"required", QJsonArray{"mode"}}
            }}
        }}
    });
    // ── transform_notes ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "transform_notes"},
            {"description", "Transform note pitch/timing. transpose: by ratio or scaleDegrees. set_pitch: set Hz. shift: move starts by offsetMs. stretch: multiply durations by factor. set_duration: set ms. scale_timing: scale starts+durations. ease_rhythm: redistribute onsets (easingType 0–28). quantize: snap pitch to scale. strum: rake stacked notes — direction down/up, speedMs total stagger, dynamicsShape flat|accent_low|accent_high, cycleVariations assigns variations by pitch (highest→0/base, descending→1,2,…)."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"transform",           actionProp("Operation", {"transpose","set_pitch","shift","stretch","set_duration","scale_timing","ease_rhythm","quantize","strum"})},
                    {"ratio",               numProp("Pitch multiplier (transpose)")},
                    {"scaleDegrees",        QJsonObject{{"type","integer"},{"description","Steps +up/-down (transpose)"}}},
                    {"pitchHz",             numProp("Hz (set_pitch / shift filter)")},
                    {"values",              QJsonObject{{"type","array"},{"description","Per-note values (set_pitch/set_duration)"}}},
                    {"offsetMs",            numProp("Ms offset (shift)")},
                    {"pitchToleranceCents", numProp("Tolerance cents (shift)")},
                    {"factor",              numProp("Duration multiplier (stretch)")},
                    {"duration",            numProp("Duration ms (set_duration)")},
                    {"proportion",          numProp("Timing scale (scale_timing)")},
                    {"easingType",          QJsonObject{{"type","integer"},{"description","0–28 (ease_rhythm)"}}},
                    {"anchorMode",          QJsonObject{{"type","integer"},{"description","0–3 (ease_rhythm)"}}},
                    {"weight",              numProp("Blend 0–1 (ease_rhythm)")},
                    {"direction",           actionProp("Strum direction", {"down","up"})},
                    {"speedMs",             numProp("Strum total stagger ms (default 30)")},
                    {"dynamicsShape",       actionProp("Strum dynamics shape", {"flat","accent_low","accent_high"})},
                    {"cycleVariations",     QJsonObject{{"type","boolean"},{"description","Spread track variations across pitches (strum)"}}},
                    {"noteIds",             QJsonObject{{"type","array"},{"description","Note ids. Empty=all."}}}
                }},
                {"required", QJsonArray{"transform"}}
            }}
        }}
    });
    // ── set_note_vibrato (standalone) ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_note_vibrato"},
            {"description", "Sets vibrato. Unspecified params preserved. active=bool, rate=Hz(3–8), pitchDepth/amplitudeDepth/onset/regularity=0–1, envelope=[{time,value,curveType}]."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"active",         QJsonObject{{"type","boolean"},{"description","Enable"}}},
                    {"rate",           numProp("Hz")},
                    {"pitchDepth",     numProp("0–1")},
                    {"amplitudeDepth", numProp("0–1")},
                    {"onset",          numProp("0–1")},
                    {"regularity",     numProp("0–1")},
                    {"envelope", QJsonObject{{"type","array"},{"description","[{time,value,curveType}]"},{"items",QJsonObject{{"type","object"}}}}},
                    {"noteIds", QJsonObject{{"type","array"},{"description","Note ids. Empty=all."}}}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });
    // ── duplicate_notes (standalone) ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "duplicate_notes"},
            {"description", "Duplicates notes at source+offsetMs. Returns new ids."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"offsetMs",   numProp("Offset from source ms")},
                    {"pitchRatio", numProp("Pitch multiplier")},
                    {"trackIndex", QJsonObject{{"type","integer"},{"description","Target track"}}},
                    {"noteIds", QJsonObject{{"type","array"},{"description","Note ids. Empty=all."}}}
                }},
                {"required", QJsonArray{"offsetMs"}}
            }}
        }}
    });
    // ── fade_out_notes (standalone) ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "fade_out_notes"},
            {"description", "Splices fade-out into tail of dynamics curve. Preserves curve up to startTime."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"startTime", numProp("Normalized time (0–1) where fade begins. Default 0.85.")},
                    {"endValue",  numProp("Target value at t=1. Default 0.0.")},
                    {"noteIds",   QJsonObject{{"type","array"},{"description","Note IDs. Empty = all on current track."}}}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });
    // ── assign_guitar_strings (standalone) ──
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "assign_guitar_strings"},
            {"description", "Assign notes to guitar strings via Viterbi pathfinding, then map each string to a variation index. Use after writing a phrase — pre-load 6 per-string sounits as variations (0=high-E, 5=low-E). Default EADGBE tuning snaps to the active scale (not hardcoded ET), so it adapts to Just Intonation, Maqam, Raga, etc. Wound strings (default lowest 3): brighter/metallic. Unwound (highest 3): mellower. Notes out of range are assigned to the nearest string. Returns per-note assignments with string/fret/variation/wound."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"noteIds",           QJsonObject{{"type","array"},{"description","Note IDs. Empty = all on current track."}}},
                    {"tuning",            QJsonObject{{"type","array"},{"description","Open-string pitches. Hz or MIDI (>900). 6 entries: [high-E,B,G,D,A,low-E]. Default EADGBE standard."}}},
                    {"maxFret",           numProp("Highest playable fret. Default 19.")},
                    {"maxStretch",        numProp("Max fret span within one hand position. Default 4.")},
                    {"preferOpen",        QJsonObject{{"type","boolean"},{"description","Favor open strings when viable. Default false."}}},
                    {"woundStrings",      QJsonObject{{"type","integer"},{"description","How many lowest strings are wound (timbre distinction). Default 3."}}},
                    {"stringSkipPenalty", numProp("Cost weight for jumping between non-adjacent strings. Default 0.7.")}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });
    return schemas;
}

// ─────────────────────────────────────────────────────────────────────────────
// Instance method: assemble schemas based on current tool mode
// ─────────────────────────────────────────────────────────────────────────────

QJsonArray KalaTools::getToolSchemas() const
{
    QJsonArray schemas = coreSchemas();
    if (m_toolMode == ToolMode::Sounit || m_toolMode == ToolMode::Full)
        for (const QJsonValue &v : sounitSchemas()) schemas.append(v);
    if (m_toolMode == ToolMode::Composition || m_toolMode == ToolMode::Full)
        for (const QJsonValue &v : compositionSchemas()) schemas.append(v);
    return schemas;
}

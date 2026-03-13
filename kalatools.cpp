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
    if (toolName == "get_graph_state")       return toolGetGraphState();
    if (toolName == "get_sounit_list")       return toolGetSounitList();
    if (toolName == "clear_graph")           return toolClearGraph();
    if (toolName == "add_container")         return toolAddContainer(args);
    if (toolName == "set_parameter")         return toolSetParameter(args);
    if (toolName == "connect_containers")    return toolConnectContainers(args);
    if (toolName == "remove_container")      return toolRemoveContainer(args);
    if (toolName == "load_sounit")           return toolLoadSounit(args);
    if (toolName == "save_sounit")           return toolSaveSounit(args);
    if (toolName == "play_preview")          return toolPlayPreview();
    if (toolName == "get_spectrum_list")     return toolGetSpectrumList();
    if (toolName == "get_project_list")      return toolGetProjectList();
    if (toolName == "get_ir_list")           return toolGetIRList();
    if (toolName == "set_library_root")      return toolSetLibraryRoot(args);
    if (toolName == "load_ir")               return toolLoadIR(args);
    if (toolName == "load_spectrum")         return toolLoadSpectrum(args);
    if (toolName == "get_envelope_list")     return toolGetEnvelopeList();
    if (toolName == "load_envelope")         return toolLoadEnvelope(args);
    if (toolName == "set_envelope_shape")    return toolSetEnvelopeShape(args);
    if (toolName == "get_composition_state") return toolGetCompositionState(args);
    if (toolName == "get_track_list")        return toolGetTrackList();
    if (toolName == "add_note")              return toolAddNote(args);
    if (toolName == "delete_note")           return toolDeleteNote(args);
    if (toolName == "clear_notes")           return toolClearNotes(args);
    if (toolName == "set_scale")             return toolSetScale(args);
    if (toolName == "set_tempo")             return toolSetTempo(args);
    if (toolName == "get_variation_list")        return toolGetVariationList();
    if (toolName == "apply_variation")           return toolApplyVariation(args);
    if (toolName == "create_variation")          return toolCreateVariation(args);
    if (toolName == "create_variation_from_sounit") return toolCreateVariationFromSounit(args);
    if (toolName == "delete_variation")          return toolDeleteVariation(args);
    if (toolName == "rename_variation")          return toolRenameVariation(args);
    if (toolName == "switch_variation")          return toolSwitchVariation(args);
    if (toolName == "shift_notes")               return toolShiftNotes(args);
    if (toolName == "apply_beat_dynamics")        return toolApplyBeatDynamics(args);
    if (toolName == "set_note_dynamics")          return toolSetNoteDynamics(args);
    if (toolName == "select_notes")              return toolSelectNotes(args);
    if (toolName == "get_selected_notes")        return toolGetSelectedNotes();
    if (toolName == "apply_dynamics_curve")  return toolApplyDynamicsCurve(args);
    if (toolName == "scale_timing")          return toolScaleTiming(args);
    if (toolName == "apply_rhythm_easing")   return toolApplyRhythmEasing(args);
    if (toolName == "link_legato")           return toolLinkLegato(args);
    if (toolName == "unlink_legato")         return toolUnlinkLegato(args);
    if (toolName == "quantize_to_scale")     return toolQuantizeToScale(args);
    if (toolName == "make_continuous")       return toolMakeContinuous(args);
    if (toolName == "make_discrete")         return toolMakeDiscrete(args);
    if (toolName == "set_note_pitch")               return toolSetNotePitch(args);
    if (toolName == "transpose_notes")              return toolTransposeNotes(args);
    if (toolName == "set_note_duration")            return toolSetNoteDuration(args);
    if (toolName == "stretch_notes")                return toolStretchNotes(args);
    if (toolName == "duplicate_notes")              return toolDuplicateNotes(args);
    if (toolName == "set_time_signature")           return toolSetTimeSignature(args);
    if (toolName == "get_time_signature_changes")   return toolGetTimeSignatureChanges();
    if (toolName == "add_time_signature_change")    return toolAddTimeSignatureChange(args);
    if (toolName == "remove_time_signature_change") return toolRemoveTimeSignatureChange(args);
    if (toolName == "get_note_curves")       return toolGetNoteCurves(args);
    if (toolName == "set_note_curve")        return toolSetNoteCurve(args);
    if (toolName == "get_note_vibrato")      return toolGetNoteVibrato(args);
    if (toolName == "set_note_vibrato")      return toolSetNoteVibrato(args);
    if (toolName == "play_score")            return toolPlayScore();
    if (toolName == "stop_score")            return toolStopScore();
    if (toolName == "undo")                  return toolUndo();
    if (toolName == "redo")                  return toolRedo();
    if (toolName == "open_project")          return toolOpenProject(args);
    if (toolName == "save_project")          return toolSaveProject(args);
    if (toolName == "add_track")             return toolAddTrack(args);
    if (toolName == "rename_track")          return toolRenameTrack(args);
    if (toolName == "delete_track")          return toolDeleteTrack(args);
    if (toolName == "get_engine_settings")   return toolGetEngineSettings();
    if (toolName == "set_engine_settings")   return toolSetEngineSettings(args);
    if (toolName == "remove_connection")     return toolRemoveConnection(args);
    if (toolName == "seek")                  return toolSeek(args);
    if (toolName == "set_loop")              return toolSetLoop(args);
    if (toolName == "clear_loop")            return toolClearLoop();
    return error("Unknown tool: " + toolName);
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

    // Apply any extra parameter overrides from the args
    const QJsonObject params = args["params"].toObject();
    if (!params.isEmpty()) {
        newC->beginParameterUpdate();
        for (auto it = params.constBegin(); it != params.constEnd(); ++it)
            newC->setParameter(it.key(), it.value().toDouble());
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

    c->setParameter(param, args["value"].toDouble());
    // parameterChanged() signal fires automatically → rebuildGraph via existing connection
    return ok("Parameter set.");
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
    return ok(QString("Switched to %1. Canvas is now ready for editing.").arg(label));
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

QJsonObject KalaTools::toolSelectNotes(const QJsonObject &args)
{
    ScoreCanvas *sc = getScoreCanvas(m_scoreCanvasWindow);
    if (!sc) return error("Score canvas not available.");

    const QVector<Note> &notes = sc->getPhrase().getNotes();

    const bool hasMin  = args.contains("pitchMinHz");
    const bool hasMax  = args.contains("pitchMaxHz");
    const bool hasTrk  = args.contains("trackIndex");
    const double minHz = hasMin ? args["pitchMinHz"].toDouble()  : 0.0;
    const double maxHz = hasMax ? args["pitchMaxHz"].toDouble()  : 1e12;
    const int trackIdx = hasTrk ? args["trackIndex"].toInt()     : -1;

    if (hasMin && hasMax && minHz > maxHz)
        return error("pitchMinHz must be <= pitchMaxHz.");

    QVector<int> indices;
    QJsonArray ids;
    for (int i = 0; i < notes.size(); ++i) {
        const Note &n = notes[i];
        if (hasTrk && n.getTrackIndex() != trackIdx) continue;
        if (n.getPitchHz() < minHz || n.getPitchHz() > maxHz) continue;
        indices.append(i);
        ids.append(n.getId());
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
        new AddExpressiveCurveCommand(&sc->getPhrase(), indices, name, pts, weight, perNote, sc));
    return ok(QString("Expressive curve '%1' applied to %2 note(s).").arg(name).arg(indices.size()));
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

QJsonArray KalaTools::getToolSchemas()
{
    // All valid container type names
    const QJsonArray containerTypes = {
        "Harmonic Generator", "Spectrum to Signal", "Karplus Strong",
        "Signal Mixer", "Note Tail", "Attack", "Wavetable Synth",
        "Recorder", "Bowed", "Reed",
        "Rolloff Processor", "Spectrum Blender", "Formant Body",
        "Breath Turbulence", "Noise Color Filter",
        "Physics System", "Easing Applicator", "Envelope Engine",
        "Drift Engine", "LFO", "Frequency Mapper",
        "10-Band EQ", "Comb Filter", "LP/HP Filter", "IR Convolution"
    };

    // All valid connection functions
    const QJsonArray connectionFunctions = {
        "passthrough", "add", "subtract", "multiply", "replace", "modulate"
    };

    QJsonArray schemas;

    // ── get_graph_state ──────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_graph_state"},
            {"description",
             "Returns the current sounit graph: all containers (type, instance name, "
             "position, parameters) and all connections. Use this to inspect what has "
             "already been built before making changes."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{}},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── get_sounit_list ──────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_sounit_list"},
            {"description",
             "Returns a list of available .sounit files (name + path) from the user's "
             "sounit library. Use this before load_sounit to find the right file path."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{}},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── clear_graph ──────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "clear_graph"},
            {"description",
             "Removes all containers and connections from the current sounit graph. "
             "Use before building a new instrument from scratch."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{}},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── add_container ────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "add_container"},
            {"description",
             "Adds a new audio processing container to the graph. Returns the assigned "
             "instance name (e.g. 'Envelope Engine 2'). Default parameters are set "
             "automatically; use the params object to override specific values. "
             "Position is optional — containers stack automatically if omitted."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"type", QJsonObject{
                        {"type", "string"},
                        {"description", "Container type name. Must be one of the valid types."},
                        {"enum", containerTypes}
                    }},
                    {"params", QJsonObject{
                        {"type", "object"},
                        {"description",
                         "Optional parameter overrides. Keys are parameter names (e.g. "
                         "'numHarmonics', 'dnaSelect', 'amount', 'frequency'). Values are "
                         "numbers. Only specify parameters you want to change from defaults."}
                    }},
                    {"position", QJsonObject{
                        {"type", "object"},
                        {"description", "Optional {x, y} pixel position on the canvas."},
                        {"properties", QJsonObject{
                            {"x", numProp("Horizontal position in pixels")},
                            {"y", numProp("Vertical position in pixels")}
                        }}
                    }}
                }},
                {"required", QJsonArray{"type"}}
            }}
        }}
    });

    // ── set_parameter ────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_parameter"},
            {"description",
             "Sets a single parameter on an existing container. The audio graph "
             "rebuilds automatically. Use the instance name returned by add_container."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"instanceName", strProp("Instance name of the container (e.g. 'Drift Engine 1')")},
                    {"param",        strProp("Parameter name (e.g. 'amount', 'rate', 'dnaSelect')")},
                    {"value",        numProp("New numeric value for the parameter")}
                }},
                {"required", QJsonArray{"instanceName", "param", "value"}}
            }}
        }}
    });

    // ── connect_containers ───────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "connect_containers"},
            {"description",
             "Creates a connection between an output port of one container and an input "
             "port of another. Port names are validated — call get_graph_state first if "
             "unsure of instance names. Common patterns: "
             "spectrum→spectrumIn (spectral routing), "
             "signalOut→signalIn (audio signal), "
             "controlOut→param (modulation). "
             "function 'passthrough' for direct routing, 'add' for additive modulation, "
             "'multiply' for amplitude control, 'modulate' for bipolar LFO-style."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"fromInstance", strProp("Instance name of the source container")},
                    {"fromPort",     strProp("Output port name on the source container")},
                    {"toInstance",   strProp("Instance name of the destination container")},
                    {"toPort",       strProp("Input port name on the destination container")},
                    {"function",     QJsonObject{
                        {"type", "string"},
                        {"description", "How the source value combines with the destination. Default: passthrough"},
                        {"enum", connectionFunctions}
                    }},
                    {"weight",       numProp("Scaling factor for add/subtract/multiply/modulate. Default: 1.0")}
                }},
                {"required", QJsonArray{"fromInstance", "fromPort", "toInstance", "toPort"}}
            }}
        }}
    });

    // ── remove_connection ─────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "remove_connection"},
            {"description",
             "Removes a single connection between two containers without affecting the containers "
             "themselves. Use get_graph_state to see current connections and exact port/instance names. "
             "Supports undo."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"fromInstance", strProp("Instance name of the source container")},
                    {"fromPort",     strProp("Output port name on the source container")},
                    {"toInstance",   strProp("Instance name of the destination container")},
                    {"toPort",       strProp("Input port name on the destination container")}
                }},
                {"required", QJsonArray{"fromInstance", "fromPort", "toInstance", "toPort"}}
            }}
        }}
    });

    // ── remove_container ─────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "remove_container"},
            {"description", "Removes a container (and all its connections) from the graph."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"instanceName", strProp("Instance name of the container to remove")}
                }},
                {"required", QJsonArray{"instanceName"}}
            }}
        }}
    });

    // ── load_sounit ──────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "load_sounit"},
            {"description",
             "Loads a .sounit file into the current track, replacing the existing graph. "
             "Call get_sounit_list first to find the correct file path."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"filePath", strProp("Absolute path to the .sounit file to load")}
                }},
                {"required", QJsonArray{"filePath"}}
            }}
        }}
    });

    // ── save_sounit ──────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "save_sounit"},
            {"description", "Saves the current graph to a .sounit file."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"filePath", strProp("Absolute path where the .sounit file will be saved")},
                    {"name",     strProp("Optional display name to embed in the sounit metadata")}
                }},
                {"required", QJsonArray{"filePath"}}
            }}
        }}
    });

    // ── play_preview ─────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "play_preview"},
            {"description",
             "Triggers playback of the current track so the user can audition the "
             "graph that was just built. Call this after finishing graph construction."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{}},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── get_spectrum_list ─────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_spectrum_list"},
            {"description",
             "Returns all custom DNA spectrum files available in the user's spectrum library "
             "(~/Music/kala/spectrum). These are mathematical/organic harmonic profiles that "
             "can be loaded into a Harmonic Generator using dnaSelect=-1 and a dnaFilePath. "
             "Includes spectra like fibonacci, phi, prime numbers, vocal warm, brass, etc. "
             "Call this when the user asks about custom timbres or unusual harmonic content."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{}},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── get_project_list ──────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_project_list"},
            {"description",
             "Returns all Kala project files (.kala) in the user's projects directory. "
             "Each entry has a name and an absolute file path. "
             "Use this when the user asks what compositions exist or wants to reference "
             "a project by name."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{}},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── get_ir_list ───────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_ir_list"},
            {"description",
             "Returns all impulse response (.wav) files available in the user's IR library. "
             "These can be used with IR Convolution containers to place an instrument in a "
             "space. IMPORTANT: IR Convolution files cannot be set programmatically — the "
             "user must assign them manually in the IR Convolution inspector. Use this tool "
             "to tell the user which IR files are available so they can choose one."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{}},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── set_library_root ──────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_library_root"},
            {"description",
             "Sets the root folder of the user's Kala file library. All subsequent calls to "
             "get_sounit_list, get_spectrum_list, get_project_list, and get_ir_list will scan "
             "from this root. The path is saved permanently. "
             "Use this when the user tells you their files are in a different location than "
             "~/Music/kala (e.g. 'my kala files are on D:/Music/kala')."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"path", strProp("Absolute path to the root of the kala library folder "
                                    "(the folder that contains sounit/, spectrum/, projects/, etc.)")}
                }},
                {"required", QJsonArray{"path"}}
            }}
        }}
    });

    // ── load_ir ───────────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "load_ir"},
            {"description",
             "Loads a WAV impulse response file into an IR Convolution container. "
             "Call get_ir_list first to find the available file paths. "
             "The audio graph rebuilds automatically after loading. "
             "Workflow: add_container(IR Convolution) → connect it → load_ir with the path."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"instanceName", strProp("Instance name of the IR Convolution container "
                                            "(e.g. 'IR Convolution 1')")},
                    {"filePath",     strProp("Absolute path to the .wav IR file")}
                }},
                {"required", QJsonArray{"instanceName", "filePath"}}
            }}
        }}
    });

    // ── get_envelope_list ─────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_envelope_list"},
            {"description",
             "Returns all saved envelope shapes (.env.json) from the user's envelope library. "
             "Each entry has a name and absolute file path. "
             "Call this before load_envelope to find the right path."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{}},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── load_envelope ─────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "load_envelope"},
            {"description",
             "Loads a saved envelope shape (.env.json) into an Envelope Engine container. "
             "Call get_envelope_list first to find the file path. "
             "Sets envelopeSelect=7 (custom) and stores all control points. "
             "Workflow: add_container(Envelope Engine) → load_envelope."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"instanceName", strProp("Instance name of the Envelope Engine container "
                                            "(e.g. 'Envelope Engine 1')")},
                    {"filePath",     strProp("Absolute path to the .env.json file")}
                }},
                {"required", QJsonArray{"instanceName", "filePath"}}
            }}
        }}
    });

    // ── set_envelope_shape ────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_envelope_shape"},
            {"description",
             "Builds a custom envelope shape from scratch and applies it to an Envelope Engine. "
             "Each point has time (0.0–1.0), value (0.0–1.0), and optional curveType "
             "(0=Linear, 1=Smooth, 2=Step; default 1). "
             "The envelope maps to the note lifetime: time=0 is note start, time=1 is note end. "
             "First and last points are auto-clamped to time=0 and time=1 if not provided. "
             "loopMode: 0=None (default), 1=Loop, 2=Ping-pong. "
             "Examples: "
             "Attack-sustain-release: [{0,0},{0.05,1},{0.2,0.7},{1,0.7}]. "
             "Swell: [{0,0},{0.5,1},{1,0}]. "
             "Fade in: [{0,0},{1,1}]. "
             "Fade out: [{0,1},{1,0}]. "
             "Bell: [{0,0},{0.1,1},{0.4,0.3},{1,0}]."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"instanceName", strProp("Instance name of the Envelope Engine container")},
                    {"points", QJsonObject{
                        {"type", "array"},
                        {"description", "Array of control points. Each: {time, value, curveType (optional)}."},
                        {"items", QJsonObject{
                            {"type", "object"},
                            {"properties", QJsonObject{
                                {"time",      numProp("Normalised time 0.0–1.0")},
                                {"value",     numProp("Normalised value 0.0–1.0")},
                                {"curveType", numProp("0=Linear, 1=Smooth (default), 2=Step")}
                            }}
                        }}
                    }},
                    {"name",     strProp("Optional display name for the envelope")},
                    {"loopMode", numProp("0=None (default), 1=Loop, 2=Ping-pong")}
                }},
                {"required", QJsonArray{"instanceName", "points"}}
            }}
        }}
    });

    // ── load_spectrum ─────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "load_spectrum"},
            {"description",
             "Loads a custom DNA harmonic spectrum (.dna.json) into a Harmonic Generator "
             "container. Call get_spectrum_list first to find the file path. "
             "Sets dnaSelect=-1 and stores all amplitude values as numeric parameters. "
             "The spectrum name is preserved for display in the inspector. "
             "Workflow: add_container(Harmonic Generator) → load_spectrum with the HG "
             "instance name and the .dna.json file path."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"instanceName", strProp("Instance name of the Harmonic Generator container "
                                            "(e.g. 'Harmonic Generator 1')")},
                    {"filePath",     strProp("Absolute path to the .dna.json spectrum file")}
                }},
                {"required", QJsonArray{"instanceName", "filePath"}}
            }}
        }}
    });

    // ── get_composition_state ─────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_composition_state"},
            {"description",
             "Returns all notes currently on the score canvas, plus scale, tempo and "
             "time signature. Each note has: id, startTime (ms), duration (ms), pitchHz, "
             "dynamics (0–1), trackIndex. Optionally filter to a single track."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"trackIndex", QJsonObject{
                        {"type", "integer"},
                        {"description", "If provided, return only notes on this track index. Omit to return all notes."}
                    }}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── get_track_list ────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_track_list"},
            {"description",
             "Returns all tracks in the composition with their index, name, and loaded "
             "sounit name. Use this to find track indices before adding notes."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{}},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── add_note ─────────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "add_note"},
            {"description",
             "Adds a note to the score canvas. startTime and duration are in milliseconds. "
             "pitchHz is the fundamental frequency (e.g. A4=440, C4=261.63, G4=392, D5=587.33). "
             "dynamics is 0–1 (0.7 is a good default). trackIndex selects which instrument track "
             "plays the note. Returns the note's unique id. Supports full undo."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"startTime",  numProp("Note start time in milliseconds (>= 0)")},
                    {"duration",   numProp("Note duration in milliseconds (> 0)")},
                    {"pitchHz",    numProp("Fundamental pitch in Hz (e.g. 440 = A4)")},
                    {"dynamics",   numProp("Loudness 0.0–1.0 (default 0.7)")},
                    {"trackIndex", QJsonObject{
                        {"type", "integer"},
                        {"description", "Which track/instrument to assign the note to (0-based). "
                                        "Use get_track_list to find available tracks."}
                    }}
                }},
                {"required", QJsonArray{"startTime", "duration", "pitchHz"}}
            }}
        }}
    });

    // ── delete_note ──────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "delete_note"},
            {"description",
             "Deletes a single note by its id. Use get_composition_state to find note ids. "
             "Supports undo."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"id", strProp("Unique note id returned by add_note or get_composition_state")}
                }},
                {"required", QJsonArray{"id"}}
            }}
        }}
    });

    // ── clear_notes ───────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "clear_notes"},
            {"description",
             "Removes notes from the score canvas. Without trackIndex, removes ALL notes. "
             "With trackIndex, removes only notes on that track. Supports undo."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"trackIndex", QJsonObject{
                        {"type", "integer"},
                        {"description", "Optional. If provided, only clear notes on this track."}
                    }}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── set_scale ─────────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_scale"},
            {"description",
             "Sets the tuning scale and optionally the base frequency (tonic). "
             "Provide either scaleId (integer) or scaleName (string). "
             "Common scales and IDs: 0=Just Intonation, 1=Pythagorean, 2=Equal Temperament, "
             "3=Quarter-Comma Meantone, 4=Maqam Rast, 5=Maqam Bayati, 7=Maqam Hijaz, "
             "10=Raga Bhairav, 11=Raga Yaman, 13=Raga Bhairavi, 16=Chinese Pentatonic, "
             "17=Hirajoshi, 20=Whole Tone, 30=Persian Shur. "
             "baseFreqHz is the tonic frequency (default 261.63 = C4)."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"scaleId",    QJsonObject{{"type","integer"},{"description","Scale ID (see description)"}}},
                    {"scaleName",  strProp("Scale name (e.g. 'Just Intonation', 'Raga Yaman')")},
                    {"baseFreqHz", numProp("Tonic/base frequency in Hz (e.g. 261.63 for C4, 293.66 for D4)")}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── set_tempo ─────────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_tempo"},
            {"description",
             "Sets the default composition tempo in beats per minute (BPM). "
             "Valid range: 20–400. This controls bar lines and musical time display. "
             "Typical values: 60=Largo, 80=Andante, 100=Moderato, 120=Allegro, 160=Vivace."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"bpm", numProp("Tempo in beats per minute (20–400)")}
                }},
                {"required", QJsonArray{"bpm"}}
            }}
        }}
    });

    // ── get_variation_list ────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_variation_list"},
            {"description",
             "Returns all sound variations available on the current track. "
             "Index 0 is the base sounit; higher indices are named variations "
             "(e.g. 'Wood Room', 'Plate', 'Cantabile'). "
             "Call this before apply_variation to discover valid variation indices."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{}},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── apply_variation ───────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "apply_variation"},
            {"description",
             "Assigns a sound variation to one or more notes on the score canvas. "
             "Call get_variation_list first to find the variation index. "
             "Call get_composition_state first to find note ids. "
             "Leave noteIds empty to apply to all notes on the current track. "
             "Variation 0 = base sounit, 1+ = named variations."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"variationIndex", QJsonObject{
                        {"type", "integer"},
                        {"description", "Variation index to assign (0=base, 1+=named variations)"}
                    }},
                    {"noteIds", QJsonObject{
                        {"type", "array"},
                        {"description", "Array of note id strings to target. Empty = all notes on current track."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{"variationIndex"}}
            }}
        }}
    });

    // ── create_variation ──────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "create_variation"},
            {"description",
             "Snapshots the current canvas state as a named variation on the current track. "
             "Use this after building or modifying a sounit to save it as a named alternative "
             "(e.g. 'Wood Room', 'Bright Plate'). The base sounit on the canvas is unchanged. "
             "Returns the variation index for use with apply_variation."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"name", strProp("Name for the new variation (e.g. 'Wood Room', 'Bright Plate')")}
                }},
                {"required", QJsonArray{"name"}}
            }}
        }}
    });

    // ── create_variation_from_sounit ──────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "create_variation_from_sounit"},
            {"description",
             "Loads a .sounit file as a named variation without touching the current canvas. "
             "Use get_sounit_list first to find the file path. "
             "Returns the variation index for use with apply_variation."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"filePath", strProp("Absolute path to the .sounit file")},
                    {"name",     strProp("Name for the new variation")}
                }},
                {"required", QJsonArray{"filePath", "name"}}
            }}
        }}
    });

    // ── delete_variation ──────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "delete_variation"},
            {"description",
             "Deletes a variation from the current track by index. "
             "Call get_variation_list first to find the index. "
             "Index 0 (base sounit) cannot be deleted."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"variationIndex", QJsonObject{
                        {"type", "integer"},
                        {"description", "1-based variation index to delete"}
                    }}
                }},
                {"required", QJsonArray{"variationIndex"}}
            }}
        }}
    });

    // ── rename_variation ──────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "rename_variation"},
            {"description", "Renames an existing variation on the current track."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"variationIndex", QJsonObject{
                        {"type", "integer"},
                        {"description", "1-based variation index to rename"}
                    }},
                    {"name", strProp("New name for the variation")}
                }},
                {"required", QJsonArray{"variationIndex", "name"}}
            }}
        }}
    });

    // ── switch_variation ──────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "switch_variation"},
            {"description",
             "Loads a variation (or the base sounit) onto the canvas so it can be edited. "
             "Use index 0 to restore the base sounit. "
             "After switching, the sounit builder shows the variation's graph and all "
             "add_container / set_parameter / connect_containers calls will modify it. "
             "Call create_variation to save edits as a new snapshot when done."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"variationIndex", QJsonObject{
                        {"type", "integer"},
                        {"description", "Variation index to load (0 = base sounit, 1+ = named variations)"}
                    }}
                }},
                {"required", QJsonArray{"variationIndex"}}
            }}
        }}
    });

    // ── apply_dynamics_curve ──────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "apply_dynamics_curve"},
            {"description",
             "Applies a dynamics (loudness) envelope curve to a set of notes. "
             "The curve is defined by control points {time 0–1, value 0–1, curveType}. "
             "time=0 maps to the first note, time=1 maps to the last note. "
             "value=1 keeps the note's original dynamics; value=0 silences it. "
             "weight controls blend between original and target dynamics (1.0=full replace). "
             "perNote=true resets time to 0–1 within each note independently. "
             "Supports undo. Leave noteIds empty for all notes."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"points", QJsonObject{
                        {"type", "array"},
                        {"description", "Dynamics curve control points. Each: {time 0–1, value 0–1, curveType 0/1/2}."},
                        {"items", QJsonObject{
                            {"type","object"},
                            {"properties", QJsonObject{
                                {"time",      numProp("0.0–1.0 position across the note selection")},
                                {"value",     numProp("0.0–1.0 dynamics multiplier (1.0 = unchanged)")},
                                {"curveType", numProp("0=Linear, 1=Smooth, 2=Step")}
                            }}
                        }}
                    }},
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids to target. Empty = all notes."},
                        {"items", QJsonObject{{"type","string"}}}
                    }},
                    {"weight",  numProp("Blend factor 0–1 (default 1.0 = full apply)")},
                    {"perNote", QJsonObject{{"type","boolean"},{"description","If true, apply the curve independently to each note's internal timeline"}}}
                }},
                {"required", QJsonArray{"points"}}
            }}
        }}
    });

    // ── scale_timing ──────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "scale_timing"},
            {"description",
             "Scales the timing (start times and durations) of notes by a factor. "
             "proportion=2.0 doubles all timings (half tempo). "
             "proportion=0.5 halves them (double tempo). "
             "Supports undo. Leave noteIds empty for all notes."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"proportion", numProp("Timing scale factor (e.g. 1.5 = 50% slower, 0.5 = 50% faster)")},
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids to target. Empty = all notes."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{"proportion"}}
            }}
        }}
    });

    // ── apply_rhythm_easing ───────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "apply_rhythm_easing"},
            {"description",
             "Redistributes the temporal spacing between notes using an easing curve. "
             "Makes note onsets follow an acceleration/deceleration pattern instead of "
             "uniform timing. Minimum 2 notes required. Supports undo. "
             "easingType: 0=Linear, 1=QuadIn, 2=QuadOut, 3=QuadInOut, 4=CubicIn, 5=CubicOut, "
             "6=CubicInOut, 7=QuartIn, 8=QuartOut, 9=QuartInOut, 10=SineIn, 11=SineOut, "
             "12=SineInOut, 13=ExpoIn, 14=ExpoOut, 15=ExpoInOut, 16=CircIn, 17=CircOut, "
             "18=CircInOut, 19=BackIn, 20=BackOut, 21=BackInOut, 22=ElasticIn, 23=ElasticOut, "
             "24=ElasticInOut, 25=BounceIn, 26=BounceOut, 27=BounceInOut, 28=Wobble. "
             "anchorMode: 0=None, 1=AnchorFirst, 2=AnchorLast, 3=AnchorBoth (default). "
             "Leave noteIds empty for all notes."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"easingType", QJsonObject{{"type","integer"},{"description","Easing curve type (0=Linear default; see description for full list)"}}},
                    {"anchorMode", QJsonObject{{"type","integer"},{"description","0=None, 1=AnchorFirst, 2=AnchorLast, 3=AnchorBoth (default)"}}},
                    {"weight",     numProp("Blend 0–1 (default 1.0 = full easing)")},
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids to target. Empty = all notes."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── link_legato ───────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "link_legato"},
            {"description",
             "Links two or more notes as a legato group so they play connected without "
             "retriggering the envelope between them. Each note sustains into the next. "
             "Requires at least 2 notes. Supports undo."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids to link (at least 2 required)."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{"noteIds"}}
            }}
        }}
    });

    // ── unlink_legato ─────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "unlink_legato"},
            {"description",
             "Removes legato linking from notes, making them discrete/staccato again. "
             "Each note will retrigger its envelope independently. Supports undo. "
             "Leave noteIds empty to unlink all notes."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids to unlink. Empty = all notes."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── quantize_to_scale ─────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "quantize_to_scale"},
            {"description",
             "Snaps the pitch of selected notes to the nearest degree in the current scale. "
             "Useful after placing notes by frequency to make them scale-correct. "
             "Leave noteIds empty to quantize all notes."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids to quantize. Empty = all notes."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── make_continuous ───────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "make_continuous"},
            {"description",
             "Converts notes to continuous mode: each note extends in time until the next "
             "note starts, creating a flowing glide-like texture. "
             "Leave noteIds empty to convert all notes."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids to convert. Empty = all notes."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── make_discrete ─────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "make_discrete"},
            {"description",
             "Converts notes to discrete mode: each note plays for exactly its stated duration "
             "and stops. The opposite of make_continuous. "
             "Leave noteIds empty to convert all notes."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids to convert. Empty = all notes."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── set_note_dynamics ─────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_note_dynamics"},
            {"description",
             "Sets an individual dynamics value (0.0–1.0) for each specified note. "
             "noteIds and values must be the same length — each value maps to the note at the same position. "
             "Use this for per-note variation: call get_composition_state to get note IDs and start times, "
             "compute a dynamics value for each note (e.g. beat-pattern base + small random offset), "
             "then call this tool once with all IDs and values. "
             "The entire operation is a single undoable command (Ctrl+Z restores all previous dynamics)."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note IDs to set dynamics on (from get_composition_state)."},
                        {"items", QJsonObject{{"type","string"}}}
                    }},
                    {"values", QJsonObject{
                        {"type","array"},
                        {"description","Dynamics value (0.0–1.0) for each note, in the same order as noteIds."},
                        {"items", QJsonObject{{"type","number"}}}
                    }}
                }},
                {"required", QJsonArray{"noteIds","values"}}
            }}
        }}
    });

    // ── apply_beat_dynamics ───────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "apply_beat_dynamics"},
            {"description",
             "Applies a per-beat accent pattern to notes based on their position within each measure. "
             "The pattern is an array of dynamics values (0.0–1.0) indexed by beat (beat 1 = index 0). "
             "For example, a 5/4 accent pattern [0.9, 0.4, 0.7, 0.5, 0.35] sets beat 1 strong, "
             "beat 3 medium, beat 4 light, beats 2 and 5 lighter. "
             "If the pattern is shorter than the time signature numerator it wraps. "
             "The time signature and tempo are read from the score canvas at each note's position "
             "so mid-piece time signature changes are handled correctly. "
             "Leave noteIds empty to apply to all notes. "
             "Fully undoable with Ctrl+Z."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"pattern", QJsonObject{
                        {"type", "array"},
                        {"description", "Dynamics value (0.0–1.0) for each beat, starting from beat 1 (index 0)."},
                        {"items", QJsonObject{{"type","number"}}}
                    }},
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids to target. Empty = all notes on the canvas."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{"pattern"}}
            }}
        }}
    });

    // ── get_selected_notes ────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_selected_notes"},
            {"description",
             "Returns the IDs of the notes currently selected on the score canvas (by the user). "
             "Use this when the user says 'work on the selected notes', 'apply this to my selection', etc. "
             "The returned noteIds can be passed directly to shift_notes, apply_variation, "
             "apply_dynamics_curve, delete_note, link_legato, and any other tool that accepts noteIds."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{}},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── select_notes ─────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "select_notes"},
            {"description",
             "Selects notes on the score canvas by pitch range and/or track. "
             "At least one filter (pitchMinHz, pitchMaxHz, or trackIndex) should be provided, "
             "otherwise all notes are selected. "
             "The matched notes are visually highlighted on the canvas AND their IDs are returned "
             "so they can be passed directly to shift_notes, apply_variation, apply_dynamics_curve, etc."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"pitchMinHz",  numProp("Select only notes at or above this frequency (Hz). Omit for no lower bound.")},
                    {"pitchMaxHz",  numProp("Select only notes at or below this frequency (Hz). Omit for no upper bound.")},
                    {"trackIndex",  QJsonObject{{"type","integer"},{"description","If set, only consider notes on this track index."}}}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── shift_notes ───────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "shift_notes"},
            {"description",
             "Shifts the start time of notes by a given offset in milliseconds. "
             "Positive offsetMs = later, negative = earlier. "
             "Optionally filter by pitch using pitchHz + pitchToleranceCents. "
             "Leave noteIds empty to target all notes (then pitch filter applies to all). "
             "All shifts are pushed as a single undoable command (Ctrl+Z reverses all at once)."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"offsetMs", numProp("Time offset in milliseconds (positive = later, negative = earlier)")},
                    {"pitchHz",  numProp("If set, only shift notes whose pitch is within pitchToleranceCents of this frequency")},
                    {"pitchToleranceCents", numProp("Tolerance for pitch matching in cents (default 50). Only used when pitchHz is set.")},
                    {"noteIds",  QJsonObject{
                        {"type","array"},
                        {"description","Explicit note ids to target. Empty = all notes (pitch filter still applies if pitchHz is set)."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{"offsetMs"}}
            }}
        }}
    });

    // ── get_note_vibrato ──────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_note_vibrato"},
            {"description",
             "Returns the vibrato settings on a note: active flag, rate (Hz), pitchDepth, "
             "amplitudeDepth, onset, regularity, and the intensity envelope. "
             "Inspects the first note from noteIds (or first note overall if empty). "
             "Use before set_note_vibrato to see current settings."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids — first one is inspected. Empty = first note."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── set_note_vibrato ──────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_note_vibrato"},
            {"description",
             "Applies vibrato settings to one or more notes. All parameters are optional — "
             "unspecified ones are preserved from the first targeted note's current settings. "
             "Parameters:\n"
             "  active (bool): true = enable vibrato.\n"
             "  rate (Hz): oscillation speed, typical 3–8 Hz. Default 5.\n"
             "  pitchDepth (0–1): pitch variation fraction. 0.02 = 2% = subtle, 0.06 = expressive. Default 0.02.\n"
             "  amplitudeDepth (0–1): loudness variation depth. 0 = pitch-only, 0.3 = natural. Default 0.3.\n"
             "  onset (0–1): when vibrato starts (fraction into note). 0.2 = after 20% of note. Default 0.2.\n"
             "  regularity (0–1): 0 = mechanical/metronome, 1 = organic/human. Default 0.5.\n"
             "  envelope: [{time, value, curveType}] — shape of vibrato intensity over note lifetime. "
             "Default is a linear rise from 0 to 1. Example delayed-then-full: [{0,0,1},{0.3,0,1},{0.6,1,1},{1,1,1}].\n"
             "Leave noteIds empty to apply to all notes. Supports undo."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"active",         QJsonObject{{"type","boolean"},{"description","Enable or disable vibrato."}}},
                    {"rate",           numProp("Oscillation speed in Hz (3–8 typical, default 5).")},
                    {"pitchDepth",     numProp("Pitch variation as fraction (0.02 = subtle, 0.06 = expressive).")},
                    {"amplitudeDepth", numProp("Loudness variation 0–1 (0 = pitch-only, 0.3 = natural).")},
                    {"onset",          numProp("Fraction into the note when vibrato starts (0–1, default 0.2).")},
                    {"regularity",     numProp("0 = mechanical, 1 = organic (default 0.5).")},
                    {"envelope", QJsonObject{
                        {"type","array"},
                        {"description","Vibrato intensity curve over note lifetime: [{time 0–1, value 0–1, curveType 0/1/2}]."},
                        {"items", QJsonObject{{"type","object"}}}
                    }},
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids to target. Empty = all notes."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── undo ─────────────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "undo"},
            {"description",
             "Undoes the last action on the score canvas (equivalent to Ctrl+Z). "
             "Use this to recover from mistakes made by previous tool calls. "
             "Returns the name of the undone action, or an error if nothing to undo."},
            {"parameters", QJsonObject{{"type","object"},{"properties",QJsonObject{}},{"required",QJsonArray{}}}}
        }}
    });

    // ── redo ─────────────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "redo"},
            {"description",
             "Redoes the last undone action on the score canvas (equivalent to Ctrl+Y). "
             "Returns the name of the redone action, or an error if nothing to redo."},
            {"parameters", QJsonObject{{"type","object"},{"properties",QJsonObject{}},{"required",QJsonArray{}}}}
        }}
    });

    // ── play_score ────────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "play_score"},
            {"description",
             "Starts playback of the full score canvas composition from the current now-marker position. "
             "Use after composing or modifying notes to let the user hear the result."},
            {"parameters", QJsonObject{{"type","object"},{"properties",QJsonObject{}},{"required",QJsonArray{}}}}
        }}
    });

    // ── stop_score ────────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "stop_score"},
            {"description", "Stops score canvas playback."},
            {"parameters", QJsonObject{{"type","object"},{"properties",QJsonObject{}},{"required",QJsonArray{}}}}
        }}
    });

    // ── seek ──────────────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "seek"},
            {"description",
             "Moves the now-marker (playback position) to a specific time. "
             "Provide either 'timeMs' (milliseconds from start) or 'barsAndBeats' ({bar, beat}) "
             "for musical positioning. bar and beat are 1-based. "
             "Call play_score after seek to start playback from that position."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"timeMs", numProp("Absolute position in milliseconds from the start.")},
                    {"barsAndBeats", QJsonObject{
                        {"type", "object"},
                        {"description", "Musical position. bar and beat are 1-based integers."},
                        {"properties", QJsonObject{
                            {"bar",  QJsonObject{{"type","integer"},{"description","Bar number (1-based)."}}},
                            {"beat", QJsonObject{{"type","integer"},{"description","Beat within the bar (1-based, default 1)."}}}
                        }}
                    }}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── set_loop ──────────────────────────────────────────────────────────────
    {
        auto bbProp = [](const QString &label) {
            return QJsonObject{
                {"type", "object"},
                {"description", label + " as musical position. bar and beat are 1-based integers."},
                {"properties", QJsonObject{
                    {"bar",  QJsonObject{{"type","integer"},{"description","Bar number (1-based)."}}},
                    {"beat", QJsonObject{{"type","integer"},{"description","Beat within the bar (1-based, default 1)."}}}
                }}
            };
        };
        schemas.append(QJsonObject{
            {"type", "function"},
            {"function", QJsonObject{
                {"name", "set_loop"},
                {"description",
                 "Sets a loop region on the timeline and seeks to the loop start. "
                 "Playback will cycle between start and end when play_score is called. "
                 "For start/end use either the Ms suffix keys (startMs/endMs) or the "
                 "musical position keys (start/end with {bar, beat}). "
                 "Example — loop bars 3–5: "
                 "{\"start\": {\"bar\":3,\"beat\":1}, \"end\": {\"bar\":5,\"beat\":1}}"},
                {"parameters", QJsonObject{
                    {"type", "object"},
                    {"properties", QJsonObject{
                        {"startMs", numProp("Loop start in milliseconds.")},
                        {"endMs",   numProp("Loop end in milliseconds.")},
                        {"start",   bbProp("Loop start")},
                        {"end",     bbProp("Loop end")}
                    }},
                    {"required", QJsonArray{}}
                }}
            }}
        });
    }

    // ── clear_loop ────────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "clear_loop"},
            {"description", "Removes the loop region from the timeline. Playback will run to the end of the piece."},
            {"parameters", QJsonObject{{"type","object"},{"properties",QJsonObject{}},{"required",QJsonArray{}}}}
        }}
    });

    // ── transpose_notes ───────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "transpose_notes"},
            {"description",
             "Transposes note pitches either by a frequency ratio or by a number of scale degrees. "
             "Use ratio for interval transposition: 2.0 = octave up, 0.5 = octave down, "
             "1.5 = perfect fifth up (JI), 4.0/3.0 = perfect fourth up (JI). "
             "Use scaleDegrees for scale-step transposition: +1 = one step up in the current scale, "
             "-3 = three steps down. Scale degree walking respects the scale active at each note's time position. "
             "Pitch curves (glissandi) are transposed together with the note. "
             "Leave noteIds empty to transpose all notes. Supports undo."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"ratio",        numProp("Pitch multiplier (e.g. 2.0 = octave up, 0.5 = octave down). Use instead of scaleDegrees.")},
                    {"scaleDegrees", QJsonObject{{"type","integer"},{"description","Steps to walk in the current scale (+up, -down). Use instead of ratio."}}},
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids to transpose. Empty = all notes."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── set_note_duration ─────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_note_duration"},
            {"description",
             "Changes the duration of one or more notes. "
             "Provide 'duration' (single value in ms) to set all targeted notes to the same length. "
             "Provide 'values' (array of ms values, same length as noteIds) for per-note control. "
             "Leave noteIds empty to target all notes. Supports undo."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"duration", numProp("Duration in ms to apply to all targeted notes.")},
                    {"values", QJsonObject{
                        {"type","array"},
                        {"description","Per-note durations in ms (must match noteIds length)."},
                        {"items", QJsonObject{{"type","number"}}}
                    }},
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids to target. Empty = all notes."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── duplicate_notes ───────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "duplicate_notes"},
            {"description",
             "Duplicates a set of notes and places the copies at a new time position. "
             "'offsetMs' is relative to the start of the source phrase (earliest note start time). "
             "Example: if source phrase starts at 2000ms and offsetMs=4000, copies start at 6000ms. "
             "To place copies immediately after the source, set offsetMs = phrase duration. "
             "Optional 'pitchRatio' transposes the copies (2.0 = octave up, 0.5 = octave down). "
             "Optional 'trackIndex' sends copies to a different track. "
             "Returns the new note IDs so they can immediately be piped into scale_timing, "
             "apply_rhythm_easing, transpose_notes, etc. Supports undo."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"offsetMs",    numProp("Time of first copy = source_start + offsetMs. Use phrase duration to place copies directly after.")},
                    {"pitchRatio",  numProp("Optional pitch multiplier for copies (e.g. 2.0 = octave up). Default 1.0 = same pitch.")},
                    {"trackIndex",  QJsonObject{{"type","integer"},{"description","Optional target track for copies. Default = current track."}}},
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids to duplicate. Empty = all notes on current track."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{"offsetMs"}}
            }}
        }}
    });

    // ── get_note_curves ───────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_note_curves"},
            {"description",
             "Returns all expressive curves on a note (dynamics plus any named curves like "
             "'Brightness', 'Breathiness', etc.). "
             "Inspects the first note from the noteIds list (or the first note overall if empty). "
             "Returns curve names, point counts, and point data (time 0–1, value 0–1). "
             "Use before set_note_curve to see what curves already exist."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids — first one is inspected. Empty = inspect first note."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── set_note_curve ────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_note_curve"},
            {"description",
             "Sets a named expressive curve on one or more notes. "
             "'name' can be 'Dynamics' (default) or any named expressive curve such as "
             "'Brightness', 'Breathiness', 'Resonance', 'Bow Pressure', etc. "
             "For non-Dynamics curves, the curve modulates the parameter routed to that curve name "
             "in the synthesis graph. "
             "Points use time 0–1 (note lifetime) and value 0–1. "
             "'weight' scales the applied curve (1.0 = full, 0.5 = half). "
             "'perNote' true = curve is applied within each note's own lifetime. "
             "Leave noteIds empty to apply to all notes. Supports undo."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"name",    strProp("Curve name: 'Dynamics' (default) or a named expressive curve.")},
                    {"points",  QJsonObject{
                        {"type","array"},
                        {"description","Curve points: [{time: 0–1, value: 0–1, curveType: 0=Linear/1=Smooth/2=Step}]. "
                                       "Common shapes: Swell [{0,0},{0.5,1},{1,0}], Fade-in [{0,0},{1,1}], "
                                       "Bell [{0,0},{0.1,1},{0.4,0.3},{1,0}]."},
                        {"items", QJsonObject{{"type","object"}}}
                    }},
                    {"weight",  numProp("Blend weight 0–1 (default 1.0 = full curve).")},
                    {"perNote", QJsonObject{{"type","boolean"},{"description","If true, curve maps to each note's own lifetime. Default false."}}},
                    {"noteIds", QJsonObject{
                        {"type","array"},
                        {"description","Note ids to apply to. Empty = all notes."},
                        {"items", QJsonObject{{"type","string"}}}
                    }}
                }},
                {"required", QJsonArray{"points"}}
            }}
        }}
    });

    // ── open_project ──────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "open_project"},
            {"description",
             "Opens a .kala project file, replacing the current session. "
             "Use get_project_list first to find available project file paths. "
             "Warning: unsaved changes to the current project will be lost."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"filePath", strProp("Absolute path to the .kala file to open.")}
                }},
                {"required", QJsonArray{"filePath"}}
            }}
        }}
    });

    // ── save_project ──────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "save_project"},
            {"description",
             "Saves the current project. If filePath is omitted, saves to the current project path. "
             "If the project has never been saved, filePath is required. "
             "Use after making significant changes to preserve the user's work."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"filePath", strProp("Optional absolute path to save to. Omit to save to current path.")}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── add_track ─────────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "add_track"},
            {"description",
             "Adds a new instrument track to the project. "
             "Returns the new track's index so you can immediately load a sounit onto it. "
             "Each track is independent and holds its own sounit + notes."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"name",  strProp("Track display name (e.g. 'Violin', 'Cello').")},
                    {"color", strProp("Optional track color as hex string (e.g. '#3498db').")}
                }},
                {"required", QJsonArray{"name"}}
            }}
        }}
    });

    // ── rename_track ──────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "rename_track"},
            {"description", "Renames an existing track. Use get_track_list to find track indices."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"trackIndex", QJsonObject{{"type","integer"},{"description","0-based track index."}}},
                    {"name",       strProp("New track name.")}
                }},
                {"required", QJsonArray{"trackIndex","name"}}
            }}
        }}
    });

    // ── delete_track ──────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "delete_track"},
            {"description",
             "Deletes a track and all its notes. Cannot delete the last remaining track. "
             "Track indices above the deleted one shift down by one. "
             "This action is not undoable — use with care."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"trackIndex", QJsonObject{{"type","integer"},{"description","0-based index of the track to delete."}}}
                }},
                {"required", QJsonArray{"trackIndex"}}
            }}
        }}
    });

    // ── set_note_pitch ────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_note_pitch"},
            {"description",
             "Sets the pitch of one or more notes by absolute frequency (Hz). "
             "Use 'noteIds' to target specific notes; omit to target all notes in the phrase. "
             "Provide either a single 'pitchHz' applied to all, or a 'values' array with one Hz per note. "
             "Any attached glissando/pitch curves are preserved (scaled proportionally). "
             "Push-undoable."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"noteIds",  QJsonObject{{"type","array"},{"description","Optional array of note ID strings to target. Omit to affect all notes."}}},
                    {"pitchHz",  QJsonObject{{"type","number"},{"description","Target pitch in Hz, applied to every targeted note."}}},
                    {"values",   QJsonObject{{"type","array"},{"description","Array of Hz values, one per targeted note (parallel to noteIds / note order)."}}}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── stretch_notes ─────────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "stretch_notes"},
            {"description",
             "Multiplies the duration of targeted notes by a factor without moving their start times. "
             "factor > 1 makes notes longer; factor < 1 makes them shorter. "
             "Use 'noteIds' to target specific notes; omit for all notes. Push-undoable."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"factor",  QJsonObject{{"type","number"},{"description","Duration multiplier (e.g. 2.0 = double length, 0.5 = halve)."}}},
                    {"noteIds", QJsonObject{{"type","array"},{"description","Optional array of note ID strings to stretch. Omit to affect all notes."}}}
                }},
                {"required", QJsonArray{"factor"}}
            }}
        }}
    });

    // ── set_time_signature ────────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_time_signature"},
            {"description",
             "Sets the default (opening) tempo and time signature of the score — the marker at time 0. "
             "All parameters are optional; omitted values keep their current setting. "
             "Push-undoable."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"bpm",          QJsonObject{{"type","number"},{"description","Tempo in beats per minute."}}},
                    {"numerator",    QJsonObject{{"type","integer"},{"description","Time signature numerator (e.g. 3 for 3/4)."}}},
                    {"denominator",  QJsonObject{{"type","integer"},{"description","Time signature denominator (e.g. 4 for 3/4)."}}},
                    {"gradual",      QJsonObject{{"type","boolean"},{"description","If true the transition to this tempo is gradual rather than a hard cut."}}}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── get_time_signature_changes ────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_time_signature_changes"},
            {"description",
             "Returns all tempo/time-signature markers in the score, including the default marker at time 0. "
             "Each entry has: timeMs, bpm, numerator, denominator, gradual."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{}},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── add_time_signature_change ─────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "add_time_signature_change"},
            {"description",
             "Inserts (or replaces) a tempo/time-signature change at a specific time position. "
             "timeMs must be > 0 (use set_time_signature for the opening marker). "
             "If 'bpm' is omitted it inherits the current bpm at that point. "
             "Push-undoable."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"timeMs",      QJsonObject{{"type","number"},{"description","Position in milliseconds (must be > 0)."}}},
                    {"bpm",         QJsonObject{{"type","number"},{"description","Tempo at this marker. Omit to inherit."}}},
                    {"numerator",   QJsonObject{{"type","integer"},{"description","Time signature numerator."}}},
                    {"denominator", QJsonObject{{"type","integer"},{"description","Time signature denominator."}}},
                    {"gradual",     QJsonObject{{"type","boolean"},{"description","Gradual tempo transition."}}}
                }},
                {"required", QJsonArray{"timeMs","numerator","denominator"}}
            }}
        }}
    });

    // ── remove_time_signature_change ──────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "remove_time_signature_change"},
            {"description",
             "Removes the tempo/time-signature change marker at the given time position. "
             "timeMs must be > 0 (the default marker at 0 cannot be removed). "
             "Returns an error if no marker exists at that position. Push-undoable."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"timeMs", QJsonObject{{"type","number"},{"description","Position in milliseconds of the marker to remove."}}}
                }},
                {"required", QJsonArray{"timeMs"}}
            }}
        }}
    });

    // ── get_engine_settings ───────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "get_engine_settings"},
            {"description",
             "Returns all Sound Engine Settings: the container parameter ranges (min/max for every "
             "container type) and the current project settings (name, sampleRate, bitDepth, lengthMs). "
             "Call this before set_engine_settings to inspect current values and exact key names. "
             "The result has two top-level keys: 'containerSettings' (nested by container group) "
             "and 'project'."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{}},
                {"required", QJsonArray{}}
            }}
        }}
    });

    // ── set_engine_settings ───────────────────────────────────────────────────
    schemas.append(QJsonObject{
        {"type", "function"},
        {"function", QJsonObject{
            {"name", "set_engine_settings"},
            {"description",
             "Updates Sound Engine Settings. Only the groups/fields you provide are changed — "
             "everything else is preserved. "
             "'containerSettings' is a nested partial object matching the structure from get_engine_settings. "
             "Container groups: harmonicGenerator, rolloffProcessor, spectrumToSignal, formantBody, "
             "breathTurbulence, noiseColorFilter, physicsSystem, driftEngine, gateProcessor, "
             "karplusStrong, attack, lfo, wavetableSynth, frequencyMapper, bandpassEQ, combFilter, "
             "lowHighPassFilter, irConvolution, recorder, bowed, reed, easing. "
             "Each group contains Min/Max (and sometimes Default) fields for the slider ranges shown "
             "in the inspector. "
             "'project' can contain: compositionName (string), sampleRate (int: 44100/48000/96000/192000), "
             "bitDepth (int: 16/24/32), lengthMs (number). "
             "All changes are saved immediately to QSettings."},
            {"parameters", QJsonObject{
                {"type", "object"},
                {"properties", QJsonObject{
                    {"containerSettings", QJsonObject{
                        {"type", "object"},
                        {"description",
                         "Partial container settings. Include only the groups/fields to change. "
                         "Example: {\"driftEngine\": {\"amountMax\": 0.3}} raises the drift slider ceiling."}
                    }},
                    {"project", QJsonObject{
                        {"type", "object"},
                        {"description",
                         "Partial project settings. Fields: compositionName, sampleRate, bitDepth, lengthMs."}
                    }}
                }},
                {"required", QJsonArray{}}
            }}
        }}
    });

    return schemas;
}

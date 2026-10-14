#include "track.h"
#include "audioengine.h"
#include "vibrato.h"
#include "dr_wav.h"
#include <QDebug>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QtConcurrent>
#include <QElapsedTimer>
#include <algorithm>
#include <utility>
#include <QUuid>
#include <QHash>

// ========== Construction / Destruction ==========

Track::Track(int trackId, const QString &name, const QColor &color, QObject *parent)
    : QObject(parent)
    , m_trackId(trackId)
    , m_name(name)
    , m_color(color)
    , m_canvas(nullptr)
    , m_graph(nullptr)
    , m_sounitName("Untitled Sounit")
    , m_sounitComment("")
    , m_sounitFilePath("")
    , m_sounitDirty(false)
    , m_loadingInProgress(false)
    , m_sampleRate(44100.0)
    , m_notesDirty(false)
    , m_renderDirty(true)
    , m_segmentDurationMs(1000.0)
    , m_graphHash(0)
    , m_muted(false)
    , m_solo(false)
    , m_volume(1.0f)
    , m_gain(1.0f)
    , m_pan(0.0f)
{
    // Create this track's own Canvas instance
    m_canvas = new Canvas(nullptr);  // Parent will be set when added to UI
    m_canvas->hide();  // Start hidden

    // Connect canvas signals
    connectCanvasSignals();

    // Set up watcher for non-blocking background renders
    m_renderWatcher = new QFutureWatcher<void>(this);
    connect(m_renderWatcher, &QFutureWatcher<void>::finished,
            this, &Track::onRenderWatcherFinished);
    connect(m_renderWatcher, &QFutureWatcher<void>::progressValueChanged,
            this, [this](int value) {
        if (m_backgroundTasks.isEmpty()) return;
        int pct = static_cast<int>((double)value / m_backgroundTasks.size() * 100.0);
        emit renderProgressChanged(pct);
    });

    qDebug() << "Track created: ID" << m_trackId << "Name:" << m_name;
}

Track::~Track()
{
    qDebug() << "Track destroyed: ID" << m_trackId << "Name:" << m_name;

    // Cancel any in-flight background render and wait for it to finish,
    // otherwise the QtConcurrent lambda captures 'this' and will crash.
    if (m_rendering.load()) {
        m_cancelRender.store(true);
        if (m_renderWatcher && m_renderWatcher->isRunning()) {
            m_renderWatcher->waitForFinished();
        }
        m_rendering.store(false);
    }

    // Clean up render cache
    clearRenderCache();

    // Delete graph
    if (m_graph) {
        delete m_graph;
        m_graph = nullptr;
    }

    // Delete canvas (which deletes all its child containers)
    if (m_canvas) {
        delete m_canvas;
        m_canvas = nullptr;
    }

    // Clean up variations
    qDeleteAll(m_variations);
    m_variations.clear();
}

// ========== Track Metadata ==========

void Track::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged(m_name);
        qDebug() << "Track" << m_trackId << "renamed to:" << m_name;
    }
}

void Track::setColor(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        emit colorChanged(m_color);
        qDebug() << "Track" << m_trackId << "color changed to:" << m_color.name();
    }
}

// ========== Sounit Management ==========

bool Track::loadSounit(const QString &filePath)
{
    qDebug() << "Track" << m_trackId << "loading sounit from:" << filePath;

    // Suppress dirty marking during loading (save previous state for nested calls)
    bool wasLoadingInProgress = m_loadingInProgress;
    m_loadingInProgress = true;

    // CRITICAL: Clear ALL existing containers before loading new sounit
    // Each Track owns its own canvas, so we can safely delete all containers
    // This prevents old containers from polluting the graph
    QList<Container*> existingContainers = m_canvas->findChildren<Container*>();
    qDebug() << "Track" << m_trackId << "clearing" << existingContainers.size() << "existing containers";
    for (Container *c : existingContainers) {
        c->setParent(nullptr);  // Detach from canvas so findChildren won't find them
        c->deleteLater();       // Schedule for deletion (destructor is private)
    }
    m_canvas->getConnections().clear();
    m_canvas->clearSelection();

    // Process events to ensure deleteLater() completes for old containers
    // before loading new ones (prevents findChildren from finding stale objects)
    QCoreApplication::processEvents();

    // Load sounit into canvas
    QString loadedName;
    bool success = m_canvas->loadFromJson(filePath, loadedName);

    if (!success) {
        qWarning() << "Track" << m_trackId << "failed to load sounit:" << filePath;
        m_loadingInProgress = wasLoadingInProgress;
        return false;
    }

    // Update sounit metadata
    m_sounitName = loadedName;
    m_sounitComment = m_canvas->getSounitComment();
    m_sounitFilePath = filePath;
    m_sounitDirty = false;

    // Invalidate graph and render cache
    if (m_graph) {
        delete m_graph;
        m_graph = nullptr;
    }
    invalidateRenderCache();

    // CRITICAL: Build the base graph immediately after loading
    // This captures the original sounit state BEFORE the user makes any edits.
    // Variations will capture subsequent modifications, but the base graph
    // (variation 0) should always represent the original loaded state.
    rebuildGraph(m_sampleRate > 0 ? m_sampleRate : 44100.0);
    qDebug() << "Track" << m_trackId << "built base graph from original sounit state";

    emit sounitLoaded(m_sounitName);
    emit sounitDirtyChanged(false);

    // Restore previous loading state (for nested calls from fromJson)
    m_loadingInProgress = wasLoadingInProgress;

    qDebug() << "Track" << m_trackId << "loaded sounit:" << m_sounitName;
    return true;
}

bool Track::saveSounit(const QString &filePath)
{
    qDebug() << "Track" << m_trackId << "saving sounit to:" << filePath;

    // Canvas::saveToJson expects (filePath, sounitName)
    bool success = m_canvas->saveToJson(filePath, m_sounitName);

    if (!success) {
        qWarning() << "Track" << m_trackId << "failed to save sounit:" << filePath;
        return false;
    }

    // Update state
    m_sounitFilePath = filePath;
    m_sounitDirty = false;
    emit sounitDirtyChanged(false);

    qDebug() << "Track" << m_trackId << "saved sounit successfully";
    return true;
}

void Track::newSounit()
{
    qDebug() << "Track" << m_trackId << "creating new sounit";

    // Clear all containers from canvas
    QList<Container*> containers = m_canvas->findChildren<Container*>();
    for (Container *c : containers) {
        c->deleteLater();
    }

    // Clear connections
    m_canvas->getConnections().clear();
    m_canvas->clearSelection();
    m_canvas->update();

    // Reset metadata
    m_sounitName = "Untitled Sounit";
    m_sounitComment = "";
    m_sounitFilePath.clear();
    m_sounitDirty = false;

    m_canvas->setSounitName(m_sounitName);
    m_canvas->setSounitComment(m_sounitComment);
    m_canvas->setExpressiveCurveNames(QStringList{});

    // Invalidate graph and cache
    if (m_graph) {
        delete m_graph;
        m_graph = nullptr;
    }
    invalidateRenderCache();

    emit sounitLoaded(m_sounitName);
    emit sounitDirtyChanged(false);
    emit graphRebuilt(false);

    qDebug() << "Track" << m_trackId << "new sounit created";
}

bool Track::rebuildGraph(double sampleRate)
{
    qDebug() << "Track" << m_trackId << "rebuilding graph at" << sampleRate << "Hz";

    m_sampleRate = sampleRate;

    // Delete old graph
    if (m_graph) {
        delete m_graph;
        m_graph = nullptr;
    }

    // Create and build new graph
    m_graph = new SounitGraph(sampleRate);
    m_graph->buildFromCanvas(m_canvas);

    bool isValid = m_graph->isValid();

    if (isValid) {
        qDebug() << "Track" << m_trackId << "graph built successfully";
    } else {
        qWarning() << "Track" << m_trackId << "graph is invalid";
    }

    // Invalidate render cache - graph changed
    invalidateRenderCache();

    // Update graph hash to invalidate all note renders
    m_graphHash++;
    qDebug() << "Track" << m_trackId << "graph hash updated to" << m_graphHash;

    emit graphRebuilt(isValid);

    return isValid;
}

void Track::rebuildAllGraphsAtRate(double sampleRate)
{
    qDebug() << "Track" << m_trackId << "rebuilding all graphs at" << sampleRate << "Hz";

    // Wait for any in-progress background render so its results (made with
    // the old graphs and labeled with the old rate) don't land after we
    // swap the graphs and change m_sampleRate.
    if (m_rendering.load()) {
        qDebug() << "Track" << m_trackId << "waiting for in-progress render before rate rebuild";
        if (m_renderWatcher && m_renderWatcher->isRunning()) {
            m_renderWatcher->waitForFinished();
        }
    }

    // Base graph (bumps the graph hash and invalidates the render cache).
    rebuildGraph(sampleRate);

    // Variation graphs are compiled from their stored graphData at the rate
    // the variation was created with; recompile them at the new rate too,
    // freeing the old compiled graph + source canvas.
    for (SounitVariation *var : m_variations) {
        delete var->compiledGraph;
        var->compiledGraph = nullptr;
        delete var->sourceCanvas;
        var->sourceCanvas = nullptr;
        if (!var->graphData.isEmpty()) {
            buildGraphFromJson(var->graphData, sampleRate, var);
        }
    }
}

// ========== Note Management ==========

void Track::addNote(const Note &note)
{
    m_notes.append(note);
    m_notesDirty = true;

    // Invalidate render segments that overlap this note
    invalidateSegmentsForNote(note);

    emit noteAdded(note);
    emit notesDirtyChanged(true);

    qDebug() << "Track" << m_trackId << "note added:" << note.getId()
             << "at" << note.getStartTime() << "ms";
}

bool Track::removeNote(const QString &noteId)
{
    for (int i = 0; i < m_notes.size(); ++i) {
        if (m_notes[i].getId() == noteId) {
            Note removedNote = m_notes[i];
            m_notes.removeAt(i);
            m_notesDirty = true;

            // Invalidate render segments that overlapped this note
            invalidateSegmentsForNote(removedNote);

            emit noteRemoved(noteId);
            emit notesDirtyChanged(true);

            qDebug() << "Track" << m_trackId << "note removed:" << noteId;
            return true;
        }
    }

    qWarning() << "Track" << m_trackId << "note not found for removal:" << noteId;
    return false;
}

bool Track::updateNote(const Note &note)
{
    for (int i = 0; i < m_notes.size(); ++i) {
        if (m_notes[i].getId() == note.getId()) {
            Note oldNote = m_notes[i];
            m_notes[i] = note;
            m_notesDirty = true;

            // Invalidate segments at both old and new positions
            invalidateSegmentsForNote(oldNote);
            invalidateSegmentsForNote(note);

            emit noteUpdated(note);
            emit notesDirtyChanged(true);

            qDebug() << "Track" << m_trackId << "note updated:" << note.getId();
            return true;
        }
    }

    qWarning() << "Track" << m_trackId << "note not found for update:" << note.getId();
    return false;
}

void Track::clearNotes()
{
    qDebug() << "Track" << m_trackId << "clearing all notes";

    m_notes.clear();
    m_notesDirty = true;
    invalidateRenderCache();

    emit notesDirtyChanged(true);
}

void Track::syncNotes(const QList<Note> &notes)
{
    // Thread-safe note synchronization
    // Lock mutex to prevent race conditions with audio callback accessing getMixedBuffer
    std::lock_guard<std::mutex> lock(m_playbackMutex);

    m_notes.clear();
    for (const Note &note : notes) {
        m_notes.append(note);
    }

    qDebug() << "Track" << m_trackId << "synced" << m_notes.size() << "notes (thread-safe)";
}

// ========== Variation Management ==========

int Track::createVariation(const QString &name)
{
    qDebug() << "Track" << m_trackId << "creating variation:" << name;
    qDebug() << "Track" << m_trackId << "base graph at:" << (void*)m_graph;

    SounitVariation* var = new SounitVariation();
    var->name = name;

    // Serialize current canvas state
    QJsonObject graphData;
    QJsonArray containersArray;
    for (Container* c : m_canvas->findChildren<Container*>()) {
        containersArray.append(m_canvas->serializeContainer(c));
    }
    graphData["containers"] = containersArray;

    QJsonArray connectionsArray;
    for (const Canvas::Connection& conn : m_canvas->getConnections()) {
        connectionsArray.append(m_canvas->serializeConnection(conn));
    }
    graphData["connections"] = connectionsArray;

    QJsonArray curveNamesArray;
    for (const QString &n : m_canvas->getExpressiveCurveNames()) curveNamesArray.append(n);
    graphData["expressiveCurveNames"] = curveNamesArray;

    qDebug() << "Track" << m_trackId << "serialized" << containersArray.size()
             << "containers and" << connectionsArray.size() << "connections";

    var->graphData = graphData;
    var->graphHash = m_graphHash;

    // Build compiled graph and canvas for this variation
    buildGraphFromJson(graphData, m_sampleRate, var);

    if (!var->compiledGraph || !var->compiledGraph->isValid()) {
        qWarning() << "Track" << m_trackId << "failed to build graph for variation:" << name;
        delete var;
        return 0;
    }

    qDebug() << "Track" << m_trackId << "variation graph at:" << (void*)var->compiledGraph
             << "canvas at:" << (void*)var->sourceCanvas;

    m_variations.append(var);
    int index = m_variations.size();  // 1-based

    // Debug: List all variation graphs
    qDebug() << "Track" << m_trackId << "now has" << m_variations.size() << "variations:";
    for (int i = 0; i < m_variations.size(); ++i) {
        qDebug() << "  Variation" << (i+1) << ":" << m_variations[i]->name
                 << "graph at" << (void*)m_variations[i]->compiledGraph;
    }

    emit variationCreated(index, name);
    qDebug() << "Track" << m_trackId << "created variation" << index << ":" << name;

    return index;
}

int Track::createVariationFromJson(const QJsonObject &graphData, const QString &name)
{
    qDebug() << "Track" << m_trackId << "creating variation from JSON:" << name;

    SounitVariation* var = new SounitVariation();
    var->name = name;
    var->graphData = graphData;
    var->graphHash = 0;  // Not derived from current canvas state

    buildGraphFromJson(graphData, m_sampleRate, var);

    if (!var->compiledGraph || !var->compiledGraph->isValid()) {
        qWarning() << "Track" << m_trackId << "failed to build graph for variation from JSON:" << name;
        delete var;
        return 0;
    }

    m_variations.append(var);
    int index = m_variations.size();  // 1-based

    emit variationCreated(index, name);
    qDebug() << "Track" << m_trackId << "created variation" << index << "from JSON:" << name;

    return index;
}

bool Track::deleteVariation(int index)
{
    // index is 1-based, convert to 0-based for list access
    int listIndex = index - 1;

    if (listIndex < 0 || listIndex >= m_variations.size()) {
        qWarning() << "Track" << m_trackId << "invalid variation index for deletion:" << index;
        return false;
    }

    QString name = m_variations[listIndex]->name;
    delete m_variations[listIndex];
    m_variations.removeAt(listIndex);

    // Reset notes using this variation to base sounit (index 0)
    for (Note &note : m_notes) {
        if (note.getVariationIndex() == index) {
            note.setVariationIndex(0);
            note.setRenderDirty(true);
        } else if (note.getVariationIndex() > index) {
            // Shift down indices for variations after the deleted one
            note.setVariationIndex(note.getVariationIndex() - 1);
        }
    }

    emit variationDeleted(index);
    qDebug() << "Track" << m_trackId << "deleted variation" << index << ":" << name;

    return true;
}

QString Track::getVariationName(int index) const
{
    // index is 1-based
    int listIndex = index - 1;

    if (listIndex < 0 || listIndex >= m_variations.size()) {
        return QString();
    }

    return m_variations[listIndex]->name;
}

bool Track::setVariationName(int index, const QString &name)
{
    // index is 1-based
    int listIndex = index - 1;

    if (listIndex < 0 || listIndex >= m_variations.size()) {
        qWarning() << "Track" << m_trackId << "invalid variation index for rename:" << index;
        return false;
    }

    m_variations[listIndex]->name = name;
    emit variationRenamed(index, name);
    qDebug() << "Track" << m_trackId << "renamed variation" << index << "to:" << name;

    return true;
}

SounitGraph* Track::getGraphForVariation(int index)
{
    qDebug() << "Track" << m_trackId << "getGraphForVariation: index=" << index
             << "variationCount=" << m_variations.size()
             << "baseGraph=" << (void*)m_graph;

    if (index == 0) {
        qDebug() << "Track" << m_trackId << "returning BASE graph at" << (void*)m_graph;
        return m_graph;  // Base sounit
    }

    // index is 1-based, convert to 0-based
    int listIndex = index - 1;

    if (listIndex >= 0 && listIndex < m_variations.size()) {
        SounitGraph* graph = m_variations[listIndex]->compiledGraph;
        if (graph && graph->isValid()) {
            qDebug() << "Track" << m_trackId << "returning VARIATION" << index
                     << "graph at" << (void*)graph
                     << "name:" << m_variations[listIndex]->name;
            return graph;
        } else {
            qWarning() << "Track" << m_trackId << "variation" << index
                       << "graph is null or invalid, graph=" << (void*)graph;
        }
    }

    qWarning() << "Track" << m_trackId << "invalid variation index" << index << ", using base";
    return m_graph;  // Fallback to base
}

Canvas* Track::getCanvasForVariation(int index) const
{
    if (index == 0) return m_canvas;
    int listIndex = index - 1;
    if (listIndex >= 0 && listIndex < m_variations.size()) {
        return m_variations[listIndex]->sourceCanvas;
    }
    return nullptr;
}

QStringList Track::getVariationNames() const
{
    QStringList names;
    for (const SounitVariation* var : m_variations) {
        names.append(var->name);
    }
    return names;
}

int Track::findVariationByName(const QString &name) const
{
    for (int i = 0; i < m_variations.size(); ++i) {
        if (m_variations[i]->name == name) {
            return i + 1;  // 1-based index
        }
    }
    return 0;  // Not found
}

int Track::createOrUpdateInternalVariation(int existingIndex)
{
    qDebug() << "Track" << m_trackId << "createOrUpdateInternalVariation existingIndex=" << existingIndex;

    // Serialize current canvas state
    QJsonObject graphData;
    QJsonArray containersArray;
    for (Container* c : m_canvas->findChildren<Container*>()) {
        containersArray.append(m_canvas->serializeContainer(c));
    }
    graphData["containers"] = containersArray;

    QJsonArray connectionsArray;
    for (const Canvas::Connection& conn : m_canvas->getConnections()) {
        connectionsArray.append(m_canvas->serializeConnection(conn));
    }
    graphData["connections"] = connectionsArray;

    QJsonArray curveNamesArray;
    for (const QString &n : m_canvas->getExpressiveCurveNames()) curveNamesArray.append(n);
    graphData["expressiveCurveNames"] = curveNamesArray;

    // Check if we should update an existing internal variation
    int listIndex = existingIndex - 1;
    if (existingIndex > 0 && listIndex >= 0 && listIndex < m_variations.size()
        && m_variations[listIndex]->isInternal) {
        // Update in-place
        SounitVariation* var = m_variations[listIndex];
        var->graphData = graphData;
        var->graphHash = m_graphHash;

        // Rebuild compiled graph
        delete var->compiledGraph;
        var->compiledGraph = nullptr;
        delete var->sourceCanvas;
        var->sourceCanvas = nullptr;

        buildGraphFromJson(graphData, m_sampleRate, var);

        if (!var->compiledGraph || !var->compiledGraph->isValid()) {
            qWarning() << "Track" << m_trackId << "failed to rebuild internal variation" << existingIndex;
            return 0;
        }

        qDebug() << "Track" << m_trackId << "updated internal variation" << existingIndex;
        return existingIndex;
    }

    // Create new internal variation
    SounitVariation* var = new SounitVariation();
    var->name = QUuid::createUuid().toString();
    var->isInternal = true;
    var->graphData = graphData;
    var->graphHash = m_graphHash;

    buildGraphFromJson(graphData, m_sampleRate, var);

    if (!var->compiledGraph || !var->compiledGraph->isValid()) {
        qWarning() << "Track" << m_trackId << "failed to build internal variation graph";
        delete var;
        return 0;
    }

    m_variations.append(var);
    int index = m_variations.size();  // 1-based

    qDebug() << "Track" << m_trackId << "created internal variation" << index << ":" << var->name;
    return index;
}

bool Track::isInternalVariation(int index) const
{
    int i = index - 1;
    return (i >= 0 && i < m_variations.size()) ? m_variations[i]->isInternal : false;
}

void Track::saveBaseCanvasState()
{
    // Serialize current canvas state to JSON
    QJsonObject state;

    // Serialize containers
    QJsonArray containersArray;
    QList<Container*> containers = m_canvas->findChildren<Container*>();
    for (Container *c : containers) {
        containersArray.append(m_canvas->serializeContainer(c));
    }
    state["containers"] = containersArray;

    // Serialize connections
    QJsonArray connectionsArray;
    for (const Canvas::Connection &conn : m_canvas->getConnections()) {
        connectionsArray.append(m_canvas->serializeConnection(conn));
    }
    state["connections"] = connectionsArray;

    QJsonArray curveNamesArray;
    for (const QString &n : m_canvas->getExpressiveCurveNames()) curveNamesArray.append(n);
    state["expressiveCurveNames"] = curveNamesArray;

    m_baseCanvasState = state;
    qDebug() << "Track" << m_trackId << "saved base canvas state:"
             << containers.size() << "containers,"
             << m_canvas->getConnections().size() << "connections";
}

bool Track::loadVariationToCanvas(int index)
{
    QJsonObject graphData;

    if (index == 0) {
        // Load base sounit
        if (m_baseCanvasState.isEmpty()) {
            qDebug() << "Track" << m_trackId << "no base state saved, nothing to restore";
            return false;
        }
        graphData = m_baseCanvasState;
        qDebug() << "Track" << m_trackId << "restoring base sounit";
    } else if (index > 0 && index <= m_variations.size()) {
        // Load variation
        SounitVariation* var = m_variations[index - 1];
        if (!var) {
            qWarning() << "Track" << m_trackId << "variation" << index << "is null";
            return false;
        }
        graphData = var->graphData;
        qDebug() << "Track" << m_trackId << "loading variation" << index << ":" << var->name;
    } else {
        qWarning() << "Track" << m_trackId << "invalid variation index:" << index;
        return false;
    }

    // Clear the current canvas - delete all containers and connections
    m_canvas->getConnections().clear();
    QList<Container*> existingContainers = m_canvas->findChildren<Container*>();
    for (Container *c : existingContainers) {
        c->setParent(nullptr);  // Detach from canvas first
        c->deleteLater();
    }
    m_canvas->clearSelection();

    // Process events to ensure deleteLater completes before adding new containers
    QCoreApplication::processEvents();

    m_canvas->setLoading(true);

    // Deserialize containers from graphData
    QJsonArray containersArray = graphData["containers"].toArray();
    QMap<QString, Container*> containerMap;

    for (const QJsonValue &val : containersArray) {
        QJsonObject containerJson = val.toObject();
        Container* container = m_canvas->deserializeContainer(containerJson, m_canvas);
        if (container) {
            container->setParent(m_canvas);
            container->show();  // Make sure container is visible
            containerMap[container->getInstanceName()] = container;
        }
    }

    // Deserialize connections
    QJsonArray connectionsArray = graphData["connections"].toArray();
    for (const QJsonValue &val : connectionsArray) {
        QJsonObject connJson = val.toObject();

        QString fromName = connJson["fromContainer"].toString();
        QString fromPort = connJson["fromPort"].toString();
        QString toName = connJson["toContainer"].toString();
        QString toPort = connJson["toPort"].toString();
        QString function = connJson["function"].toString("passthrough");
        double weight = connJson["weight"].toDouble(1.0);

        Container* fromContainer = containerMap.value(fromName, nullptr);
        Container* toContainer = containerMap.value(toName, nullptr);

        if (fromContainer && toContainer) {
            Canvas::Connection conn;
            conn.fromContainer = fromContainer;
            conn.fromPort = fromPort;
            conn.toContainer = toContainer;
            conn.toPort = toPort;
            conn.function = function;
            conn.weight = weight;
            m_canvas->getConnections().append(conn);
        }
    }

    // Restore the canvas's declared expressive curve names so the envelope
    // inspector + score canvas see the names that belong to this variation /
    // base sounit. Older saves that predate the field will yield an empty
    // list, matching the prior behaviour.
    QStringList loadedCurveNames;
    QJsonArray curveNamesArray = graphData["expressiveCurveNames"].toArray();
    for (const QJsonValue &v : curveNamesArray) {
        QString s = v.toString();
        if (!s.isEmpty()) loadedCurveNames.append(s);
    }
    m_canvas->setExpressiveCurveNames(loadedCurveNames);

    m_canvas->setLoading(false);
    m_canvas->update();

    // Only mark dirty if loading a variation (not restoring base)
    if (index > 0) {
        markSounitDirty();
    }

    qDebug() << "Track" << m_trackId << "loaded" << containerMap.size() << "containers,"
             << m_canvas->getConnections().size() << "connections";
    return true;
}

void Track::buildGraphFromJson(const QJsonObject &graphData, double sampleRate, SounitVariation* outVariation)
{
    qDebug() << "Track" << m_trackId << "building graph from JSON";

    if (!outVariation) {
        qWarning() << "Track" << m_trackId << "buildGraphFromJson called with null variation";
        return;
    }

    // Create canvas on heap - it will be owned by the variation
    // The canvas keeps containers alive, which the graph needs during execution
    Canvas* canvas = new Canvas(nullptr);
    canvas->setLoading(true);

    // Deserialize containers
    QJsonArray containersArray = graphData["containers"].toArray();
    QMap<QString, Container*> containerMap;  // Map instanceName -> Container*

    for (const QJsonValue &val : containersArray) {
        QJsonObject containerJson = val.toObject();
        Container* container = canvas->deserializeContainer(containerJson, canvas);
        if (container) {
            container->setParent(canvas);
            containerMap[container->getInstanceName()] = container;
        }
    }

    // Deserialize connections
    QJsonArray connectionsArray = graphData["connections"].toArray();
    for (const QJsonValue &val : connectionsArray) {
        QJsonObject connJson = val.toObject();

        QString fromName = connJson["fromContainer"].toString();
        QString fromPort = connJson["fromPort"].toString();
        QString toName = connJson["toContainer"].toString();
        QString toPort = connJson["toPort"].toString();
        QString function = connJson["function"].toString("passthrough");
        double weight = connJson["weight"].toDouble(1.0);

        Container* fromContainer = containerMap.value(fromName, nullptr);
        Container* toContainer = containerMap.value(toName, nullptr);

        if (fromContainer && toContainer) {
            Canvas::Connection conn;
            conn.fromContainer = fromContainer;
            conn.fromPort = fromPort;
            conn.toContainer = toContainer;
            conn.toPort = toPort;
            conn.function = function;
            conn.weight = weight;
            canvas->getConnections().append(conn);
        }
    }

    // Restore the sounit's declared expressive curve names so code that queries
    // this canvas (e.g. score canvas right-click / "Show curve" combo) sees the
    // same names the variation was created with.
    QStringList loadedCurveNames;
    QJsonArray curveNamesArray = graphData["expressiveCurveNames"].toArray();
    for (const QJsonValue &v : curveNamesArray) {
        QString s = v.toString();
        if (!s.isEmpty()) loadedCurveNames.append(s);
    }
    canvas->setExpressiveCurveNames(loadedCurveNames);

    canvas->setLoading(false);

    // Build graph from canvas
    SounitGraph* graph = new SounitGraph(sampleRate);
    graph->buildFromCanvas(canvas);

    // Store both in the variation - variation takes ownership
    outVariation->sourceCanvas = canvas;
    outVariation->compiledGraph = graph;

    if (!graph->isValid()) {
        qWarning() << "Track" << m_trackId << "built graph is invalid";
    }

    qDebug() << "Track" << m_trackId << "built variation graph with"
             << containerMap.size() << "containers";
}

// ========== Audio Rendering ==========

bool Track::prerender(double sampleRate, double segmentDurationMs)
{
    qDebug() << "Track" << m_trackId << "pre-rendering" << m_notes.size() << "notes";

    // Can't render without a valid graph
    if (!hasValidGraph()) {
        qWarning() << "Track" << m_trackId << "cannot render - no valid graph";
        return false;
    }

    if (m_notes.isEmpty()) {
        qDebug() << "Track" << m_trackId << "no notes to render";
        clearRenderCache();
        return true;
    }

    m_sampleRate = sampleRate;
    m_segmentDurationMs = segmentDurationMs;

    emit renderStarted();

    // Calculate total composition duration
    double totalDurationMs = 0.0;
    for (const Note &note : m_notes) {
        double noteEndTime = note.getStartTime() + note.getDuration();
        if (noteEndTime > totalDurationMs) {
            totalDurationMs = noteEndTime;
        }
    }

    // Calculate number of segments needed
    int numSegments = static_cast<int>(std::ceil(totalDurationMs / m_segmentDurationMs));
    if (numSegments == 0) numSegments = 1;

    size_t segmentSizeSamples = static_cast<size_t>((m_segmentDurationMs / 1000.0) * sampleRate);

    qDebug() << "Track" << m_trackId << "render: duration" << totalDurationMs << "ms,"
             << numSegments << "segments," << segmentSizeSamples << "samples/segment";

    // Detect changes and mark dirty segments
    detectNoteChangesAndInvalidate();

    // Grow segments if needed (composition extended)
    if (static_cast<int>(m_renderSegments.size()) < numSegments) {
        int oldSize = m_renderSegments.size();
        for (int i = oldSize; i < numSegments; ++i) {
            RenderSegment seg;
            seg.startTimeMs = i * m_segmentDurationMs;
            seg.endTimeMs = (i + 1) * m_segmentDurationMs;
            seg.samples.resize(segmentSizeSamples, 0.0f);
            seg.isDirty = true;
            m_renderSegments.append(seg);
        }
        qDebug() << "Track" << m_trackId << "grew from" << oldSize << "to" << numSegments << "segments";
    }

    // Shrink segments if needed (composition shortened)
    if (static_cast<int>(m_renderSegments.size()) > numSegments) {
        m_renderSegments.resize(numSegments);
        qDebug() << "Track" << m_trackId << "shrunk to" << numSegments << "segments";
    }

    // Clear dirty segments
    for (RenderSegment &seg : m_renderSegments) {
        if (seg.isDirty) {
            std::fill(seg.samples.begin(), seg.samples.end(), 0.0f);
            seg.noteIds.clear();
        }
    }

    // Render each note
    int notesRendered = 0;
    for (int noteIdx = 0; noteIdx < m_notes.size(); ++noteIdx) {
        const Note &note = m_notes[noteIdx];

        // Check if this note affects any dirty segments
        bool affectsDirtySegment = false;
        for (const RenderSegment &seg : m_renderSegments) {
            if (seg.isDirty) {
                double noteEndTime = note.getStartTime() + note.getDuration();
                if (noteEndTime > seg.startTimeMs && note.getStartTime() < seg.endTimeMs) {
                    affectsDirtySegment = true;
                    break;
                }
            }
        }

        if (!affectsDirtySegment) {
            continue;  // Skip notes that don't affect dirty segments
        }

        notesRendered++;

        // Reset graph for this note
        m_graph->reset();

        // Calculate note rendering parameters
        size_t noteStartSample = static_cast<size_t>((note.getStartTime() / 1000.0) * sampleRate);
        size_t noteDurationSamples = static_cast<size_t>((note.getDuration() / 1000.0) * sampleRate);

        // Precompute expressive curve names/curves for this note (see renderNoteImpl).
        const int expressiveCount = note.getExpressiveCurveCount();
        QStringList scoreCurveNames;
        scoreCurveNames.reserve(expressiveCount);
        scoreCurveNames.append(QStringLiteral("Dynamics"));
        QVector<const Curve*> expressiveCurves;
        expressiveCurves.reserve(expressiveCount - 1);
        for (int ci = 1; ci < expressiveCount; ++ci) {
            scoreCurveNames.append(note.getExpressiveCurveName(ci));
            expressiveCurves.append(&note.getExpressiveCurve(ci));
        }

        // Render each sample of the note
        for (size_t i = 0; i < noteDurationSamples; ++i) {
            // Calculate note progress (0.0 to 1.0)
            double noteProgress = static_cast<double>(i) / static_cast<double>(noteDurationSamples - 1);
            if (noteDurationSamples == 1) noteProgress = 0.5;

            // Get pitch and dynamics at this point in the note
            double pitch = note.getPitchAt(noteProgress);
            double dynamics = note.getDynamicsAt(noteProgress);

            QVector<double> scoreCurveValues;
            scoreCurveValues.reserve(scoreCurveNames.size());
            scoreCurveValues.append(dynamics);
            for (const Curve *c : expressiveCurves) {
                double val = (c && !c->isEmpty()) ? c->valueAt(std::min(noteProgress, 1.0)) : 0.5;
                scoreCurveValues.append(val);
            }

            // Generate audio sample
            double sample = m_graph->generateSample(pitch, noteProgress, false, false,
                                                    dynamics, scoreCurveValues, scoreCurveNames);

            // Apply dynamics and simple envelope
            double envelope = 1.0;
            if (noteProgress < 0.05) {
                envelope = noteProgress / 0.05;  // Attack (5%)
            } else if (noteProgress > 0.9) {
                envelope = (1.0 - noteProgress) / 0.1;  // Release (10%)
            }

            float finalSample = static_cast<float>(sample * dynamics * envelope);

            // Find which segment this sample belongs to
            size_t globalSamplePos = noteStartSample + i;
            int segmentIndex = static_cast<int>(globalSamplePos / segmentSizeSamples);

            if (segmentIndex >= 0 && segmentIndex < m_renderSegments.size()) {
                RenderSegment &seg = m_renderSegments[segmentIndex];

                // Only write to dirty segments
                if (seg.isDirty) {
                    size_t posInSegment = globalSamplePos - (segmentIndex * segmentSizeSamples);

                    if (posInSegment < seg.samples.size()) {
                        // Mix (add) samples for overlapping notes
                        seg.samples[posInSegment] += finalSample;
                        seg.noteIds.insert(note.getId());
                    }
                }
            }
        }

        // Update progress
        int progress = static_cast<int>((static_cast<double>(noteIdx + 1) / m_notes.size()) * 100.0);
        emit renderProgressChanged(progress);
    }

    // Clamp samples to prevent clipping
    for (RenderSegment &seg : m_renderSegments) {
        if (seg.isDirty) {
            for (float &sample : seg.samples) {
                sample = std::clamp(sample, -1.0f, 1.0f);
            }
            seg.isDirty = false;  // Mark as clean
        }
    }

    // Cache notes for next render comparison
    m_cachedNotes = m_notes;
    m_renderDirty = false;

    qDebug() << "Track" << m_trackId << "rendered" << notesRendered << "of" << m_notes.size()
             << "notes into" << m_renderSegments.size() << "segments";

    emit renderCompleted();

    return true;
}

void Track::invalidateRenderCache()
{
    qDebug() << "Track" << m_trackId << "invalidating render cache";

    m_renderDirty = true;

    // Mark all segments dirty
    for (RenderSegment &seg : m_renderSegments) {
        seg.isDirty = true;
    }

    emit renderCacheInvalidated();
}

void Track::clearRenderCache()
{
    qDebug() << "Track" << m_trackId << "clearing render cache";

    m_renderSegments.clear();
    m_cachedNotes.clear();
    m_renderDirty = true;

    emit renderCacheInvalidated();
}

// ========== Playback State ==========

void Track::setMuted(bool muted)
{
    if (m_muted != muted) {
        m_muted = muted;
        emit muteChanged(m_muted);
        qDebug() << "Track" << m_trackId << "mute:" << m_muted;
    }
}

void Track::setSolo(bool solo)
{
    if (m_solo != solo) {
        m_solo = solo;
        emit soloChanged(m_solo);
        qDebug() << "Track" << m_trackId << "solo:" << m_solo;
    }
}

void Track::setVolume(float volume)
{
    float clampedVolume = std::clamp(volume, 0.0f, 1.0f);
    if (m_volume != clampedVolume) {
        m_volume = clampedVolume;
        emit volumeChanged(m_volume);
        qDebug() << "Track" << m_trackId << "volume:" << m_volume;
    }
}

void Track::setGain(float gain)
{
    float clampedGain = std::clamp(gain, 0.0f, 8.0f);
    if (m_gain != clampedGain) {
        m_gain = clampedGain;
        emit gainChanged(m_gain);
        qDebug() << "Track" << m_trackId << "gain:" << m_gain;
    }
}

void Track::setPan(float pan)
{
    float clampedPan = std::clamp(pan, -1.0f, 1.0f);
    if (m_pan != clampedPan) {
        m_pan = clampedPan;
        emit panChanged(m_pan);
        qDebug() << "Track" << m_trackId << "pan:" << m_pan;
    }
}

// ========== Serialization ==========

QJsonObject Track::toJson() const
{
    QJsonObject json;

    // Track metadata
    json["trackId"] = m_trackId;
    json["name"] = m_name;
    json["color"] = m_color.name();

    // Sounit reference (file path, not contents)
    json["sounitFilePath"] = m_sounitFilePath;
    json["sounitName"] = m_sounitName;

    // Playback state
    json["muted"] = m_muted;
    json["solo"] = m_solo;
    json["volume"] = static_cast<double>(m_volume);
    json["gain"] = static_cast<double>(m_gain);
    json["pan"] = static_cast<double>(m_pan);

    // Notes array
    QJsonArray notesArray;
    for (const Note &note : m_notes) {
        notesArray.append(note.toJson());
    }
    json["notes"] = notesArray;

    // Variations array
    QJsonArray variationsArray;
    for (const SounitVariation* var : m_variations) {
        variationsArray.append(var->toJson());
    }
    json["variations"] = variationsArray;

    // Sample rate (needed to rebuild variation graphs on load)
    json["sampleRate"] = m_sampleRate;

    // Graph hash (needed for render cache validation on load)
    json["graphHash"] = static_cast<qint64>(m_graphHash);

    // Recording clip (Phase 9 bake) - metadata only, the WAV lives on disk
    if (m_hasClip) {
        QJsonObject clipJson;
        clipJson["filePath"] = m_clip.filePath;
        clipJson["offsetMs"] = m_clip.offsetMs;
        clipJson["onsetMs"] = m_clip.onsetMs;
        clipJson["manualNudgeMs"] = m_clip.manualNudgeMs;
        clipJson["durationMs"] = m_clip.durationMs;
        clipJson["gain"] = static_cast<double>(m_clip.gain);
        clipJson["enabled"] = m_clip.enabled;
        clipJson["liveMidi"] = m_clip.liveMidi;
        json["clip"] = clipJson;
    }

    return json;
}

bool Track::fromJson(const QJsonObject &json)
{
    qDebug() << "Track" << m_trackId << "deserializing from JSON";

    // Suppress dirty marking during entire deserialization process
    m_loadingInProgress = true;

    // Read track metadata
    m_trackId = json["trackId"].toInt(m_trackId);
    setName(json["name"].toString(m_name));

    QString colorName = json["color"].toString("#3498db");
    setColor(QColor(colorName));

    // Read sounit reference
    QString savedSounitName = json["sounitName"].toString();
    QString sounitPath = json["sounitFilePath"].toString();
    if (!sounitPath.isEmpty()) {
        if (!loadSounit(sounitPath)) {
            qWarning() << "Track" << m_trackId << "failed to load referenced sounit:" << sounitPath;
            // Continue anyway - track can exist without sounit
        }
        // loadSounit() overwrites m_sounitName with the name from the .sounit file.
        // Restore the user-saved name (may have been renamed after the sounit was loaded).
        if (!savedSounitName.isEmpty() && m_sounitName != savedSounitName) {
            m_sounitName = savedSounitName;
            m_canvas->setSounitName(savedSounitName);
        }
    } else {
        m_sounitName = savedSounitName;
    }

    // Read playback state
    setMuted(json["muted"].toBool(false));
    setSolo(json["solo"].toBool(false));
    setVolume(static_cast<float>(json["volume"].toDouble(1.0)));
    setGain(static_cast<float>(json["gain"].toDouble(1.0)));
    setPan(static_cast<float>(json["pan"].toDouble(0.0)));

    // Read recording clip (Phase 9 bake). The PCM itself is loaded lazily
    // via prepareClipForPlayback(); only the metadata lives in the project.
    m_hasClip = false;
    m_clipLoaded = false;
    m_clipPcm.clear();
    QJsonObject clipJson = json["clip"].toObject();
    if (!clipJson.isEmpty() && !clipJson["filePath"].toString().isEmpty()) {
        m_hasClip = true;
        m_clip.filePath = clipJson["filePath"].toString();
        m_clip.offsetMs = clipJson["offsetMs"].toDouble(0.0);
        m_clip.onsetMs = clipJson["onsetMs"].toDouble(0.0);
        m_clip.manualNudgeMs = clipJson["manualNudgeMs"].toDouble(0.0);
        m_clip.durationMs = clipJson["durationMs"].toDouble(0.0);
        m_clip.gain = static_cast<float>(clipJson["gain"].toDouble(1.0));
        m_clip.enabled = clipJson["enabled"].toBool(true);
        m_clip.liveMidi = clipJson["liveMidi"].toBool(false);
    }

    // Read notes
    m_notes.clear();
    QJsonArray notesArray = json["notes"].toArray();
    for (const QJsonValue &val : notesArray) {
        Note note = Note::fromJson(val.toObject());
        m_notes.append(note);
    }

    m_notesDirty = false;

    // Read sample rate (needed for rebuilding variation graphs)
    m_sampleRate = json["sampleRate"].toDouble(44100.0);

    // Read variations
    qDeleteAll(m_variations);
    m_variations.clear();
    QJsonArray variationsArray = json["variations"].toArray();
    for (const QJsonValue &val : variationsArray) {
        QJsonObject varJson = val.toObject();

        // Create variation from JSON (name, graphData, graphHash, isInternal)
        SounitVariation* var = new SounitVariation();
        var->name = varJson["name"].toString();
        var->graphData = varJson["graphData"].toObject();
        var->graphHash = static_cast<uint64_t>(varJson["graphHash"].toInteger());
        var->isInternal = varJson["isInternal"].toBool(false);

        // Rebuild the compiled graph from the saved graphData
        buildGraphFromJson(var->graphData, m_sampleRate, var);

        if (var->compiledGraph && var->compiledGraph->isValid()) {
            m_variations.append(var);
            qDebug() << "Track" << m_trackId << "loaded variation:" << var->name;
        } else {
            qWarning() << "Track" << m_trackId << "failed to rebuild variation graph:" << var->name;
            delete var;
        }
    }

    // Restore graph hash (for render cache validation)
    // This must be done AFTER loadSounit() which increments graphHash
    if (json.contains("graphHash")) {
        m_graphHash = static_cast<uint64_t>(json["graphHash"].toInteger());
        qDebug() << "Track" << m_trackId << "restored graphHash:" << m_graphHash;
    }

    qDebug() << "Track" << m_trackId << "deserialized:" << m_notes.size() << "notes,"
             << m_variations.size() << "variations";

    // Re-enable dirty marking and ensure clean state
    m_loadingInProgress = false;
    m_sounitDirty = false;

    return true;
}

// ========== Private Slots ==========

void Track::onCanvasGraphChanged()
{
    qDebug() << "Track" << m_trackId << "canvas graph changed";
    markSounitDirty();

    // NOTE: We do NOT delete or rebuild m_graph here. The base sounit graph
    // represents the original loaded sounit state. User edits affect the canvas
    // and AudioEngine's preview graph, but not the Track's base graph.
    // Variations capture user edits when explicitly created.
}

void Track::onContainerParameterChanged()
{
    qDebug() << "Track" << m_trackId << "container parameter changed";
    markSounitDirty();

    // NOTE: We do NOT delete or rebuild m_graph here. The base sounit graph
    // represents the original loaded sounit state. User edits affect the canvas
    // and AudioEngine's preview graph, but not the Track's base graph.
}

void Track::onSounitNameChanged(const QString &name)
{
    if (m_sounitName != name) {
        m_sounitName = name;
        emit sounitLoaded(m_sounitName);
    }
}

// ========== Private Helper Methods ==========

void Track::connectCanvasSignals()
{
    // Connect canvas graph changes
    connect(m_canvas, &Canvas::graphChanged, this, &Track::onCanvasGraphChanged);

    // Connect sounit name changes
    connect(m_canvas, &Canvas::sounitNameChanged, this, &Track::onSounitNameChanged);

    // Note: Container parameter changes will be connected when containers are created
    // This is handled in the canvas/container creation code
}

void Track::markSounitDirty()
{
    // Don't mark dirty during project loading
    if (m_loadingInProgress) {
        qDebug() << "Track" << m_trackId << "markSounitDirty BLOCKED (loading in progress)";
        return;
    }

    if (!m_sounitDirty) {
        m_sounitDirty = true;
        emit sounitDirtyChanged(true);
        qDebug() << "Track" << m_trackId << "sounit marked dirty - CALLER STACK TRACE NEEDED";
    }
}

void Track::invalidateSegmentsForNote(const Note &note)
{
    if (m_renderSegments.isEmpty()) {
        return;  // No segments to invalidate
    }

    double noteStartTime = note.getStartTime();
    double noteEndTime = noteStartTime + note.getDuration();

    // Find segments that overlap this note's time range
    for (RenderSegment &seg : m_renderSegments) {
        if (noteEndTime > seg.startTimeMs && noteStartTime < seg.endTimeMs) {
            seg.isDirty = true;
            m_renderDirty = true;
        }
    }
}

void Track::detectNoteChangesAndInvalidate()
{
    // Build maps for quick lookup
    QMap<QString, const Note*> cachedNoteMap;
    QMap<QString, const Note*> currentNoteMap;

    for (const Note &note : m_cachedNotes) {
        cachedNoteMap[note.getId()] = &note;
    }

    for (const Note &note : m_notes) {
        currentNoteMap[note.getId()] = &note;
    }

    // Detect removed notes
    for (const Note &cachedNote : m_cachedNotes) {
        if (!currentNoteMap.contains(cachedNote.getId())) {
            // Note was removed - invalidate its old position
            invalidateSegmentsForNote(cachedNote);
        }
    }

    // Detect added and modified notes
    for (const Note &currentNote : m_notes) {
        QString noteId = currentNote.getId();

        if (!cachedNoteMap.contains(noteId)) {
            // Note was added - invalidate its position
            invalidateSegmentsForNote(currentNote);
        } else {
            // Note exists in both - check if modified
            const Note *cachedNote = cachedNoteMap[noteId];

            bool modified = (cachedNote->getStartTime() != currentNote.getStartTime() ||
                           cachedNote->getDuration() != currentNote.getDuration() ||
                           cachedNote->getPitchHz() != currentNote.getPitchHz() ||
                           cachedNote->getDynamics() != currentNote.getDynamics() ||
                           cachedNote->getPitchCurve().getPointCount() != currentNote.getPitchCurve().getPointCount() ||
                           cachedNote->getDynamicsCurve().getPointCount() != currentNote.getDynamicsCurve().getPointCount());

            if (modified) {
                // Note modified - invalidate both old and new positions
                invalidateSegmentsForNote(*cachedNote);
                invalidateSegmentsForNote(currentNote);
            }
        }
    }
}

// ========== Note-Based Rendering (New System) ==========

// ========== Legato chains ==========

// True when `next` starts inside the continuation window after `prev`'s end.
// Mirrors the "a legato note follows" scan in renderNoteImpl, so the score-side
// view of a chain matches what the renderer does.
static bool legatoConnects(const Note &prev, const Note &next)
{
    const double window = std::max(200.0, prev.getDuration() * 0.1);
    const double timeDiff = prev.getEndTime() - next.getStartTime();
    return timeDiff > -50.0 && timeDiff < window;
}

Curve Track::defaultLegatoCurve(const QList<Note> &notes, const QVector<int> &memberIndices)
{
    Curve curve;
    if (memberIndices.isEmpty()) return curve;

    const Note &first = notes[memberIndices.first()];
    const double chainStart = first.getStartTime();
    const double chainSpan = notes[memberIndices.last()].getEndTime() - chainStart;

    if (memberIndices.size() == 1 || chainSpan <= 0.0) {
        return Curve(qBound(0.0, first.getDynamicsAt(0.25), 1.0));
    }

    // The phrase's very start and end, plus one sample per member taken just
    // past its attack (25% in) so a per-note attack transient cannot leak into
    // the joint it would otherwise create.
    QVector<QPair<double, double>> samples;
    samples.reserve(memberIndices.size() + 2);
    samples.append(qMakePair(0.0, first.getDynamicsAt(0.0)));
    for (int idx : memberIndices) {
        const Note &n = notes[idx];
        const double t = (n.getStartTime() - chainStart + 0.25 * n.getDuration()) / chainSpan;
        samples.append(qMakePair(t, n.getDynamicsAt(0.25)));
    }
    samples.append(qMakePair(1.0, notes[memberIndices.last()].getDynamicsAt(1.0)));

    std::sort(samples.begin(), samples.end(),
              [](const QPair<double, double> &a, const QPair<double, double> &b) {
                  return a.first < b.first;
              });

    // Coincident times (very short members) collapse to the later sample; the
    // curve's linear interpolation must never divide by a zero segment.
    QVector<Curve::Point> pts;
    pts.reserve(samples.size());
    for (const QPair<double, double> &s : samples) {
        const double t = qBound(0.0, s.first, 1.0);
        if (!pts.isEmpty() && t <= pts.last().time + 1e-9) {
            pts.last().value = s.second;
            continue;
        }
        pts.append(Curve::Point(t, s.second, 1.0));
    }
    for (const Curve::Point &p : pts) curve.addPoint(p);
    return curve;
}

// The phrase curve a legato chain renders with: the first member (start order)
// that carries a stored legato curve wins; otherwise the interpolated default
// is derived from the members' own dynamics curves. `fallback` must outlive
// the returned pointer. Returns nullptr for a chain of one (not a phrase).
static const Curve *effectiveLegatoCurve(const QList<Note> &notes,
                                         const QVector<int> &members,
                                         Curve &fallback)
{
    if (members.size() < 2) return nullptr;
    for (int idx : members) {
        if (idx >= 0 && idx < notes.size() && notes[idx].hasLegatoCurve())
            return &notes[idx].getLegatoCurve();
    }
    fallback = Track::defaultLegatoCurve(notes, members);
    return &fallback;
}

QVector<Track::LegatoChainInfo> Track::computeLegatoChains(const QList<Note> &notes)
{
    const int n = notes.size();
    QVector<LegatoChainInfo> info(n);

    // Start-time order (stable so equal starts keep list order).
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::stable_sort(order.begin(), order.end(), [&notes](int a, int b) {
        return notes[a].getStartTime() < notes[b].getStartTime();
    });

    // Who continues whom: the predecessor is the closest earlier note on the
    // same variation (same graph = same state to continue from).
    QHash<int, int> lastOfVariation;   // variationIndex -> list index of its last note
    for (int k = 0; k < n; ++k) {
        const int idx = order[k];
        const Note &note = notes[idx];

        const int prevIdx = lastOfVariation.value(note.getVariationIndex(), -1);
        if (prevIdx >= 0 && note.isLegato() && legatoConnects(notes[prevIdx], note)) {
            info[idx].isContinuation = true;
            info[idx].chainFirst = info[prevIdx].chainFirst >= 0
                                   ? info[prevIdx].chainFirst : prevIdx;
        } else {
            info[idx].chainFirst = idx;
        }
        info[idx].chainLast = idx;
        lastOfVariation[note.getVariationIndex()] = idx;
    }

    // A chain ends at its latest member.
    for (int idx = 0; idx < n; ++idx) {
        if (!info[idx].isContinuation) continue;
        const int head = info[idx].chainFirst;
        if (notes[idx].getStartTime() >= notes[info[head].chainLast].getStartTime())
            info[head].chainLast = idx;
    }
    for (int idx = 0; idx < n; ++idx) {
        if (info[idx].isContinuation)
            info[idx].chainLast = info[info[idx].chainFirst].chainLast;
    }

    return info;
}

uint64_t Track::effectiveNoteHash(const QList<Note> &notes, int index)
{
    return effectiveNoteHash(notes, index, computeLegatoChains(notes));
}

uint64_t Track::effectiveNoteHash(const QList<Note> &notes, int index,
                                  const QVector<LegatoChainInfo> &chains)
{
    if (index < 0 || index >= notes.size()) return 0;
    if (index >= chains.size() || chains[index].chainFirst < 0)
        return notes[index].computeHash();

    // Fold in the whole chain: state accumulates across its members, so any
    // edit inside the chain invalidates every member of it. FNV-1a style
    // (same basis as Note::computeHash) — a plain XOR fold would cancel a
    // chain-of-one's own hash down to zero and freeze its cached render.
    const int head = chains[index].chainFirst;
    QVector<int> members;
    for (int i = 0; i < notes.size(); ++i)
        if (chains[i].chainFirst == head) members.append(i);
    std::sort(members.begin(), members.end(), [&notes](int a, int b) {
        return notes[a].getStartTime() < notes[b].getStartTime();
    });

    uint64_t hash = 14695981039346656037ULL;  // FNV offset basis
    const uint64_t prime = 1099511628211ULL;
    for (int m = 0; m < members.size(); ++m) {
        hash ^= notes[members[m]].computeHash();
        hash *= prime;

        // Relative gaps to the neighbours decide whether the connection still
        // holds (the release/tail decisions ride on them). Relative, so
        // dragging the whole chain — a pure time shift — needs no re-render.
        if (m > 0) {
            const double gapPrevMs = (notes[members[m - 1]].getEndTime()
                                      - notes[members[m]].getStartTime()) * 1000.0;
            hash ^= static_cast<uint64_t>(static_cast<qint64>(std::llround(gapPrevMs)));
            hash *= prime;
        }
        if (m + 1 < members.size()) {
            const double gapNextMs = (notes[members[m]].getEndTime()
                                      - notes[members[m + 1]].getStartTime()) * 1000.0;
            hash ^= static_cast<uint64_t>(static_cast<qint64>(std::llround(gapNextMs)));
            hash *= prime;
        }
    }

    return hash;
}

QVector<Track::RenderTask> Track::buildRenderTasks(const QList<Note> &notes,
                                                   const QVector<char> &needsRender,
                                                   const QVector<LegatoChainInfo> &chains)
{
    QVector<RenderTask> tasks;

    // One task per legato chain that has a stale member. The whole chain is
    // re-rendered in order — engine state carries from note to note, so a
    // member cannot be rendered on its own.
    QVector<int> chainsToRender;
    for (int i = 0; i < notes.size(); ++i) {
        if (i >= needsRender.size() || !needsRender[i]) continue;
        const int head = chains[i].chainFirst;
        if (!chainsToRender.contains(head)) chainsToRender.append(head);
    }

    for (int head : chainsToRender) {
        QVector<int> members;
        for (int i = 0; i < notes.size(); ++i)
            if (chains[i].chainFirst == head) members.append(i);
        std::sort(members.begin(), members.end(), [&notes](int a, int b) {
            return notes[a].getStartTime() < notes[b].getStartTime();
        });

        RenderTask task;
        task.noteIndices.reserve(members.size());
        task.noteIds.reserve(members.size());
        task.continuations.reserve(members.size());
        task.results.reserve(members.size());
        for (int idx : members) {
            task.noteIndices.append(idx);
            task.noteIds.append(notes[idx].getId());
            task.continuations.append(chains[idx].isContinuation ? 1 : 0);
            task.results.append(NoteRender());
        }
        tasks.append(task);
    }

    return tasks;
}

bool Track::renderNoteImpl(const Note &note, SounitGraph *graph, NoteRender &outRender,
                            bool isContinuation, bool legatoContinues,
                            double chainStartMs, double chainSpanMs,
                            const Curve *legatoCurve) const
{
    // Calculate note duration in samples
    size_t noteSamples = static_cast<size_t>((note.getDuration() / 1000.0) * m_sampleRate);
    if (noteSamples == 0) {
        return false;
    }

    // Maximum tail: 30 seconds safety cap
    size_t maxTailSamples = static_cast<size_t>(30.0 * m_sampleRate);
    // Silence threshold: ~-60dB (high enough to avoid chasing FFT float residue)
    const float silenceThreshold = 0.001f;
    // Must be silent for 100ms to stop
    size_t silenceRunRequired = static_cast<size_t>(0.1 * m_sampleRate);

    outRender.sampleRate = m_sampleRate;

    // Pre-allocate for note + reasonable tail estimate, will grow if needed
    size_t reserveSize = noteSamples + static_cast<size_t>(m_sampleRate * 5);
    outRender.samples.reserve(reserveSize);
    bool graphHasPan = graph->hasPan();
    if (graphHasPan) {
        outRender.panValues.reserve(reserveSize);
    }

    // Reset graph for this note
    // For legato notes, don't fully reset - preserve K-S state so string keeps vibrating.
    // Only a genuine chain member counts (see computeLegatoChains): a stray
    // `legato` flag with nothing to continue from would otherwise skip the
    // attack entirely - and on models like Karplus-Strong, render silence.
    bool isLegato = isContinuation;
    graph->reset(isLegato);
    // A continuation's progress is the chain's timeline, not a fresh note's.
    graph->setChainTimelineProgress(isContinuation);

    // Chain timeline. Everything the *graph* sees — note progress and the model
    // envelopes driven by it — runs on the legato chain's timeline, so a linked
    // phrase behaves like the one continuous note it used to be: models attack
    // once at the phrase start and release once at its end instead of
    // re-articulating at every member boundary (a recorder fades its breath
    // over the last 5% of *every* note, bowed fires a full ADSR release at 98%).
    // The note's own curves keep using its own 0-1 progress. A standalone note
    // is a chain of one, so this reduces to the note's own timeline.
    const bool haveChainSpan = chainSpanMs > 0.0;
    const double chainOffset = haveChainSpan
        ? (note.getStartTime() - chainStartMs) / chainSpanMs : 0.0;
    const double chainScale = haveChainSpan
        ? (note.getDuration() / chainSpanMs) : 1.0;
    const double envelopeSpanMs = haveChainSpan ? chainSpanMs : note.getDuration();

    // Whether a legato continuation follows this note (skip release/tail).
    // Reported by the caller from the chain info, so it mirrors
    // Track::computeLegatoChains exactly — the old per-note scan here could
    // find a flagged note on another variation and suppress the tail wrongly.
    const bool hasLegatoFollowing = legatoContinues;

    // Skip release envelope when graph has a tail — the reverb/decay handles the fade naturally
    bool graphHasTail = graph->hasTail();

    // Precompute expressive curve names for this note (index 0 = Dynamics).
    // These feed Envelope Engines with followDynamics=on via scoreCurveName match.
    // Without this, the pre-render path sees empty score curves and every
    // non-Dynamics follow falls back to unity, silently ignoring the shape.
    const int expressiveCount = note.getExpressiveCurveCount();
    QStringList scoreCurveNames;
    scoreCurveNames.reserve(expressiveCount);
    scoreCurveNames.append(QStringLiteral("Dynamics"));
    QVector<const Curve*> expressiveCurves;
    expressiveCurves.reserve(expressiveCount - 1);
    for (int ci = 1; ci < expressiveCount; ++ci) {
        scoreCurveNames.append(note.getExpressiveCurveName(ci));
        expressiveCurves.append(&note.getExpressiveCurve(ci));
    }

    // --- Phase 1: Render the note's nominal duration ---

    // Vibrato phase accumulator — needed for time-varying rate (rate envelope)
    double vibratoPhaseAccum = 0.0;
    double organicPhaseAccum = 0.0;

    // Reused across samples: clearing keeps the capacity, so no per-sample
    // heap allocation (this loop runs millions of times per note).
    QVector<double> scoreCurveValues;
    scoreCurveValues.reserve(scoreCurveNames.size());

    for (size_t i = 0; i < noteSamples; ++i) {
        double progress = (noteSamples > 1) ? static_cast<double>(i) / static_cast<double>(noteSamples - 1) : 0.5;
        // The graph runs on the chain's timeline (see chainOffset/chainScale).
        const double graphProgress = std::clamp(chainOffset + progress * chainScale, 0.0, 1.0);

        double pitch = note.getPitchAt(progress);
        // Phrase-wide legato curve: while this note is part of a legato phrase,
        // ONE curve spanning the phrase drives dynamics (evaluated at chain
        // progress), so a per-note swell cannot read as a fresh attack at a
        // joint. Standalone notes keep their own dynamics curve.
        double dynamics = legatoCurve ? legatoCurve->valueAt(graphProgress)
                                      : note.getDynamicsAt(progress);

        // Apply vibrato if active (shared helper so the MIDI bake produces
        // the same continuous vibrato as the audio renderer)
        applyVibrato(note.getVibrato(), progress, 1.0 / m_sampleRate,
                     pitch, dynamics, vibratoPhaseAccum, organicPhaseAccum);

        // Build per-sample expressive curve values to pass to the graph.
        scoreCurveValues.clear();
        scoreCurveValues.append(dynamics);  // index 0: dynamics (with vibrato applied)
        for (const Curve *c : expressiveCurves) {
            double val = (c && !c->isEmpty()) ? c->valueAt(std::min(progress, 1.0)) : 0.5;
            scoreCurveValues.append(val);
        }

        double sample = graph->generateSample(pitch, graphProgress, isLegato, false,
                                              dynamics, scoreCurveValues, scoreCurveNames);

        // Outer envelope on the chain timeline too: exactly one attack at the
        // phrase start and one release at its end, whatever member covers
        // them — the shape the old single merged note had. Capped for long
        // phrases (e.g. legato chains).
        double envAttackMs = std::min(50.0, envelopeSpanMs * 0.05);
        double envReleaseMs = std::min(100.0, envelopeSpanMs * 0.10);
        double attackThreshold = envAttackMs / envelopeSpanMs;
        double releaseThreshold = 1.0 - (envReleaseMs / envelopeSpanMs);
        double envelope = 1.0;
        if (graphProgress < attackThreshold) {
            envelope = graphProgress / attackThreshold;
        } else if (!graphHasTail && graphProgress > releaseThreshold) {
            envelope = (1.0 - graphProgress) / (1.0 - releaseThreshold);
        }

        // Post-render IR: apply convolution after dynamics so reverb is independent of dynamics style.
        float outputSample;
        if (graph->hasPostRenderIR()) {
            // irInput = signal × envelope (before outer dynamics).
            // dryOutput = signal × envelope × dynamics (fully processed dry).
            double irInput   = sample * envelope;
            double dryOutput = irInput * dynamics;
            outputSample = static_cast<float>(graph->processPostIR(irInput, dryOutput, false));
        } else {
            outputSample = static_cast<float>(sample * dynamics * envelope);
        }
        outRender.samples.push_back(outputSample);
        if (graphHasPan) {
            outRender.panValues.push_back(static_cast<float>(graph->getPanValue()));
        }
    }

    // --- Phase 2: Render tail until silence (reverb/decay ring-out) ---
    // Skip tail for legato transitions (next note continues the sound)
    if (!hasLegatoFollowing && graphHasTail) {
        double lastPitch = note.getPitchAt(1.0);
        double lastDynamics = legatoCurve ? legatoCurve->valueAt(1.0)
                                          : note.getDynamicsAt(1.0);
        float lastPan = graphHasPan ? static_cast<float>(graph->getPanValue()) : 0.0f;
        size_t silenceRun = 0;

        // In tail mode, freeze each expressive curve to its final value (curve.valueAt(1.0))
        // so any parameter being driven by the curve carries smoothly into the tail
        // instead of snapping to 1.0 at the note→tail boundary. Matches audioengine.cpp.
        QVector<double> tailCurveValues;
        tailCurveValues.reserve(scoreCurveNames.size());
        tailCurveValues.append(lastDynamics);  // index 0: Dynamics
        for (const Curve *c : expressiveCurves) {
            tailCurveValues.append((c && !c->isEmpty()) ? c->valueAt(1.0) : 0.5);
        }

        for (size_t t = 0; t < maxTailSamples; ++t) {
            double sample = graph->generateSample(lastPitch, 1.0, isLegato, true,
                                                  lastDynamics, tailCurveValues, scoreCurveNames);

            // In tail mode generators are silent (sample ≈ 0); post-render IR rings out
            // via its FDL and tail injection mechanism.
            float outputSample;
            if (graph->hasPostRenderIR()) {
                outputSample = static_cast<float>(
                    graph->processPostIR(sample, sample * lastDynamics, true));
            } else {
                outputSample = static_cast<float>(sample * lastDynamics);
            }

            // Track consecutive silent samples
            if (std::fabs(outputSample) < silenceThreshold) {
                silenceRun++;
                if (silenceRun >= silenceRunRequired) {
                    break;  // Signal has died out
                }
            } else {
                silenceRun = 0;
            }

            outRender.samples.push_back(outputSample);
            if (graphHasPan) {
                outRender.panValues.push_back(lastPan);
            }
        }

        // Apply short fade-out at the very end to prevent any click
        size_t totalSamples = outRender.samples.size();
        size_t fadeLen = std::min(static_cast<size_t>(m_sampleRate * 0.01), totalSamples);  // 10ms fade
        for (size_t i = 0; i < fadeLen; ++i) {
            size_t idx = totalSamples - fadeLen + i;
            float fade = static_cast<float>(fadeLen - i) / static_cast<float>(fadeLen);
            outRender.samples[idx] *= fade;
        }
    }

    // Store hashes for cache validation
    outRender.noteHash = note.computeHash();
    outRender.graphHash = m_graphHash;
    outRender.valid = true;

    return true;
}

bool Track::prerenderNote(const Note &note)
{
    // Get the correct graph based on variation index
    int varIndex = note.getVariationIndex();
    SounitGraph* graph = getGraphForVariation(varIndex);

    if (!graph) {
        qWarning() << "Track" << m_trackId << "cannot render note - no graph for variation" << varIndex;
        return false;
    }

    // MIDI-only graphs produce no audio - nothing to pre-render
    if (graph->hasMidiOut() && !graph->isValid()) {
        qDebug() << "Track" << m_trackId << "note" << note.getId() << "is MIDI-only - no audio render";
        return true;
    }

    if (!graph->isValid()) {
        qWarning() << "Track" << m_trackId << "cannot render note - no valid graph for variation" << varIndex;
        return false;
    }

    QString noteId = note.getId();

    // Check zero duration
    size_t numSamples = static_cast<size_t>((note.getDuration() / 1000.0) * m_sampleRate);
    if (numSamples == 0) {
        qWarning() << "Track" << m_trackId << "note has zero duration:" << noteId;
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        m_noteRenders[noteId].clear();
        return false;
    }

    NoteRender tempRender;
    const QList<Note> &notes = m_notes;
    int noteIndex = -1;
    for (int i = 0; i < notes.size(); ++i) {
        if (notes[i].getId() == noteId) { noteIndex = i; break; }
    }
    const QVector<LegatoChainInfo> chains = computeLegatoChains(notes);
    const bool isContinuation = noteIndex >= 0 && chains[noteIndex].isContinuation;
    // Chain geometry so the graph runs on the chain's timeline
    double chainStartMs = note.getStartTime();
    double chainSpanMs = note.getDuration();
    QVector<int> chainMembers;   // start order, for the phrase curve
    if (noteIndex >= 0 && chains[noteIndex].chainFirst >= 0) {
        const int head = chains[noteIndex].chainFirst;
        const int last = chains[noteIndex].chainLast;
        if (head >= 0 && head < notes.size() && last >= 0 && last < notes.size()) {
            chainStartMs = notes[head].getStartTime();
            chainSpanMs = notes[last].getEndTime() - chainStartMs;
        }
        for (int i = 0; i < notes.size(); ++i)
            if (chains[i].chainFirst == head) chainMembers.append(i);
        std::sort(chainMembers.begin(), chainMembers.end(), [&notes](int a, int b) {
            return notes[a].getStartTime() < notes[b].getStartTime();
        });
    }
    Curve legatoFallback;
    const Curve *legatoCurve =
        effectiveLegatoCurve(notes, chainMembers, legatoFallback);
    const bool legatoContinues = noteIndex >= 0 && chains[noteIndex].chainLast != noteIndex;
    if (!renderNoteImpl(note, graph, tempRender, isContinuation, legatoContinues,
                        chainStartMs, chainSpanMs, legatoCurve)) return false;
    if (noteIndex >= 0)
        tempRender.noteHash = effectiveNoteHash(notes, noteIndex, chains);

    // Swap the rendered buffer into m_noteRenders under mutex protection
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        m_noteRenders[noteId] = std::move(tempRender);
    }

    qDebug() << "Track" << m_trackId << "rendered note" << noteId
             << "duration:" << note.getDuration() << "ms,"
             << numSamples << "samples";

    return true;
}

void Track::cancelRender()
{
    m_cancelRender.store(true);
}

bool Track::prerenderDirtyNotes(double sampleRate)
{
    qDebug() << "Track" << m_trackId << "prerenderDirtyNotes called with" << m_notes.size() << "notes";

    m_sampleRate = sampleRate;

    // Ensure we have a valid graph. A MIDI-only graph (VL70-m without an audio
    // signal chain) is acceptable - it just has nothing to pre-render.
    if (!hasValidGraph()) {
        qDebug() << "Track" << m_trackId << "rebuilding graph before rendering";
        if (!rebuildGraph(sampleRate) && !(m_graph && m_graph->hasMidiOut())) {
            qWarning() << "Track" << m_trackId << "cannot render - graph build failed";
            return false;
        }
    }

    if (m_notes.isEmpty()) {
        qDebug() << "Track" << m_trackId << "no notes to render";
        m_noteRenders.clear();
        return true;
    }

    // If a background render is already running, wait for it to finish.
    // This prevents two render passes from racing on the same track data.
    if (m_rendering.load()) {
        qDebug() << "Track" << m_trackId << "waiting for in-progress background render to finish";
        if (m_renderWatcher && m_renderWatcher->isRunning()) {
            m_renderWatcher->waitForFinished();
        }
    }

    // Snapshot notes under mutex so worker threads never touch m_notes.
    // The snapshot is captured by value in the QtConcurrent lambda (QList is
    // copy-on-write, so the data is shared read-only across threads).
    QList<Note> snapshotNotes;
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        snapshotNotes = m_notes;
    }

    // Collect dirty notes into render tasks.
    // Hold the mutex while reading m_noteRenders to avoid racing the audio callback.
    const QVector<LegatoChainInfo> chains = computeLegatoChains(snapshotNotes);
    QVector<char> needsRender(snapshotNotes.size(), 0);
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        for (int i = 0; i < snapshotNotes.size(); ++i) {
            const Note &note = snapshotNotes[i];
            QString noteId = note.getId();
            // The chain-aware hash: editing any member of a legato chain
            // invalidates every member of it (they share engine state).
            uint64_t noteHash = effectiveNoteHash(snapshotNotes, i, chains);

            bool needed = false;

            if (!m_noteRenders.contains(noteId)) {
                needed = true;
            } else {
                const NoteRender &existing = m_noteRenders[noteId];
                // Sample-rate aware: a render made at the device rate is not
                // valid for an export at the project rate (or vice versa).
                // Mixing it anyway time-compresses the audio (the "shorter
                // export" bug: 44.1 kHz renders on a 48 kHz timeline).
                if (!existing.valid || existing.graphHash != m_graphHash ||
                    existing.noteHash != noteHash ||
                    existing.sampleRate != sampleRate) {
                    needed = true;
                }
            }

            // MIDI-only graphs need no audio render (events are baked at playback)
            if (needed) {
                SounitGraph *noteGraph = getGraphForVariation(snapshotNotes[i].getVariationIndex());
                if (noteGraph && noteGraph->hasMidiOut() && !noteGraph->isValid()) {
                    needed = false;
                }
            }

            needsRender[i] = needed ? 1 : 0;
        }
    }

    // One task per stale legato chain: the whole chain re-renders in order.
    QVector<RenderTask> tasks = buildRenderTasks(snapshotNotes, needsRender, chains);

    qDebug() << "Track" << m_trackId << tasks.size() << "render task(s) covering"
             << m_notes.size() << "notes";

    if (tasks.isEmpty()) {
        emit renderStarted();
        emit renderProgressChanged(100);
        emit renderCompleted();
        return true;
    }

    m_cancelRender.store(false);
    m_rendering.store(true);
    emit renderStarted();

    // Render in parallel using thread pool — each thread gets its own cloned graph
    std::atomic<int> completedCount{0};

    QFuture<void> future = QtConcurrent::map(tasks, [this, &completedCount, &snapshotNotes, &chains](RenderTask &task) {
        if (m_cancelRender.load() || task.noteIndices.isEmpty()) return;

        const Note &firstNote = snapshotNotes[task.noteIndices.first()];
        SounitGraph *srcGraph = getGraphForVariation(firstNote.getVariationIndex());
        if (!srcGraph || !srcGraph->isValid()) return;

        QElapsedTimer timer;
        timer.start();
        SounitGraph *graph = srcGraph->clone();
        // Members render in order on the same graph: reset(isLegato) preserves
        // the engine state, so the chain flows as one continuous phrase and
        // each note still owns its buffer.
        // The graph runs on the chain's timeline (members are in start order,
        // so first/last give the span): models that ramp their envelope on
        // note progress attack once at the phrase start and release once at
        // its end, however many notes the chain holds.
        const double chainStartMs = snapshotNotes[task.noteIndices.first()].getStartTime();
        const double chainSpanMs =
            snapshotNotes[task.noteIndices.last()].getEndTime() - chainStartMs;

        // The phrase curve for this chain (nullptr for a standalone note):
        // a stored legato curve wins, else the interpolated default derived
        // from the members' own shapes.
        Curve legatoFallback;
        const Curve *legatoCurve =
            effectiveLegatoCurve(snapshotNotes, task.noteIndices, legatoFallback);

        for (int m = 0; m < task.noteIndices.size(); ++m) {
            const int noteIdx = task.noteIndices[m];
            const Note &note = snapshotNotes[noteIdx];
            // A continuation following this member defers the release to the
            // chain's last member (mirrors computeLegatoChains).
            const bool legatoContinues = chains[noteIdx].chainLast != noteIdx;
            renderNoteImpl(note, graph, task.results[m],
                           task.continuations[m] != 0, legatoContinues,
                           chainStartMs, chainSpanMs, legatoCurve);
            task.results[m].noteHash =
                effectiveNoteHash(snapshotNotes, noteIdx, chains);
        }
        delete graph;
        task.success = true;
        task.renderMs = timer.nsecsElapsed() / 1000000;

        completedCount.fetch_add(1);
    });

    // Block until complete — no processEvents() to avoid re-entrancy
    // (track destruction during event processing would leave worker threads
    // with a dangling `this`).
    future.waitForFinished();

    // Handle cancellation
    if (m_cancelRender.load()) {
        m_rendering.store(false);
        emit renderCancelled();
        return false;
    }

    // Diagnostics: per-task render times, total samples, and curve point
    // counts. Renders scale with curve points (stretch-wide workflow), so
    // keeping these visible makes future slow-render reports quick to triage.
    {
        qint64 totalMs = 0;
        qint64 minMs = 0x7FFFFFFFFFFFFFFFLL;
        qint64 maxMs = 0;
        qint64 totalSamples = 0;
        int succeeded = 0;
        int maxPitchPts = 0;
        int maxDynPts = 0;
        int maxExpPts = 0;
        long long sumDynPts = 0;
        for (const RenderTask &task : tasks) {
            if (!task.success) continue;
            totalMs += task.renderMs;
            minMs = std::min(minMs, task.renderMs);
            maxMs = std::max(maxMs, task.renderMs);
            for (int m = 0; m < task.noteIndices.size(); ++m) {
                if (!task.results[m].valid) continue;
                totalSamples += task.results[m].samples.size();
                ++succeeded;

                const Note &note = snapshotNotes[task.noteIndices[m]];
                int pitchPts = note.getPitchCurve().getPoints().size();
                int dynPts = note.getDynamicsCurve().getPoints().size();
                int expPts = 0;
                for (int ci = 1; ci < note.getExpressiveCurveCount(); ++ci) {
                    expPts += note.getExpressiveCurve(ci).getPoints().size();
                }
                maxPitchPts = std::max(maxPitchPts, pitchPts);
                maxDynPts = std::max(maxDynPts, dynPts);
                maxExpPts = std::max(maxExpPts, expPts);
                sumDynPts += dynPts;
            }
        }
        qDebug() << "Track" << m_trackId << "render timing:" << succeeded << "notes in" << totalMs << "ms"
                 << "(avg" << (succeeded ? totalMs / succeeded : 0) << "ms, min" << minMs
                 << "ms, max" << maxMs << "ms);" << totalSamples << "samples =>"
                 << (totalMs > 0 ? static_cast<double>(totalSamples) * 1000.0 / totalMs : 0.0)
                 << "samples/sec";
        qDebug() << "Track" << m_trackId << "curve points: max pitch" << maxPitchPts
                 << "max dynamics" << maxDynPts << "(avg" << (succeeded ? sumDynPts / succeeded : 0)
                 << ") max expressive" << maxExpPts;
    }

    // Store results under mutex
    int renderedCount = 0;
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        for (RenderTask &task : tasks) {
            if (!task.success) continue;
            for (int m = 0; m < task.noteIndices.size(); ++m) {
                if (!task.results[m].valid) continue;  // member failed to render
                m_noteRenders[task.noteIds[m]] = std::move(task.results[m]);
                renderedCount++;
            }
        }
    }

    // Clean up renders for deleted notes
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        QList<QString> renderIds = m_noteRenders.keys();
        QSet<QString> currentNoteIds;
        for (const Note &note : m_notes) {
            currentNoteIds.insert(note.getId());
        }

        for (const QString &renderId : renderIds) {
            if (!currentNoteIds.contains(renderId)) {
                m_noteRenders.remove(renderId);
                qDebug() << "Track" << m_trackId << "removed orphan render:" << renderId;
            }
        }
    }

    qDebug() << "Track" << m_trackId << "rendered" << renderedCount << "notes,"
             << m_noteRenders.size() << "total renders cached";

    m_rendering.store(false);
    emit renderCompleted();

    return true;
}

void Track::invalidateNoteRender(const QString &noteId)
{
    // Lock mutex when modifying m_noteRenders and m_notes
    std::lock_guard<std::mutex> lock(m_playbackMutex);

    if (m_noteRenders.contains(noteId)) {
        m_noteRenders[noteId].valid = false;
        qDebug() << "Track" << m_trackId << "invalidated render for note:" << noteId;
    }

    // Also mark the note as render dirty
    for (Note &note : m_notes) {
        if (note.getId() == noteId) {
            note.setRenderDirty(true);
            break;
        }
    }
}

bool Track::startBackgroundRender(double sampleRate)
{
    if (m_rendering.load()) return false;  // Already rendering

    m_sampleRate = sampleRate;

    // MIDI-only graphs are acceptable here too (they just skip audio rendering)
    if (!hasValidGraph()) {
        if (!rebuildGraph(sampleRate) && !(m_graph && m_graph->hasMidiOut())) return false;
    }

    if (m_notes.isEmpty()) {
        m_noteRenders.clear();
        return false;  // No render started — don't increment caller's counter
    }

    // Snapshot notes under mutex so worker threads never touch m_notes
    QList<Note> snapshotNotes;
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        snapshotNotes = m_notes;
    }

    // Collect dirty notes.
    // Hold the mutex while reading m_noteRenders to avoid racing the audio callback.
    m_backgroundTasks.clear();
    const QVector<LegatoChainInfo> chains = computeLegatoChains(snapshotNotes);
    QVector<char> needsRender(snapshotNotes.size(), 0);
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        for (int i = 0; i < snapshotNotes.size(); ++i) {
            const Note &note = snapshotNotes[i];
            QString noteId = note.getId();
            // The chain-aware hash: editing any member of a legato chain
            // invalidates every member of it (they share engine state).
            uint64_t noteHash = effectiveNoteHash(snapshotNotes, i, chains);

            bool needed = false;
            if (!m_noteRenders.contains(noteId)) {
                needed = true;
            } else {
                const NoteRender &existing = m_noteRenders[noteId];
                // Sample-rate aware: a render made at the device rate is not
                // valid for an export at the project rate (or vice versa).
                // Mixing it anyway time-compresses the audio (the "shorter
                // export" bug: 44.1 kHz renders on a 48 kHz timeline).
                if (!existing.valid || existing.graphHash != m_graphHash ||
                    existing.noteHash != noteHash ||
                    existing.sampleRate != sampleRate) {
                    needed = true;
                }
            }

            // MIDI-only graphs need no audio render (events are baked at playback)
            if (needed) {
                SounitGraph *noteGraph = getGraphForVariation(snapshotNotes[i].getVariationIndex());
                if (noteGraph && noteGraph->hasMidiOut() && !noteGraph->isValid()) {
                    needed = false;
                }
            }

            needsRender[i] = needed ? 1 : 0;
        }
    }

    // One task per stale legato chain: the whole chain re-renders in order.
    m_backgroundTasks = buildRenderTasks(snapshotNotes, needsRender, chains);

    if (m_backgroundTasks.isEmpty()) {
        return false;  // Nothing dirty — no render started, don't increment caller's counter
    }

    m_cancelRender.store(false);
    m_rendering.store(true);
    emit renderStarted();

    QFuture<void> future = QtConcurrent::map(m_backgroundTasks, [this, snapshotNotes, chains](RenderTask &task) {
        if (m_cancelRender.load() || task.noteIndices.isEmpty()) return;

        const Note &firstNote = snapshotNotes[task.noteIndices.first()];
        SounitGraph *srcGraph = getGraphForVariation(firstNote.getVariationIndex());
        if (!srcGraph || !srcGraph->isValid()) return;

        SounitGraph *graph = srcGraph->clone();
        // Members render in order on the same graph: reset(isLegato) preserves
        // the engine state, so the chain flows as one continuous phrase and
        // each note still owns its buffer.
        // The graph runs on the chain's timeline (members are in start order,
        // so first/last give the span): models that ramp their envelope on
        // note progress attack once at the phrase start and release once at
        // its end, however many notes the chain holds.
        const double chainStartMs = snapshotNotes[task.noteIndices.first()].getStartTime();
        const double chainSpanMs =
            snapshotNotes[task.noteIndices.last()].getEndTime() - chainStartMs;

        // The phrase curve for this chain (nullptr for a standalone note):
        // a stored legato curve wins, else the interpolated default derived
        // from the members' own shapes.
        Curve legatoFallback;
        const Curve *legatoCurve =
            effectiveLegatoCurve(snapshotNotes, task.noteIndices, legatoFallback);

        for (int m = 0; m < task.noteIndices.size(); ++m) {
            const int noteIdx = task.noteIndices[m];
            const Note &note = snapshotNotes[noteIdx];
            // A continuation following this member defers the release to the
            // chain's last member (mirrors computeLegatoChains).
            const bool legatoContinues = chains[noteIdx].chainLast != noteIdx;
            renderNoteImpl(note, graph, task.results[m],
                           task.continuations[m] != 0, legatoContinues,
                           chainStartMs, chainSpanMs, legatoCurve);
            task.results[m].noteHash =
                effectiveNoteHash(snapshotNotes, noteIdx, chains);
        }
        delete graph;
        task.success = true;
    });

    m_renderWatcher->setFuture(future);  // Returns immediately
    return true;
}

void Track::onRenderWatcherFinished()
{
    if (m_cancelRender.load()) {
        m_rendering.store(false);
        m_backgroundTasks.clear();
        emit renderCancelled();
        return;
    }

    // Store results under mutex
    int renderedCount = 0;
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        for (RenderTask &task : m_backgroundTasks) {
            if (!task.success) continue;
            for (int m = 0; m < task.noteIndices.size(); ++m) {
                if (!task.results[m].valid) continue;  // member failed to render
                m_noteRenders[task.noteIds[m]] = std::move(task.results[m]);
                renderedCount++;
            }
        }
    }

    // Clean up renders for deleted notes
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        QList<QString> renderIds = m_noteRenders.keys();
        QSet<QString> currentNoteIds;
        for (const Note &note : m_notes) {
            currentNoteIds.insert(note.getId());
        }
        for (const QString &renderId : renderIds) {
            if (!currentNoteIds.contains(renderId)) {
                m_noteRenders.remove(renderId);
            }
        }
    }

    qDebug() << "Track" << m_trackId << "(background) rendered" << renderedCount
             << "notes," << m_noteRenders.size() << "total renders cached";

    m_backgroundTasks.clear();
    m_rendering.store(false);
    emit renderCompleted();
}

void Track::invalidateAllNoteRenders()
{
    qDebug() << "Track" << m_trackId << "invalidating all" << m_noteRenders.size() << "note renders";

    // Lock mutex when modifying m_noteRenders and m_notes
    std::lock_guard<std::mutex> lock(m_playbackMutex);

    for (auto it = m_noteRenders.begin(); it != m_noteRenders.end(); ++it) {
        it.value().valid = false;
    }

    // Mark all notes as render dirty
    for (Note &note : m_notes) {
        note.setRenderDirty(true);
    }

    // NOTE: We do NOT delete m_graph here. The base sounit graph should remain
    // unchanged - it represents the original loaded sounit. User edits are captured
    // in variations, not in the base graph.

    // Increment graph hash to ensure re-render
    m_graphHash++;

    emit renderCacheInvalidated();
}

const NoteRender* Track::getNoteRender(const QString &noteId) const
{
    auto it = m_noteRenders.find(noteId);
    if (it != m_noteRenders.end()) {
        return &it.value();
    }
    return nullptr;
}

void Track::setNoteRender(const QString &noteId, const NoteRender &render)
{
    // Lock mutex when modifying m_noteRenders
    std::lock_guard<std::mutex> lock(m_playbackMutex);
    m_noteRenders[noteId] = render;
    qDebug() << "Track" << m_trackId << "loaded cached render for note" << noteId
             << "with" << render.samples.size() << "samples";
}

double Track::cachedRenderSampleRate() const
{
    std::lock_guard<std::mutex> lock(m_playbackMutex);
    for (auto it = m_noteRenders.begin(); it != m_noteRenders.end(); ++it) {
        if (it.value().valid && !it.value().samples.empty() && it.value().sampleRate > 0.0) {
            return it.value().sampleRate;
        }
    }
    return 0.0;
}

bool Track::hasRenderWork() const
{
    for (const Note &note : m_notes) {
        if (note.isRenderDirty()) {
            return true;
        }

        QString noteId = note.getId();
        if (!m_noteRenders.contains(noteId)) {
            return true;
        }

        const NoteRender &render = m_noteRenders[noteId];
        if (!render.valid || render.graphHash != m_graphHash) {
            return true;
        }
    }

    return false;
}

std::vector<float> Track::getMixedBuffer(double startTimeMs, double durationMs)
{
    size_t numSamples = static_cast<size_t>((durationMs / 1000.0) * m_sampleRate);
    // Stereo interleaved: [L0, R0, L1, R1, ...]
    std::vector<float> buffer(numSamples * 2, 0.0f);

    if (m_muted || numSamples == 0) {
        return buffer;
    }

    double endTimeMs = startTimeMs + durationMs;

    // Lock mutex to prevent race conditions with UI thread syncing notes
    std::lock_guard<std::mutex> lock(m_playbackMutex);

    // String damping is per-variation: a fretted-KS graph in one variation
    // should only damp earlier notes routed through the same variation,
    // not notes plucked on a different sounit on the same track.
    auto canonicalVarIdx = [this](int idx) -> int {
        if (idx == 0) return 0;
        int li = idx - 1;
        if (li >= 0 && li < m_variations.size()
            && m_variations[li]->compiledGraph
            && m_variations[li]->compiledGraph->isValid()) {
            return idx;
        }
        return 0;
    };
    auto graphForCanonical = [this](int idx) -> SounitGraph* {
        return idx == 0 ? m_graph : m_variations[idx - 1]->compiledGraph;
    };

    QHash<int, bool> dampsByVariation;
    // Every note's start per variation, as (startTimeMs, pitchHz). Feeds the
    // "one string per note" rule: a same-pitch repeat silences the still
    // ringing copy before its own attack — in harp mode AND fretted mode.
    QHash<int, std::vector<std::pair<double,double>>> noteStartsByVariation;
    // Fretted note starts per variation. A new fretted pluck damps the whole
    // string (monophonic cut at any pitch).
    QHash<int, std::vector<std::pair<double,double>>> dampStartsByVariation;
    // A legato continuation is not a new strike — it keeps the same string
    // ringing, so it must not damp the note it continues. Skipping chain
    // continuations here is what the old single merged note gave for free.
    const QVector<LegatoChainInfo> chains = computeLegatoChains(m_notes);
    for (int noteIdx = 0; noteIdx < m_notes.size(); ++noteIdx) {
        if (chains[noteIdx].isContinuation) continue;
        const Note &note = m_notes[noteIdx];
        int varIdx = canonicalVarIdx(note.getVariationIndex());
        noteStartsByVariation[varIdx].push_back({note.getStartTime(), note.getPitchHz()});
        auto dIt = dampsByVariation.find(varIdx);
        if (dIt == dampsByVariation.end()) {
            SounitGraph* g = graphForCanonical(varIdx);
            dIt = dampsByVariation.insert(varIdx, g && g->hasStringDamping());
        }

        // Per-note fretted check via named expressive curve "fretted".
        // When hasStringDamping() is true, this allows individual notes to
        // opt out of damping (curve value < 0.5 = open/ringing).
        // When hasStringDamping() is false, a note can still opt IN
        // (curve value >= 0.5 = fretted).
        bool noteIsFretted = dIt.value();  // default: graph-level setting
        int curveIdx = note.findExpressiveCurveIndexByName("fretted");
        if (curveIdx >= 1) {
            const Curve &c = note.getExpressiveCurve(curveIdx);
            noteIsFretted = c.valueAt(0.0) >= 0.5;
        }
        if (noteIsFretted) {
            dampStartsByVariation[varIdx].push_back({note.getStartTime(), note.getPitchHz()});
        }
    }
    auto sortStarts = [](std::vector<std::pair<double,double>> &v) {
        std::sort(v.begin(), v.end(),
                  [](const std::pair<double,double> &a, const std::pair<double,double> &b) {
                      return a.first < b.first;
                  });
    };
    for (auto it = noteStartsByVariation.begin(); it != noteStartsByVariation.end(); ++it) {
        sortStarts(it.value());
    }
    for (auto it = dampStartsByVariation.begin(); it != dampStartsByVariation.end(); ++it) {
        sortStarts(it.value());
    }

    // String damping fade-out duration (5ms = fast but click-free)
    static constexpr double DAMP_FADE_MS = 5.0;

    for (const Note &note : m_notes) {
        // Get this note's render
        QString noteId = note.getId();
        if (!m_noteRenders.contains(noteId)) {
            continue;  // No render available
        }

        const NoteRender &render = m_noteRenders[noteId];
        if (!render.valid || render.samples.empty()) {
            continue;  // Invalid render
        }

        bool hasPan = !render.panValues.empty();

        // Use actual render length (includes tail for reverb/decay)
        double noteStartMs = note.getStartTime();
        double renderDurationMs = (static_cast<double>(render.samples.size()) / render.sampleRate) * 1000.0;
        double noteEndMs = noteStartMs + renderDurationMs;

        if (noteEndMs <= startTimeMs || noteStartMs >= endTimeMs) {
            continue;  // Note doesn't overlap requested range
        }

        // String damping: find the next fretted note's start after this note
        // starts. That's when the string gets re-plucked and this note should
        // be damped. Scoped to this note's variation so notes on other
        // sounits aren't damped.
        double dampTimeMs = -1.0;
        auto sIt = dampStartsByVariation.find(canonicalVarIdx(note.getVariationIndex()));
        if (sIt != dampStartsByVariation.end()) {
            const std::vector<std::pair<double,double>> &starts = sIt.value();
            auto upIt = std::upper_bound(starts.begin(), starts.end(), noteStartMs,
                                         [](double val, const std::pair<double,double> &p) {
                                             return val < p.first;
                                         });
            if (upIt != starts.end()) dampTimeMs = upIt->first;
        }

        // Same-pitch repeat: the first later note (fretted or not) at the same
        // pitch (~1 cent tolerance). One string can't ring twice at once, so
        // this note's tail gets a fade that ENDS at the repeat's start — in
        // harp mode too, where the fretted list is empty.
        double samePitchDampTimeMs = -1.0;
        auto aIt = noteStartsByVariation.find(canonicalVarIdx(note.getVariationIndex()));
        if (aIt != noteStartsByVariation.end()) {
            const std::vector<std::pair<double,double>> &starts = aIt.value();
            auto upIt = std::upper_bound(starts.begin(), starts.end(), noteStartMs,
                                         [](double val, const std::pair<double,double> &p) {
                                             return val < p.first;
                                         });
            const double pitchHz = note.getPitchHz();
            for (auto spIt = upIt; spIt != starts.end(); ++spIt) {
                if (std::abs(spIt->second - pitchHz) <= pitchHz * 0.0006) {
                    samePitchDampTimeMs = spIt->first;
                    break;
                }
            }
        }

        // Mix this note's samples into the stereo buffer
        for (size_t i = 0; i < numSamples; ++i) {
            // Calculate absolute time for this sample
            double timeMs = startTimeMs + (static_cast<double>(i) / m_sampleRate) * 1000.0;

            // Calculate time offset within the note
            double noteOffsetMs = timeMs - noteStartMs;

            // Check if this sample is within the render's length (note + tail)
            if (noteOffsetMs >= 0 && noteOffsetMs < renderDurationMs) {
                // Calculate index into note's render buffer
                size_t noteIdx = static_cast<size_t>((noteOffsetMs / 1000.0) * render.sampleRate);

                if (noteIdx < render.samples.size()) {
                    float sample = render.samples[noteIdx];

                    // Apply string damping fade-out if a later note cuts this one.
                    // Same-pitch repeats fade BEFORE the re-pluck (the old ring
                    // must be silent when the new attack sounds); different-pitch
                    // successors fade after it (the string is damped as struck).
                    double dampGain = 1.0;
                    bool fullyDamped = false;
                    if (samePitchDampTimeMs > 0 && timeMs >= samePitchDampTimeMs - DAMP_FADE_MS) {
                        if (timeMs >= samePitchDampTimeMs) {
                            fullyDamped = true;
                        } else {
                            // Cosine fade that ENDS at the repeat's start
                            double fadeRemaining = samePitchDampTimeMs - timeMs;
                            dampGain *= 0.5 * (1.0 - std::cos(fadeRemaining / DAMP_FADE_MS * M_PI));
                        }
                    }
                    if (!fullyDamped && dampTimeMs > 0 && timeMs >= dampTimeMs) {
                        double fadeElapsed = timeMs - dampTimeMs;
                        if (fadeElapsed >= DAMP_FADE_MS) {
                            fullyDamped = true;
                        } else {
                            dampGain *= 0.5 * (1.0 + std::cos(fadeElapsed / DAMP_FADE_MS * M_PI));
                        }
                    }
                    if (fullyDamped) {
                        continue;  // Fully damped, skip
                    }
                    if (dampGain < 1.0) {
                        sample *= static_cast<float>(dampGain);
                    }

                    // Apply per-note pan (from Pan container in sounit graph)
                    float pan = (hasPan && noteIdx < render.panValues.size())
                                ? render.panValues[noteIdx] : 0.0f;
                    float leftGain  = std::min(1.0f, 1.0f - pan);
                    float rightGain = std::min(1.0f, 1.0f + pan);

                    buffer[i * 2]     += sample * leftGain;
                    buffer[i * 2 + 1] += sample * rightGain;
                }
            }
        }
    }

    // Mix the baked recording clip (Phase 9). The clip rides the track's
    // gain/volume applied below and mutes with the track via the early
    // return at the top. The PCM cache is prepared on the UI thread
    // (prepareClipForPlayback) - the audio callback only indexes into it.
    if (m_hasClip && m_clip.enabled && !m_clip.liveMidi && m_clipLoaded && !m_clipPcm.empty()) {
        const double clipStart = clipOffsetMs();
        const size_t clipFrames = m_clipPcm.size() / 2;
        const double clipEnd = clipStart + (static_cast<double>(clipFrames) / m_sampleRate) * 1000.0;
        if (startTimeMs < clipEnd && endTimeMs > clipStart) {
            for (size_t i = 0; i < numSamples; ++i) {
                const double timeMs = startTimeMs + (static_cast<double>(i) / m_sampleRate) * 1000.0;
                const double clipMs = timeMs - clipStart;
                if (clipMs < 0.0) continue;
                const size_t idx = static_cast<size_t>((clipMs / 1000.0) * m_sampleRate);
                if (idx >= clipFrames) break;
                buffer[i * 2]     += m_clipPcm[idx * 2]     * m_clip.gain;
                buffer[i * 2 + 1] += m_clipPcm[idx * 2 + 1] * m_clip.gain;
            }
        }
    }

    // Apply track gain and volume, then clamp
    float totalGain = m_gain * m_volume;
    for (float &s : buffer) {
        s = std::clamp(s * totalGain, -1.0f, 1.0f);
    }

    return buffer;
}

double Track::getRenderedEndTimeMs() const
{
    std::lock_guard<std::mutex> lock(m_playbackMutex);
    double endMs = 0.0;

    for (const Note &note : m_notes) {
        double noteStartMs = note.getStartTime();
        double noteEndMs = noteStartMs + note.getDuration();

        // Check if this note has a rendered buffer (which includes tail)
        QString noteId = note.getId();
        if (m_noteRenders.contains(noteId)) {
            const NoteRender &render = m_noteRenders[noteId];
            if (render.valid && !render.samples.empty()) {
                double renderEndMs = noteStartMs +
                    (static_cast<double>(render.samples.size()) / render.sampleRate) * 1000.0;
                noteEndMs = std::max(noteEndMs, renderEndMs);
            }
        }

        if (noteEndMs > endMs) {
            endMs = noteEndMs;
        }
    }

    // Recording clip extends the track's audible length (Phase 9)
    if (m_hasClip && m_clip.enabled && !m_clip.liveMidi) {
        const double clipEnd = getClipEndTimeMs();
        if (clipEnd > endMs) endMs = clipEnd;
    }

    return endMs;
}

// ========== Recording Clip (Phase 9: bake to audio) ==========

void Track::setClip(const TrackClip &clip)
{
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        m_clip = clip;
        m_hasClip = true;
        m_clipLoaded = false;
        m_clipPcmRate = 0;
        m_clipPcm.clear();
    }
    markNotesDirty();  // clip is composition data - prompts for save
    emit clipChanged();
}

void Track::clearClip()
{
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        if (!m_hasClip) return;
        m_hasClip = false;
        m_clipLoaded = false;
        m_clipPcmRate = 0;
        m_clipPcm.clear();
        m_clip = TrackClip();
    }
    markNotesDirty();
    emit clipChanged();
}

void Track::setClipLiveMidi(bool live)
{
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        if (!m_hasClip || m_clip.liveMidi == live) return;
        m_clip.liveMidi = live;
    }
    markNotesDirty();
    emit clipChanged();
}

void Track::setClipNudgeMs(double ms)
{
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        if (!m_hasClip || m_clip.manualNudgeMs == ms) return;
        m_clip.manualNudgeMs = ms;
    }
    markNotesDirty();
}

double Track::getClipEndTimeMs() const
{
    if (!m_hasClip || !m_clip.enabled || m_clip.liveMidi) return 0.0;
    return clipOffsetMs() + m_clip.durationMs;
}

bool Track::prepareClipForPlayback()
{
    if (!m_hasClip || m_clip.filePath.isEmpty()) return true;  // nothing to prepare
    // The cache must match the rate it will be consumed at: live playback
    // and export can run the track at different rates (e.g. 44.1 kHz device
    // vs 48 kHz export), and mixing a cache at the wrong rate time-compresses
    // the clip (the "19 s shorter" export bug).
    if (m_clipLoaded && m_clipPcmRate == m_sampleRate) return true;

    // UI-thread only (play/export setup, never the audio callback), but the
    // cache below is read by the callback - take the playback mutex.
    std::lock_guard<std::mutex> lock(m_playbackMutex);
    if (m_clipLoaded && m_clipPcmRate == m_sampleRate) return true;

    m_clipPcm.clear();
    m_clipLoaded = false;

    drwav wav;
    if (!drwav_init_file(&wav, m_clip.filePath.toLocal8Bit().constData(), nullptr)) {
        qWarning() << "Track" << m_name << ": cannot open clip" << m_clip.filePath;
        return false;
    }
    const drwav_uint64 fileFrames = wav.totalPCMFrameCount;
    const unsigned int fileChannels = wav.channels;
    const double fileRate = wav.sampleRate > 0 ? static_cast<double>(wav.sampleRate) : 48000.0;
    if (fileFrames == 0 || fileChannels < 1 || fileChannels > 2) {
        qWarning() << "Track" << m_name << ": clip unreadable or unsupported channels";
        drwav_uninit(&wav);
        return false;
    }
    std::vector<float> raw(static_cast<size_t>(fileFrames) * fileChannels);
    const drwav_uint64 readFrames = drwav_read_pcm_frames_f32(&wav, fileFrames, raw.data());
    drwav_uninit(&wav);
    if (readFrames == 0) {
        qWarning() << "Track" << m_name << ": clip read failed";
        return false;
    }

    // Drop the pre-roll ahead of the measured onset (Phase 9 bake): the
    // recording starts before the transport and may contain a stream-open
    // click. Trimmed away, the clip's offsetMs points straight at the
    // first note. onsetMs == 0 (old clips, or no onset found) trims nothing.
    const drwav_uint64 startFrame = static_cast<drwav_uint64>(
        (std::max(0.0, m_clip.onsetMs) / 1000.0) * fileRate);
    if (startFrame >= readFrames) {
        qWarning() << "Track" << m_name << ": clip onset trim beyond recording - clip is silent";
        m_clipPcm.clear();
        m_clipLoaded = true;
        m_clipPcmRate = m_sampleRate;
        return true;
    }
    const float *base = raw.data() + static_cast<size_t>(startFrame) * fileChannels;
    const size_t usedFrames = static_cast<size_t>(readFrames - startFrame);

    // Upmix mono to stereo and linear-resample to this track's rate in one pass.
    const double ratio = m_sampleRate / fileRate;
    const size_t outFrames = static_cast<size_t>(static_cast<double>(usedFrames) * ratio) + 1;
    m_clipPcm.assign(outFrames * 2, 0.0f);
    for (size_t i = 0; i < outFrames; ++i) {
        const double srcPos = static_cast<double>(i) / ratio;
        const size_t s0 = static_cast<size_t>(srcPos);
        const size_t s1 = std::min(s0 + 1, usedFrames - 1);
        const double frac = srcPos - static_cast<double>(s0);
        for (int ch = 0; ch < 2; ++ch) {
            const size_t sch = (fileChannels == 2) ? ch : 0;
            const float a = base[s0 * fileChannels + sch];
            const float b = base[s1 * fileChannels + sch];
            m_clipPcm[i * 2 + ch] = a + static_cast<float>(frac) * (b - a);
        }
    }
    m_clipLoaded = true;
    m_clipPcmRate = m_sampleRate;
    return true;
}

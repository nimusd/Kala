#include "track.h"
#include "audioengine.h"
#include "vibrato.h"
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
#include <algorithm>
#include <QUuid>

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

    qDebug() << "Track created: ID" << m_trackId << "Name:" << m_name;
}

Track::~Track()
{
    qDebug() << "Track destroyed: ID" << m_trackId << "Name:" << m_name;

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

        // Render each sample of the note
        for (size_t i = 0; i < noteDurationSamples; ++i) {
            // Calculate note progress (0.0 to 1.0)
            double noteProgress = static_cast<double>(i) / static_cast<double>(noteDurationSamples - 1);
            if (noteDurationSamples == 1) noteProgress = 0.5;

            // Get pitch and dynamics at this point in the note
            double pitch = note.getPitchAt(noteProgress);
            double dynamics = note.getDynamicsAt(noteProgress);

            // Generate audio sample
            double sample = m_graph->generateSample(pitch, noteProgress, false, false, dynamics);

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
    m_sounitName = json["sounitName"].toString();
    QString sounitPath = json["sounitFilePath"].toString();
    if (!sounitPath.isEmpty()) {
        if (!loadSounit(sounitPath)) {
            qWarning() << "Track" << m_trackId << "failed to load referenced sounit:" << sounitPath;
            // Continue anyway - track can exist without sounit
        }
    }

    // Read playback state
    setMuted(json["muted"].toBool(false));
    setSolo(json["solo"].toBool(false));
    setVolume(static_cast<float>(json["volume"].toDouble(1.0)));
    setGain(static_cast<float>(json["gain"].toDouble(1.0)));
    setPan(static_cast<float>(json["pan"].toDouble(0.0)));

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

bool Track::renderNoteImpl(const Note &note, SounitGraph *graph, NoteRender &outRender) const
{
    QString noteId = note.getId();

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
    outRender.samples.reserve(noteSamples + static_cast<size_t>(m_sampleRate * 5));

    // Reset graph for this note
    // For legato notes, don't fully reset - preserve K-S state so string keeps vibrating
    bool isLegato = note.isLegato();
    graph->reset(isLegato);

    // Check if a legato note follows this one (to skip release)
    bool hasLegatoFollowing = false;
    double noteEndTime = note.getStartTime() + note.getDuration();
    for (const Note &otherNote : m_notes) {
        if (otherNote.getId() != noteId && otherNote.isLegato()) {
            double overlapTolerance = std::max(200.0, note.getDuration() * 0.1);
            double timeDiff = noteEndTime - otherNote.getStartTime();
            if (timeDiff > -50.0 && timeDiff < overlapTolerance) {
                hasLegatoFollowing = true;
                break;
            }
        }
    }

    bool skipAttack = note.isLegato();
    // Skip release envelope when graph has a tail — the reverb/decay handles the fade naturally
    bool graphHasTail = graph->hasTail();
    bool skipRelease = hasLegatoFollowing || graphHasTail;

    // --- Phase 1: Render the note's nominal duration ---
    for (size_t i = 0; i < noteSamples; ++i) {
        double progress = (noteSamples > 1) ? static_cast<double>(i) / static_cast<double>(noteSamples - 1) : 0.5;

        double pitch = note.getPitchAt(progress);
        double dynamics = note.getDynamicsAt(progress);

        // Apply vibrato if active
        const Vibrato &vibrato = note.getVibrato();
        if (vibrato.active && progress >= vibrato.onset) {
            double vibratoProgress = (progress - vibrato.onset) / (1.0 - vibrato.onset);
            double envelopeIntensity = vibrato.envelopeAt(vibratoProgress);
            double noteTimeSeconds = (note.getDuration() / 1000.0) * progress;
            double vibratoPhase = noteTimeSeconds * vibrato.rate * 2.0 * M_PI;

            double organicMod = 1.0;
            if (vibrato.regularity > 0.0) {
                double slowPhase = noteTimeSeconds * vibrato.rate * 0.23 * 2.0 * M_PI;
                organicMod = 1.0 + vibrato.regularity * 0.3 * std::sin(slowPhase);
                vibratoPhase += vibrato.regularity * 0.5 * std::sin(slowPhase * 0.7);
            }

            double lfoValue = std::sin(vibratoPhase) * organicMod;
            pitch *= 1.0 + (lfoValue * vibrato.pitchDepth * envelopeIntensity);
            dynamics *= std::max(0.0, 1.0 + (lfoValue * vibrato.amplitudeDepth * envelopeIntensity));
        }

        double sample = graph->generateSample(pitch, progress, isLegato, false, dynamics);

        // Simple envelope
        double envelope = 1.0;
        if (!skipAttack && progress < 0.05) {
            envelope = progress / 0.05;
        } else if (!skipRelease && progress > 0.9) {
            envelope = (1.0 - progress) / 0.1;
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
    }

    // --- Phase 2: Render tail until silence (reverb/decay ring-out) ---
    // Skip tail for legato transitions (next note continues the sound)
    if (!hasLegatoFollowing && graphHasTail) {
        double lastPitch = note.getPitchAt(1.0);
        double lastDynamics = note.getDynamicsAt(1.0);
        size_t silenceRun = 0;

        for (size_t t = 0; t < maxTailSamples; ++t) {
            double sample = graph->generateSample(lastPitch, 1.0, isLegato, true, lastDynamics);

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

    if (!graph || !graph->isValid()) {
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
    if (!renderNoteImpl(note, graph, tempRender)) return false;

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

    // Ensure we have a valid graph
    if (!hasValidGraph()) {
        qDebug() << "Track" << m_trackId << "rebuilding graph before rendering";
        if (!rebuildGraph(sampleRate)) {
            qWarning() << "Track" << m_trackId << "cannot render - graph build failed";
            return false;
        }
    }

    if (m_notes.isEmpty()) {
        qDebug() << "Track" << m_trackId << "no notes to render";
        m_noteRenders.clear();
        return true;
    }

    emit renderStarted();
    m_cancelRender.store(false);
    m_rendering.store(true);

    // Collect dirty notes into render tasks
    struct RenderTask {
        int noteIndex;
        NoteRender result;
        bool success = false;
    };

    QVector<RenderTask> tasks;
    for (int i = 0; i < m_notes.size(); ++i) {
        const Note &note = m_notes[i];
        QString noteId = note.getId();
        uint64_t noteHash = note.computeHash();

        bool needsRender = false;

        if (!m_noteRenders.contains(noteId)) {
            needsRender = true;
        } else {
            const NoteRender &existing = m_noteRenders[noteId];
            if (!existing.valid || existing.graphHash != m_graphHash ||
                existing.noteHash != noteHash) {
                needsRender = true;
            }
        }

        if (needsRender) {
            tasks.append({i, NoteRender(), false});
        }
    }

    qDebug() << "Track" << m_trackId << tasks.size() << "of" << m_notes.size() << "notes need rendering";

    // Render in parallel using thread pool — each thread gets its own cloned graph
    std::atomic<int> completedCount{0};

    QFuture<void> future = QtConcurrent::map(tasks, [this, &completedCount](RenderTask &task) {
        if (m_cancelRender.load()) return;

        const Note &note = m_notes[task.noteIndex];
        SounitGraph *srcGraph = getGraphForVariation(note.getVariationIndex());
        if (!srcGraph || !srcGraph->isValid()) return;

        SounitGraph *graph = srcGraph->clone();
        task.success = renderNoteImpl(note, graph, task.result);
        delete graph;

        completedCount.fetch_add(1);
    });

    // Wait for completion while keeping UI responsive
    while (!future.isFinished()) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        int done = completedCount.load();
        int progress = tasks.isEmpty() ? 100
            : static_cast<int>((double)done / tasks.size() * 100.0);
        emit renderProgressChanged(progress);
    }

    // Handle cancellation
    if (m_cancelRender.load()) {
        m_rendering.store(false);
        emit renderCancelled();
        return false;
    }

    // Store results under mutex
    int renderedCount = 0;
    {
        std::lock_guard<std::mutex> lock(m_playbackMutex);
        for (RenderTask &task : tasks) {
            if (task.success) {
                m_noteRenders[m_notes[task.noteIndex].getId()] = std::move(task.result);
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
    std::vector<float> buffer(numSamples, 0.0f);

    if (m_muted || numSamples == 0) {
        return buffer;
    }

    double endTimeMs = startTimeMs + durationMs;

    // Lock mutex to prevent race conditions with UI thread syncing notes
    std::lock_guard<std::mutex> lock(m_playbackMutex);

    // Check if string damping is active (K-S with stringDamping toggled on)
    bool stringDamping = m_graph && m_graph->hasStringDamping();

    // For string damping: collect note start times sorted chronologically
    // so we can find when the next note cuts off each note's tail
    std::vector<double> noteStartTimes;
    if (stringDamping) {
        noteStartTimes.reserve(m_notes.size());
        for (const Note &note : m_notes) {
            noteStartTimes.push_back(note.getStartTime());
        }
        std::sort(noteStartTimes.begin(), noteStartTimes.end());
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

        // Use actual render length (includes tail for reverb/decay)
        double noteStartMs = note.getStartTime();
        double renderDurationMs = (static_cast<double>(render.samples.size()) / render.sampleRate) * 1000.0;
        double noteEndMs = noteStartMs + renderDurationMs;

        if (noteEndMs <= startTimeMs || noteStartMs >= endTimeMs) {
            continue;  // Note doesn't overlap requested range
        }

        // String damping: find the next note's start time after this note starts.
        // That's when the string gets re-plucked and this note should be damped.
        double dampTimeMs = -1.0;
        if (stringDamping) {
            // Find the first start time strictly after this note's start
            auto it = std::upper_bound(noteStartTimes.begin(), noteStartTimes.end(), noteStartMs);
            if (it != noteStartTimes.end()) {
                dampTimeMs = *it;
            }
        }

        // Mix this note's samples into the buffer
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

                    // Apply string damping fade-out if a later note cuts this one
                    if (dampTimeMs > 0 && timeMs >= dampTimeMs) {
                        double fadeElapsed = timeMs - dampTimeMs;
                        if (fadeElapsed >= DAMP_FADE_MS) {
                            continue;  // Fully damped, skip
                        }
                        // Cosine fade for smooth damping
                        sample *= static_cast<float>(0.5 * (1.0 + std::cos(fadeElapsed / DAMP_FADE_MS * M_PI)));
                    }

                    buffer[i] += sample;
                }
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

    return endMs;
}

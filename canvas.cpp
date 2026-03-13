#include "canvas.h"
#include "sounitbuildercommands.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QApplication>
#include <QMainWindow>
#include <cmath>
#include <cstring>
#include "envelopelibraryDialog.h"

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("background-color: white;");
    setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);  // Receive mouse moves even without button pressed
    undoStack = new QUndoStack(this);

    // Auto-scroll timer: fires at ~60 fps while the mouse is near an edge during lasso
    m_autoScrollTimer = new QTimer(this);
    m_autoScrollTimer->setInterval(16);
    connect(m_autoScrollTimer, &QTimer::timeout, this, [this]() {
        if (!m_isDrawingLasso) { m_autoScrollTimer->stop(); return; }

        const int EDGE_ZONE = 40;
        const int MAX_SPEED = 15;
        int dx = 0, dy = 0;
        QPoint pos = m_lassoMousePos;

        if (pos.x() > width() - EDGE_ZONE)
            dx = qMin((pos.x() - (width() - EDGE_ZONE)) * MAX_SPEED / EDGE_ZONE, MAX_SPEED);
        else if (pos.x() < EDGE_ZONE)
            dx = -qMin((EDGE_ZONE - pos.x()) * MAX_SPEED / EDGE_ZONE, MAX_SPEED);

        if (pos.y() > height() - EDGE_ZONE)
            dy = qMin((pos.y() - (height() - EDGE_ZONE)) * MAX_SPEED / EDGE_ZONE, MAX_SPEED);
        else if (pos.y() < EDGE_ZONE)
            dy = -qMin((EDGE_ZONE - pos.y()) * MAX_SPEED / EDGE_ZONE, MAX_SPEED);

        if (dx == 0 && dy == 0) { m_autoScrollTimer->stop(); return; }

        // Pan all containers in the opposite direction to bring off-screen content into view
        const QList<Container*> ctrs = findChildren<Container*>();
        for (Container *c : ctrs)
            c->move(c->pos() - QPoint(dx, dy));

        update();
    });
}

void Canvas::clearSelection()
{
    selectedConnectionIndex = -1;
    if (selectedContainer) {
        selectedContainer->setSelected(false);
        selectedContainer = nullptr;
    }
    for (Container *c : m_selectedContainers) {
        c->setSelected(false);
    }
    m_selectedContainers.clear();
    emit selectionCleared();
    update();
}

void Canvas::deleteSelected()
{
    if (!m_selectedContainers.isEmpty()) {
        // Multi-delete via lasso selection
        QVector<Container*> toDelete = m_selectedContainers;
        for (Container *c : toDelete) {
            c->setSelected(false);
        }
        m_selectedContainers.clear();
        selectedContainer = nullptr;

        if (toDelete.size() == 1) {
            undoStack->push(new DeleteContainerCommand(toDelete[0], this));
        } else {
            undoStack->beginMacro(QString("Delete %1 Containers").arg(toDelete.size()));
            for (Container *c : toDelete) {
                undoStack->push(new DeleteContainerCommand(c, this));
            }
            undoStack->endMacro();
        }
    }
    else if (selectedContainer) {
        Container *container = selectedContainer;
        selectedContainer->setSelected(false);
        selectedContainer = nullptr;
        undoStack->push(new DeleteContainerCommand(container, this));
    }
    else if (selectedConnectionIndex >= 0 && selectedConnectionIndex < connections.size()) {
        int index = selectedConnectionIndex;
        selectedConnectionIndex = -1;
        undoStack->push(new DeleteConnectionCommand(index, this));
    }
}

void Canvas::removeContainer(Container *container)
{
    // Deselect if this is the selected container
    if (selectedContainer == container) {
        selectedContainer->setSelected(false);
        selectedContainer = nullptr;
    }

    container->hide();
    container->setParent(nullptr);
    update();
}

void Canvas::restoreContainer(Container *container, const QPoint &pos)
{
    container->setParent(this);
    container->move(pos);
    container->show();
    update();
}

QPainterPath Canvas::createConnectionPath(const QPoint &start, const QPoint &end)
{
    int dx = abs(end.x() - start.x());
    int offset = qMax(50, dx / 2);

    QPoint cp1(start.x() + offset, start.y());
    QPoint cp2(end.x() - offset, end.y());

    QPainterPath path;
    path.moveTo(start);
    path.cubicTo(cp1, cp2, end);
    return path;
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    // Handle zoom mode drag start
    if (m_zoomMode && event->button() == Qt::LeftButton) {
        m_isDraggingZoom = true;
        m_zoomDragStartPos = event->pos();
        m_zoomDragStartFactor = m_zoomFactor;
        setCursor(Qt::SizeAllCursor);
        event->accept();
        return;
    }

    // Handle pan mode drag start
    if (m_panMode && event->button() == Qt::LeftButton) {
        m_isDraggingPan = true;
        m_panDragStartPos = event->globalPosition().toPoint();
        m_panDragStartOffset = QPoint(0, 0);  // We'll track delta directly
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    // Deselect container
    if (selectedContainer) {
        selectedContainer->setSelected(false);
    }
    selectedContainer = nullptr;
    selectedConnectionIndex = -1;

    bool somethingSelected = false;

    if (event->button() == Qt::LeftButton) {
        // Check each connection for hit
        for (int i = 0; i < connections.size(); ++i) {
            const Connection &conn = connections[i];
            QPoint start = getPortPosition(conn.fromContainer, conn.fromPort, true);
            QPoint end = getPortPosition(conn.toContainer, conn.toPort, false);

            if (!start.isNull() && !end.isNull()) {
                QPainterPath path = createConnectionPath(start, end);

                // Create a "fat" version for hit testing
                QPainterPathStroker stroker;
                stroker.setWidth(10);  // 10px click tolerance
                QPainterPath clickArea = stroker.createStroke(path);

                if (clickArea.contains(event->pos())) {
                    selectedConnectionIndex = i;
                    emit connectionSelected(i);
                    somethingSelected = true;
                    break;
                }
            }
        }
    }

    if (!somethingSelected) {
        emit selectionCleared();

        // Start lasso selection if in selection mode
        if (m_selectionMode && event->button() == Qt::LeftButton) {
            m_lassoAddToSelection = (event->modifiers() & Qt::ControlModifier);
            if (!m_lassoAddToSelection) {
                // Deselect any lasso multi-selection on press
                for (Container *c : m_selectedContainers) {
                    c->setSelected(false);
                }
                m_selectedContainers.clear();
            }
            m_isDrawingLasso = true;
            m_lassoStartPos = event->pos();
            m_lassoCurrentPos = event->pos();
        } else if (event->button() == Qt::LeftButton && !(event->modifiers() & Qt::ControlModifier)) {
            // Clear multi-selection when clicking empty space in normal mode
            for (Container *c : m_selectedContainers) {
                c->setSelected(false);
            }
            m_selectedContainers.clear();
        }
    }

    update();
}

void Canvas::keyPressEvent(QKeyEvent *event)
{
    // Forward Tab key to parent window (SounitBuilder)
    if (event->key() == Qt::Key_Tab && event->modifiers() == Qt::NoModifier) {
        // Find parent SounitBuilder and forward the event
        QWidget *parent = parentWidget();
        while (parent && !qobject_cast<QMainWindow*>(parent)) {
            parent = parent->parentWidget();
        }
        if (parent) {
            QApplication::sendEvent(parent, event);
        }
        return;
    }

    if (event->key() == Qt::Key_Delete) {
        deleteSelected();
        event->accept();
        return;
    }

    // Undo: Ctrl+Z
    if (event->matches(QKeySequence::Undo)) {
        undoStack->undo();
        event->accept();
        return;
    }

    // Redo: Ctrl+Y or Ctrl+Shift+Z
    if (event->matches(QKeySequence::Redo)) {
        undoStack->redo();
        event->accept();
        return;
    }

    // Select All: Ctrl+A
    if (event->matches(QKeySequence::SelectAll)) {
        if (selectedContainer) {
            selectedContainer->setSelected(false);
            selectedContainer = nullptr;
        }
        for (Container *c : m_selectedContainers)
            c->setSelected(false);
        m_selectedContainers.clear();
        selectedConnectionIndex = -1;

        QList<Container*> all = findChildren<Container*>();
        for (Container *c : all) {
            c->setSelected(true);
            m_selectedContainers.append(c);
        }

        if (m_selectedContainers.size() == 1) {
            selectedContainer = m_selectedContainers[0];
            emit containerSelected(selectedContainer);
        } else if (!m_selectedContainers.isEmpty()) {
            emit containersSelected(m_selectedContainers);
        } else {
            emit selectionCleared();
        }

        update();
        event->accept();
        return;
    }

    // Pass unhandled key events to parent widget (SounitBuilder)
    QWidget::keyPressEvent(event);
}

void Canvas::addConnection(const Connection &conn)
{
    connections.append(conn);
    update();

    // Notify that graph structure changed
    emit graphChanged();
}

QPoint Canvas::getPortPosition(Container *container, const QString &portName, bool isOutput)
{
    for (const Container::PortInfo &port : container->getPorts()) {
        if (port.name == portName) {
            QPoint circlePos = port.circle->pos();
            QPoint containerPos = container->pos();
            return containerPos + circlePos + QPoint(12, 12);  // Center of circle
        }
    }
    return QPoint();
}

void Canvas::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (int i = 0; i < connections.size(); ++i) {
        const Connection &conn = connections[i];
        QPoint start = getPortPosition(conn.fromContainer, conn.fromPort, true);
        QPoint end = getPortPosition(conn.toContainer, conn.toPort, false);

        if (!start.isNull() && !end.isNull()) {
            QPainterPath path = createConnectionPath(start, end);

            // Different style for selected vs normal
            if (i == selectedConnectionIndex) {
                QPen pen(QColor("#0066CC"), 3);  // Blue, thicker
                painter.setPen(pen);
            } else {
                QPen pen(conn.fromContainer->getColor(), 2);  // Source container color
                painter.setPen(pen);
            }

            painter.drawPath(path);
        }
    }

    // Draw pending connection (being dragged)
    if (hasPendingConnection && pendingFromContainer) {
        QPoint start = getPortPosition(pendingFromContainer, pendingFromPort, pendingIsOutput);

        if (!start.isNull()) {
            QPainterPath path = createConnectionPath(start, pendingEndPoint);

            QPen pen(pendingFromContainer->getColor(), 2, Qt::DashLine);
            painter.setPen(pen);
            painter.drawPath(path);
        }
    }

    // Draw lasso rectangle
    drawLassoRectangle(painter);
}


void Canvas::startPendingConnection(Container *container, const QString &portName, bool isOutput)
{
    hasPendingConnection = true;
    pendingFromContainer = container;
    pendingFromPort = portName;
    pendingIsOutput = isOutput;
    pendingEndPoint = mapFromGlobal(QCursor::pos());  // Convert to local coordinates  // Initialize to current cursor
    update();
}

void Canvas::cancelPendingConnection()
{
    hasPendingConnection = false;
    pendingFromContainer = nullptr;
    pendingFromPort.clear();
    update();
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    // Handle zoom mode drag
    if (m_zoomMode && m_isDraggingZoom) {
        // Calculate drag distance - drag up/left = zoom in, drag down/right = zoom out
        int deltaY = m_zoomDragStartPos.y() - event->pos().y();
        int deltaX = m_zoomDragStartPos.x() - event->pos().x();

        // Combined delta (average of X and Y movement)
        int delta = (deltaY + deltaX) / 2;

        // 100 pixels of drag = 2x zoom change
        double zoomMultiplier = std::pow(2.0, delta / 100.0);
        double newZoomFactor = m_zoomDragStartFactor * zoomMultiplier;

        setZoomFactor(newZoomFactor);
        event->accept();
        return;
    }

    // Handle pan mode drag
    if (m_panMode && m_isDraggingPan) {
        QPoint currentPos = event->globalPosition().toPoint();
        QPoint delta = currentPos - m_panDragStartPos;

        // Move all containers by the delta
        QList<Container*> containers = findChildren<Container*>();
        for (Container *c : containers) {
            c->move(c->pos() + delta);
        }

        // Update start position for next move event (incremental panning)
        m_panDragStartPos = currentPos;

        update();
        event->accept();
        return;
    }

    if (m_isDrawingLasso) {
        m_lassoCurrentPos = event->pos();
        m_lassoMousePos = event->pos();

        // Start auto-scroll timer when mouse enters the edge zone
        const int EDGE_ZONE = 40;
        bool nearEdge = (event->pos().x() > width() - EDGE_ZONE  ||
                         event->pos().x() < EDGE_ZONE             ||
                         event->pos().y() > height() - EDGE_ZONE  ||
                         event->pos().y() < EDGE_ZONE);
        if (nearEdge && !m_autoScrollTimer->isActive())
            m_autoScrollTimer->start();
        else if (!nearEdge && m_autoScrollTimer->isActive())
            m_autoScrollTimer->stop();

        update();
        return;
    }

    if (hasPendingConnection) {
        pendingEndPoint = event->pos();
        update();
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    // End zoom drag
    if (m_isDraggingZoom && event->button() == Qt::LeftButton) {
        m_isDraggingZoom = false;
        if (m_zoomMode) {
            setCursor(Qt::CrossCursor);  // Back to zoom cursor
        } else {
            setCursor(Qt::ArrowCursor);
        }
        event->accept();
        return;
    }

    // End pan drag
    if (m_isDraggingPan && event->button() == Qt::LeftButton) {
        m_isDraggingPan = false;
        if (m_panMode) {
            setCursor(Qt::OpenHandCursor);  // Back to pan cursor
        } else {
            setCursor(Qt::ArrowCursor);
        }
        event->accept();
        return;
    }

    // Apply lasso selection
    if (m_isDrawingLasso && event->button() == Qt::LeftButton) {
        m_isDrawingLasso = false;
        m_autoScrollTimer->stop();

        int x = std::min(m_lassoStartPos.x(), m_lassoCurrentPos.x());
        int y = std::min(m_lassoStartPos.y(), m_lassoCurrentPos.y());
        int w = std::abs(m_lassoCurrentPos.x() - m_lassoStartPos.x());
        int h = std::abs(m_lassoCurrentPos.y() - m_lassoStartPos.y());
        QRect lassoRect(x, y, w, h);

        QVector<Container*> found = findContainersInRect(lassoRect);

        if (m_lassoAddToSelection) {
            for (Container *c : found) {
                if (!m_selectedContainers.contains(c)) {
                    m_selectedContainers.append(c);
                    c->setSelected(true);
                }
            }
        } else {
            for (Container *c : m_selectedContainers) {
                c->setSelected(false);
            }
            m_selectedContainers.clear();
            for (Container *c : found) {
                m_selectedContainers.append(c);
                c->setSelected(true);
            }
        }

        // Keep single-select pointer consistent
        if (m_selectedContainers.size() == 1) {
            selectedContainer = m_selectedContainers[0];
            emit containerSelected(selectedContainer);
        } else {
            selectedContainer = nullptr;
            emit containersSelected(m_selectedContainers);
        }

        update();
        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

void Canvas::selectContainer(Container *container)
{
    // Ctrl+click: toggle container in/out of multi-selection
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        selectedConnectionIndex = -1;

        // Absorb any existing single-selection into m_selectedContainers
        if (selectedContainer && !m_selectedContainers.contains(selectedContainer)) {
            m_selectedContainers.append(selectedContainer);
        }
        selectedContainer = nullptr;

        // Toggle the clicked container
        if (m_selectedContainers.contains(container)) {
            m_selectedContainers.removeOne(container);
            container->setSelected(false);
        } else {
            m_selectedContainers.append(container);
            container->setSelected(true);
        }

        // Emit appropriate signal
        if (m_selectedContainers.isEmpty()) {
            emit selectionCleared();
        } else if (m_selectedContainers.size() == 1) {
            selectedContainer = m_selectedContainers[0];
            emit containerSelected(selectedContainer);
        } else {
            emit containersSelected(m_selectedContainers);
        }

        update();
        return;
    }

    // If container is already in the multi-selection, preserve it so dragging
    // a lasso-selected group doesn't collapse the selection to one container.
    if (!m_selectedContainers.isEmpty() && m_selectedContainers.contains(container)) {
        if (selectedContainer && selectedContainer != container) {
            selectedContainer->setSelected(false);
        }
        selectedContainer = container;
        selectedConnectionIndex = -1;
        update();
        return;
    }

    // Deselect previous container
    if (selectedContainer) {
        selectedContainer->setSelected(false);
    }

    // Clear any lasso multi-selection
    for (Container *c : m_selectedContainers) {
        c->setSelected(false);
    }
    m_selectedContainers.clear();

    // Select new container
    selectedContainer = container;
    if (selectedContainer) {
        selectedContainer->setSelected(true);
        emit containerSelected(container);
    }

    selectedConnectionIndex = -1;  // Deselect any connection
    update();
}

void Canvas::setSounitName(const QString &name)
{
    if (sounitName != name) {
        sounitName = name;

        // *** NEW: Only emit signal if NOT loading ***
        if (!m_isLoading) {
            emit sounitNameChanged(name);
        }

        update();  // Redraw if name display is part of canvas
    }
}

void Canvas::setSounitComment(const QString &comment)
{
    sounitComment = comment;
    emit sounitCommentChanged(comment);
}

QJsonObject Canvas::serializeContainer(const Container *container) const
{
    QJsonObject json;

    // Basic info
    json["type"] = container->getName();
    json["instanceName"] = container->getInstanceName();
    json["color"] = container->getColor().name();

    // Position
    QJsonObject pos;
    pos["x"] = container->pos().x();
    pos["y"] = container->pos().y();
    json["position"] = pos;

    // Parameters
    QJsonObject params;
    QMap<QString, double> containerParams = container->getParameters();
    for (auto it = containerParams.begin(); it != containerParams.end(); ++it) {
        // Skip customDna_N parameters - they'll be stored separately
        if (!it.key().startsWith("customDna_") && it.key() != "customDnaCount") {
            params[it.key()] = it.value();
        }
    }
    json["parameters"] = params;

    // Custom DNA (only if using custom mode)
    if (containerParams.value("dnaSelect", 0.0) == -1.0) {
        int count = static_cast<int>(containerParams.value("customDnaCount", 0.0));
        if (count > 0) {
            QJsonObject customDna;
            customDna["count"] = count;
            customDna["name"] = container->getCustomDnaName();
            QJsonArray amps;
            for (int i = 0; i < count; i++) {
                QString key = QString("customDna_%1").arg(i);
                amps.append(containerParams.value(key, 0.0));
            }
            customDna["amplitudes"] = amps;
            json["customDna"] = customDna;
        }
    }

    // Digit formula string (only if in digit-formula mode)
    if (!container->getDigitString().isEmpty()) {
        json["digitString"] = container->getDigitString();
    }

    // Wavetable data (only if present)
    if (container->hasWavetableData()) {
        QJsonObject wtObj;
        wtObj["filePath"] = container->getWavetableFilePath();

        // Base64-encode the raw float samples
        const std::vector<float> &wtData = container->getWavetableData();
        QByteArray rawBytes(reinterpret_cast<const char*>(wtData.data()),
                            static_cast<int>(wtData.size() * sizeof(float)));
        wtObj["samples"] = QString::fromLatin1(rawBytes.toBase64());
        wtObj["sampleCount"] = static_cast<int>(wtData.size());
        json["wavetableData"] = wtObj;
    }

    // IR Convolution data (only if present)
    if (container->hasIRData()) {
        QJsonObject irObj;
        irObj["filePath"] = container->getIRFilePath();

        const std::vector<float> &irSamples = container->getIRData();
        QByteArray rawBytes(reinterpret_cast<const char*>(irSamples.data()),
                            static_cast<int>(irSamples.size() * sizeof(float)));
        irObj["samples"] = QString::fromLatin1(rawBytes.toBase64());
        irObj["sampleCount"] = static_cast<int>(irSamples.size());
        json["irData"] = irObj;
    }

    // Custom Envelope (only if present)
    if (container->hasCustomEnvelopeData()) {
        EnvelopeData envData = container->getCustomEnvelopeData();
        QJsonObject customEnv;
        customEnv["name"] = envData.name;
        customEnv["loopMode"] = envData.loopMode;

        QJsonArray points;
        for (const EnvelopePoint &pt : envData.points) {
            QJsonObject ptObj;
            ptObj["time"] = pt.time;
            ptObj["value"] = pt.value;
            ptObj["curveType"] = pt.curveType;
            points.append(ptObj);
        }
        customEnv["points"] = points;
        json["customEnvelope"] = customEnv;
    }

    return json;
}

QJsonObject Canvas::serializeConnection(const Connection &conn) const
{
    QJsonObject json;
    json["fromContainer"] = conn.fromContainer->getInstanceName();
    json["fromPort"] = conn.fromPort;
    json["toContainer"] = conn.toContainer->getInstanceName();
    json["toPort"] = conn.toPort;
    json["function"] = conn.function;
    json["weight"] = conn.weight;
    return json;
}

bool Canvas::saveToJson(const QString &filePath, const QString &sounitName)
{
    QJsonObject root;

    // Sounit metadata
    QJsonObject sounitMeta;
    sounitMeta["name"] = sounitName;
    sounitMeta["version"] = "1.0";
    sounitMeta["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    sounitMeta["comment"] = sounitComment;
    root["sounit"] = sounitMeta;

    // CRITICAL: Ensure all containers have unique instance names before saving
    // This fixes a bug where multiple containers of the same type had the same
    // instance name, causing connections to be incorrectly restored on load
    QList<Container*> containers = findChildren<Container*>();
    QSet<QString> usedNames;
    for (Container *container : containers) {
        QString originalName = container->getInstanceName();
        if (usedNames.contains(originalName)) {
            // Generate unique name
            QString baseName = container->getName();  // Use type name as base
            int suffix = 1;
            QString newName;
            do {
                newName = QString("%1 %2").arg(baseName).arg(suffix++);
            } while (usedNames.contains(newName));
            container->setInstanceName(newName);
            qDebug() << "Renamed duplicate instance" << originalName << "to" << newName << "before save";
        }
        usedNames.insert(container->getInstanceName());
    }

    // Serialize containers
    QJsonArray containersArray;
    for (const Container *container : containers) {
        containersArray.append(serializeContainer(container));
    }
    root["containers"] = containersArray;

    // Serialize connections
    QJsonArray connectionsArray;
    for (const Connection &conn : connections) {
        connectionsArray.append(serializeConnection(conn));
    }
    root["connections"] = connectionsArray;

    // Write to file
    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    qDebug() << "Sounit saved to:" << filePath;
    return true;
}

// Static helper to map container type to port lists
void Canvas::getPortsForContainerType(const QString &type, QStringList &inputs, QStringList &outputs)
{
    if (type == "Harmonic Generator") {
        inputs = {"purity", "drift", "digitWindowOffset"};
        outputs = {"spectrum"};
    } else if (type == "Spectrum to Signal") {
        inputs = {"spectrumIn", "pitchMultiplier", "normalize"};
        outputs = {"signalOut"};
    } else if (type == "Rolloff Processor") {
        inputs = {"spectrumIn", "lowRolloff", "highRolloff", "crossover", "transition"};
        outputs = {"spectrumOut"};
    } else if (type == "Formant Body") {
        inputs = {"signalIn", "f1Freq", "f2Freq", "f1Q", "f2Q", "directMix", "f1f2Balance"};
        outputs = {"signalOut"};
    } else if (type == "Breath Turbulence") {
        inputs = {"voiceIn", "noiseIn", "blend"};
        outputs = {"signalOut"};
    } else if (type == "Noise Color Filter") {
        inputs = {"audioIn", "color", "filterQ", "pitchMultiplier"};
        outputs = {"noiseOut"};
    } else if (type == "Physics System") {
        inputs = {"targetValue", "mass", "springK", "damping", "impulse", "impulseAmount"};
        outputs = {"currentValue"};
    } else if (type == "Easing Applicator") {
        inputs = {"startValue", "endValue", "progress", "easingSelect"};
        outputs = {"easedValue"};
    } else if (type == "Envelope Engine") {
        inputs = {"timeScale", "valueScale", "valueOffset"};
        outputs = {"envelopeValue"};
    } else if (type == "Drift Engine") {
        inputs = {"amount", "rate"};
        outputs = {"driftOut"};
    } else if (type == "Gate Processor") {
        inputs = {"velocity", "attackTime", "releaseTime", "attackCurve", "releaseCurve", "velocitySens"};
        outputs = {"envelopeOut", "stateOut", "attackTrigger", "releaseTrigger"};
    } else if (type == "Karplus Strong" || type == "Karplus Strong Attack") {
        // "Karplus Strong Attack" is legacy name for backwards compatibility
        inputs = {"signalIn", "trigger", "mode", "attackPortion", "damping", "pluckPosition", "mix",
                  "brightness", "excitationType", "blendRatio", "pluckHardness", "bodyResonance", "bodyFreq", "pickDirection",
                  "stringDamping", "pitchMultiplier"};
        outputs = {"signalOut"};
    } else if (type == "Attack") {
        inputs = {"signalIn", "trigger", "attackType", "duration", "intensity", "mix",
                  "noiseAmount", "jetRatio", "reedStiffness", "reedAperture", "blowPosition",
                  "lipTension", "buzzQ", "hardness", "brightness", "tightness", "tone"};
        outputs = {"signalOut"};
    } else if (type == "LFO") {
        inputs = {"frequency", "amplitude", "waveType"};
        outputs = {"valueOut"};
    } else if (type == "Frequency Mapper") {
        inputs = {"curve", "pitchMultiplier"};
        outputs = {"controlOut"};
    } else if (type == "Signal Mixer") {
        inputs = {"signalA", "signalB", "gainA", "gainB"};
        outputs = {"signalOut"};
    } else if (type == "Spectrum Blender") {
        inputs = {"spectrumA", "spectrumB", "position"};
        outputs = {"spectrumOut"};
    } else if (type == "Wavetable Synth") {
        inputs = {"position", "pitchMultiplier"};
        outputs = {"spectrum", "signalOut"};
    } else if (type == "10-Band EQ") {
        inputs = {"signalIn", "band1", "band2", "band3", "band4", "band5",
                  "band6", "band7", "band8", "band9", "band10"};
        outputs = {"signalOut"};
    } else if (type == "Comb Filter") {
        inputs = {"signalIn", "delayTime", "feedback", "damping"};
        outputs = {"signalOut"};
    } else if (type == "LP/HP Filter") {
        inputs = {"signalIn", "cutoff", "resonance"};
        outputs = {"signalOut"};
    } else if (type == "IR Convolution") {
        inputs = {"signalIn", "irIn", "wetDry", "predelay", "highDamp", "lowCut"};
        outputs = {"signalOut"};
    } else if (type == "Recorder") {
        inputs = {"breathPressure", "jetRatio", "noiseGain",
                  "vibratoFreq", "vibratoGain",
                  "endReflection", "jetReflection", "pitchMultiplier"};
        outputs = {"signalOut"};
    } else if (type == "Bowed") {
        inputs = {"bowPressure", "bowVelocity", "bowPosition",
                  "nlType", "nlAmount", "nlFreqMod", "nlAttack", "pitchMultiplier"};
        outputs = {"signalOut"};
    } else if (type == "Saxophone" || type == "Reed") {
        inputs = {"breathPressure", "reedStiffness", "reedAperture",
                  "blowPosition", "noiseGain", "vibratoFreq", "vibratoGain",
                  "nlType", "nlAmount", "nlFreqMod", "nlAttack", "pitchMultiplier"};
        outputs = {"signalOut"};
    } else {
        qWarning() << "Unknown container type:" << type;
    }
}

Container* Canvas::deserializeContainer(const QJsonObject &json, QWidget *parent)
{
    QString type = json["type"].toString();
    QString instanceName = json["instanceName"].toString();
    QColor color(json["color"].toString());

    // Migrate legacy container names
    if (type == "Karplus Strong Attack") {
        type = "Karplus Strong";
        qDebug() << "Migrated legacy 'Karplus Strong Attack' to 'Karplus Strong'";
    }

    // Get position
    QJsonObject posObj = json["position"].toObject();
    QPoint position(posObj["x"].toInt(), posObj["y"].toInt());

    // Determine inputs/outputs based on type
    QStringList inputs, outputs;
    getPortsForContainerType(type, inputs, outputs);

    if (inputs.isEmpty() && outputs.isEmpty()) {
        qWarning() << "Cannot deserialize unknown container type:" << type;
        return nullptr;
    }

    // Create container
    Container *container = new Container(parent, type, color, inputs, outputs);
    container->setInstanceName(instanceName);
    container->move(position);

    // Batch parameter updates to avoid multiple graph rebuilds during load
    container->beginParameterUpdate();

    // Restore parameters
    QJsonObject params = json["parameters"].toObject();
    for (auto it = params.begin(); it != params.end(); ++it) {
        container->setParameter(it.key(), it.value().toDouble());
    }

    // Restore custom DNA if present
    if (json.contains("customDna")) {
        QJsonObject customDna = json["customDna"].toObject();
        int count = customDna["count"].toInt();
        QJsonArray amps = customDna["amplitudes"].toArray();

        container->setParameter("customDnaCount", static_cast<double>(count));
        container->setParameter("dnaSelect", -1.0);  // Mark as custom
        for (int i = 0; i < count && i < amps.size(); i++) {
            QString key = QString("customDna_%1").arg(i);
            container->setParameter(key, amps[i].toDouble());
        }
        container->setCustomDnaName(customDna["name"].toString());
    }

    // End batch update - triggers single graph rebuild
    container->endParameterUpdate();

    // Restore digit formula string if present
    if (json.contains("digitString")) {
        container->setDigitString(json["digitString"].toString());
    }

    // Restore custom envelope if present
    if (json.contains("customEnvelope")) {
        QJsonObject envObj = json["customEnvelope"].toObject();
        EnvelopeData envData;
        envData.name = envObj["name"].toString();
        envData.loopMode = envObj["loopMode"].toInt();
        envData.isFactory = false;

        QJsonArray pointsArray = envObj["points"].toArray();
        for (const QJsonValue &ptVal : pointsArray) {
            QJsonObject ptObj = ptVal.toObject();
            EnvelopePoint pt;
            pt.time = ptObj["time"].toDouble();
            pt.value = ptObj["value"].toDouble();
            pt.curveType = ptObj["curveType"].toInt();
            envData.points.append(pt);
        }

        container->setCustomEnvelopeData(envData);
    }

    // Restore wavetable data if present
    if (json.contains("wavetableData")) {
        QJsonObject wtObj = json["wavetableData"].toObject();
        container->setWavetableFilePath(wtObj["filePath"].toString());

        int sampleCount = wtObj["sampleCount"].toInt();
        if (sampleCount > 0 && wtObj.contains("samples")) {
            QByteArray base64 = wtObj["samples"].toString().toLatin1();
            QByteArray rawBytes = QByteArray::fromBase64(base64);
            int expectedSize = sampleCount * static_cast<int>(sizeof(float));
            if (rawBytes.size() == expectedSize) {
                std::vector<float> wtData(sampleCount);
                std::memcpy(wtData.data(), rawBytes.constData(), rawBytes.size());
                container->setWavetableData(wtData);
            }
        }
    }

    // Restore IR convolution data if present
    if (json.contains("irData")) {
        QJsonObject irObj = json["irData"].toObject();
        container->setIRFilePath(irObj["filePath"].toString());

        int sampleCount = irObj["sampleCount"].toInt();
        if (sampleCount > 0 && irObj.contains("samples")) {
            QByteArray base64 = irObj["samples"].toString().toLatin1();
            QByteArray rawBytes = QByteArray::fromBase64(base64);
            int expectedSize = sampleCount * static_cast<int>(sizeof(float));
            if (rawBytes.size() == expectedSize) {
                std::vector<float> irSamples(sampleCount);
                std::memcpy(irSamples.data(), rawBytes.constData(), rawBytes.size());
                container->setIRData(irSamples);
            }
        }
    }

    return container;
}

bool Canvas::loadFromJson(const QString &filePath, QString &outSounitName)
{

    // ***  Set loading flag ***
    m_isLoading = true;


    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file for reading:" << filePath;
        m_isLoading = false;  // Clear loading flag before early return
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON format";
        m_isLoading = false;  // Clear loading flag before early return
        return false;
    }

    QJsonObject root = doc.object();

    // Read sounit metadata
    QJsonObject sounitMeta = root["sounit"].toObject();
    outSounitName = sounitMeta["name"].toString("Untitled Sounit");
    setSounitName(outSounitName);

    // Load comment
    QString comment = sounitMeta["comment"].toString("");
    setSounitComment(comment);

    // CRITICAL: Don't delete existing containers - just hide them and clear connections
    // Deleting containers invalidates all SounitGraph pointers for other tracks!
    // The containers will be managed by KalaMain's track switching logic
    connections.clear();
    selectedConnectionIndex = -1;
    selectedContainer = nullptr;

    // Deserialize containers
    QMap<QString, Container*> containerMap;  // instanceName -> Container*
    QJsonArray containersArray = root["containers"].toArray();

    for (const QJsonValue &val : containersArray) {
        QJsonObject containerJson = val.toObject();
        Container *container = deserializeContainer(containerJson, this);

        if (!container) {
            qWarning() << "Failed to deserialize container";
            continue;
        }

        // Check for duplicate instance names and auto-rename if needed
        QString originalName = container->getInstanceName();
        if (containerMap.contains(originalName)) {
            qWarning() << "Duplicate instance name detected:" << originalName;
            QString baseName = originalName;
            int suffix = 1;
            QString newName;
            do {
                newName = QString("%1-%2").arg(baseName).arg(suffix++);
            } while (containerMap.contains(newName));
            container->setInstanceName(newName);
            qDebug() << "Renamed to:" << newName;
        }

        // Show container
        container->show();

        // Store in map for connection reconstruction
        containerMap[container->getInstanceName()] = container;
    }

    // Deserialize connections (after all containers exist)
    QJsonArray connectionsArray = root["connections"].toArray();
    int skippedConnections = 0;

    for (const QJsonValue &val : connectionsArray) {
        QJsonObject connJson = val.toObject();

        QString fromName = connJson["fromContainer"].toString();
        QString toName = connJson["toContainer"].toString();

        if (!containerMap.contains(fromName) || !containerMap.contains(toName)) {
            qWarning() << "Connection references missing container:" << fromName << "->" << toName;
            skippedConnections++;
            continue;
        }

        Connection conn;
        conn.fromContainer = containerMap[fromName];
        conn.fromPort = connJson["fromPort"].toString();
        conn.toContainer = containerMap[toName];
        conn.toPort = connJson["toPort"].toString();
        conn.function = connJson["function"].toString("passthrough");
        conn.weight = connJson["weight"].toDouble(1.0);

        // Validate ports exist
        if (!conn.fromContainer->getOutputPorts().contains(conn.fromPort)) {
            qWarning() << "Invalid output port:" << conn.fromPort << "on" << fromName;
            skippedConnections++;
            continue;
        }
        if (!conn.toContainer->getInputPorts().contains(conn.toPort)) {
            qWarning() << "Invalid input port:" << conn.toPort << "on" << toName;
            skippedConnections++;
            continue;
        }

        connections.append(conn);
    }

    update();
    emit graphChanged();

    if (skippedConnections > 0) {
        qWarning() << "Skipped" << skippedConnections << "invalid connections during load";
    }

    qDebug() << "Sounit loaded from:" << filePath;
    qDebug() << "  Containers:" << containerMap.size();
    qDebug() << "  Connections:" << connections.size();

    // *** Clear loading flag ***
    m_isLoading = false;

    return true;
}

void Canvas::selectContainers(const QVector<Container*> &containers)
{
    if (selectedContainer) {
        selectedContainer->setSelected(false);
        selectedContainer = nullptr;
    }
    for (Container *c : m_selectedContainers) {
        c->setSelected(false);
    }
    m_selectedContainers.clear();
    selectedConnectionIndex = -1;

    if (containers.isEmpty()) {
        emit selectionCleared();
        update();
        return;
    }

    for (Container *c : containers) {
        c->setSelected(true);
        m_selectedContainers.append(c);
    }

    if (m_selectedContainers.size() == 1) {
        selectedContainer = m_selectedContainers[0];
        emit containerSelected(selectedContainer);
    } else {
        emit containersSelected(m_selectedContainers);
    }
    update();
}

// ========== Selection Mode / Lasso ==========

void Canvas::setSelectionMode(bool active)
{
    m_selectionMode = active;

    if (!active) {
        // Cancel any in-progress lasso when leaving selection mode
        m_isDrawingLasso = false;
        m_autoScrollTimer->stop();
        setCursor(Qt::ArrowCursor);
    } else {
        setCursor(Qt::CrossCursor);
    }

    update();
    emit selectionModeChanged(active);
}

void Canvas::drawLassoRectangle(QPainter &painter)
{
    if (!m_isDrawingLasso) return;

    int x = std::min(m_lassoStartPos.x(), m_lassoCurrentPos.x());
    int y = std::min(m_lassoStartPos.y(), m_lassoCurrentPos.y());
    int w = std::abs(m_lassoCurrentPos.x() - m_lassoStartPos.x());
    int h = std::abs(m_lassoCurrentPos.y() - m_lassoStartPos.y());

    painter.save();
    painter.setBrush(QColor(100, 150, 255, 50));
    painter.setPen(QPen(QColor(50, 100, 200), 2, Qt::DashLine));
    painter.drawRect(x, y, w, h);
    painter.restore();
}

QVector<Container*> Canvas::findContainersInRect(const QRect &rect) const
{
    QVector<Container*> found;
    QList<Container*> containers = findChildren<Container*>();
    for (Container *c : containers) {
        QRect containerRect(c->pos(), c->size());
        if (containerRect.intersects(rect)) {
            found.append(c);
        }
    }
    return found;
}

// ========== Zoom Functions ==========

void Canvas::setZoomMode(bool active)
{
    m_zoomMode = active;
    if (active) {
        setCursor(Qt::CrossCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
    qDebug() << "Canvas zoom mode:" << (active ? "ON" : "OFF");
}

void Canvas::setZoomFactor(double factor)
{
    // Clamp zoom factor to reasonable limits
    factor = qBound(0.25, factor, 4.0);

    if (qFuzzyCompare(factor, m_zoomFactor)) {
        return;
    }

    double oldFactor = m_zoomFactor;
    m_zoomFactor = factor;

    // Scale all container positions and sizes
    QList<Container*> containers = findChildren<Container*>();
    for (Container *c : containers) {
        // Scale position relative to origin
        QPoint oldPos = c->pos();
        QPoint newPos = (oldPos.toPointF() * (factor / oldFactor)).toPoint();
        c->move(newPos);

        // Scale size
        QSize oldSize = c->size();
        QSize newSize = (oldSize.toSizeF() * (factor / oldFactor)).toSize();
        c->resize(newSize);
    }

    emit zoomChanged(factor);
    update();
    qDebug() << "Canvas zoom factor:" << factor;
}

void Canvas::zoomIn(const QPoint &center)
{
    Q_UNUSED(center);  // For future: zoom toward center point
    setZoomFactor(m_zoomFactor * 1.2);
}

void Canvas::zoomOut(const QPoint &center)
{
    Q_UNUSED(center);  // For future: zoom away from center point
    setZoomFactor(m_zoomFactor / 1.2);
}

void Canvas::resetZoom()
{
    setZoomFactor(1.0);
}

void Canvas::wheelEvent(QWheelEvent *event)
{
    if (m_zoomMode) {
        // Zoom with mouse wheel when in zoom mode
        if (event->angleDelta().y() > 0) {
            zoomIn(event->position().toPoint());
        } else if (event->angleDelta().y() < 0) {
            zoomOut(event->position().toPoint());
        }
        event->accept();
    } else {
        // Pass to parent for default handling (scroll)
        QWidget::wheelEvent(event);
    }
}

// ========== Pan Functions ==========

void Canvas::setPanMode(bool active)
{
    m_panMode = active;

    if (active) {
        // Disable zoom mode if it's active
        if (m_zoomMode) {
            setZoomMode(false);
        }
        setCursor(Qt::OpenHandCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
    qDebug() << "Canvas pan mode:" << (active ? "ON" : "OFF");
}

void Canvas::resetPan()
{
    // Move all containers back by the accumulated pan offset
    // For simplicity, we track pan as container position changes
    // Reset would need to track original positions - for now just log
    qDebug() << "Canvas pan reset (containers stay at current positions)";
}

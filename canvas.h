#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QVector>
#include <QJsonObject>
#include <QWheelEvent>
#include <QUndoStack>
#include <QTimer>
#include "container.h"
#include <QKeyEvent>

class Canvas : public QWidget
{
    Q_OBJECT

public:
    struct Connection {
        Container *fromContainer;
        QString fromPort;
        Container *toContainer;
        QString toPort;
        QString function = "passthrough";  // Connection function
        double weight = 1.0;               // Connection weight
    };

    explicit Canvas(QWidget *parent = nullptr);

    void addConnection(const Connection &conn);
    QVector<Connection>& getConnections() { return connections; }
    void clearSelection();
    void deleteSelected();  // Deletes selected container or connection
    void startPendingConnection(Container *container, const QString &portName, bool isOutput);
    void cancelPendingConnection();

    // Pending connection (being dragged)
    bool hasPendingConnection = false;
    Container *pendingFromContainer = nullptr;
    QString pendingFromPort;
    bool pendingIsOutput = true;
    QPoint pendingEndPoint;

    Container *selectedContainer = nullptr;
    void selectContainer(Container *container);

    // Undo/Redo support
    QUndoStack* getUndoStack() { return undoStack; }
    void removeContainer(Container *container);
    void restoreContainer(Container *container, const QPoint &pos);

    // Get selected connection (nullptr if none selected)
    Connection* getSelectedConnection() {
        if (selectedConnectionIndex >= 0 && selectedConnectionIndex < connections.size()) {
            return &connections[selectedConnectionIndex];
        }
        return nullptr;
    }

    // Sounit save/load

    // *** NEW: Loading state methods ***
    void setLoading(bool loading) {
        m_isLoading = loading;
        qDebug() << "Canvas loading state:" << (loading ? "true" : "false");
    }
    bool isLoading() const { return m_isLoading; }
    bool saveToJson(const QString &filePath, const QString &sounitName);  // Non-const: fixes duplicate instance names before save
    bool loadFromJson(const QString &filePath, QString &outSounitName);
    QString getSounitName() const { return sounitName; }
    void setSounitName(const QString &name);
    QString getSounitComment() const { return sounitComment; }
    void setSounitComment(const QString &comment);

    // Zoom functionality
    void setZoomMode(bool active);
    bool isZoomMode() const { return m_zoomMode; }
    void setZoomFactor(double factor);
    double getZoomFactor() const { return m_zoomFactor; }
    void zoomIn(const QPoint &center = QPoint());
    void zoomOut(const QPoint &center = QPoint());
    void resetZoom();

    // Pan functionality
    void setPanMode(bool active);
    bool isPanMode() const { return m_panMode; }
    void resetPan();

    // Selection mode (enables lasso / multi-select)
    void setSelectionMode(bool active);
    bool isSelectionMode() const { return m_selectionMode; }
    const QVector<Container*>& getSelectedContainers() const { return m_selectedContainers; }
    void selectContainers(const QVector<Container*> &containers);  // Set multi-selection programmatically

signals:
    void zoomChanged(double factor);
    void containerSelected(Container *container);
    void connectionSelected(int connectionIndex);
    void selectionCleared();
    void containersSelected(const QVector<Container*> &containers);
    void graphChanged();  // Emitted when containers/connections change
    void sounitNameChanged(const QString &name);
    void sounitCommentChanged(const QString &comment);
    void selectionModeChanged(bool active);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QVector<Connection> connections;
    QUndoStack *undoStack;
    QString sounitName = "Untitled Sounit";
    QString sounitComment;

    QPoint getPortPosition(Container *container, const QString &portName, bool isOutput);
    int selectedConnectionIndex = -1;
    QPainterPath createConnectionPath(const QPoint &start, const QPoint &end);

    // Selection mode + lasso state
    bool m_selectionMode = false;
    bool m_isDrawingLasso = false;
    bool m_lassoAddToSelection = false;
    QPoint m_lassoStartPos;
    QPoint m_lassoCurrentPos;
    QVector<Container*> m_selectedContainers;  // Multi-selection from lasso

    // Auto-scroll during lasso: when mouse is near canvas edge, pan containers
    QTimer *m_autoScrollTimer = nullptr;
    QPoint m_lassoMousePos;  // Current mouse pos in canvas coords (kept updated during lasso)

    void drawLassoRectangle(QPainter &painter);
    QVector<Container*> findContainersInRect(const QRect &rect) const;

public:
    // Serialization helpers (public for Track variation support)
    QJsonObject serializeContainer(const Container *container) const;
    Container* deserializeContainer(const QJsonObject &json, QWidget *parent);
    QJsonObject serializeConnection(const Connection &conn) const;

    // Port definition lookup (single source of truth for all container types)
    static void getPortsForContainerType(const QString &type, QStringList &inputs, QStringList &outputs);

private:

    // *** Loading state flag ***
    bool m_isLoading = false;

    // Zoom state
    bool m_zoomMode = false;
    double m_zoomFactor = 1.0;
    QPoint m_panOffset;  // For future pan support

    // Zoom drag state
    bool m_isDraggingZoom = false;
    QPoint m_zoomDragStartPos;
    double m_zoomDragStartFactor = 1.0;

    // Pan state
    bool m_panMode = false;
    bool m_isDraggingPan = false;
    QPoint m_panDragStartPos;
    QPoint m_panDragStartOffset;
};

#endif // CANVAS_H

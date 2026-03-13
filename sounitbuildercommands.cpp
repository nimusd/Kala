#include "sounitbuildercommands.h"
#include <QDebug>

// ============================================================================
// Add Container Command
// ============================================================================

AddContainerCommand::AddContainerCommand(Container *container, Canvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_container(container)
    , m_canvas(canvas)
    , m_ownsContainer(false)
{
    setText(QString("Add %1").arg(container->getName()));
}

AddContainerCommand::~AddContainerCommand()
{
    // If the container is in "removed" state (undo was last action), we own it
    if (m_ownsContainer) {
        delete m_container;
    }
}

void AddContainerCommand::undo()
{
    m_canvas->removeContainer(m_container);
    m_ownsContainer = true;
    emit m_canvas->graphChanged();
    qDebug() << "Undo: Container removed -" << m_container->getInstanceName();
}

void AddContainerCommand::redo()
{
    m_canvas->restoreContainer(m_container, m_container->pos());
    m_ownsContainer = false;
    emit m_canvas->graphChanged();
    qDebug() << "Redo: Container restored -" << m_container->getInstanceName();
}

// ============================================================================
// Delete Container Command
// ============================================================================

DeleteContainerCommand::DeleteContainerCommand(Container *container, Canvas *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_container(container)
    , m_canvas(canvas)
    , m_position(container->pos())
    , m_ownsContainer(false)
{
    setText(QString("Delete %1").arg(container->getInstanceName()));

    // Save all connections involving this container
    const QVector<Canvas::Connection> &connections = canvas->getConnections();
    for (const Canvas::Connection &conn : connections) {
        if (conn.fromContainer == container || conn.toContainer == container) {
            m_savedConnections.append(conn);
        }
    }
}

DeleteContainerCommand::~DeleteContainerCommand()
{
    // If the container is in "removed" state (redo was last action), we own it
    if (m_ownsContainer) {
        delete m_container;
    }
}

void DeleteContainerCommand::undo()
{
    // Restore container to canvas
    m_canvas->restoreContainer(m_container, m_position);
    m_ownsContainer = false;

    // Restore all saved connections
    for (const Canvas::Connection &conn : m_savedConnections) {
        m_canvas->getConnections().append(conn);
    }

    m_canvas->update();
    emit m_canvas->graphChanged();
    qDebug() << "Undo: Container restored -" << m_container->getInstanceName()
             << "with" << m_savedConnections.size() << "connections";
}

void DeleteContainerCommand::redo()
{
    // Remove all connections involving this container
    QVector<Canvas::Connection> &connections = m_canvas->getConnections();
    for (int i = connections.size() - 1; i >= 0; --i) {
        if (connections[i].fromContainer == m_container ||
            connections[i].toContainer == m_container) {
            connections.removeAt(i);
        }
    }

    // Remove container from canvas
    m_canvas->removeContainer(m_container);
    m_ownsContainer = true;

    emit m_canvas->graphChanged();
    qDebug() << "Redo: Container deleted -" << m_container->getInstanceName();
}

// ============================================================================
// Move Container Command
// ============================================================================

MoveContainerCommand::MoveContainerCommand(Container *container, const QPoint &oldPos,
                                           const QPoint &newPos, Canvas *canvas,
                                           QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_container(container)
    , m_oldPos(oldPos)
    , m_newPos(newPos)
    , m_canvas(canvas)
{
    setText(QString("Move %1").arg(container->getInstanceName()));
}

void MoveContainerCommand::undo()
{
    m_container->move(m_oldPos);
    m_canvas->update();
    qDebug() << "Undo: Container moved back to" << m_oldPos;
}

void MoveContainerCommand::redo()
{
    m_container->move(m_newPos);
    m_canvas->update();
    qDebug() << "Redo: Container moved to" << m_newPos;
}

bool MoveContainerCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id())
        return false;

    const MoveContainerCommand *moveCmd = static_cast<const MoveContainerCommand*>(other);
    if (moveCmd->m_container != m_container)
        return false;

    // Update to the latest position
    m_newPos = moveCmd->m_newPos;
    return true;
}

// ============================================================================
// Add Connection Command
// ============================================================================

AddConnectionCommand::AddConnectionCommand(const Canvas::Connection &connection, Canvas *canvas,
                                           QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_connection(connection)
    , m_canvas(canvas)
{
    setText(QString("Connect %1.%2 -> %3.%4")
                .arg(connection.fromContainer->getInstanceName())
                .arg(connection.fromPort)
                .arg(connection.toContainer->getInstanceName())
                .arg(connection.toPort));
}

void AddConnectionCommand::undo()
{
    // Find and remove this connection
    QVector<Canvas::Connection> &connections = m_canvas->getConnections();
    for (int i = connections.size() - 1; i >= 0; --i) {
        const Canvas::Connection &conn = connections[i];
        if (conn.fromContainer == m_connection.fromContainer &&
            conn.fromPort == m_connection.fromPort &&
            conn.toContainer == m_connection.toContainer &&
            conn.toPort == m_connection.toPort) {
            connections.removeAt(i);
            break;
        }
    }

    m_canvas->update();
    emit m_canvas->graphChanged();
    qDebug() << "Undo: Connection removed";
}

void AddConnectionCommand::redo()
{
    m_canvas->getConnections().append(m_connection);
    m_canvas->update();
    emit m_canvas->graphChanged();
    qDebug() << "Redo: Connection added";
}

// ============================================================================
// Delete Connection Command
// ============================================================================

DeleteConnectionCommand::DeleteConnectionCommand(int connectionIndex, Canvas *canvas,
                                                 QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_index(connectionIndex)
    , m_canvas(canvas)
{
    // Store the connection before it's deleted
    const QVector<Canvas::Connection> &connections = canvas->getConnections();
    if (connectionIndex >= 0 && connectionIndex < connections.size()) {
        m_connection = connections[connectionIndex];
    }

    setText(QString("Delete Connection %1.%2 -> %3.%4")
                .arg(m_connection.fromContainer->getInstanceName())
                .arg(m_connection.fromPort)
                .arg(m_connection.toContainer->getInstanceName())
                .arg(m_connection.toPort));
}

void DeleteConnectionCommand::undo()
{
    // Re-insert connection at original index
    QVector<Canvas::Connection> &connections = m_canvas->getConnections();
    if (m_index <= connections.size()) {
        connections.insert(m_index, m_connection);
    } else {
        connections.append(m_connection);
    }

    m_canvas->update();
    emit m_canvas->graphChanged();
    qDebug() << "Undo: Connection restored at index" << m_index;
}

void DeleteConnectionCommand::redo()
{
    QVector<Canvas::Connection> &connections = m_canvas->getConnections();
    if (m_index >= 0 && m_index < connections.size()) {
        connections.removeAt(m_index);
    }

    m_canvas->update();
    emit m_canvas->graphChanged();
    qDebug() << "Redo: Connection deleted at index" << m_index;
}

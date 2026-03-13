#ifndef SOUNITBUILDERCOMMANDS_H
#define SOUNITBUILDERCOMMANDS_H

#include <QUndoCommand>
#include <QPoint>
#include <QVector>
#include "canvas.h"
#include "container.h"

// ============================================================================
// Add Container Command
// ============================================================================
class AddContainerCommand : public QUndoCommand
{
public:
    AddContainerCommand(Container *container, Canvas *canvas, QUndoCommand *parent = nullptr);
    ~AddContainerCommand() override;

    void undo() override;
    void redo() override;

private:
    Container *m_container;
    Canvas *m_canvas;
    bool m_ownsContainer;  // True when container is removed from canvas (undo state)
};

// ============================================================================
// Delete Container Command
// ============================================================================
class DeleteContainerCommand : public QUndoCommand
{
public:
    DeleteContainerCommand(Container *container, Canvas *canvas, QUndoCommand *parent = nullptr);
    ~DeleteContainerCommand() override;

    void undo() override;
    void redo() override;

private:
    Container *m_container;
    Canvas *m_canvas;
    QPoint m_position;
    QVector<Canvas::Connection> m_savedConnections;
    bool m_ownsContainer;  // True when container is removed from canvas (redo state)
};

// ============================================================================
// Move Container Command
// ============================================================================
class MoveContainerCommand : public QUndoCommand
{
public:
    MoveContainerCommand(Container *container, const QPoint &oldPos, const QPoint &newPos,
                         Canvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    int id() const override { return 1001; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    Container *m_container;
    QPoint m_oldPos;
    QPoint m_newPos;
    Canvas *m_canvas;
};

// ============================================================================
// Add Connection Command
// ============================================================================
class AddConnectionCommand : public QUndoCommand
{
public:
    AddConnectionCommand(const Canvas::Connection &connection, Canvas *canvas,
                         QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Canvas::Connection m_connection;
    Canvas *m_canvas;
};

// ============================================================================
// Delete Connection Command
// ============================================================================
class DeleteConnectionCommand : public QUndoCommand
{
public:
    DeleteConnectionCommand(int connectionIndex, Canvas *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Canvas::Connection m_connection;
    int m_index;
    Canvas *m_canvas;
};

#endif // SOUNITBUILDERCOMMANDS_H

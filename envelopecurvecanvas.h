#ifndef ENVELOPECURVECANVAS_H
#define ENVELOPECURVECANVAS_H

#include <QWidget>
#include "envelopelibraryDialog.h"

/**
 * EnvelopeCurveCanvas - Interactive envelope curve editor
 * 
 * Features:
 * - Click to add control points
 * - Drag to move points
 * - Right-click or drag off canvas to delete
 * - Shift+drag to snap to grid
 * - Visual feedback with grid and curve preview
 */
class EnvelopeCurveCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit EnvelopeCurveCanvas(QWidget *parent = nullptr);
    
    // Set/get curve data
    void setPoints(const QVector<EnvelopePoint> &points);
    QVector<EnvelopePoint> getPoints() const { return points; }
    
    // Reset to default
    void reset();

signals:
    void curveChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QVector<EnvelopePoint> points;
    int selectedPointIndex;
    int hoveredPointIndex;
    bool isDragging;
    
    // Convert between screen and curve coordinates
    QPointF curveToScreen(double time, double value) const;
    void screenToCurve(const QPoint &screen, double &time, double &value) const;
    
    // Find point near screen position
    int findPointAt(const QPoint &pos) const;
    
    // Snap to grid if shift is held
    void snapToGrid(double &time, double &value, bool snapEnabled) const;
    
    // Evaluate curve at given time
    double evaluateCurve(double time) const;
    
    // Drawing helpers
    void drawGrid(QPainter &painter);
    void drawCurve(QPainter &painter);
    void drawPoints(QPainter &painter);
    
    const int POINT_RADIUS = 6;
    const int GRID_DIVISIONS = 4;
};

#endif // ENVELOPECURVECANVAS_H

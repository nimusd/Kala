#include "envelopecurvecanvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>

EnvelopeCurveCanvas::EnvelopeCurveCanvas(QWidget *parent)
    : QWidget(parent)
    , selectedPointIndex(-1)
    , hoveredPointIndex(-1)
    , isDragging(false)
{
    setMinimumSize(400, 200);
    setMouseTracking(true);

    // Start with default linear rise
    reset();
}

void EnvelopeCurveCanvas::setPoints(const QVector<EnvelopePoint> &newPoints)
{
    points = newPoints;
    // Ensure points are sorted by time
    std::sort(points.begin(), points.end(), [](const EnvelopePoint &a, const EnvelopePoint &b) {
        return a.time < b.time;
    });
    update();
}

void EnvelopeCurveCanvas::reset()
{
    points.clear();
    // Default: simple linear rise (0,0) to (1,1)
    points.append(EnvelopePoint(0.0, 0.0, 0));
    points.append(EnvelopePoint(1.0, 1.0, 0));
    selectedPointIndex = -1;
    hoveredPointIndex = -1;
    isDragging = false;
    update();
    emit curveChanged();
}

void EnvelopeCurveCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Fill background
    painter.fillRect(rect(), QColor(30, 30, 30));

    // Draw grid and curve
    drawGrid(painter);
    drawCurve(painter);
    drawPoints(painter);
}

void EnvelopeCurveCanvas::drawGrid(QPainter &painter)
{
    painter.setPen(QPen(QColor(60, 60, 60), 1));

    // Vertical grid lines
    for (int i = 0; i <= GRID_DIVISIONS; ++i) {
        double fraction = static_cast<double>(i) / GRID_DIVISIONS;
        QPointF top = curveToScreen(fraction, 1.0);
        QPointF bottom = curveToScreen(fraction, 0.0);
        painter.drawLine(top, bottom);
    }

    // Horizontal grid lines
    for (int i = 0; i <= GRID_DIVISIONS; ++i) {
        double fraction = static_cast<double>(i) / GRID_DIVISIONS;
        QPointF left = curveToScreen(0.0, fraction);
        QPointF right = curveToScreen(1.0, fraction);
        painter.drawLine(left, right);
    }

    // Draw border
    painter.setPen(QPen(QColor(100, 100, 100), 2));
    QPointF topLeft = curveToScreen(0.0, 1.0);
    QPointF topRight = curveToScreen(1.0, 1.0);
    QPointF bottomLeft = curveToScreen(0.0, 0.0);
    QPointF bottomRight = curveToScreen(1.0, 0.0);
    painter.drawRect(QRectF(topLeft, bottomRight));
}

void EnvelopeCurveCanvas::drawCurve(QPainter &painter)
{
    if (points.size() < 2) return;

    painter.setPen(QPen(QColor(100, 200, 255), 2));

    // Draw curve segments between control points
    const int steps = 50;
    for (int i = 0; i < points.size() - 1; ++i) {
        const EnvelopePoint &p1 = points[i];
        const EnvelopePoint &p2 = points[i + 1];

        for (int step = 0; step < steps; ++step) {
            double t = static_cast<double>(step) / steps;
            double nextT = static_cast<double>(step + 1) / steps;

            // Interpolate based on curve type
            double value1, value2;
            double time1 = p1.time + t * (p2.time - p1.time);
            double time2 = p1.time + nextT * (p2.time - p1.time);

            if (p1.curveType == 0) {
                // Linear
                value1 = p1.value + t * (p2.value - p1.value);
                value2 = p1.value + nextT * (p2.value - p1.value);
            } else if (p1.curveType == 1) {
                // Smooth (cosine interpolation)
                double smoothT = (1.0 - std::cos(t * M_PI)) * 0.5;
                double smoothNextT = (1.0 - std::cos(nextT * M_PI)) * 0.5;
                value1 = p1.value + smoothT * (p2.value - p1.value);
                value2 = p1.value + smoothNextT * (p2.value - p1.value);
            } else {
                // Step (hold first value until next point)
                value1 = p1.value;
                value2 = (step == steps - 1) ? p2.value : p1.value;
            }

            QPointF pt1 = curveToScreen(time1, value1);
            QPointF pt2 = curveToScreen(time2, value2);
            painter.drawLine(pt1, pt2);
        }
    }
}

void EnvelopeCurveCanvas::drawPoints(QPainter &painter)
{
    for (int i = 0; i < points.size(); ++i) {
        QPointF screenPos = curveToScreen(points[i].time, points[i].value);

        // Determine color based on state
        QColor pointColor;
        if (i == selectedPointIndex) {
            pointColor = QColor(255, 200, 0);    // Yellow for selected
        } else if (i == hoveredPointIndex) {
            pointColor = QColor(150, 220, 255);  // Light blue for hovered
        } else {
            // Tint by curve type of the segment starting at this point
            switch (points[i].curveType) {
                case 1:  pointColor = QColor(80, 220, 150);  break;  // Smooth  - green
                case 2:  pointColor = QColor(255, 160, 60);  break;  // Step    - orange
                default: pointColor = QColor(100, 200, 255); break;  // Linear  - blue
            }
        }

        // Draw point
        painter.setPen(QPen(QColor(40, 40, 40), 2));
        painter.setBrush(pointColor);
        painter.drawEllipse(screenPos, POINT_RADIUS, POINT_RADIUS);
    }
}

QPointF EnvelopeCurveCanvas::curveToScreen(double time, double value) const
{
    const int margin = 20;
    int usableWidth = width() - 2 * margin;
    int usableHeight = height() - 2 * margin;

    double x = margin + time * usableWidth;
    double y = margin + (1.0 - value) * usableHeight;  // Flip Y axis

    return QPointF(x, y);
}

void EnvelopeCurveCanvas::screenToCurve(const QPoint &screen, double &time, double &value) const
{
    const int margin = 20;
    int usableWidth = width() - 2 * margin;
    int usableHeight = height() - 2 * margin;

    time = static_cast<double>(screen.x() - margin) / usableWidth;
    value = 1.0 - static_cast<double>(screen.y() - margin) / usableHeight;  // Flip Y axis

    // Clamp to valid range
    time = std::clamp(time, 0.0, 1.0);
    value = std::clamp(value, 0.0, 1.0);
}

int EnvelopeCurveCanvas::findPointAt(const QPoint &pos) const
{
    for (int i = 0; i < points.size(); ++i) {
        QPointF screenPos = curveToScreen(points[i].time, points[i].value);
        double distance = QLineF(screenPos, pos).length();
        if (distance <= POINT_RADIUS + 2) {
            return i;
        }
    }
    return -1;
}

void EnvelopeCurveCanvas::snapToGrid(double &time, double &value, bool snapEnabled) const
{
    if (!snapEnabled) return;

    double gridStep = 1.0 / GRID_DIVISIONS;
    time = std::round(time / gridStep) * gridStep;
    value = std::round(value / gridStep) * gridStep;
}

double EnvelopeCurveCanvas::evaluateCurve(double time) const
{
    if (points.isEmpty()) return 0.0;
    if (time <= points.first().time) return points.first().value;
    if (time >= points.last().time) return points.last().value;

    // Find the two points we're between
    for (int i = 0; i < points.size() - 1; ++i) {
        if (time >= points[i].time && time <= points[i + 1].time) {
            const EnvelopePoint &p1 = points[i];
            const EnvelopePoint &p2 = points[i + 1];

            double t = (time - p1.time) / (p2.time - p1.time);

            if (p1.curveType == 0) {
                // Linear
                return p1.value + t * (p2.value - p1.value);
            } else if (p1.curveType == 1) {
                // Smooth (cosine)
                double smoothT = (1.0 - std::cos(t * M_PI)) * 0.5;
                return p1.value + smoothT * (p2.value - p1.value);
            } else {
                // Step
                return p1.value;
            }
        }
    }

    return 0.0;
}

void EnvelopeCurveCanvas::mousePressEvent(QMouseEvent *event)
{
    int pointIndex = findPointAt(event->pos());

    if (event->button() == Qt::LeftButton && (event->modifiers() & Qt::ControlModifier)) {
        if (pointIndex >= 0) {
            // Ctrl+click: cycle curve type Linear → Smooth → Step → Linear
            points[pointIndex].curveType = (points[pointIndex].curveType + 1) % 3;
            emit curveChanged();
            update();
            return;
        }
    }

    if (event->button() == Qt::LeftButton) {
        if (pointIndex >= 0) {
            // Start dragging existing point
            selectedPointIndex = pointIndex;
            isDragging = true;
        } else {
            // Add new point
            double time, value;
            screenToCurve(event->pos(), time, value);
            snapToGrid(time, value, event->modifiers() & Qt::ShiftModifier);

            // Insert point in sorted order
            EnvelopePoint newPoint(time, value, 0);
            int insertIndex = 0;
            for (int i = 0; i < points.size(); ++i) {
                if (points[i].time < time) {
                    insertIndex = i + 1;
                }
            }
            points.insert(insertIndex, newPoint);
            selectedPointIndex = insertIndex;
            isDragging = true;

            emit curveChanged();
        }
        update();
    } else if (event->button() == Qt::RightButton) {
        if (pointIndex >= 0) {
            // Delete point (but keep at least 2 points)
            if (points.size() > 2) {
                points.removeAt(pointIndex);
                selectedPointIndex = -1;
                emit curveChanged();
                update();
            }
        }
    }
}

void EnvelopeCurveCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging && selectedPointIndex >= 0) {
        double time, value;
        screenToCurve(event->pos(), time, value);
        snapToGrid(time, value, event->modifiers() & Qt::ShiftModifier);

        // Don't allow moving first or last point horizontally
        if (selectedPointIndex == 0) {
            time = 0.0;
        } else if (selectedPointIndex == points.size() - 1) {
            time = 1.0;
        }

        points[selectedPointIndex].time = time;
        points[selectedPointIndex].value = value;

        // Re-sort if needed (but keep selected index valid)
        std::sort(points.begin(), points.end(), [](const EnvelopePoint &a, const EnvelopePoint &b) {
            return a.time < b.time;
        });

        emit curveChanged();
        update();
    } else {
        // Update hovered point
        int newHoveredIndex = findPointAt(event->pos());
        if (newHoveredIndex != hoveredPointIndex) {
            hoveredPointIndex = newHoveredIndex;
            update();
        }
    }

    // Delete point if dragged off canvas
    if (isDragging && selectedPointIndex >= 0 && points.size() > 2) {
        if (!rect().contains(event->pos())) {
            points.removeAt(selectedPointIndex);
            selectedPointIndex = -1;
            isDragging = false;
            emit curveChanged();
            update();
        }
    }
}

void EnvelopeCurveCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    isDragging = false;
}

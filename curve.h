#ifndef CURVE_H
#define CURVE_H

#include <QVector>
#include <QPair>
#include <QJsonObject>
#include <QJsonArray>

/**
 * Curve - Stores parameter values over time
 *
 * Used for dynamics, pitch modulation, and other time-varying parameters.
 * Stores (time, value) pairs and provides linear interpolation between points.
 */
class Curve
{
public:
    // Point in the curve: (normalized time 0.0-1.0, value, pressure)
    struct Point {
        double time;      // Normalized time within note (0.0 = start, 1.0 = end)
        double value;     // Parameter value
        double pressure;  // Pen pressure (0.0-1.0), default 1.0 for mouse input

        Point(double t = 0.0, double v = 0.0, double p = 1.0)
            : time(t), value(v), pressure(p) {}
    };

    Curve();
    explicit Curve(double constantValue);  // Create constant curve

    // Point management
    void addPoint(double time, double value);
    void addPoint(double time, double value, double pressure);
    void addPoint(const Point &point);
    void clearPoints();
    int getPointCount() const { return points.size(); }
    const QVector<Point>& getPoints() const { return points; }

    // Value query with interpolation
    double valueAt(double time) const;
    double pressureAt(double time) const;

    // Utility
    bool isEmpty() const { return points.isEmpty(); }
    void sortPoints();  // Ensure points are sorted by time

    // Serialization
    QJsonObject toJson() const;
    static Curve fromJson(const QJsonObject &json);

private:
    QVector<Point> points;
};

// Rebuild a curve after dragging one of its on-screen control dots.
//
// "Never coarsen, but still refine" (D8 in Claude-curve-lanes.md). Dots sit at
// i/(numDots+1) for i in 1..numDots, and numDots is derived from the note's
// pixel width - so stretching a note wide genuinely buys editing resolution,
// which is a technique in daily use and must keep working. The naive rebuild
// (clear, re-sample at numDots+2) delivered that but destroyed detail: one drag
// on a narrow note replaced a 30-point dialog-drawn curve with 6 points.
//
// This keeps every point the curve already has, refining only when it carries
// fewer points than the dots on screen, then displaces points near the dragged
// dot with a linear falloff reaching zero at the adjacent dots. Detail
// elsewhere survives untouched and the endpoints stay pinned.
//
// Regression-tested by test_curve_dotdrag.cpp.
Curve curveAfterDotDrag(const Curve &start, int numDots, int dotIndex, double newValue);

#endif // CURVE_H

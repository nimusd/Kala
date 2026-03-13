#include "curve.h"
#include <algorithm>
#include <QJsonObject>
#include <QJsonArray>

Curve::Curve()
{
}

Curve::Curve(double constantValue)
{
    // Create a flat curve with two points
    addPoint(0.0, constantValue);
    addPoint(1.0, constantValue);
}

void Curve::addPoint(double time, double value)
{
    points.append(Point(time, value, 1.0));  // Default pressure = 1.0
}

void Curve::addPoint(double time, double value, double pressure)
{
    points.append(Point(time, value, pressure));
}

void Curve::addPoint(const Point &point)
{
    points.append(point);
}

void Curve::clearPoints()
{
    points.clear();
}

void Curve::sortPoints()
{
    std::sort(points.begin(), points.end(),
              [](const Point &a, const Point &b) { return a.time < b.time; });
}

double Curve::valueAt(double time) const
{
    if (points.isEmpty()) {
        return 0.0;  // Default value
    }

    if (points.size() == 1) {
        return points.first().value;
    }

    // Clamp time to [0.0, 1.0]
    time = qBound(0.0, time, 1.0);

    // Find the two points to interpolate between
    int i = 0;
    for (; i < points.size() - 1; ++i) {
        if (time <= points[i + 1].time) {
            break;
        }
    }

    // If we're at or before the first point
    if (i == 0 && time <= points[0].time) {
        return points[0].value;
    }

    // If we're at or after the last point
    if (i >= points.size() - 1) {
        return points.last().value;
    }

    // Linear interpolation between points[i] and points[i+1]
    const Point &p1 = points[i];
    const Point &p2 = points[i + 1];

    double timeDiff = p2.time - p1.time;
    if (timeDiff < 0.0001) {  // Avoid division by zero
        return p1.value;
    }

    double t = (time - p1.time) / timeDiff;  // Normalized between the two points
    return p1.value + t * (p2.value - p1.value);
}

double Curve::pressureAt(double time) const
{
    if (points.isEmpty()) {
        return 1.0;  // Default pressure (uniform thickness)
    }

    if (points.size() == 1) {
        return points.first().pressure;
    }

    // Clamp time to [0.0, 1.0]
    time = qBound(0.0, time, 1.0);

    // Find the two points to interpolate between
    int i = 0;
    for (; i < points.size() - 1; ++i) {
        if (time <= points[i + 1].time) {
            break;
        }
    }

    // If we're at or before the first point
    if (i == 0 && time <= points[0].time) {
        return points[0].pressure;
    }

    // If we're at or after the last point
    if (i >= points.size() - 1) {
        return points.last().pressure;
    }

    // Linear interpolation between points[i] and points[i+1]
    const Point &p1 = points[i];
    const Point &p2 = points[i + 1];

    double timeDiff = p2.time - p1.time;
    if (timeDiff < 0.0001) {  // Avoid division by zero
        return p1.pressure;
    }

    double t = (time - p1.time) / timeDiff;  // Normalized between the two points
    return p1.pressure + t * (p2.pressure - p1.pressure);
}

QJsonObject Curve::toJson() const
{
    QJsonObject json;
    QJsonArray pointsArray;

    for (const Point &p : points) {
        QJsonObject pointJson;
        pointJson["time"] = p.time;
        pointJson["value"] = p.value;
        pointJson["pressure"] = p.pressure;
        pointsArray.append(pointJson);
    }

    json["points"] = pointsArray;
    return json;
}

Curve Curve::fromJson(const QJsonObject &json)
{
    Curve curve;

    QJsonArray pointsArray = json["points"].toArray();
    for (const QJsonValue &val : pointsArray) {
        QJsonObject pointJson = val.toObject();
        double time = pointJson["time"].toDouble(0.0);
        double value = pointJson["value"].toDouble(0.0);
        double pressure = pointJson["pressure"].toDouble(1.0);
        curve.addPoint(time, value, pressure);
    }

    return curve;
}

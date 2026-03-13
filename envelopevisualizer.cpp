#include "envelopevisualizer.h"
#include <QPainter>
#include <QPainterPath>
#include <cmath>

EnvelopeVisualizer::EnvelopeVisualizer(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void EnvelopeVisualizer::setEnvelope(int type, double a, double d, double s, double r, double fade)
{
    envelopeType = type;
    attack = a;
    decay = d;
    sustain = s;
    release = r;
    fadeTime = fade;
    update();
}

void EnvelopeVisualizer::setCustomEnvelope(const QVector<EnvelopePoint> &points)
{
    envelopeType = 7;
    customPoints = points;
    update();
}

double EnvelopeVisualizer::calculateEnvelopeValue(double t)
{
    // t is normalized time from 0.0 to 1.0
    switch (envelopeType) {
    case 0: // Linear
        return t;

    case 1: // Attack-Decay
        {
            double totalTime = attack + decay;
            if (totalTime <= 0.0) return 0.0;
            
            double scaledTime = t * totalTime;
            if (scaledTime < attack) {
                // Attack phase
                return scaledTime / attack;
            } else {
                // Decay phase
                double decayProgress = (scaledTime - attack) / decay;
                return 1.0 - decayProgress;
            }
        }

    case 2: // ADSR
        {
            double totalTime = attack + decay + release;
            double sustainTime = 1.0 - totalTime;
            if (sustainTime < 0.0) sustainTime = 0.0;
            
            double scaledTime = t;
            
            if (scaledTime < attack) {
                // Attack phase
                return scaledTime / attack;
            } else if (scaledTime < attack + decay) {
                // Decay phase
                double decayProgress = (scaledTime - attack) / decay;
                return 1.0 - decayProgress * (1.0 - sustain);
            } else if (scaledTime < attack + decay + sustainTime) {
                // Sustain phase
                return sustain;
            } else {
                // Release phase
                double releaseProgress = (scaledTime - attack - decay - sustainTime) / release;
                return sustain * (1.0 - releaseProgress);
            }
        }

    case 3: // Fade In
        {
            if (t < fadeTime) {
                return t / fadeTime;
            } else {
                return 1.0;
            }
        }

    case 4: // Fade Out
        {
            if (t < (1.0 - fadeTime)) {
                return 1.0;
            } else {
                double fadeProgress = (t - (1.0 - fadeTime)) / fadeTime;
                return 1.0 - fadeProgress;
            }
        }

    case 5: // Log Fade In
        return std::log10(1.0 + 9.0 * t);

    case 6: // Log Fade Out
        return 1.0 - std::log10(1.0 + 9.0 * t);

    case 7: // Custom
        return calculateCustomEnvelopeValue(t);

    default:
        return t;
    }
}

double EnvelopeVisualizer::calculateCustomEnvelopeValue(double t)
{
    if (customPoints.isEmpty()) return 0.0;
    if (customPoints.size() == 1) return customPoints.first().value;

    // Clamp to bounds
    if (t <= customPoints.first().time) {
        return customPoints.first().value;
    }
    if (t >= customPoints.last().time) {
        return customPoints.last().value;
    }

    // Find the two points we're between
    for (int i = 0; i < customPoints.size() - 1; ++i) {
        const EnvelopePoint &p1 = customPoints[i];
        const EnvelopePoint &p2 = customPoints[i + 1];

        if (t >= p1.time && t <= p2.time) {
            // Calculate interpolation parameter
            double param = (t - p1.time) / (p2.time - p1.time);

            // Interpolate based on curve type
            if (p1.curveType == 0) {
                // Linear interpolation
                return p1.value + param * (p2.value - p1.value);
            } else if (p1.curveType == 1) {
                // Smooth (cosine) interpolation
                double smoothParam = (1.0 - std::cos(param * M_PI)) * 0.5;
                return p1.value + smoothParam * (p2.value - p1.value);
            } else {
                // Step (hold first value until next point)
                return p1.value;
            }
        }
    }

    return 0.0;
}

void EnvelopeVisualizer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    painter.fillRect(rect(), QColor("#2c3e50"));

    // Grid
    painter.setPen(QPen(QColor("#34495e"), 1));
    for (int i = 1; i < 4; i++) {
        int y = height() * i / 4;
        painter.drawLine(0, y, width(), y);
    }
    for (int i = 1; i < 4; i++) {
        int x = width() * i / 4;
        painter.drawLine(x, 0, x, height());
    }

    // Draw envelope curve
    QPainterPath path;
    bool firstPoint = true;

    int numPoints = width();
    for (int x = 0; x < numPoints; x++) {
        double t = static_cast<double>(x) / (numPoints - 1);
        double value = calculateEnvelopeValue(t);
        
        // Map value (0.0-1.0) to screen coordinates (bottom to top)
        int screenY = height() - static_cast<int>(value * height());
        
        if (firstPoint) {
            path.moveTo(x, screenY);
            firstPoint = false;
        } else {
            path.lineTo(x, screenY);
        }
    }

    // Draw the curve
    painter.setPen(QPen(QColor("#3498db"), 2));
    painter.drawPath(path);

    // Draw axis labels
    painter.setPen(QColor("#ecf0f1"));
    painter.setFont(QFont("Arial", 8));
    painter.drawText(5, height() - 5, "0.0");
    painter.drawText(width() - 25, height() - 5, "1.0");
    painter.drawText(5, 12, "1.0");
    painter.drawText(5, height() / 2 + 5, "0.5");
}

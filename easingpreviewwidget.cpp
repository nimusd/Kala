#include "easingpreviewwidget.h"
#include <QPainter>
#include <QPainterPath>

EasingPreviewWidget::EasingPreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(180, 120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void EasingPreviewWidget::setEasing(const Easing &easing)
{
    currentEasing = easing;
    update();
}

QSize EasingPreviewWidget::sizeHint() const
{
    return QSize(200, 140);
}

QSize EasingPreviewWidget::minimumSizeHint() const
{
    return QSize(180, 120);
}

void EasingPreviewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Calculate drawing area
    int graphLeft = MARGIN;
    int graphRight = width() - 10;
    int graphTop = 10;
    int graphBottom = height() - MARGIN;
    int graphWidth = graphRight - graphLeft;
    int graphHeight = graphBottom - graphTop;

    // Fill background
    painter.fillRect(rect(), QColor(250, 250, 250));

    // Draw graph background
    painter.fillRect(graphLeft, graphTop, graphWidth, graphHeight, Qt::white);

    // Draw grid lines (light gray)
    painter.setPen(QPen(QColor(230, 230, 230), 1));
    for (int i = 1; i < 4; ++i) {
        int x = graphLeft + (graphWidth * i) / 4;
        int y = graphTop + (graphHeight * i) / 4;
        painter.drawLine(x, graphTop, x, graphBottom);
        painter.drawLine(graphLeft, y, graphRight, y);
    }

    // Draw border
    painter.setPen(QPen(QColor(180, 180, 180), 1));
    painter.drawRect(graphLeft, graphTop, graphWidth, graphHeight);

    // Sample the easing curve and find min/max for scaling
    QVector<double> samples(SAMPLE_COUNT + 1);
    double minVal = 0.0;
    double maxVal = 1.0;

    for (int i = 0; i <= SAMPLE_COUNT; ++i) {
        double t = static_cast<double>(i) / SAMPLE_COUNT;
        double val = currentEasing.calculate(t);
        samples[i] = val;
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
    }

    // Add some padding to the range if there's overshoot
    double range = maxVal - minVal;
    if (range < 1.0) range = 1.0;

    // Function to map value to Y coordinate
    auto valueToY = [&](double val) -> double {
        // Map value to graph coordinates (inverted because Y grows downward)
        double normalized = (val - minVal) / range;
        return graphBottom - normalized * graphHeight;
    };

    // Draw linear reference line (diagonal, dashed)
    painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::DashLine));
    painter.drawLine(
        graphLeft, valueToY(0.0),
        graphRight, valueToY(1.0)
    );

    // Draw 0 and 1 horizontal reference lines if there's overshoot
    if (minVal < 0.0 || maxVal > 1.0) {
        painter.setPen(QPen(QColor(255, 200, 200), 1, Qt::DotLine));
        if (minVal < 0.0) {
            int y0 = static_cast<int>(valueToY(0.0));
            painter.drawLine(graphLeft, y0, graphRight, y0);
        }
        if (maxVal > 1.0) {
            int y1 = static_cast<int>(valueToY(1.0));
            painter.drawLine(graphLeft, y1, graphRight, y1);
        }
    }

    // Draw the easing curve
    QPainterPath path;
    for (int i = 0; i <= SAMPLE_COUNT; ++i) {
        double t = static_cast<double>(i) / SAMPLE_COUNT;
        double x = graphLeft + t * graphWidth;
        double y = valueToY(samples[i]);

        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }

    // Draw curve with nice blue color
    painter.setPen(QPen(QColor(50, 100, 200), 2));
    painter.drawPath(path);

    // Draw overshoot regions with subtle red tint
    if (currentEasing.canOvershoot()) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 100, 100, 30));
        if (minVal < 0.0) {
            int y0 = static_cast<int>(valueToY(0.0));
            painter.drawRect(graphLeft, y0, graphWidth, graphBottom - y0);
        }
        if (maxVal > 1.0) {
            int y1 = static_cast<int>(valueToY(1.0));
            painter.drawRect(graphLeft, graphTop, graphWidth, y1 - graphTop);
        }
    }

    // Draw axis labels
    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    // X-axis labels
    painter.drawText(graphLeft - 2, graphBottom + 12, "0");
    painter.drawText(graphRight - 6, graphBottom + 12, "1");

    // Y-axis labels (show actual range if overshoot)
    QString minLabel = QString::number(minVal, 'f', 1);
    QString maxLabel = QString::number(maxVal, 'f', 1);
    painter.drawText(2, graphBottom + 4, minLabel);
    painter.drawText(2, graphTop + 10, maxLabel);

    // Draw "t" label for x-axis
    painter.drawText(graphLeft + graphWidth / 2 - 3, graphBottom + 14, "t");
}

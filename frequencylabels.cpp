#include "frequencylabels.h"
#include <QPainter>
#include <QFont>
#include <cmath>

FrequencyLabels::FrequencyLabels(QWidget *parent)
    : QWidget(parent)
    , visibleMinHz(27.5)
    , visibleMaxHz(4186.0)
    , verticalScrollOffset(0)
    , canvasHeight(2000)  // Default canvas height, will be synchronized
{
    setMinimumWidth(LABEL_WIDTH);
    setMaximumWidth(LABEL_WIDTH);
    setMinimumHeight(2000);
    setStyleSheet("background-color: white;");

    generateScaleLines();
}

void FrequencyLabels::setFrequencyRange(double minHz, double maxHz)
{
    visibleMinHz = minHz;
    visibleMaxHz = maxHz;
    update();
}

void FrequencyLabels::setVerticalOffset(int offsetPixels)
{
    verticalScrollOffset = offsetPixels;
    update();
}

void FrequencyLabels::setCanvasHeight(int height)
{
    canvasHeight = height;
    update();
}

void FrequencyLabels::setScaleLines(const QVector<ScaleLine> &lines)
{
    scaleLines = lines;
    update();
}

void FrequencyLabels::generateScaleLines()
{
    scaleLines.clear();

    // Just intonation major scale ratios (same as ScoreCanvas)
    const double majorScaleRatios[7] = {
        1.0,        // C (Tonic)
        9.0/8.0,    // D
        5.0/4.0,    // E
        4.0/3.0,    // F
        3.0/2.0,    // G (Fifth)
        5.0/3.0,    // A
        15.0/8.0    // B
    };

    const QString noteNames[7] = {"C", "D", "E", "F", "G", "A", "B"};

    // Generate scale lines from 25 Hz to 8000 Hz
    for (int octave = 0; octave <= 9; ++octave) {
        double octaveBaseFreq = BASE_FREQUENCY * std::pow(2.0, octave);

        for (int degree = 0; degree < 7; ++degree) {
            double freq = octaveBaseFreq * majorScaleRatios[degree];

            if (freq >= 20.0 && freq <= 8000.0) {
                ScaleLine line;
                line.frequencyHz = freq;
                line.scaleDegree = degree + 1;
                line.noteName = noteNames[degree] + QString::number(octave);
                line.isThicker = (degree == 0 || degree == 4);

                scaleLines.append(line);
            }
        }
    }

    update();
}

int FrequencyLabels::frequencyToPixel(double hz) const
{
    if (hz <= 0 || BASE_FREQUENCY <= 0) {
        return 0;
    }

    double octaveNumber = std::log2(hz / BASE_FREQUENCY);
    double minOctave = std::log2(visibleMinHz / BASE_FREQUENCY);
    double maxOctave = std::log2(visibleMaxHz / BASE_FREQUENCY);
    double visibleOctaveRange = maxOctave - minOctave;

    double normalizedPos = (octaveNumber - minOctave) / visibleOctaveRange;
    int pixel = canvasHeight - static_cast<int>(normalizedPos * canvasHeight);
    return pixel;
}

void FrequencyLabels::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw background
    painter.fillRect(rect(), Qt::white);

    // Draw frequency labels
    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    for (const ScaleLine &line : scaleLines) {
        int y = frequencyToPixel(line.frequencyHz);

        // Only draw if visible
        if (y >= -10 && y <= height() + 10) {
            QString label = QString::number(line.frequencyHz, 'f', 1) + " Hz";
            QRect labelRect(0, y - 8, LABEL_WIDTH - 5, 16);
            painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, label);
        }
    }

    // Draw right border
    painter.setPen(QColor("#ccc"));
    painter.drawLine(width() - 1, 0, width() - 1, height());
}

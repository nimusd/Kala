#include "spectrumvisualizer.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QKeyEvent>
#include <cmath>

SpectrumVisualizer::SpectrumVisualizer(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(200, 300);
    setMaximumHeight(400);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Start with empty spectrum
    harmonicAmplitudes.clear();

    // Enable mouse tracking for hover effects
    setMouseTracking(true);

    // Enable keyboard focus for delete key
    setFocusPolicy(Qt::StrongFocus);
}

void SpectrumVisualizer::setSpectrum(const QVector<double> &amplitudes)
{
    harmonicAmplitudes = amplitudes;
    update();  // Trigger repaint
}

void SpectrumVisualizer::setParameters(int numHarmonics, double rolloffPower)
{
    generatePreviewSpectrum(numHarmonics, rolloffPower);
    update();  // Trigger repaint
}

void SpectrumVisualizer::setDnaPreset(int dnaPreset, int numHarmonics, double purity)
{
    // Limit to reasonable display size
    int displayCount = std::min(numHarmonics, maxHarmonicsToShow);
    harmonicAmplitudes.resize(displayCount);

    // First, generate the base DNA pattern
    QVector<double> basePattern(displayCount, 0.0);
    double totalAmp = 0.0;

    switch (dnaPreset) {
    case 0: // Vocal (Bright)
        for (int i = 0; i < displayCount; i++) {
            int harmonicNum = i + 1;
            double amp = 1.0 / std::pow(harmonicNum, 1.2);
            if ((harmonicNum >= 3 && harmonicNum <= 5) || (harmonicNum >= 8 && harmonicNum <= 12)) {
                amp *= 1.4;
            }
            basePattern[i] = amp;
            totalAmp += amp;
        }
        break;

    case 1: // Vocal (Warm)
        for (int i = 0; i < displayCount; i++) {
            int harmonicNum = i + 1;
            double amp = 1.0 / std::pow(harmonicNum, 1.6);
            if (harmonicNum >= 2 && harmonicNum <= 4) {
                amp *= 1.5;
            }
            basePattern[i] = amp;
            totalAmp += amp;
        }
        break;

    case 2: // Brass (Trumpet-like)
        for (int i = 0; i < displayCount; i++) {
            int harmonicNum = i + 1;
            double amp = 1.0 / std::pow(harmonicNum, 1.5);
            if (harmonicNum % 2 == 1) {
                amp *= 1.3;
            }
            basePattern[i] = amp;
            totalAmp += amp;
        }
        break;

    case 3: // Reed (Clarinet-like)
        for (int i = 0; i < displayCount; i++) {
            int harmonicNum = i + 1;
            if (harmonicNum % 2 == 1) {
                double amp = 1.0 / std::pow(harmonicNum, 1.4);
                basePattern[i] = amp;
                totalAmp += amp;
            } else {
                double amp = 0.1 / std::pow(harmonicNum, 2.0);
                basePattern[i] = amp;
                totalAmp += amp;
            }
        }
        break;

    case 4: // String (Violin-like)
        for (int i = 0; i < displayCount; i++) {
            int harmonicNum = i + 1;
            double amp = 1.0 / std::pow(harmonicNum, 1.3);
            if (harmonicNum >= 5 && harmonicNum <= 10) {
                amp *= 1.2;
            }
            basePattern[i] = amp;
            totalAmp += amp;
        }
        break;

    default: // Custom or unknown - use simple rolloff
        for (int i = 0; i < displayCount; i++) {
            int harmonicNum = i + 1;
            double amp = 1.0 / std::pow(harmonicNum, 1.82);
            basePattern[i] = amp;
            totalAmp += amp;
        }
        break;
    }

    // Normalize base pattern
    if (totalAmp > 0.0) {
        for (int i = 0; i < displayCount; i++) {
            basePattern[i] /= totalAmp;
        }
    }

    // Apply purity blend: mix DNA pattern with flat spectrum
    // purity 0.0 = pure DNA pattern
    // purity 1.0 = all harmonics equal (flat with gentle rolloff)
    totalAmp = 0.0;
    for (int i = 0; i < displayCount; i++) {
        int harmonicNum = i + 1;
        double dnaAmp = basePattern[i];
        double flatAmp = 1.0 / harmonicNum;
        harmonicAmplitudes[i] = dnaAmp * (1.0 - purity) + flatAmp * purity;
        totalAmp += harmonicAmplitudes[i];
    }

    // Final normalization
    if (totalAmp > 0.0) {
        for (int i = 0; i < displayCount; i++) {
            harmonicAmplitudes[i] /= totalAmp;
        }
    }

    update();  // Trigger repaint
}

void SpectrumVisualizer::generatePreviewSpectrum(int numHarmonics, double rolloffPower)
{
    // Limit to reasonable display size
    int displayCount = std::min(numHarmonics, maxHarmonicsToShow);
    harmonicAmplitudes.resize(displayCount);

    // Generate simple rolloff spectrum
    double totalAmp = 0.0;
    for (int i = 0; i < displayCount; i++) {
        int harmonicNum = i + 1;
        double amp = 1.0 / std::pow(harmonicNum, rolloffPower);
        harmonicAmplitudes[i] = amp;
        totalAmp += amp;
    }

    // Normalize
    if (totalAmp > 0.0) {
        for (int i = 0; i < displayCount; i++) {
            harmonicAmplitudes[i] /= totalAmp;
        }
    }
}

void SpectrumVisualizer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    painter.fillRect(rect(), QColor(30, 30, 35));

    if (harmonicAmplitudes.isEmpty()) {
        // Draw "No Data" text
        painter.setPen(QColor(150, 150, 150));
        painter.drawText(rect(), Qt::AlignCenter, "No spectrum");
        return;
    }

    // Calculate bar width and spacing
    int numBars = harmonicAmplitudes.size();
    int margin = 10;
    int availableWidth = width() - (2 * margin);
    double barWidth = static_cast<double>(availableWidth) / numBars;

    // Clamp bar width for visual clarity
    if (barWidth < 1.0) barWidth = 1.0;
    if (barWidth > 20.0) barWidth = 20.0;

    // Find max amplitude for scaling
    double maxAmp = 0.0;
    for (double amp : harmonicAmplitudes) {
        if (amp > maxAmp) maxAmp = amp;
    }

    if (maxAmp == 0.0) maxAmp = 1.0;  // Avoid division by zero

    // Draw harmonic bars
    int barHeight = height() - (2 * margin);
    for (int i = 0; i < numBars; i++) {
        double normalizedAmp = harmonicAmplitudes[i] / maxAmp;
        int h = static_cast<int>(normalizedAmp * barHeight);

        // Ensure visible harmonics have at least 1 pixel height if they have non-zero amplitude
        if (h == 0 && harmonicAmplitudes[i] > 0.0) {
            h = 1;
        }

        int x = margin + static_cast<int>(i * barWidth);
        int y = height() - margin - h;

        // Color gradient based on harmonic number (lower harmonics = warmer colors)
        QColor barColor;
        if (i < 8) {
            // Fundamental and low harmonics - orange/red
            barColor = QColor(255, 120 + i * 15, 50);
        } else if (i < 16) {
            // Mid harmonics - yellow
            barColor = QColor(255, 200, 100 - (i - 8) * 10);
        } else {
            // High harmonics - blue/cyan
            barColor = QColor(100, 180, 255 - (i - 16) * 3);
        }

        painter.fillRect(x, y, static_cast<int>(barWidth - 1), h, barColor);
    }

    // Draw grid lines (horizontal)
    painter.setPen(QColor(60, 60, 70));
    for (int i = 1; i <= 4; i++) {
        int y = margin + (barHeight * i) / 4;
        painter.drawLine(margin, y, width() - margin, y);
    }

    // Draw border
    painter.setPen(QColor(100, 100, 110));
    painter.drawRect(0, 0, width() - 1, height() - 1);

    // In edit mode, highlight selected harmonic
    if (editMode && selectedHarmonic >= 0 && selectedHarmonic < numBars) {
        int x = margin + static_cast<int>(selectedHarmonic * barWidth);
        double normalizedAmp = harmonicAmplitudes[selectedHarmonic] / maxAmp;
        int h = static_cast<int>(normalizedAmp * barHeight);
        if (h == 0 && harmonicAmplitudes[selectedHarmonic] > 0.0) {
            h = 1;
        }
        int y = height() - margin - h;

        // Draw selection outline
        painter.setPen(QPen(QColor(255, 255, 0), 2));  // Yellow outline
        painter.drawRect(x, y, static_cast<int>(barWidth - 1), h);

        // Draw amplitude value at top
        QString ampText = QString::number(harmonicAmplitudes[selectedHarmonic], 'f', 3);
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(x, y - 5, ampText);
    }
}

void SpectrumVisualizer::setEditable(bool editable)
{
    editMode = editable;
    selectedHarmonic = -1;  // Clear selection when toggling mode
    dragging = false;
    update();
}

int SpectrumVisualizer::getHarmonicAtX(int x) const
{
    if (harmonicAmplitudes.isEmpty()) return -1;

    int margin = 10;
    int availableWidth = width() - (2 * margin);
    int numBars = harmonicAmplitudes.size();
    double barWidth = static_cast<double>(availableWidth) / numBars;

    if (barWidth < 1.0) barWidth = 1.0;
    if (barWidth > 20.0) barWidth = 20.0;

    // Check if x is within the bar area
    if (x < margin || x >= width() - margin) return -1;

    int index = static_cast<int>((x - margin) / barWidth);
    if (index >= 0 && index < numBars) {
        return index;
    }
    return -1;
}

bool SpectrumVisualizer::isOverBarTop(int x, int y, int harmonicIndex) const
{
    if (harmonicIndex < 0 || harmonicIndex >= harmonicAmplitudes.size()) return false;

    int margin = 10;
    int availableWidth = width() - (2 * margin);
    int numBars = harmonicAmplitudes.size();
    double barWidth = static_cast<double>(availableWidth) / numBars;

    if (barWidth < 1.0) barWidth = 1.0;
    if (barWidth > 20.0) barWidth = 20.0;

    // Find max amplitude for scaling
    double maxAmp = 0.0;
    for (double amp : harmonicAmplitudes) {
        if (amp > maxAmp) maxAmp = amp;
    }
    if (maxAmp == 0.0) maxAmp = 1.0;

    int barHeight = height() - (2 * margin);
    double normalizedAmp = harmonicAmplitudes[harmonicIndex] / maxAmp;
    int h = static_cast<int>(normalizedAmp * barHeight);
    if (h == 0 && harmonicAmplitudes[harmonicIndex] > 0.0) {
        h = 1;
    }

    int barX = margin + static_cast<int>(harmonicIndex * barWidth);
    int barY = height() - margin - h;

    // Check if mouse is within horizontal bounds of bar
    if (x < barX || x >= barX + static_cast<int>(barWidth)) return false;

    // Define "top area" as top 20 pixels of bar (or entire bar if shorter)
    int topAreaHeight = std::min(20, h);
    return (y >= barY && y <= barY + topAreaHeight);
}

void SpectrumVisualizer::setHarmonicAmplitude(int index, double amplitude)
{
    if (index < 0 || index >= harmonicAmplitudes.size()) return;

    // Clamp amplitude to valid range
    amplitude = std::clamp(amplitude, 0.0, 1.0);
    harmonicAmplitudes[index] = amplitude;

    update();
    emit spectrumChanged(harmonicAmplitudes);
}

void SpectrumVisualizer::mousePressEvent(QMouseEvent *event)
{
    if (!editMode) return;

    int harmonicIndex = getHarmonicAtX(event->pos().x());
    if (harmonicIndex < 0) return;

    // Select the harmonic
    selectedHarmonic = harmonicIndex;

    // Check if clicking on the draggable top area
    if (isOverBarTop(event->pos().x(), event->pos().y(), harmonicIndex)) {
        dragging = true;
        dragStartY = event->pos().y();
        dragStartAmp = harmonicAmplitudes[harmonicIndex];
        setCursor(Qt::SizeVerCursor);
    }

    update();
}

void SpectrumVisualizer::mouseMoveEvent(QMouseEvent *event)
{
    if (!editMode) return;

    if (dragging && selectedHarmonic >= 0) {
        // Calculate amplitude change based on vertical mouse movement
        int margin = 10;
        int barHeight = height() - (2 * margin);

        // Invert delta (moving up = increase amplitude)
        int deltaY = dragStartY - event->pos().y();
        double amplitudeChange = static_cast<double>(deltaY) / barHeight;

        // Find max amplitude for scaling
        double maxAmp = 0.0;
        for (double amp : harmonicAmplitudes) {
            if (amp > maxAmp) maxAmp = amp;
        }
        if (maxAmp == 0.0) maxAmp = 1.0;

        // Apply change to the starting amplitude
        double newAmp = dragStartAmp + (amplitudeChange * maxAmp);
        setHarmonicAmplitude(selectedHarmonic, newAmp);

    } else {
        // Update cursor based on hover
        int harmonicIndex = getHarmonicAtX(event->pos().x());
        if (harmonicIndex >= 0 && isOverBarTop(event->pos().x(), event->pos().y(), harmonicIndex)) {
            setCursor(Qt::SizeVerCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void SpectrumVisualizer::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (!editMode) return;

    dragging = false;
    setCursor(Qt::ArrowCursor);
}

void SpectrumVisualizer::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!editMode) return;

    int harmonicIndex = getHarmonicAtX(event->pos().x());
    if (harmonicIndex < 0) return;

    // Restore to default amplitude (1/n rolloff)
    double defaultAmp = 1.0 / (harmonicIndex + 1);
    setHarmonicAmplitude(harmonicIndex, defaultAmp);

    selectedHarmonic = harmonicIndex;
    update();
}

void SpectrumVisualizer::keyPressEvent(QKeyEvent *event)
{
    if (!editMode) return;

    if (event->key() == Qt::Key_Delete && selectedHarmonic >= 0) {
        // Zero out the selected harmonic
        setHarmonicAmplitude(selectedHarmonic, 0.0);
        update();
    }
}

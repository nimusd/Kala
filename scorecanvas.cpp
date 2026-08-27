#include "scorecanvas.h"
#include "scorecanvascommands.h"
#include "curve.h"
#include "trackselect.h"
#include "track.h"
#include "canvas.h"
#include "easingdialog.h"
#include "dynamicscurvedialog.h"
#include "expressivecurveapplydialog.h"
#include "eqcurvedialog.h"
#include "tempochangedialog.h"
#include <QPainter>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPainterPath>
#include <QFont>
#include <QMouseEvent>
#include <QTabletEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSet>
#include <QDateTime>
#include <QTimer>
#include <cmath>
#include <algorithm>

// Extended color system for scale degrees (supports up to 10 notes)
const QColor ScoreCanvas::SCALE_COLORS[10] = {
    QColor(255, 0, 0),      // 1 (Tonic) - Red
    QColor(255, 127, 0),    // 2 - Orange
    QColor(255, 215, 0),    // 3 - Yellow (gold)
    QColor(0, 128, 0),      // 4 - Green
    QColor(0, 0, 255),      // 5 (Fifth) - Blue
    QColor(75, 0, 130),     // 6 - Indigo
    QColor(148, 0, 211),    // 7 - Violet
    QColor(255, 20, 147),   // 8 - Deep Pink
    QColor(0, 255, 255),    // 9 - Cyan
    QColor(128, 128, 128)   // 10 - Gray
};

ScoreCanvas::ScoreCanvas(QWidget *parent)
    : QWidget(parent)
    , currentScale(Scale::justIntonation())  // Initialize with Just Intonation
    , baseFrequency(18.75)       // Default base frequency: 25 Hz
    , currentInputMode(DrawModeDiscrete)  // Default to discrete drawing mode
    , activeTrackIndex(0)       // Default to track 0
    , trackSelector(nullptr)    // Will be set by ScoreCanvasWindow
    , visibleMinHz(27.5)        // A0
    , visibleMaxHz(4186.0)      // C8
    , pixelsPerHz(0.1)
    , verticalScrollOffset(0)
    , pixelsPerSecond(100.0)    // Default: 100 pixels per second
    , horizontalScrollOffset(0)
    , musicalModeEnabled(false)
    , musicalTempo(120)
    , musicalTimeSigTop(4)
    , musicalTimeSigBottom(4)
    , isDrawingNote(false)
    , pendingNote(nullptr)
    , usingTablet(false)
    , penPressure(0.0)
    , pasteTargetTime(0.0)      // Initialize paste position to start
    , isDrawingLasso(false)
    , lassoAddToSelection(false)
    , currentDragMode(NoDrag)
    , pendingDragMode(NoDrag)
    , dragThresholdExceeded(false)
    , dragStartTime(0.0)
    , dragStartPitch(0.0)
    , dragStartDuration(0.0)
    , editingDotIndex(-1)
    , editingDotTimePos(0.0)
    , lastTabletPressTime(0)
    , lastTabletPressPos(0, 0)
    , lastContextMenuTime(0)
{
    setMinimumSize(400, 400);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background-color: white;");
    setMouseTracking(true);  // Enable hover cursor updates
    setFocusPolicy(Qt::StrongFocus);  // Enable keyboard input

    // Initialize undo stack
    undoStack = new QUndoStack(this);

    // Auto-scroll timer: fires at ~60 fps while the mouse is near a viewport edge during lasso.
    // The ScoreCanvas widget is very wide (e.g. 30,000 px), so we measure proximity to the
    // *viewport* edges (parentWidget() == QScrollArea::viewport()), not the canvas boundaries.
    // Emits requestAutoScroll so ScoreCanvasWindow can shift the QScrollArea scroll bars.
    m_autoScrollTimer = new QTimer(this);
    m_autoScrollTimer->setInterval(16);
    connect(m_autoScrollTimer, &QTimer::timeout, this, [this]() {
        if (!isDrawingLasso) { m_autoScrollTimer->stop(); return; }

        QWidget *vp = parentWidget();
        if (!vp) { m_autoScrollTimer->stop(); return; }

        // Use the real-time global cursor position so scrolling continues even when
        // the mouse/pen is held still at the screen edge or moved off-screen.
        // (m_lassoMousePos is stale once events stop arriving and would drift
        //  out of the edge zone as the canvas scrolls beneath the cursor.)
        QPoint vpPos = vp->mapFromGlobal(QCursor::pos());
        int vpW = vp->width();
        int vpH = vp->height();

        const int EDGE_ZONE = 40;
        const int MAX_SPEED = 15;
        int dx = 0, dy = 0;

        if (vpPos.x() > vpW - EDGE_ZONE)
            dx = qMin((vpPos.x() - (vpW - EDGE_ZONE)) * MAX_SPEED / EDGE_ZONE, MAX_SPEED);
        else if (vpPos.x() < EDGE_ZONE)
            dx = -qMin((EDGE_ZONE - vpPos.x()) * MAX_SPEED / EDGE_ZONE, MAX_SPEED);

        if (vpPos.y() > vpH - EDGE_ZONE)
            dy = qMin((vpPos.y() - (vpH - EDGE_ZONE)) * MAX_SPEED / EDGE_ZONE, MAX_SPEED);
        else if (vpPos.y() < EDGE_ZONE)
            dy = -qMin((EDGE_ZONE - vpPos.y()) * MAX_SPEED / EDGE_ZONE, MAX_SPEED);

        if (dx == 0 && dy == 0) return;  // Cursor not near edge — keep timer alive for re-entry

        // Extend the lasso to the cursor's current canvas position so the selection
        // rectangle grows as the canvas scrolls under the held cursor.
        lassoCurrentPos = mapFrom(vp, vpPos);

        emit requestAutoScroll(dx, dy);
        update();
    });

    // Generate default C major scale
    generateScaleLines();

    // Auto-exit transform mode when the selection is no longer a single note
    // (clicking outside, lasso-selecting multiple, deselecting, etc.).
    connect(this, &ScoreCanvas::noteSelectionChanged, this, [this]() {
        if (transformMode && selectedNoteIndices.size() != 1) {
            setTransformMode(false);
        }
    });
}

void ScoreCanvas::setFrequencyRange(double minHz, double maxHz)
{
    visibleMinHz = minHz;
    visibleMaxHz = maxHz;
    update();
    emit frequencyRangeChanged(minHz, maxHz);
}

void ScoreCanvas::setPixelsPerHz(double ratio)
{
    pixelsPerHz = ratio;
    update();
    emit zoomChanged(ratio);
}

void ScoreCanvas::setVerticalOffset(int offsetPixels)
{
    verticalScrollOffset = offsetPixels;
    update();
}

void ScoreCanvas::setPixelsPerSecond(double pps)
{
    pixelsPerSecond = pps;
    update();  // Repaint with new horizontal zoom
}

void ScoreCanvas::setHorizontalScrollOffset(int offset)
{
    horizontalScrollOffset = offset;
    update();  // Redraw with new scroll position
}

void ScoreCanvas::setScale(const Scale &scale)
{
    currentScale = scale;
    generateScaleLines();
}

void ScoreCanvas::setBaseFrequency(double hz)
{
    baseFrequency = hz;
    generateScaleLines();
}

void ScoreCanvas::addScaleChange(double timeMs, const Scale &scale, double baseFreq)
{
    // Add or update scale change at this time point
    scaleChanges[timeMs] = qMakePair(scale, baseFreq);

    qDebug() << "ScoreCanvas: Added scale change at" << timeMs << "ms:"
             << scale.getName() << "with base freq" << baseFreq << "Hz";

    generateScaleLines();
    update();
}

void ScoreCanvas::removeScaleChange(double timeMs)
{
    if (scaleChanges.remove(timeMs) > 0) {
        qDebug() << "ScoreCanvas: Removed scale change at" << timeMs << "ms";
        generateScaleLines();
        update();
    }
}

void ScoreCanvas::clearScaleChanges()
{
    scaleChanges.clear();
    qDebug() << "ScoreCanvas: Cleared all scale changes";
    generateScaleLines();
    update();
}

Scale ScoreCanvas::getScaleAtTime(double timeMs) const
{
    // Find the most recent scale change before or at this time
    if (scaleChanges.isEmpty()) {
        return currentScale;  // No modulations, use default
    }

    // Iterate through scale changes in reverse order (most recent first)
    auto it = scaleChanges.upperBound(timeMs);
    if (it == scaleChanges.begin()) {
        // No scale change before this time, use default
        return currentScale;
    }

    // Go back one to get the most recent change at or before timeMs
    --it;
    return it.value().first;
}

double ScoreCanvas::getBaseFrequencyAtTime(double timeMs) const
{
    // Find the most recent base frequency change before or at this time
    if (scaleChanges.isEmpty()) {
        return baseFrequency;  // No modulations, use default
    }

    // Iterate through scale changes in reverse order
    auto it = scaleChanges.upperBound(timeMs);
    if (it == scaleChanges.begin()) {
        // No scale change before this time, use default
        return baseFrequency;
    }

    // Go back one to get the most recent change at or before timeMs
    --it;
    return it.value().second;
}

void ScoreCanvas::generateScaleLines()
{
    scaleLines.clear();

    // TODO: Get current visible time range from timeline/scrolling
    // For now, generate scale lines based on scale at time 0
    // Future: generate different scale lines for different time regions
    double referenceTime = 0.0;

    // Get the scale and base frequency active at the reference time
    Scale activeScale = getScaleAtTime(referenceTime);
    double activeBaseFreq = getBaseFrequencyAtTime(referenceTime);

    int degreeCount = activeScale.getDegreeCount();

    // Generate scale lines starting from activeBaseFreq
    // Cover octaves from activeBaseFreq up to 8000 Hz
    for (int octave = 0; octave <= 9; ++octave) {
        double octaveBaseFreq = activeBaseFreq * std::pow(2.0, octave);

        for (int degree = 0; degree < degreeCount; ++degree) {
            // Get ratio from active scale
            double freq = octaveBaseFreq * activeScale.getRatio(degree);

            // Only add lines within a reasonable musical range
            if (freq >= 20.0 && freq <= 8000.0) {
                ScaleLine line;
                line.frequencyHz = freq;
                line.scaleDegree = degree + 1;  // 1-based
                line.isAccidental = activeScale.getIsAccidental(degree);

                // Special handling for whole tone scale: all lines dark grey, no thick lines
                if (activeScale.getScaleId() == 20) { // Whole Tone
                    line.color = QColor(210, 210, 210);
                    line.isThicker = false;
                    line.noteName = activeScale.getNoteName(degree) + QString::number(octave);
                    scaleLines.append(line);
                    continue;
                }

                // For Equal Temperament with custom tonic, shift the color assignment
                // The tonicIndex tells us which note in the 12-note scale should be colored as tonic (red)
                int tonicIndex = activeScale.getTonicIndex();
                bool isET = (activeScale.getScaleId() == 2);

                if (isET && tonicIndex > 0) {
                    // ET with non-C tonic: determine if this degree is diatonic in the major scale
                    // Major scale pattern: whole, whole, half, whole, whole, whole, half (2,2,1,2,2,2,1 semitones)
                    // Diatonic degrees relative to tonic: 0,2,4,5,7,9,11
                    int rel = (degree - tonicIndex + 12) % 12;
                    bool isDiatonic = false;
                    int diatonicRank = 0; // 1-7
                    switch (rel) {
                        case 0: isDiatonic = true; diatonicRank = 1; break; // tonic
                        case 2: isDiatonic = true; diatonicRank = 2; break; // second
                        case 4: isDiatonic = true; diatonicRank = 3; break; // third
                        case 5: isDiatonic = true; diatonicRank = 4; break; // fourth
                        case 7: isDiatonic = true; diatonicRank = 5; break; // fifth
                        case 9: isDiatonic = true; diatonicRank = 6; break; // sixth
                        case 11: isDiatonic = true; diatonicRank = 7; break; // seventh
                        default: isDiatonic = false; break;
                    }

                    if (isDiatonic) {
                        line.color = getScaleDegreeColor(diatonicRank);
                    } else {
                        line.color = QColor(210, 210, 210); // gray for non-diatonic
                    }

                    // Tonic and fifth thick lines
                    bool isTonic = (degree == tonicIndex);
                    bool isFifth = (degree == (tonicIndex + 7) % 12);
                    line.isThicker = (isTonic || isFifth);
                } else {
                    // Normal coloring (C tonic or non-ET)
                    int diatonicRank = 0;
                    for (int d = 0; d < degree; ++d)
                        if (!activeScale.getIsAccidental(d)) diatonicRank++;
                    line.color = line.isAccidental ? QColor(210, 210, 210) : getScaleDegreeColor(diatonicRank + 1);

                    // Tonic always thick
                    bool isTonic = (degree == 0);
                    bool isFifth = false;
                    if (isET) {
                        // ET with C tonic: fifth is degree 7 (G)
                        isFifth = (degree == 7);
                    } else {
                        // For 7-note scales, fifth is degree 4 (if scale has at least 5 degrees)
                        isFifth = (degreeCount >= 5 && degree == 4);
                    }
                    line.isThicker = (isTonic || isFifth);
                }

                line.noteName = activeScale.getNoteName(degree) + QString::number(octave);

                scaleLines.append(line);
            }
        }
    }

    update();
}

QSize ScoreCanvas::sizeHint() const
{
    return QSize(800, 600);
}

QSize ScoreCanvas::minimumSizeHint() const
{
    return QSize(400, 400);
}

void ScoreCanvas::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Per-variation lane contexts are memoized for this pass only - building
    // one walks the canvas's child widgets, far too costly per note.
    m_laneContextCache.clear();

    // Draw background
    painter.fillRect(rect(), Qt::white);

    // Draw time-aware scale lines (different scale lines for different time regions)
    drawTimeAwareScaleLines(painter);

    // Draw bar lines (if in musical mode)
    drawBarLines(painter);

    // Draw all notes
    const QVector<Note> &notes = phrase.getNotes();
    for (int i = 0; i < notes.size(); ++i) {
        bool isSelected = isNoteSelected(i);
        drawNote(painter, notes[i], isSelected);
    }

    // Curve lanes (Claude-curve-lanes.md). Drawn as its own pass rather than
    // inside drawNote, which is already carrying segments, legato, variation
    // badges and pending notes. The tier is decided per note, so a crowded
    // note degrades on its own while its neighbours keep their lanes.
    if (m_densityTier != DensityTier::Off) {
        // Dim-others (D9) applies to the ribbons too; painter opacity matches
        // the alpha 51 the note bodies get in drawNote. The hover label is
        // drawn last, outside the dimming.
        const bool dimming = m_dimOthers && selectedNoteIndices.size() == 1;
        for (int i = 0; i < notes.size(); ++i) {
            const bool dimThis = dimming && !isNoteSelected(i);
            if (dimThis) painter.save();
            if (dimThis) painter.setOpacity(0.2);
            switch (effectiveTierForNote(i, notes[i])) {
            case DensityTier::Lanes:   drawNoteLanes(painter, i, notes[i]); break;
            case DensityTier::Overlay: drawNoteOverlay(painter, notes[i]); break;
            case DensityTier::Off:     break;
            }
            if (dimThis) painter.restore();
        }
        drawHoverLabel(painter);   // last, so it sits above every band
    }

    // Draw pending note being drawn
    drawPendingNote(painter);

    // Draw lasso rectangle
    drawLassoRectangle(painter);
}

void ScoreCanvas::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}

int ScoreCanvas::frequencyToPixel(double hz) const
{
    // Convert Hz to pixel position using logarithmic spacing.
    // Maps the FULL frequency range (FULL_MIN_HZ..FULL_MAX_HZ) to the widget height.
    // The QScrollArea viewport shows the portion corresponding to the current zoom.
    // Higher frequencies = lower Y values (top of widget).

    if (hz <= 0 || baseFrequency <= 0) {
        return 0;
    }

    double octaveNumber = std::log2(hz / baseFrequency);
    double fullMinOctave = std::log2(FULL_MIN_HZ / baseFrequency);
    double fullMaxOctave = std::log2(FULL_MAX_HZ / baseFrequency);
    double fullOctaveRange = fullMaxOctave - fullMinOctave;

    // Normalize position within the full scrollable range
    double normalizedPos = (octaveNumber - fullMinOctave) / fullOctaveRange;

    // Convert to pixel position (flip for top-down coordinate system)
    int pixel = height() - static_cast<int>(normalizedPos * height());
    return pixel;
}

double ScoreCanvas::pixelToFrequency(int pixel) const
{
    // Convert pixel position to Hz using logarithmic spacing.
    // Maps the widget height to the FULL frequency range (FULL_MIN_HZ..FULL_MAX_HZ).
    double normalizedPos = 1.0 - (static_cast<double>(pixel) / height());
    normalizedPos = qBound(0.0, normalizedPos, 1.0);

    double fullMinOctave = std::log2(FULL_MIN_HZ / baseFrequency);
    double fullMaxOctave = std::log2(FULL_MAX_HZ / baseFrequency);
    double fullOctaveRange = fullMaxOctave - fullMinOctave;

    double octaveNumber = fullMinOctave + (normalizedPos * fullOctaveRange);

    return baseFrequency * std::pow(2.0, octaveNumber);
}

QColor ScoreCanvas::getScaleDegreeColor(int degree) const
{
    if (degree >= 1 && degree <= 10) {
        return SCALE_COLORS[degree - 1];
    }
    // Cycle through colors for scales with more than 10 notes
    if (degree > 10) {
        return SCALE_COLORS[(degree - 1) % 10];
    }
    return Qt::black;  // Fallback
}

void ScoreCanvas::drawScaleLine(QPainter &painter, const ScaleLine &line, int y)
{
    // Set pen based on whether this is a thick line (tonic/fifth)
    QPen pen(line.color);
    pen.setWidth(line.isThicker ? THICK_LINE_WIDTH : NORMAL_LINE_WIDTH);
    painter.setPen(pen);

    // Draw line from left edge to right edge (Hz labels are in separate widget)
    int startX = 0;
    int endX = width();
    painter.drawLine(startX, y, endX, y);
}

void ScoreCanvas::drawScaleLineInRegion(QPainter &painter, const ScaleLine &line, int y, int startX, int endX)
{
    // Set pen based on whether this is a thick line (tonic/fifth)
    QPen pen(line.color);
    pen.setWidth(line.isThicker ? THICK_LINE_WIDTH : NORMAL_LINE_WIDTH);
    painter.setPen(pen);

    // Draw line in the specified horizontal region
    painter.drawLine(startX, y, endX, y);
}

QVector<ScoreCanvas::ScaleLine> ScoreCanvas::generateScaleLinesForScale(const Scale &scale, double baseFreq) const
{
    QVector<ScaleLine> lines;
    int degreeCount = scale.getDegreeCount();

    // Generate scale lines starting from baseFreq
    // Cover octaves from baseFreq up to 8000 Hz
    for (int octave = 0; octave <= 9; ++octave) {
        double octaveBaseFreq = baseFreq * std::pow(2.0, octave);

        for (int degree = 0; degree < degreeCount; ++degree) {
            // Get ratio from scale
            double freq = octaveBaseFreq * scale.getRatio(degree);

            // Only add lines within a reasonable musical range
            if (freq >= 20.0 && freq <= 8000.0) {
                ScaleLine line;
                line.frequencyHz = freq;
                line.scaleDegree = degree + 1;  // 1-based
                line.isAccidental = scale.getIsAccidental(degree);

                // Special handling for whole tone scale: all lines dark grey, no thick lines
                if (scale.getScaleId() == 20) { // Whole Tone
                    line.color = QColor(210, 210, 210);
                    line.isThicker = false;
                    line.noteName = scale.getNoteName(degree) + QString::number(octave);
                    lines.append(line);
                    continue;
                }

                // For Equal Temperament with custom tonic, shift the color assignment
                int tonicIndex = scale.getTonicIndex();
                bool isET = (scale.getScaleId() == 2);

                if (isET && tonicIndex > 0) {
                    // ET with non-C tonic: determine if this degree is diatonic in the major scale
                    // Major scale pattern: whole, whole, half, whole, whole, whole, half (2,2,1,2,2,2,1 semitones)
                    // Diatonic degrees relative to tonic: 0,2,4,5,7,9,11
                    int rel = (degree - tonicIndex + 12) % 12;
                    bool isDiatonic = false;
                    int diatonicRank = 0; // 1-7
                    switch (rel) {
                        case 0: isDiatonic = true; diatonicRank = 1; break; // tonic
                        case 2: isDiatonic = true; diatonicRank = 2; break; // second
                        case 4: isDiatonic = true; diatonicRank = 3; break; // third
                        case 5: isDiatonic = true; diatonicRank = 4; break; // fourth
                        case 7: isDiatonic = true; diatonicRank = 5; break; // fifth
                        case 9: isDiatonic = true; diatonicRank = 6; break; // sixth
                        case 11: isDiatonic = true; diatonicRank = 7; break; // seventh
                        default: isDiatonic = false; break;
                    }

                    if (isDiatonic) {
                        line.color = getScaleDegreeColor(diatonicRank);
                    } else {
                        line.color = QColor(210, 210, 210); // gray for non-diatonic
                    }

                    // Tonic and fifth thick lines
                    bool isTonic = (degree == tonicIndex);
                    bool isFifth = (degree == (tonicIndex + 7) % 12);
                    line.isThicker = (isTonic || isFifth);
                } else {
                    // Normal coloring (C tonic or non-ET)
                    int diatonicRank = 0;
                    for (int d = 0; d < degree; ++d)
                        if (!scale.getIsAccidental(d)) diatonicRank++;
                    line.color = line.isAccidental ? QColor(210, 210, 210) : getScaleDegreeColor(diatonicRank + 1);

                    // Tonic always thick
                    bool isTonic = (degree == 0);
                    bool isFifth = false;
                    if (isET) {
                        // ET with C tonic: fifth is degree 7 (G)
                        isFifth = (degree == 7);
                    } else {
                        // For 7-note scales, fifth is degree 4 (if scale has at least 5 degrees)
                        isFifth = (degreeCount >= 5 && degree == 4);
                    }
                    line.isThicker = (isTonic || isFifth);
                }

                line.noteName = scale.getNoteName(degree) + QString::number(octave);

                lines.append(line);
            }
        }
    }

    // DEBUG: Print scale lines for equal temperament with base ~32.7
    if (scale.getScaleId() == 2 && fabs(baseFreq - 32.7) < 0.1) {
        qDebug() << "DEBUG generateScaleLinesForScale: ET baseFreq=" << baseFreq;
        for (int octave = 2; octave <= 4; ++octave) {
            for (int degree = 0; degree < scale.getDegreeCount(); ++degree) {
                double freq = baseFreq * std::pow(2.0, octave) * scale.getRatio(degree);
                if (freq >= 20.0 && freq <= 8000.0) {
                    qDebug() << "  octave" << octave << "degree" << degree
                             << "freq" << freq << "name" << (scale.getNoteName(degree) + QString::number(octave));
                }
            }
        }
    }

    return lines;
}

void ScoreCanvas::drawTimeAwareScaleLines(QPainter &painter)
{
    // Determine visible time range
    double startTime = pixelToTime(0);
    double endTime = pixelToTime(width());

    // Build a list of time regions with their associated scales
    struct TimeRegion {
        double startTime;
        double endTime;
        Scale scale;
        double baseFreq;
    };
    QVector<TimeRegion> timeRegions;

    // Start with default scale at time 0
    TimeRegion firstRegion;
    firstRegion.startTime = 0.0;
    firstRegion.scale = currentScale;
    firstRegion.baseFreq = baseFrequency;

    // Find all scale changes that affect the visible range
    QList<double> changeTimes;
    changeTimes.append(0.0);  // Always include time 0
    for (auto it = scaleChanges.constBegin(); it != scaleChanges.constEnd(); ++it) {
        if (it.key() <= endTime) {  // Only include changes that are visible or before visible range
            changeTimes.append(it.key());
        }
    }
    std::sort(changeTimes.begin(), changeTimes.end());

    // Build time regions
    for (int i = 0; i < changeTimes.size(); ++i) {
        TimeRegion region;
        region.startTime = changeTimes[i];
        region.endTime = (i + 1 < changeTimes.size()) ? changeTimes[i + 1] : endTime + 10000.0;  // Extend beyond visible range
        region.scale = getScaleAtTime(changeTimes[i]);
        region.baseFreq = getBaseFrequencyAtTime(changeTimes[i]);
        timeRegions.append(region);
    }

    // Draw scale lines for each time region
    for (const TimeRegion &region : timeRegions) {
        // Skip if region is not visible
        if (region.endTime < startTime) continue;
        if (region.startTime > endTime) continue;

        // Calculate pixel bounds for this region
        int regionStartX = timeToPixel(region.startTime);
        int regionEndX = timeToPixel(region.endTime);

        // Clip to visible area
        if (regionStartX < 0) regionStartX = 0;
        if (regionEndX > width()) regionEndX = width();

        // Generate scale lines for this region's scale
        QVector<ScaleLine> regionLines = generateScaleLinesForScale(region.scale, region.baseFreq);

        // Draw each scale line, clipped to this time region
        for (const ScaleLine &line : regionLines) {
            int y = frequencyToPixel(line.frequencyHz);

            // Only draw if visible vertically
            if (y >= -10 && y <= height() + 10) {
                drawScaleLineInRegion(painter, line, y, regionStartX, regionEndX);
            }
        }

        // Draw vertical demarcation line at the START of this region (except at time 0)
        if (region.startTime > 0.0 && region.startTime >= startTime && region.startTime <= endTime) {
            int demarcX = timeToPixel(region.startTime);
            painter.setPen(QPen(QColor(150, 150, 150), 2, Qt::DashLine));
            painter.drawLine(demarcX, 0, demarcX, height());

            // Draw scale name label at the top
            painter.setPen(Qt::black);
            QFont font = painter.font();
            font.setPointSize(8);
            font.setBold(true);
            painter.setFont(font);
            QString label = region.scale.getName();
            painter.drawText(demarcX + 5, 15, label);
        }
    }
}

void ScoreCanvas::drawHzLabel(QPainter &painter, double hz, int y)
{
    // Draw Hz value on the left edge
    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    QString label = QString::number(hz, 'f', 1) + " Hz";
    QRect labelRect(0, y - 8, HZ_LABEL_WIDTH - 5, 16);
    painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, label);
}

// ============================================================================
// Coordinate Conversion
// ============================================================================

double ScoreCanvas::pixelToTime(int x) const
{
    // Convert pixel X position to time in milliseconds
    // Note: x is in widget-local coordinates (QScrollArea delivers events this way)
    // so no offset adjustment needed - the coordinate already reflects scroll position
    return (static_cast<double>(x) / pixelsPerSecond) * 1000.0;
}

int ScoreCanvas::timeToPixel(double timeMs) const
{
    // Convert time in milliseconds to pixel X position
    // Note: No offset subtraction here - the QScrollArea handles viewport scrolling.
    // The canvas paints at absolute coordinates; the scroll area shows the visible portion.
    double seconds = timeMs / 1000.0;
    return static_cast<int>(seconds * pixelsPerSecond);
}

double ScoreCanvas::snapToNearestScaleLine(double hz) const
{
    // TIME-AWARE VERSION: This is deprecated, use snapToNearestScaleLineAtTime instead
    // For backward compatibility, snap to scale at time 0
    return snapToNearestScaleLineAtTime(hz, 0.0);
}

double ScoreCanvas::snapToNearestScaleLineAtTime(double hz, double timeMs) const
{
    // Get the scale and base frequency active at this time
    Scale activeScale = getScaleAtTime(timeMs);
    double activeBaseFreq = getBaseFrequencyAtTime(timeMs);

    // Generate scale lines for this specific time's scale
    QVector<ScaleLine> timeScaleLines = generateScaleLinesForScale(activeScale, activeBaseFreq);
    qDebug() << "snapToNearestScaleLineAtTime: hz=" << hz << "timeMs=" << timeMs
             << "activeScale=" << activeScale.getName() << "activeBaseFreq=" << activeBaseFreq
             << "scaleLines count=" << timeScaleLines.size();
    if (timeScaleLines.size() <= 10) {
        for (const ScaleLine &line : timeScaleLines) {
            qDebug() << "  line: degree" << line.scaleDegree << "freq" << line.frequencyHz << "name" << line.noteName;
        }
    }

    if (timeScaleLines.isEmpty()) return hz;

    // Find the closest scale line frequency
    double closestHz = timeScaleLines.first().frequencyHz;
    double minDistance = std::abs(hz - closestHz);

    for (const ScaleLine &line : timeScaleLines) {
        double distance = std::abs(hz - line.frequencyHz);
        if (distance < minDistance) {
            minDistance = distance;
            closestHz = line.frequencyHz;
        }
    }
    qDebug() << "snapToNearestScaleLineAtTime: closestHz=" << closestHz << "minDistance=" << minDistance;
    return closestHz;
}

void ScoreCanvas::clearNotes()
{
    phrase.clearNotes();
    update();
}

void ScoreCanvas::deleteNotesOnTrack(int trackIndex)
{
    // Delete all notes that are assigned to the specified track
    QVector<Note>& notes = phrase.getNotes();

    // Iterate backwards to safely remove elements
    for (int i = notes.size() - 1; i >= 0; --i) {
        if (notes[i].getTrackIndex() == trackIndex) {
            notes.removeAt(i);
        }
    }

    // Clear selection (indices may be invalid now)
    selectedNoteIndices.clear();

    update();
    qDebug() << "ScoreCanvas: Deleted all notes on track" << trackIndex;
}

void ScoreCanvas::updateTrackIndicesAfterDeletion(int deletedTrackIndex)
{
    // After a track is deleted, all notes on higher-indexed tracks
    // need their track index decremented by 1
    QVector<Note>& notes = phrase.getNotes();

    for (Note& note : notes) {
        int noteTrack = note.getTrackIndex();
        if (noteTrack > deletedTrackIndex) {
            note.setTrackIndex(noteTrack - 1);
        }
    }

    qDebug() << "ScoreCanvas: Updated track indices after deletion of track" << deletedTrackIndex;
}

QVector<ScoreCanvas::NotePitchChange> ScoreCanvas::remapDiscreteNotesForModulation(
    double modulationTimeMs,
    const Scale &oldScale, double oldBaseFreq,
    const Scale &newScale, double newBaseFreq)
{
    QVector<NotePitchChange> changes;

    // Find the next modulation time after this one (scope boundary)
    double nextModulationTime = std::numeric_limits<double>::max();
    auto it = scaleChanges.upperBound(modulationTimeMs);
    if (it != scaleChanges.end()) {
        nextModulationTime = it.key();
    }

    // Generate old scale lines to match against
    QVector<ScaleLine> oldLines = generateScaleLinesForScale(oldScale, oldBaseFreq);

    const QVector<Note> &notes = phrase.getNotes();
    for (int i = 0; i < notes.size(); ++i) {
        const Note &note = notes[i];

        // Only remap discrete notes in the modulation region
        if (note.getStartTime() < modulationTimeMs) continue;
        if (note.getStartTime() >= nextModulationTime) continue;
        if (note.hasPitchCurve()) continue;

        double pitchHz = note.getPitchHz();

        // Find closest old scale line
        double closestDist = std::numeric_limits<double>::max();
        int closestDegree = 0;  // 0-based
        int closestOctave = 0;

        int degreeCount = oldScale.getDegreeCount();
        for (const ScaleLine &line : oldLines) {
            double dist = std::abs(pitchHz - line.frequencyHz);
            if (dist < closestDist) {
                closestDist = dist;
                closestDegree = line.scaleDegree - 1;  // Convert 1-based to 0-based

                // Determine octave: line.frequencyHz = oldBaseFreq * 2^octave * ratio
                double ratio = oldScale.getRatio(closestDegree);
                if (ratio > 0) {
                    double octaveFloat = std::log2(line.frequencyHz / (oldBaseFreq * ratio));
                    closestOctave = static_cast<int>(std::round(octaveFloat));
                }
            }
        }

        // Compute new frequency at same degree + octave in new scale
        int newDegreeCount = newScale.getDegreeCount();
        int mappedDegree = closestDegree;
        if (mappedDegree >= newDegreeCount) {
            mappedDegree = newDegreeCount - 1;  // Clamp if new scale has fewer degrees
        }

        double newHz = newBaseFreq * std::pow(2.0, closestOctave) * newScale.getRatio(mappedDegree);

        if (std::abs(newHz - pitchHz) > 0.001) {
            NotePitchChange change;
            change.noteIndex = i;
            change.oldPitchHz = pitchHz;
            change.newPitchHz = newHz;
            changes.append(change);
        }
    }

    return changes;
}

void ScoreCanvas::applyNotePitchChanges(const QVector<NotePitchChange> &changes, bool forward)
{
    QVector<Note> &notes = phrase.getNotes();
    for (const NotePitchChange &change : changes) {
        if (change.noteIndex >= 0 && change.noteIndex < notes.size()) {
            notes[change.noteIndex].setPitchHz(forward ? change.newPitchHz : change.oldPitchHz);
            notes[change.noteIndex].setRenderDirty(true);
        }
    }
}

void ScoreCanvas::snapSelectedNotesToScale()
{
    if (selectedNoteIndices.isEmpty()) {
        qDebug() << "ScoreCanvas::snapSelectedNotesToScale - No notes selected";
        return;
    }

    auto *cmd = new SnapToScaleCommand(&phrase, selectedNoteIndices, this);
    if (cmd->isNoop()) {
        delete cmd;
        qDebug() << "ScoreCanvas::snapSelectedNotesToScale - No continuous notes in selection";
        return;
    }
    undoStack->push(cmd);
    qDebug() << "ScoreCanvas::snapSelectedNotesToScale - pushed SnapToScaleCommand";
}

void ScoreCanvas::makeSelectedNotesContinuous()
{
    if (selectedNoteIndices.isEmpty()) {
        qDebug() << "ScoreCanvas::makeSelectedNotesContinuous - No notes selected";
        return;
    }

    // Check if any selected notes are discrete (no pitch curve)
    const QVector<Note> &notes = phrase.getNotes();
    bool hasDiscreteNote = false;
    for (int index : selectedNoteIndices) {
        if (index >= 0 && index < notes.size() && !notes[index].hasPitchCurve()) {
            hasDiscreteNote = true;
            break;
        }
    }

    if (!hasDiscreteNote) {
        qDebug() << "ScoreCanvas::makeSelectedNotesContinuous - No discrete notes in selection";
        return;
    }

    undoStack->push(new MakeNotesContinuousCommand(&phrase, selectedNoteIndices, this));
}

void ScoreCanvas::makeSelectedNotesDiscrete()
{
    if (selectedNoteIndices.isEmpty()) {
        qDebug() << "ScoreCanvas::makeSelectedNotesDiscrete - No notes selected";
        return;
    }

    // Check if any selected notes are continuous (have a pitch curve)
    const QVector<Note> &notes = phrase.getNotes();
    bool hasContinuousNote = false;
    for (int index : selectedNoteIndices) {
        if (index >= 0 && index < notes.size() && notes[index].hasPitchCurve()) {
            hasContinuousNote = true;
            break;
        }
    }

    if (!hasContinuousNote) {
        qDebug() << "ScoreCanvas::makeSelectedNotesDiscrete - No continuous notes in selection";
        return;
    }

    undoStack->push(new MakeNotesDiscreteCommand(&phrase, selectedNoteIndices, this));
}

// ============================================================================
// Input Mode Management
// ============================================================================

void ScoreCanvas::setInputMode(InputMode mode)
{
    currentInputMode = mode;

    // Cancel any ongoing drawing when switching modes
    if (isDrawingNote) {
        isDrawingNote = false;
        usingTablet = false;
        pressurePoints.clear();
        pitchPoints.clear();
        update();
    }

    // Deselect notes when switching to drawing modes
    if (mode != SelectionMode && !selectedNoteIndices.isEmpty()) {
        deselectAll();
        update();
    }

    qDebug() << "ScoreCanvas: Input mode changed to" << mode;
}

// ============================================================================
// Segment Editing Mode
// ============================================================================

void ScoreCanvas::enterSegmentEditingMode(int noteIndex)
{
    QVector<Note> &notes = phrase.getNotes();
    if (noteIndex < 0 || noteIndex >= notes.size()) return;

    Note &note = notes[noteIndex];

    // Only enter segment mode for quantized continuous notes
    if (!note.isQuantized() || !note.hasPitchCurve()) {
        qDebug() << "ScoreCanvas: Cannot enter segment mode - note is not quantized or has no pitch curve";
        return;
    }

    segmentEditingMode = true;
    segmentEditingNoteIndex = noteIndex;
    selectedSegmentIndex = -1;

    // Ensure segments are detected
    note.detectSegments();

    qDebug() << "ScoreCanvas: Entered segment editing mode for note" << noteIndex
             << "with" << note.getSegmentCount() << "segments";

    update();
}

void ScoreCanvas::exitSegmentEditingMode()
{
    if (!segmentEditingMode) return;

    segmentEditingMode = false;
    segmentEditingNoteIndex = -1;
    selectedSegmentIndex = -1;

    qDebug() << "ScoreCanvas: Exited segment editing mode";
    update();
}

// ============================================================================
// Musical Mode Management
// ============================================================================

void ScoreCanvas::setMusicalMode(bool enabled, int tempo, int timeSigTop, int timeSigBottom)
{
    musicalModeEnabled = enabled;
    musicalTempo = tempo;
    musicalTimeSigTop = timeSigTop;
    musicalTimeSigBottom = timeSigBottom;
    update();  // Repaint to show/hide bar lines
}

// ============================================================================
// Tempo/Time Signature Map Management
// ============================================================================

void ScoreCanvas::setDefaultTempo(double bpm)
{
    if (bpm >= 1.0 && bpm <= 300.0) {
        defaultTempo = bpm;
        qDebug() << "ScoreCanvas: Default tempo set to" << bpm << "BPM";
        emit tempoSettingsChanged();
        update();
    }
}

void ScoreCanvas::setDefaultTimeSignature(int num, int denom)
{
    if (num >= 1 && num <= 99 && denom >= 0 && denom <= 99) {
        defaultTimeSigNum = num;
        defaultTimeSigDenom = denom;
        qDebug() << "ScoreCanvas: Default time signature set to" << num << "/" << denom;
        emit tempoSettingsChanged();
        update();
    }
}

void ScoreCanvas::addTempoChange(double timeMs, const TempoTimeSignature &tts)
{
    tempoChanges[timeMs] = tts;
    qDebug() << "ScoreCanvas: Added tempo change at" << timeMs << "ms:"
             << tts.bpm << "BPM," << tts.timeSigNumerator << "/" << tts.timeSigDenominator
             << (tts.gradualTransition ? "(gradual)" : "");
    emit tempoSettingsChanged();
    update();
}

void ScoreCanvas::removeTempoChange(double timeMs)
{
    if (tempoChanges.remove(timeMs) > 0) {
        qDebug() << "ScoreCanvas: Removed tempo change at" << timeMs << "ms";
        emit tempoSettingsChanged();
        update();
    }
}

void ScoreCanvas::clearTempoChanges()
{
    tempoChanges.clear();
    qDebug() << "ScoreCanvas: Cleared all tempo changes";
    emit tempoSettingsChanged();
    update();
}

TempoTimeSignature ScoreCanvas::getTempoTimeSignatureAtTime(double timeMs) const
{
    // Find the most recent tempo change before or at this time
    if (tempoChanges.isEmpty()) {
        // No changes, return defaults
        TempoTimeSignature tts;
        tts.bpm = defaultTempo;
        tts.timeSigNumerator = defaultTimeSigNum;
        tts.timeSigDenominator = defaultTimeSigDenom;
        tts.gradualTransition = false;
        return tts;
    }

    auto it = tempoChanges.upperBound(timeMs);
    if (it == tempoChanges.begin()) {
        // No tempo change before this time, use defaults
        TempoTimeSignature tts;
        tts.bpm = defaultTempo;
        tts.timeSigNumerator = defaultTimeSigNum;
        tts.timeSigDenominator = defaultTimeSigDenom;
        tts.gradualTransition = false;
        return tts;
    }

    --it;
    return it.value();
}

double ScoreCanvas::getTempoAtTime(double timeMs) const
{
    // Find the marker at or before timeMs
    TempoTimeSignature currentTts = getTempoTimeSignatureAtTime(timeMs);

    // If gradual transition is enabled and there's a next marker, interpolate
    if (currentTts.gradualTransition) {
        // Find the time of this marker
        double startTime = 0.0;
        double startTempo = defaultTempo;

        if (!tempoChanges.isEmpty()) {
            auto it = tempoChanges.upperBound(timeMs);
            if (it != tempoChanges.begin()) {
                --it;
                startTime = it.key();
                startTempo = it.value().bpm;

                // Find the next marker
                ++it;
                if (it != tempoChanges.end()) {
                    double endTime = it.key();
                    double endTempo = it.value().bpm;

                    // Linear interpolation
                    double t = (timeMs - startTime) / (endTime - startTime);
                    t = qBound(0.0, t, 1.0);
                    return startTempo + t * (endTempo - startTempo);
                }
            }
        }
    }

    // No interpolation, return instantaneous tempo
    return currentTts.bpm;
}

QPair<int, int> ScoreCanvas::getTimeSignatureAtTime(double timeMs) const
{
    TempoTimeSignature tts = getTempoTimeSignatureAtTime(timeMs);
    return qMakePair(tts.timeSigNumerator, tts.timeSigDenominator);
}

// ============================================================================
// Clipboard Operations
// ============================================================================

void ScoreCanvas::setPasteTargetTime(double timeMs)
{
    pasteTargetTime = timeMs;
}

// ============================================================================
// Track Management
// ============================================================================

void ScoreCanvas::setActiveTrack(int trackIndex)
{
    if (activeTrackIndex != trackIndex) {
        activeTrackIndex = trackIndex;
        deselectAll();
    }
    setFocus();
    update();
    qDebug() << "ScoreCanvas: Active track set to" << trackIndex;
}

void ScoreCanvas::drawBarLines(QPainter &painter)
{
    if (!musicalModeEnabled) return;

    // Calculate visible time range
    double visibleStartTime = pixelToTime(0);
    double visibleEndTime = pixelToTime(width());

    // Build a list of tempo change times (including 0) that affect the visible range
    QList<double> changePoints;
    changePoints.append(0.0);
    for (auto it = tempoChanges.constBegin(); it != tempoChanges.constEnd(); ++it) {
        if (it.key() > 0.0 && it.key() <= visibleEndTime) {
            changePoints.append(it.key());
        }
    }
    std::sort(changePoints.begin(), changePoints.end());

    // Helper to calculate bar/beat durations for a given tempo/timesig
    auto calcDurations = [](double tempo, int timeSigTop, int timeSigBottom,
                           double &barDurationMs, double &beatDurationMs) {
        Q_UNUSED(timeSigBottom);
        // Bar duration = beat duration × beats per bar.
        // Denominator is informational for notation/display and does not scale duration.
        beatDurationMs = 60000.0 / tempo;
        barDurationMs = beatDurationMs * timeSigTop;
    };

    // For each tempo region, draw beat and bar lines
    for (int regionIdx = 0; regionIdx < changePoints.size(); ++regionIdx) {
        double regionStart = changePoints[regionIdx];
        double regionEnd = (regionIdx + 1 < changePoints.size()) ?
                           changePoints[regionIdx + 1] : visibleEndTime + 10000.0;

        // Skip if this region is entirely before the visible range
        if (regionEnd < visibleStartTime) continue;
        // Stop if we've passed the visible range
        if (regionStart > visibleEndTime) break;

        // Get tempo/timesig for this region
        TempoTimeSignature tts = getTempoTimeSignatureAtTime(regionStart);
        double barDurationMs, beatDurationMs;
        calcDurations(tts.bpm, tts.timeSigNumerator, tts.timeSigDenominator,
                      barDurationMs, beatDurationMs);

        // For gradual transitions, use average tempo in the region for drawing
        if (tts.gradualTransition && regionIdx + 1 < changePoints.size()) {
            TempoTimeSignature nextTts = getTempoTimeSignatureAtTime(regionEnd);
            double avgTempo = (tts.bpm + nextTts.bpm) / 2.0;
            calcDurations(avgTempo, tts.timeSigNumerator, tts.timeSigDenominator,
                          barDurationMs, beatDurationMs);
        }

        // Calculate draw range
        double drawStart = qMax(regionStart, visibleStartTime);
        double drawEnd = qMin(regionEnd, visibleEndTime);

        // For non-first regions, skip the exact boundary to avoid double lines
        // The change in measure spacing already makes the transition clear
        double epsilon = 0.1;  // Small tolerance to avoid floating point issues
        if (regionIdx > 0) {
            drawStart = qMax(drawStart, regionStart + epsilon);
        }

        // Draw beat lines (very fine, light grey)
        painter.setPen(QPen(QColor(210, 210, 210), 1));

        // Find first beat at or after drawStart
        int beatOffset = static_cast<int>(std::ceil((drawStart - regionStart) / beatDurationMs));
        for (int beat = beatOffset; ; ++beat) {
            double beatTime = regionStart + beat * beatDurationMs;
            if (beatTime > drawEnd) break;

            int x = timeToPixel(beatTime);
            if (x >= 0 && x <= width()) {
                painter.drawLine(x, 0, x, height());
            }
        }

        // Draw bar lines (thicker, pure black)
        painter.setPen(QPen(QColor(0, 0, 0), 2));

        // Find first bar at or after drawStart
        int barOffset = static_cast<int>(std::ceil((drawStart - regionStart) / barDurationMs));
        for (int bar = barOffset; ; ++bar) {
            double barTime = regionStart + bar * barDurationMs;
            if (barTime > drawEnd) break;

            int x = timeToPixel(barTime);
            if (x >= 0 && x <= width()) {
                painter.drawLine(x, 0, x, height());
            }
        }
    }
}

// ============================================================================
// Note Drawing
// ============================================================================

void ScoreCanvas::drawNote(QPainter &painter, const Note &note, bool isSelected)
{
    // Calculate position and size
    int x = timeToPixel(note.getStartTime());
    int width = timeToPixel(note.getEndTime()) - x;

    // Skip if note is completely off-screen
    if (x + width < 0 || x > this->width()) return;

    // Helper: query the active expressive curve value (fallback to dynamics if note lacks it)
    auto getActiveCurveValue = [this](const Note &n, double t) -> double {
        int idx = resolveActiveCurveIndex(n);
        if (idx <= 0)
            return n.getDynamicsAt(t);
        return n.getExpressiveCurve(idx).valueAt(t);
    };

    // Find maximum active curve value for blob height
    double maxDynamics = 0.0;
    for (int i = 0; i <= 20; ++i) {  // Sample curve at multiple points
        double t = i / 20.0;
        double value = getActiveCurveValue(note, t);
        if (value > maxDynamics) maxDynamics = value;
    }

    // Note blob height based on maximum dynamics in curve
    int blobHeight = static_cast<int>(20 + maxDynamics * 60);  // 20-80 pixels (increased range)

    // Get track state and color (needed before drawing)
    int trackIndex = note.getTrackIndex();
    QColor noteColor = QColor(100, 150, 255);  // Default blue

    if (trackSelector) {
        const QVector<TrackSelector::Track>& tracks = trackSelector->getTracks();
        if (trackIndex >= 0 && trackIndex < tracks.size()) {
            const TrackSelector::Track& track = tracks[trackIndex];

            // Don't draw if track is not selected (not visible)
            if (!track.isSelected) {
                return;  // Skip rendering this note
            }

            // Use track color
            noteColor = track.color;

            // Set opacity based on track state
            if (track.isActive) {
                noteColor.setAlpha(255);  // Full opacity for active track
            } else {
                noteColor.setAlpha(51);  // 20% opacity for selected but not active
            }
        }
    }

    // Override opacity for individually selected notes (selection rectangle)
    if (isSelected) {
        noteColor.setAlpha(255);  // Full opacity when note is selected
    }

    // Dim-others (D9): with exactly one note selected, every other note drops
    // to the low alpha - the same 51 the inactive tracks already use. Works in
    // every density tier, including Off, since this lives in drawNote.
    if (m_dimOthers && selectedNoteIndices.size() == 1 && !isSelected) {
        noteColor.setAlpha(51);
    }

    // Desaturate + lighten color only for explicitly legato-linked notes
    if (note.isLinkedAsLegato() && note.getSegmentCount() > 1) {
        int h, s, v, a;
        noteColor.getHsv(&h, &s, &v, &a);
        int newV = std::min(255, v + static_cast<int>((255 - v) * 0.5));
        noteColor.setHsv(h, s * 0.4, newV, a);
    }

    painter.setBrush(noteColor);
    painter.setPen(QPen(noteColor.darker(120), 1));

    // For segmented notes, draw each segment as a separate blob for clean pitch steps
    if (note.isQuantized() && note.getSegmentCount() > 1) {
        const auto &segments = note.getSegments();
        for (const Segment &seg : segments) {
            int segStartX = x + static_cast<int>(seg.startTime * width);
            int segEndX = x + static_cast<int>(seg.endTime * width);
            int segWidth = segEndX - segStartX;
            if (segWidth < 1) segWidth = 1;

            // Read pitch from the live pitch curve (not stale segment data)
            // so dragging/shifting the curve is immediately reflected
            double segMidT = (seg.startTime + seg.endTime) / 2.0;
            int centerY = frequencyToPixel(note.getPitchAt(segMidT));
            int numPts = std::max(2, segWidth / 5);

            QVector<QPointF> segTop, segBottom;
            for (int j = 0; j <= numPts; ++j) {
                double localT = static_cast<double>(j) / numPts;
                double globalT = seg.startTime + localT * (seg.endTime - seg.startTime);
                int posX = segStartX + static_cast<int>(localT * segWidth);

                double dynamics = getActiveCurveValue(note, globalT);
                int topOffset = static_cast<int>(dynamics * blobHeight * 0.5);
                segTop.append(QPointF(posX, centerY - topOffset));

                double bottomValue = 0.0;
                const Vibrato &vibrato = note.getVibrato();
                if (vibrato.active && globalT >= vibrato.onset) {
                    double vibratoT = (globalT - vibrato.onset) / (1.0 - vibrato.onset);
                    bottomValue = vibrato.amplitudeDepth * vibrato.envelopeAt(vibratoT);
                }
                int bottomOffset = static_cast<int>(bottomValue * blobHeight * 0.5);
                segBottom.append(QPointF(posX, centerY + bottomOffset));
            }

            QPainterPath segPath;
            segPath.moveTo(segTop.first());
            for (int j = 1; j < segTop.size(); ++j)
                segPath.lineTo(segTop[j]);
            for (int j = segBottom.size() - 1; j >= 0; --j)
                segPath.lineTo(segBottom[j]);
            segPath.closeSubpath();

            painter.drawPath(segPath);
        }
    } else {
        // Non-segmented notes: single blob following pitch curve
        QVector<QPointF> topCurve;
        QVector<QPointF> bottomCurve;
        int numPoints = std::max(10, width / 5);

        for (int i = 0; i <= numPoints; ++i) {
            double t = static_cast<double>(i) / numPoints;
            int posX = x + static_cast<int>(t * width);

            double pitchAtT = note.getPitchAt(t);
            int centerY = frequencyToPixel(pitchAtT);

            double dynamics = getActiveCurveValue(note, t);
            double envelope = dynamics;

            int topOffset = static_cast<int>(envelope * blobHeight * 0.5);
            topCurve.append(QPointF(posX, centerY - topOffset));

            double bottomValue = 0.0;
            const Vibrato &vibrato = note.getVibrato();
            if (vibrato.active && t >= vibrato.onset) {
                double vibratoT = (t - vibrato.onset) / (1.0 - vibrato.onset);
                bottomValue = vibrato.amplitudeDepth * vibrato.envelopeAt(vibratoT);
            }
            int bottomOffset = static_cast<int>(bottomValue * blobHeight * 0.5);
            bottomCurve.append(QPointF(posX, centerY + bottomOffset));
        }

        QPainterPath blobPath;
        blobPath.moveTo(topCurve.first());
        for (int i = 1; i < topCurve.size(); ++i) {
            blobPath.lineTo(topCurve[i]);
        }
        for (int i = bottomCurve.size() - 1; i >= 0; --i) {
            blobPath.lineTo(bottomCurve[i]);
        }
        blobPath.closeSubpath();

        painter.drawPath(blobPath);
    }

    // Draw selection rectangle if selected
    if (isSelected) {
        drawSelectionRectangle(painter, note);
    }

    // Pitch / variation indicator at the note's start.
    // Default: small dot. When the toolbar's active variation matches this
    // note's variation (and isn't Base), the dot expands into a circle that
    // displays the variation's digit — so clicking "1" reveals every
    // variation-1 note at a glance, and clicking "Base" leaves the canvas
    // clean.
    double startPitch = note.getPitchAt(0.0);
    int startCenterY = frequencyToPixel(startPitch);

    const int varIdx = note.getVariationIndex();
    const bool showIndicator = (varIdx > 0 && varIdx == m_activeVariationIndex);

    if (showIndicator) {
        const int circleD = 16;
        const int circleX = x - circleD / 2;            // centred on the start
        const int circleY = startCenterY - circleD / 2;

        painter.setBrush(noteColor.darker(150));
        painter.setPen(QPen(noteColor.darker(220), 1));
        painter.drawEllipse(circleX, circleY, circleD, circleD);

        QFont prevFont = painter.font();
        QFont font = prevFont;
        font.setPointSizeF(qMax(9.0, prevFont.pointSizeF()));
        font.setBold(true);
        painter.setFont(font);
        painter.setPen(QColor(255, 255, 255, 240));
        painter.drawText(QRect(circleX, circleY, circleD, circleD),
                         Qt::AlignCenter, QString::number(varIdx));
        painter.setFont(prevFont);
    } else {
        painter.setBrush(noteColor.darker(140));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(x - 3, startCenterY - 3, 6, 6);
    }

    // Draw segment overlay if this note is in segment editing mode
    if (segmentEditingMode && segmentEditingNoteIndex >= 0) {
        const QVector<Note> &notes = phrase.getNotes();
        if (segmentEditingNoteIndex < notes.size() && &note == &notes[segmentEditingNoteIndex]) {
            drawSegmentOverlay(painter, note);
        }
    }
}

void ScoreCanvas::drawPendingNote(QPainter &painter)
{
    if (!isDrawingNote) return;

    // Calculate position and size for note being drawn
    int x1 = noteStartPos.x();
    int x2 = noteCurrentPos.x();
    int y = noteStartPos.y();

    int x = std::min(x1, x2);
    int width = std::abs(x2 - x1);
    int height = 20;

    // Draw semi-transparent preview
    painter.setBrush(QColor(100, 150, 255, 100));  // More transparent
    painter.setPen(QPen(QColor(50, 100, 200), 2, Qt::DashLine));
    painter.drawRoundedRect(x, y - height/2, width, height, 5, 5);
}

void ScoreCanvas::drawLassoRectangle(QPainter &painter)
{
    if (!isDrawingLasso) return;

    // Calculate lasso rectangle bounds
    int x = std::min(lassoStartPos.x(), lassoCurrentPos.x());
    int y = std::min(lassoStartPos.y(), lassoCurrentPos.y());
    int width = std::abs(lassoCurrentPos.x() - lassoStartPos.x());
    int height = std::abs(lassoCurrentPos.y() - lassoStartPos.y());

    // Draw selection rectangle with dashed border
    painter.setBrush(QColor(100, 150, 255, 50));  // Semi-transparent blue fill
    painter.setPen(QPen(QColor(50, 100, 200), 2, Qt::DashLine));
    painter.drawRect(x, y, width, height);
}

// ============================================================================
// Mouse Interaction
// ============================================================================

void ScoreCanvas::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Check if in segment editing mode - double-click exits
        if (segmentEditingMode) {
            exitSegmentEditingMode();
            event->accept();
            return;
        }

        // Double-click on a lane: the full curve dialog, pre-pointed at that
        // lane's curve (single click = hull, double = dialog). Checked before
        // the note body so a lane never falls through to note handling.
        int laneNote = -1, laneIdx = -1;
        if (m_densityTier != DensityTier::Off &&
            findLaneAtPosition(event->pos(), laneNote, laneIdx)) {
            editLaneCurveDialog(laneNote, laneIdx);
            event->accept();
            return;
        }

        int clickedNoteIndex = findNoteAtPosition(event->pos());

        if (clickedNoteIndex >= 0) {
            const Note &note = phrase.getNotes()[clickedNoteIndex];

            // Quantized continuous notes: enter segment editing mode
            if (note.isQuantized() && note.hasPitchCurve()) {
                enterSegmentEditingMode(clickedNoteIndex);

                // Select the segment under cursor
                int segIndex = findSegmentAtPosition(event->pos(), clickedNoteIndex);
                if (segIndex >= 0) {
                    selectedSegmentIndex = segIndex;
                }
                update();
                event->accept();
                return;
            }

            // Selected note: open curve editor for the active curve
            if (selectedNoteIndices.contains(clickedNoteIndex)) {
                editNoteCurve(clickedNoteIndex);
                event->accept();
                return;
            }
        }
    }

    QWidget::mouseDoubleClickEvent(event);
}

bool ScoreCanvas::handleLanePress(const QPoint &pos)
{
    // Shared by mousePressEvent and tabletEvent - Nimus works with the pen, so
    // a lane interaction that only existed on the mouse path would be invisible
    // in practice. Returns true when the press was consumed.
    if (m_densityTier == DensityTier::Off) return false;

    const QVector<Note> &notes = phrase.getNotes();

    // An open lane hull owns its dots first.
    if (m_editLaneNote >= 0 && m_editLaneNote < notes.size()) {
        const Note &editNote = notes[m_editLaneNote];
        const QVector<CurveLanes::Lane> editLanes = resolveLanesFor(editNote);
        if (m_editLaneIndex >= 0 && m_editLaneIndex < editLanes.size()) {
            const QRect r = laneRect(editNote, m_editLaneIndex,
                                     editLanes.size(), m_editLaneIndex);
            const int dot = findLaneDotAt(pos, editNote, r);
            if (dot >= 0) {
                pendingDragMode = EditingLaneCurve;
                currentDragMode = NoDrag;
                dragThresholdExceeded = false;
                dragStartPos = pos;
                editingDotIndex = dot;
                editingDotTimePos = static_cast<double>(dot + 1) /
                                    (calculateCurveDotCount(editNote) + 1);
                dragStartCurve = editNote.getExpressiveCurve(
                    editLanes[m_editLaneIndex].curveIndex);
                return true;
            }
        }
    }

    // Otherwise a click on any lane opens it - no menu, no dropdown.
    int hitNote = -1, hitLane = -1;
    if (findLaneAtPosition(pos, hitNote, hitLane)) {
        const QVector<CurveLanes::Lane> hitLanes = resolveLanesFor(notes[hitNote]);
        if (hitLane < hitLanes.size()) {
            if (hitLanes[hitLane].curveIndex >= 0)
                openLaneEditor(hitNote, hitLane);           // Curve or Orphan
            else if (hitLanes[hitLane].state == CurveLanes::State::Empty)
                createCurveOnLane(hitNote, hitLane);        // create, then open
            else
                closeLaneEditor();                          // graph-driven: inert
        }
        return true;
    }

    // Clicking anywhere else closes the hull, but does not consume the press -
    // normal note handling continues.
    closeLaneEditor();
    return false;
}

void ScoreCanvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Handle segment editing mode first
        if (segmentEditingMode && segmentEditingNoteIndex >= 0) {
            int segIndex = findSegmentAtPosition(event->pos(), segmentEditingNoteIndex);
            if (segIndex >= 0) {
                // Clicking on a segment - select it
                selectedSegmentIndex = segIndex;
                update();
                return;
            } else {
                // Clicking outside the note - exit segment mode
                exitSegmentEditingMode();
                // Fall through to normal handling
            }
        }

        // Mode-specific behavior (note handling)
        if (currentInputMode == SelectionMode) {
            const QVector<Note> &notes = phrase.getNotes();

            // Curve lanes (D7) - checked before note handles, since lanes live
            // entirely outside the note rect and cannot shadow them.
            if (handleLanePress(event->pos())) return;

            // Ctrl+click on the body of an already-selected note toggles it off
            if (event->modifiers() & Qt::ControlModifier) {
                int toggleIndex = findNoteAtPosition(event->pos());
                if (toggleIndex >= 0 && selectedNoteIndices.contains(toggleIndex)) {
                    selectedNoteIndices.removeAll(toggleIndex);
                    emit noteSelectionChanged();
                    update();
                    return;
                }
            }

            // FIRST: Check if clicking on handles/dots of selected note (these are outside note rect)
            if (selectedNoteIndices.size() == 1) {
                int selectedIndex = selectedNoteIndices.first();
                if (selectedIndex < notes.size()) {
                    const Note &selectedNote = notes[selectedIndex];
                    DragMode detectedMode = detectDragMode(event->pos(), selectedNote);

                    if (detectedMode != NoDrag) {
                        // Clicking on a handle/dot - prepare for drag operation
                        // Use pending mode with threshold to avoid accidental drags from pen barrel buttons
                        pendingDragMode = detectedMode;
                        currentDragMode = NoDrag;  // Don't start dragging yet
                        dragThresholdExceeded = false;
                        dragStartPos = event->pos();
                        dragStartTime = selectedNote.getStartTime();
                        dragStartPitch = selectedNote.getPitchHz();
                        dragStartDuration = selectedNote.getDuration();

                        // Store original curves for editing
                        // Note: use pendingDragMode here, not currentDragMode (which is NoDrag)
                        if (pendingDragMode == EditingTopCurve ||
                            pendingDragMode == EditingTopCurveStart ||
                            pendingDragMode == EditingTopCurveEnd) {
                            // Use active expressive curve resolved by name for this specific note
                            int activeIdx = resolveActiveCurveIndex(selectedNote);
                            if (activeIdx > 0) {
                                dragStartCurve = selectedNote.getExpressiveCurve(activeIdx);
                            } else {
                                dragStartCurve = selectedNote.getDynamicsCurve();
                            }
                        } else if (pendingDragMode == EditingPitchCurve ||
                                   pendingDragMode == EditingPitchCurveStart ||
                                   pendingDragMode == EditingPitchCurveEnd) {
                            dragStartCurve = selectedNote.getPitchCurve();
                            if (dragStartCurve.isEmpty()) {
                                // Discrete note: seed with nominal pitch so valueAt() returns nominal
                                // for unedited points. First drag will auto-promote to continuous.
                                double nominal = selectedNote.getPitchHz();
                                dragStartCurve.addPoint(0.0, nominal);
                                dragStartCurve.addPoint(1.0, nominal);
                            }
                        } else if (pendingDragMode == TransformStretchTop ||
                                   pendingDragMode == TransformStretchBottom ||
                                   pendingDragMode == TransformRotate) {
                            dragStartCurve = selectedNote.getPitchCurve();
                            if (dragStartCurve.isEmpty()) {
                                // Discrete note: seed with N uniformly-spaced points at nominal
                                // pitch so transforms have material to act on (rotation tilts,
                                // and the note auto-promotes to continuous).
                                int n = calculateCurveDotCount(selectedNote) + 2;
                                double nominal = selectedNote.getPitchHz();
                                for (int i = 0; i < n; ++i) {
                                    double t = (n == 1) ? 0.0 : double(i) / (n - 1);
                                    dragStartCurve.addPoint(t, nominal);
                                }
                            }
                        } else if (pendingDragMode == DraggingNote && selectedNote.hasPitchCurve()) {
                            // Save pitch curve for continuous notes
                            dragStartCurve = selectedNote.getPitchCurve();
                        }

                        return;  // Don't proceed with other selection logic
                    }
                }
            }

            // Check resize handles on any selected note (multi-selection resize)
            if (selectedNoteIndices.size() > 1) {
                for (int selIdx : selectedNoteIndices) {
                    if (selIdx >= 0 && selIdx < notes.size()) {
                        const Note &selNote = notes[selIdx];
                        if (getLeftResizeHandle(selNote).contains(event->pos())) {
                            pendingDragMode = ResizingLeft;
                            currentDragMode = NoDrag;
                            dragThresholdExceeded = false;
                            dragStartPos = event->pos();

                            multiResizeStartTimes.clear();
                            multiResizeStartDurations.clear();
                            for (int index : selectedNoteIndices) {
                                if (index >= 0 && index < notes.size()) {
                                    const Note &note = notes[index];
                                    multiResizeStartTimes.append(qMakePair(index, note.getStartTime()));
                                    multiResizeStartDurations.append(qMakePair(index, note.getDuration()));
                                }
                            }
                            return;
                        }
                        if (getRightResizeHandle(selNote).contains(event->pos())) {
                            pendingDragMode = ResizingRight;
                            currentDragMode = NoDrag;
                            dragThresholdExceeded = false;
                            dragStartPos = event->pos();

                            multiResizeStartTimes.clear();
                            multiResizeStartDurations.clear();
                            for (int index : selectedNoteIndices) {
                                if (index >= 0 && index < notes.size()) {
                                    const Note &note = notes[index];
                                    multiResizeStartTimes.append(qMakePair(index, note.getStartTime()));
                                    multiResizeStartDurations.append(qMakePair(index, note.getDuration()));
                                }
                            }
                            return;
                        }
                    }
                }
            }

            // SECOND: Check if clicking on a note body
            int clickedNoteIndex = findNoteAtPosition(event->pos());

            if (!selectedNoteIndices.isEmpty() && selectedNoteIndices.contains(clickedNoteIndex)) {
                // Clicking on body of a selected note - prepare for drag
                if (selectedNoteIndices.size() == 1) {
                    // Already handled above with detectDragMode
                    return;
                } else {
                    // Multi-selection - allow dragging the group
                    pendingDragMode = DraggingNote;
                    currentDragMode = NoDrag;  // Don't start dragging yet
                    dragThresholdExceeded = false;
                    dragStartPos = event->pos();

                    // Store original states for all selected notes
                    multiDragStartTimes.clear();
                    multiDragStartPitches.clear();
                    multiDragStartCurves.clear();

                    for (int index : selectedNoteIndices) {
                        if (index >= 0 && index < notes.size()) {
                            const Note &note = notes[index];
                            multiDragStartTimes.append(qMakePair(index, note.getStartTime()));
                            multiDragStartPitches.append(qMakePair(index, note.getPitchHz()));
                            if (note.hasPitchCurve()) {
                                multiDragStartCurves.append(qMakePair(index, note.getPitchCurve()));
                            }
                        }
                    }

                    return;  // Don't start selecting another note
                }
            }

            // THIRD: Selecting a different note or empty space
            if (clickedNoteIndex >= 0) {
                // Switch active track to the clicked note's track, but only if that track
                // is not already part of the multi-selection (Ctrl+Click preserves the set).
                if (clickedNoteIndex < notes.size()) {
                    int noteTrackIndex = notes[clickedNoteIndex].getTrackIndex();
                    if (trackSelector && !trackSelector->isTrackSelected(noteTrackIndex)) {
                        trackSelector->setActiveTrack(noteTrackIndex);
                    }
                }

                // Then select the note (Shift = range, Ctrl = add)
                if (event->modifiers() & Qt::ShiftModifier) {
                    selectNoteRange(clickedNoteIndex);
                } else {
                    bool addToSelection = (event->modifiers() & Qt::ControlModifier);
                    selectNote(clickedNoteIndex, addToSelection);
                }

                update();
            } else {
                // Clicked empty space - start lasso selection
                lassoAddToSelection = (event->modifiers() & Qt::ControlModifier);
                if (!lassoAddToSelection) {
                    deselectAll();
                }
                isDrawingLasso = true;
                lassoStartPos = event->pos();
                lassoCurrentPos = event->pos();
                m_autoScrollTimer->start();  // Run every tick; timer checks cursor vs edge
                update();
            }
        } else {
            // In drawing modes (discrete or continuous), start drawing a new note
            isDrawingNote = true;
            noteStartPos = event->pos();
            noteCurrentPos = event->pos();

            // In continuous mode, start tracking pitch curve
            if (currentInputMode == DrawModeContinuous) {
                pitchPoints.clear();
                double startTime = pixelToTime(event->pos().x());
                double startPitch = pixelToFrequency(event->pos().y());
                pitchPoints.append(qMakePair(startTime, startPitch));
            }

            update();
        }
    }

    // Right-click: Show context menu
    if (event->button() == Qt::RightButton) {
        // Handle segment editing mode
        if (segmentEditingMode && selectedSegmentIndex >= 0) {
            showSegmentContextMenu(event->globalPosition().toPoint());
            return;
        }

        int clickedNoteIndex = findNoteAtPosition(event->pos());

        // If clicked on unselected note, select it
        if (clickedNoteIndex >= 0 && !isNoteSelected(clickedNoteIndex)) {
            selectNote(clickedNoteIndex, false);
        }

        // Only show menu if we have selection
        if (!selectedNoteIndices.isEmpty()) {
            showNoteContextMenu(event->globalPosition().toPoint());
        }
    }
}

void ScoreCanvas::mouseMoveEvent(QMouseEvent *event)
{
    // Emit cursor position for status bar (always update, regardless of what else is happening)
    double cursorTime = pixelToTime(event->pos().x());
    double cursorPitch = pixelToFrequency(event->pos().y());
    emit cursorPositionChanged(cursorTime, cursorPitch);

    // Curve lanes: track which lane or overlay band the cursor is over, so it
    // can be named. Skipped mid-drag so the readout doesn't flicker while
    // notes or curves are being moved.
    if (m_densityTier != DensityTier::Off &&
        currentDragMode == NoDrag && pendingDragMode == NoDrag) {
        updateLaneHover(event->pos());
    }

    // Check if we have a pending drag and should activate it (threshold exceeded)
    if (pendingDragMode != NoDrag && !dragThresholdExceeded) {
        QPoint delta = event->pos() - dragStartPos;
        int distance = std::abs(delta.x()) + std::abs(delta.y());  // Manhattan distance
        if (distance >= DRAG_THRESHOLD) {
            // Threshold exceeded - activate the drag
            currentDragMode = pendingDragMode;
            pendingDragMode = NoDrag;
            dragThresholdExceeded = true;
        }
    }

    // Handle dragging/resizing selected notes
    if (currentDragMode != NoDrag && !selectedNoteIndices.isEmpty()) {
        QVector<Note> &notes = phrase.getNotes();
        QPoint delta = event->pos() - dragStartPos;
        // Slide mode: lock pitch — discard vertical component when moving note bodies
        if (slideMode && currentDragMode == DraggingNote)
            delta.setY(0);

        if (selectedNoteIndices.size() == 1) {
            // Single selection - full editing capabilities
            int selectedIndex = selectedNoteIndices.first();
            if (selectedIndex >= notes.size()) return;

            Note &note = notes[selectedIndex];

            switch (currentDragMode) {
            case DraggingNote: {
            // Move note in time and pitch
            double timeDelta = pixelToTime(delta.x()) - pixelToTime(0);
            double newStartTime = dragStartTime + timeDelta;
            if (newStartTime < 0.0) newStartTime = 0.0;  // Don't go negative

            note.setStartTime(newStartTime);

            // Handle pitch change based on whether note has pitch curve
            if (note.hasPitchCurve()) {
                // Continuous note with pitch curve: shift entire curve by pixel delta
                Curve shiftedPitchCurve;
                const Curve& originalCurve = dragStartCurve;  // Use curve saved at drag start
                for (const auto& point : originalCurve.getPoints()) {
                    double t = point.time;
                    double originalPitch = point.value;
                    // Convert to pixel, apply delta, convert back
                    int originalPixelY = frequencyToPixel(originalPitch);
                    int newPixelY = originalPixelY + delta.y();
                    double newPitch = pixelToFrequency(newPixelY);
                    shiftedPitchCurve.addPoint(t, newPitch);
                }

                // If note is quantized, snap the shifted curve to scale (maintains legato/stepped effect)
                if (note.isQuantized()) {
                    Curve quantizedCurve = quantizePitchCurveToScale(shiftedPitchCurve, note);
                    note.setPitchCurve(quantizedCurve);
                } else {
                    note.setPitchCurve(shiftedPitchCurve);
                }
            } else {
                // Discrete note: snap to scale lines at note's time position
                double rawPitch = pixelToFrequency(dragStartPos.y() + delta.y());
                double snappedPitch = snapToNearestScaleLineAtTime(rawPitch, note.getStartTime());
                note.setPitchHz(snappedPitch);
            }
            break;
        }

        case ResizingLeft: {
            // Resize from left (change start time and duration)
            double timeDelta = pixelToTime(delta.x()) - pixelToTime(0);
            double newStartTime = dragStartTime + timeDelta;
            double newDuration = dragStartDuration - timeDelta;

            // Constrain minimum duration
            if (newDuration < 1.0) {
                newDuration = 1.0;
                newStartTime = dragStartTime + dragStartDuration - 1.0;
            }
            if (newStartTime < 0.0) newStartTime = 0.0;

            note.setStartTime(newStartTime);
            note.setDuration(newDuration);
            break;
        }

        case ResizingRight: {
            // Resize from right (change duration only)
            double timeDelta = pixelToTime(delta.x()) - pixelToTime(0);
            double newDuration = dragStartDuration + timeDelta;

            // Constrain minimum duration
            if (newDuration < 1.0) newDuration = 1.0;

            note.setDuration(newDuration);
            break;
        }

        case EditingTopCurve: {
            // Edit active expressive curve by dragging dot vertically
            double dynamicsChange = -delta.y() / 50.0;  // 50 pixels = 1.0 change

            double originalValue = dragStartCurve.valueAt(editingDotTimePos);
            double newValue = qBound(0.0, originalValue + dynamicsChange, 1.0);

            // Get mutable reference to active expressive curve (resolved by name for this note)
            int activeIdx = resolveActiveCurveIndex(note);
            // Never coarsen: a 30-point curve drawn in the dialog survives a
            // drag on a narrow note. See curveAfterDotDrag in curve.cpp.
            note.getExpressiveCurve(activeIdx) =
                curveAfterDotDrag(dragStartCurve, calculateCurveDotCount(note),
                                       editingDotIndex, newValue);
            break;
        }

        case EditingLaneCurve: {
            const QVector<CurveLanes::Lane> lanes = resolveLanesFor(note);
            if (m_editLaneIndex < 0 || m_editLaneIndex >= lanes.size()) break;
            const CurveLanes::Lane &lane = lanes[m_editLaneIndex];
            if (lane.curveIndex < 0) break;

            const QRect r = laneRect(note, m_editLaneIndex, lanes.size(), m_editLaneIndex);
            // Positional rather than delta-based: an opened lane is tall
            // enough that 1:1 tracking of the cursor is the natural mapping.
            const double span = std::max(1, r.height() - 1);
            const double newValue =
                qBound(0.0, (r.bottom() - event->pos().y()) / span, 1.0);

            note.getExpressiveCurve(lane.curveIndex) =
                curveAfterDotDrag(dragStartCurve, calculateCurveDotCount(note),
                                       editingDotIndex, newValue);
            break;
        }

        case EditingPitchCurve: {
            // Edit pitch curve by dragging dot vertically (Hz, log-y via canvas mapping)
            double startHz = dragStartCurve.valueAt(editingDotTimePos);
            int startPixelY = frequencyToPixel(startHz);
            double newHz = pixelToFrequency(startPixelY + delta.y());

            Curve &pitchCurve = note.getPitchCurve();
            pitchCurve.clearPoints();

            // Rebuild curve with adaptive number of control points
            int numDots = calculateCurveDotCount(note);
            for (int i = 0; i <= numDots + 1; ++i) {
                double t = i / (numDots + 1.0);
                double value;

                if (i == editingDotIndex + 1) {
                    value = newHz;
                } else {
                    value = dragStartCurve.valueAt(t);
                }

                pitchCurve.addPoint(t, value);
            }

            pitchCurve.sortPoints();
            break;
        }

        case EditingTopCurveStart: {
            // Edit active expressive curve start value (t=0.0)
            double dynamicsChange = -delta.y() / 50.0;
            double originalValue = dragStartCurve.valueAt(0.0);
            double newValue = qBound(0.0, originalValue + dynamicsChange, 1.0);

            int activeIdx = resolveActiveCurveIndex(note);
            Curve &activeCurve = note.getExpressiveCurve(activeIdx);
            activeCurve.clearPoints();

            // Rebuild curve, changing only the start point
            int numDots = calculateCurveDotCount(note);
            for (int i = 0; i <= numDots + 1; ++i) {
                double t = i / (numDots + 1.0);
                double value = (i == 0) ? newValue : dragStartCurve.valueAt(t);
                activeCurve.addPoint(t, value);
            }
            activeCurve.sortPoints();
            break;
        }

        case EditingTopCurveEnd: {
            // Edit active expressive curve end value (t=1.0)
            double dynamicsChange = -delta.y() / 50.0;
            double originalValue = dragStartCurve.valueAt(1.0);
            double newValue = qBound(0.0, originalValue + dynamicsChange, 1.0);

            int activeIdx = resolveActiveCurveIndex(note);
            Curve &activeCurve = note.getExpressiveCurve(activeIdx);
            activeCurve.clearPoints();

            // Rebuild curve, changing only the end point
            int numDots = calculateCurveDotCount(note);
            for (int i = 0; i <= numDots + 1; ++i) {
                double t = i / (numDots + 1.0);
                double value = (i == numDots + 1) ? newValue : dragStartCurve.valueAt(t);
                activeCurve.addPoint(t, value);
            }
            activeCurve.sortPoints();
            break;
        }

        case EditingPitchCurveStart: {
            // Edit pitch curve start value (t=0.0)
            double startHz = dragStartCurve.valueAt(0.0);
            int startPixelY = frequencyToPixel(startHz);
            double newHz = pixelToFrequency(startPixelY + delta.y());

            Curve &pitchCurve = note.getPitchCurve();
            pitchCurve.clearPoints();

            int numDots = calculateCurveDotCount(note);
            for (int i = 0; i <= numDots + 1; ++i) {
                double t = i / (numDots + 1.0);
                double value = (i == 0) ? newHz : dragStartCurve.valueAt(t);
                pitchCurve.addPoint(t, value);
            }
            pitchCurve.sortPoints();
            break;
        }

        case EditingPitchCurveEnd: {
            // Edit pitch curve end value (t=1.0)
            double startHz = dragStartCurve.valueAt(1.0);
            int startPixelY = frequencyToPixel(startHz);
            double newHz = pixelToFrequency(startPixelY + delta.y());

            Curve &pitchCurve = note.getPitchCurve();
            pitchCurve.clearPoints();

            int numDots = calculateCurveDotCount(note);
            for (int i = 0; i <= numDots + 1; ++i) {
                double t = i / (numDots + 1.0);
                double value = (i == numDots + 1) ? newHz : dragStartCurve.valueAt(t);
                pitchCurve.addPoint(t, value);
            }
            pitchCurve.sortPoints();
            break;
        }

        case TransformStretchTop:
        case TransformStretchBottom: {
            // Vertical pitch stretch: scale each curve point's deviation from nominal
            // by a factor derived from how far the handle has moved relative to the pivot.
            int pivotY = frequencyToPixel(note.getPitchHz());
            int origHandleY = (currentDragMode == TransformStretchTop)
                ? transformModeRect.top()
                : transformModeRect.bottom();
            int origOffset = origHandleY - pivotY;
            if (origOffset == 0) break;  // Degenerate — skip
            double scale = double(origOffset + delta.y()) / double(origOffset);

            Curve &pitchCurve = note.getPitchCurve();
            pitchCurve.clearPoints();
            for (const auto &p : dragStartCurve.getPoints()) {
                int origPixelY = frequencyToPixel(p.value);
                int newPixelY  = pivotY + static_cast<int>((origPixelY - pivotY) * scale);
                pitchCurve.addPoint(p.time, pixelToFrequency(newPixelY));
            }
            pitchCurve.sortPoints();
            break;
        }

        case TransformRotate: {
            // Anchored rotation: pivot at the corner OPPOSITE to the one being dragged
            // so the grabbed corner moves with the cursor and the other side stays put.
            // Implemented as a pitch shear around t_pivot (anchor's normalised time).
            const QPointF &anchor = transformRotateAnchor;
            double dx0 = dragStartPos.x() - anchor.x();
            double dy0 = dragStartPos.y() - anchor.y();
            double dx1 = (dragStartPos.x() + delta.x()) - anchor.x();
            double dy1 = (dragStartPos.y() + delta.y()) - anchor.y();
            double angle = std::atan2(dy1, dx1) - std::atan2(dy0, dx0);
            double tanA = std::tan(angle);
            int width = transformModeRect.width();
            double tPivot = (width > 0)
                ? double(anchor.x() - transformModeRect.left()) / double(width)
                : 0.5;

            Curve &pitchCurve = note.getPitchCurve();
            pitchCurve.clearPoints();
            for (const auto &p : dragStartCurve.getPoints()) {
                int origPixelY = frequencyToPixel(p.value);
                int newPixelY  = origPixelY + static_cast<int>(tanA * (p.time - tPivot) * width);
                pitchCurve.addPoint(p.time, pixelToFrequency(newPixelY));
            }
            pitchCurve.sortPoints();
            break;
        }

            case NoDrag:
                break;
            }
        } else {
            // Multi-selection resize or drag
            if (currentDragMode == ResizingLeft) {
                double timeDelta = pixelToTime(delta.x()) - pixelToTime(0);
                for (int i = 0; i < multiResizeStartTimes.size(); ++i) {
                    int index = multiResizeStartTimes[i].first;
                    double origStart = multiResizeStartTimes[i].second;
                    double origDur = multiResizeStartDurations[i].second;
                    if (index >= 0 && index < notes.size()) {
                        double newStart = origStart + timeDelta;
                        double newDur = origDur - timeDelta;
                        if (newDur < 1.0) {
                            newDur = 1.0;
                            newStart = origStart + origDur - 1.0;
                        }
                        if (newStart < 0.0) newStart = 0.0;
                        notes[index].setStartTime(newStart);
                        notes[index].setDuration(newDur);
                    }
                }
            } else if (currentDragMode == ResizingRight) {
                double timeDelta = pixelToTime(delta.x()) - pixelToTime(0);
                for (int i = 0; i < multiResizeStartDurations.size(); ++i) {
                    int index = multiResizeStartDurations[i].first;
                    double origDur = multiResizeStartDurations[i].second;
                    if (index >= 0 && index < notes.size()) {
                        double newDur = origDur + timeDelta;
                        if (newDur < 1.0) newDur = 1.0;
                        notes[index].setDuration(newDur);
                    }
                }
            } else if (currentDragMode == DraggingNote) {
                // Calculate time delta from original position
                double timeDelta = pixelToTime(delta.x()) - pixelToTime(0);

                // Apply time deltas to all selected notes
                for (int i = 0; i < multiDragStartTimes.size(); ++i) {
                    int index = multiDragStartTimes[i].first;
                    double originalTime = multiDragStartTimes[i].second;

                    if (index >= 0 && index < notes.size()) {
                        Note &note = notes[index];

                        // Apply time movement from original position
                        double newStartTime = originalTime + timeDelta;
                        if (newStartTime < 0.0) newStartTime = 0.0;
                        note.setStartTime(newStartTime);
                    }
                }

                // Apply pitch movement - each note moves by same PIXEL delta
                for (int i = 0; i < multiDragStartPitches.size(); ++i) {
                    int index = multiDragStartPitches[i].first;
                    double originalPitch = multiDragStartPitches[i].second;

                    if (index >= 0 && index < notes.size()) {
                        Note &note = notes[index];

                        if (!note.hasPitchCurve()) {
                            // Discrete note: apply pixel delta then snap to nearest scale degree
                            int originalPixelY = frequencyToPixel(originalPitch);
                            int newPixelY = originalPixelY + delta.y();
                            double rawPitch = pixelToFrequency(newPixelY);
                            // Snap to nearest scale line at note's time position
                            double snappedPitch = snapToNearestScaleLineAtTime(rawPitch, note.getStartTime());
                            note.setPitchHz(snappedPitch);
                        }
                    }
                }

                // Apply pitch curve shifts - each curve point moves by same PIXEL delta
                for (int i = 0; i < multiDragStartCurves.size(); ++i) {
                    int index = multiDragStartCurves[i].first;
                    const Curve &originalCurve = multiDragStartCurves[i].second;

                    if (index >= 0 && index < notes.size()) {
                        Note &note = notes[index];

                        // Shift pitch curve by pixel delta
                        Curve shiftedPitchCurve;
                        for (const auto& point : originalCurve.getPoints()) {
                            // Convert original pitch to pixel Y
                            int originalPixelY = frequencyToPixel(point.value);
                            // Apply pixel delta
                            int newPixelY = originalPixelY + delta.y();
                            // Convert back to frequency
                            double newPitch = pixelToFrequency(newPixelY);
                            shiftedPitchCurve.addPoint(point.time, newPitch);
                        }

                        // If note is quantized, snap the shifted curve to scale (maintains legato effect)
                        if (note.isQuantized()) {
                            Curve quantizedCurve = quantizePitchCurveToScale(shiftedPitchCurve, note);
                            note.setPitchCurve(quantizedCurve);
                        } else {
                            note.setPitchCurve(shiftedPitchCurve);
                        }
                    }
                }
            }
        }

        update();
        return;
    }

    // Update cursor based on hover position over selected notes
    if (selectedNoteIndices.size() == 1 && currentDragMode == NoDrag && !isDrawingNote) {
        const QVector<Note> &notes = phrase.getNotes();
        int selectedIndex = selectedNoteIndices.first();
        if (selectedIndex < notes.size()) {
            const Note &selectedNote = notes[selectedIndex];
            DragMode hoverMode = detectDragMode(event->pos(), selectedNote);

            switch (hoverMode) {
            case ResizingLeft:
            case ResizingRight:
                setCursor(Qt::SizeHorCursor);
                break;
            case EditingTopCurve:
            case EditingLaneCurve:
            case EditingPitchCurve:
            case EditingTopCurveStart:
            case EditingTopCurveEnd:
            case EditingPitchCurveStart:
            case EditingPitchCurveEnd:
            case TransformStretchTop:
            case TransformStretchBottom:
                setCursor(Qt::SizeVerCursor);
                break;
            case TransformRotate:
                setCursor(Qt::CrossCursor);
                break;
            case DraggingNote:
                setCursor(Qt::SizeAllCursor);
                break;
            case NoDrag:
                setCursor(Qt::ArrowCursor);
                break;
            default:
                break;
            }
        }
    } else if (selectedNoteIndices.size() > 1 && currentDragMode == NoDrag && !isDrawingNote) {
        // Multi-selection: show resize cursor when hovering over any selected note's handles
        const QVector<Note> &notes = phrase.getNotes();
        bool foundHandle = false;
        for (int selIdx : selectedNoteIndices) {
            if (selIdx >= 0 && selIdx < notes.size()) {
                const Note &selNote = notes[selIdx];
                if (getLeftResizeHandle(selNote).contains(event->pos()) ||
                    getRightResizeHandle(selNote).contains(event->pos())) {
                    setCursor(Qt::SizeHorCursor);
                    foundHandle = true;
                    break;
                }
            }
        }
        if (!foundHandle) {
            // Check if hovering over any selected note body
            int hoverIndex = findNoteAtPosition(event->pos());
            if (selectedNoteIndices.contains(hoverIndex)) {
                setCursor(Qt::SizeAllCursor);
            } else {
                setCursor(Qt::ArrowCursor);
            }
        }
    } else if (!isDrawingNote) {
        setCursor(Qt::ArrowCursor);
    }

    // Handle lasso selection dragging
    if (isDrawingLasso) {
        lassoCurrentPos = event->pos();
        // Timer is already running (started at lasso begin) and uses QCursor::pos()
        // for real-time position — no start/stop needed here.
        update();
        return;
    }

    // Handle drawing new note
    if (isDrawingNote) {
        noteCurrentPos = event->pos();

        // In continuous mode, track pitch changes
        if (currentInputMode == DrawModeContinuous) {
            double currentTime = pixelToTime(event->pos().x());
            double currentPitch = pixelToFrequency(event->pos().y());
            pitchPoints.append(qMakePair(currentTime, currentPitch));
        }

        update();
    }
}

void ScoreCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    // Cancel pending drag if button released without exceeding threshold
    if (event->button() == Qt::LeftButton && pendingDragMode != NoDrag && !dragThresholdExceeded) {
        // Click without significant movement - cancel the pending drag
        pendingDragMode = NoDrag;
        dragThresholdExceeded = false;
    }

    // End drag/resize operation
    if (event->button() == Qt::LeftButton && currentDragMode != NoDrag) {
        if (selectedNoteIndices.size() == 1) {
            // Single selection - full editing capabilities
            int selectedIndex = selectedNoteIndices.first();
            if (selectedIndex >= 0 && selectedIndex < phrase.getNotes().size()) {
                const Note &note = phrase.getNotes()[selectedIndex];

                switch (currentDragMode) {
                case DraggingNote: {
                    // Push move command with old and new states
                    double newStartTime = note.getStartTime();
                    double newPitch = note.getPitchHz();
                    Curve newPitchCurve;
                    bool hasCurve = note.hasPitchCurve();
                    if (hasCurve) {
                        newPitchCurve = note.getPitchCurve();
                    }

                    undoStack->push(new MoveNoteCommand(&phrase, selectedIndex,
                                                        dragStartTime, dragStartPitch,
                                                        newStartTime, newPitch,
                                                        dragStartCurve, newPitchCurve,
                                                        hasCurve, this));
                    break;
                }

                case ResizingLeft:
                case ResizingRight: {
                    // Push resize command
                    double newStartTime = note.getStartTime();
                    double newDuration = note.getDuration();

                    undoStack->push(new ResizeNoteCommand(&phrase, selectedIndex,
                                                          dragStartTime, dragStartDuration,
                                                          newStartTime, newDuration,
                                                          this));
                    break;
                }

                case EditingTopCurve:
                case EditingTopCurveStart:
                case EditingTopCurveEnd: {
                    // Push curve edit command for active expressive curve
                    int activeIdx = resolveActiveCurveIndex(note);
                    Curve newCurve = note.getExpressiveCurve(activeIdx);
                    if (activeIdx == 0) {
                        undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                             EditCurveCommand::DynamicsCurve,
                                                             dragStartCurve, newCurve,
                                                             this));
                    } else {
                        undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                             EditCurveCommand::ExpressiveCurveN,
                                                             activeIdx,
                                                             dragStartCurve, newCurve,
                                                             this));
                    }
                    break;
                }

                case EditingLaneCurve: {
                    const QVector<CurveLanes::Lane> lanes = resolveLanesFor(note);
                    if (m_editLaneIndex >= 0 && m_editLaneIndex < lanes.size() &&
                        lanes[m_editLaneIndex].curveIndex >= 0) {
                        const int ci = lanes[m_editLaneIndex].curveIndex;
                        undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                             EditCurveCommand::ExpressiveCurveN,
                                                             ci, dragStartCurve,
                                                             note.getExpressiveCurve(ci),
                                                             this));
                    }
                    break;
                }

                case EditingPitchCurve: {
                    // Push curve edit command for pitch curve
                    Curve newCurve = note.getPitchCurve();
                    undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                         EditCurveCommand::PitchCurve,
                                                         dragStartCurve, newCurve,
                                                         this));
                    break;
                }

                case EditingPitchCurveStart:
                case EditingPitchCurveEnd: {
                    // Push curve edit command for pitch curve (start/end)
                    Curve newCurve = note.getPitchCurve();
                    undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                         EditCurveCommand::PitchCurve,
                                                         dragStartCurve, newCurve,
                                                         this));
                    break;
                }

                case TransformStretchTop:
                case TransformStretchBottom:
                case TransformRotate: {
                    // Transform-mode pitch-curve edit
                    Curve newCurve = note.getPitchCurve();
                    undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                         EditCurveCommand::PitchCurve,
                                                         dragStartCurve, newCurve,
                                                         this));
                    break;
                }

                case NoDrag:
                    break;
                default:
                    break;
                }
            }
        } else if (selectedNoteIndices.size() > 1 && currentDragMode == DraggingNote) {
            // Multi-selection drag - create batch move command
            QVector<MoveMultipleNotesCommand::NoteState> oldStates;
            QVector<MoveMultipleNotesCommand::NoteState> newStates;

            const QVector<Note> &notes = phrase.getNotes();

            // Build old states from stored drag start data
            for (int i = 0; i < multiDragStartTimes.size(); ++i) {
                int index = multiDragStartTimes[i].first;
                MoveMultipleNotesCommand::NoteState state;
                state.index = index;
                state.startTime = multiDragStartTimes[i].second;

                // Find matching pitch
                for (const auto &pair : multiDragStartPitches) {
                    if (pair.first == index) {
                        state.pitch = pair.second;
                        break;
                    }
                }

                // Find matching curve if it exists
                state.hasPitchCurve = false;
                for (const auto &pair : multiDragStartCurves) {
                    if (pair.first == index) {
                        state.pitchCurve = pair.second;
                        state.hasPitchCurve = true;
                        break;
                    }
                }

                oldStates.append(state);
            }

            // Build new states from current note positions
            for (int index : selectedNoteIndices) {
                if (index >= 0 && index < notes.size()) {
                    const Note &note = notes[index];
                    MoveMultipleNotesCommand::NoteState state;
                    state.index = index;
                    state.startTime = note.getStartTime();
                    state.pitch = note.getPitchHz();
                    state.hasPitchCurve = note.hasPitchCurve();
                    if (state.hasPitchCurve) {
                        state.pitchCurve = note.getPitchCurve();
                    }
                    newStates.append(state);
                }
            }

            undoStack->push(new MoveMultipleNotesCommand(&phrase, selectedNoteIndices,
                                                         oldStates, newStates, this));
        } else if (selectedNoteIndices.size() > 1 &&
                   (currentDragMode == ResizingLeft || currentDragMode == ResizingRight)) {
            // Multi-selection resize - create batch resize command
            QVector<ResizeMultipleNotesCommand::NoteState> oldStates;
            QVector<ResizeMultipleNotesCommand::NoteState> newStates;

            const QVector<Note> &notes = phrase.getNotes();

            for (int i = 0; i < multiResizeStartTimes.size(); ++i) {
                int index = multiResizeStartTimes[i].first;
                ResizeMultipleNotesCommand::NoteState oldState;
                oldState.index = index;
                oldState.startTime = multiResizeStartTimes[i].second;
                oldState.duration = multiResizeStartDurations[i].second;
                oldStates.append(oldState);

                if (index >= 0 && index < notes.size()) {
                    ResizeMultipleNotesCommand::NoteState newState;
                    newState.index = index;
                    newState.startTime = notes[index].getStartTime();
                    newState.duration = notes[index].getDuration();
                    newStates.append(newState);
                }
            }

            undoStack->push(new ResizeMultipleNotesCommand(&phrase, oldStates, newStates, this));
        }

        currentDragMode = NoDrag;
        pendingDragMode = NoDrag;
        dragThresholdExceeded = false;
        update();
        return;
    }

    // Complete lasso selection
    if (event->button() == Qt::LeftButton && isDrawingLasso) {
        isDrawingLasso = false;
        m_autoScrollTimer->stop();

        // Calculate lasso rectangle
        int x = std::min(lassoStartPos.x(), lassoCurrentPos.x());
        int y = std::min(lassoStartPos.y(), lassoCurrentPos.y());
        int width = std::abs(lassoCurrentPos.x() - lassoStartPos.x());
        int height = std::abs(lassoCurrentPos.y() - lassoStartPos.y());
        QRect lassoRect(x, y, width, height);

        // Find all notes within the lasso rectangle
        QVector<int> notesInRect = findNotesInRectangle(lassoRect);
        if (lassoAddToSelection) {
            // Merge with existing selection
            for (int idx : notesInRect) {
                if (!selectedNoteIndices.contains(idx)) {
                    selectedNoteIndices.append(idx);
                }
            }
            emit noteSelectionChanged();
        } else {
            selectNotes(notesInRect);
        }

        update();
        return;
    }

    if (event->button() == Qt::LeftButton && isDrawingNote) {
        isDrawingNote = false;

        // Calculate note parameters
        double startTime = pixelToTime(std::min(noteStartPos.x(), noteCurrentPos.x()));
        double endTime = pixelToTime(std::max(noteStartPos.x(), noteCurrentPos.x()));
        double duration = endTime - startTime;

        // Use pixel distance to distinguish clicks from intentional drags
        double pixelDist = QLineF(noteStartPos, noteCurrentPos).length();
        if (pixelDist < 3.0) {
            // This was a click, not a drag — use default duration
            duration = 500.0;
            endTime = startTime + duration;
        }

        // Deselect when creating a new note
        deselectAll();

        // Get pitch from vertical position
        double rawPitch = pixelToFrequency(noteStartPos.y());
        double finalPitch;

        // Apply pitch snapping based on input mode
        if (currentInputMode == DrawModeDiscrete) {
            // Discrete mode: snap to nearest scale line at this time position
            finalPitch = snapToNearestScaleLineAtTime(rawPitch, startTime);
        } else {
            // Continuous mode: use literal pitch (no snapping)
            finalPitch = rawPitch;
        }

        // Create note with default dynamics first
        Note newNote(startTime, duration, finalPitch, 0.7);

        // Set the note to use the active track
        newNote.setTrackIndex(activeTrackIndex);

        // Create a default bottom curve with some variation (for visual interest)
        Curve defaultBottomCurve;
        defaultBottomCurve.addPoint(0.0, 0.5);
        defaultBottomCurve.addPoint(0.25, 0.6);
        defaultBottomCurve.addPoint(0.5, 0.65);
        defaultBottomCurve.addPoint(0.75, 0.6);
        defaultBottomCurve.addPoint(1.0, 0.55);
        newNote.setBottomCurve(defaultBottomCurve);

        // If we have captured pressure points from tablet, create a dynamics curve
        if (usingTablet && pressurePoints.size() >= 2) {
            Curve dynamicsCurve;

            // Convert absolute time points to normalized time (0.0-1.0 within the note)
            for (const auto &point : pressurePoints) {
                double absoluteTime = point.first;
                double pressure = point.second;

                // Normalize time within the note duration
                double normalizedTime = (absoluteTime - startTime) / duration;
                normalizedTime = qBound(0.0, normalizedTime, 1.0);

                dynamicsCurve.addPoint(normalizedTime, pressure);
            }

            // Sort points by time to ensure proper interpolation
            dynamicsCurve.sortPoints();

            // Set the curve on the note
            newNote.setDynamicsCurve(dynamicsCurve);
        }

        // If in continuous mode and we have pitch points, create a pitch curve for glissando
        if (currentInputMode == DrawModeContinuous && pitchPoints.size() >= 2) {
            Curve pitchCurve;

            // Convert absolute time points to normalized time (0.0-1.0 within the note)
            for (const auto &point : pitchPoints) {
                double absoluteTime = point.first;
                double pitchHz = point.second;

                // Normalize time within the note duration
                double normalizedTime = (absoluteTime - startTime) / duration;
                normalizedTime = qBound(0.0, normalizedTime, 1.0);

                pitchCurve.addPoint(normalizedTime, pitchHz);
            }

            // Sort points by time to ensure proper interpolation
            pitchCurve.sortPoints();

            // Set the pitch curve on the note
            newNote.setPitchCurve(pitchCurve);
        }

        // If in continuous mode and we have pressure points, create a dynamics curve
        if (currentInputMode == DrawModeContinuous && pressurePoints.size() >= 2) {
            Curve dynamicsCurve;

            // Convert absolute time/pressure points to normalized time (0.0-1.0 within the note)
            for (const auto &point : pressurePoints) {
                double absoluteTime = point.first;
                double pressure = point.second;

                // Normalize time within the note duration
                double normalizedTime = (absoluteTime - startTime) / duration;
                normalizedTime = qBound(0.0, normalizedTime, 1.0);

                // Use pressure as dynamics (already in 0.0-1.0 range)
                dynamicsCurve.addPoint(normalizedTime, pressure);
            }

            // Sort points by time to ensure proper interpolation
            dynamicsCurve.sortPoints();

            // Set the dynamics curve on the note
            newNote.setDynamicsCurve(dynamicsCurve);
        }

        // Stamp current toolbar variation
        newNote.setVariationIndex(m_activeVariationIndex);

        // Use undo command to add note
        undoStack->push(new AddNoteCommand(&phrase, newNote, this));

        // Reset state for next input
        usingTablet = false;
        pressurePoints.clear();
        pitchPoints.clear();

        update();
    }
}

// ============================================================================
// Selection Rectangle
// ============================================================================

void ScoreCanvas::drawSelectionRectangle(QPainter &painter, const Note &note)
{
    // In transform mode, draw against the frozen rect captured at mode entry so the
    // box doesn't visibly resize as the user transforms the curve.
    QRect frozenSelectionRect;
    if (transformMode) {
        frozenSelectionRect = transformModeRect;
    }

    // Calculate bounding rectangle for the note
    int x = timeToPixel(note.getStartTime());
    int width = timeToPixel(note.getEndTime()) - x;

    // Find maximum active curve value for blob height (same as drawNote)
    double maxDynamics = 0.0;
    for (int i = 0; i <= 20; ++i) {
        double t = i / 20.0;
        int idx = resolveActiveCurveIndex(note);
        double value = (idx <= 0)
            ? note.getDynamicsAt(t)
            : note.getExpressiveCurve(idx).valueAt(t);
        if (value > maxDynamics) maxDynamics = value;
    }

    int blobHeight = static_cast<int>(20 + maxDynamics * 60);  // Match drawNote calculation

    // Find min and max pitch across the note (for glissando notes)
    double minPitch = note.getPitchAt(0.0);
    double maxPitch = minPitch;
    for (int i = 0; i <= 20; ++i) {
        double t = i / 20.0;
        double pitch = note.getPitchAt(t);
        if (pitch < minPitch) minPitch = pitch;
        if (pitch > maxPitch) maxPitch = pitch;
    }

    // Calculate vertical bounds
    int topY = frequencyToPixel(maxPitch) - blobHeight/2 - 5;
    int bottomY = frequencyToPixel(minPitch) + blobHeight/2 + 5;
    int height = bottomY - topY;

    QRect selectionRect = transformMode ? frozenSelectionRect
                                        : QRect(x - 5, topY, width + 10, height);

    if (transformMode) {
        // Transform mode: purple outline + 8 perimeter handles for pitch-curve transforms.
        const QColor transformColor(94, 53, 177);  // #5E35B1 (deep purple)
        painter.setPen(QPen(transformColor, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(selectionRect);

        painter.setBrush(transformColor);
        painter.setPen(QPen(QColor(49, 27, 146), 1));  // #311B92 darker purple

        const int cornerSize = 10;
        const int midSize = 8;

        int leftX = selectionRect.left();
        int rightX = selectionRect.right();
        int topYr = selectionRect.top();
        int botYr = selectionRect.bottom();
        int midX = selectionRect.left() + selectionRect.width() / 2;
        int midY = selectionRect.top() + selectionRect.height() / 2;

        // Corners (squares)
        painter.drawRect(leftX  - cornerSize/2, topYr - cornerSize/2, cornerSize, cornerSize);
        painter.drawRect(rightX - cornerSize/2, topYr - cornerSize/2, cornerSize, cornerSize);
        painter.drawRect(leftX  - cornerSize/2, botYr - cornerSize/2, cornerSize, cornerSize);
        painter.drawRect(rightX - cornerSize/2, botYr - cornerSize/2, cornerSize, cornerSize);

        // Edge midpoints (diamonds — rotated squares — to read as different from corners)
        auto drawDiamond = [&](int cx, int cy) {
            QPolygon d;
            d << QPoint(cx, cy - midSize/2)
              << QPoint(cx + midSize/2, cy)
              << QPoint(cx, cy + midSize/2)
              << QPoint(cx - midSize/2, cy);
            painter.drawPolygon(d);
        };
        drawDiamond(midX,  topYr);   // top
        drawDiamond(midX,  botYr);   // bottom
        drawDiamond(leftX,  midY);   // left
        drawDiamond(rightX, midY);   // right

        return;
    }

    // Default mode: gray outline + per-point dots
    painter.setPen(QPen(QColor(128, 128, 128), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(selectionRect);

    painter.setBrush(QColor(200, 200, 200));
    painter.setPen(QPen(QColor(100, 100, 100), 1));

    int dotSize = 8;

    // Calculate adaptive number of dots based on note width
    int numDots = calculateCurveDotCount(note);
    int numSegments = numDots + 1;  // e.g., 4 dots = 5 segments

    // Top edge dots (adaptive count for shaping amplitude curve)
    for (int i = 1; i <= numDots; ++i) {
        int dotX = selectionRect.left() + (selectionRect.width() * i / numSegments);
        painter.drawEllipse(dotX - dotSize/2, selectionRect.top() - dotSize/2, dotSize, dotSize);
    }

    // Bottom edge dots (adaptive count for shaping pitch curve)
    for (int i = 1; i <= numDots; ++i) {
        int dotX = selectionRect.left() + (selectionRect.width() * i / numSegments);
        painter.drawEllipse(dotX - dotSize/2, selectionRect.bottom() - dotSize/2, dotSize, dotSize);
    }

    // Left edge dots: top (dynamics start), middle (resize, square), bottom (pitch start)
    int leftX = selectionRect.left();
    for (int i = 1; i <= 3; ++i) {
        int dotY = selectionRect.top() + (selectionRect.height() * i / 4);
        if (i == 2) {
            // Middle dot is square (resize handle)
            painter.setBrush(QColor(255, 200, 100));  // Orange for resize
            painter.drawRect(leftX - dotSize/2, dotY - dotSize/2, dotSize, dotSize);
            painter.setBrush(QColor(200, 200, 200));  // Reset to gray
        } else {
            painter.drawEllipse(leftX - dotSize/2, dotY - dotSize/2, dotSize, dotSize);
        }
    }

    // Right edge dots: top (dynamics end), middle (resize, square), bottom (pitch end)
    int rightX = selectionRect.right();
    for (int i = 1; i <= 3; ++i) {
        int dotY = selectionRect.top() + (selectionRect.height() * i / 4);
        if (i == 2) {
            // Middle dot is square (resize handle)
            painter.setBrush(QColor(255, 200, 100));  // Orange for resize
            painter.drawRect(rightX - dotSize/2, dotY - dotSize/2, dotSize, dotSize);
            painter.setBrush(QColor(200, 200, 200));  // Reset to gray
        } else {
            painter.drawEllipse(rightX - dotSize/2, dotY - dotSize/2, dotSize, dotSize);
        }
    }
}

// ============================================================================
// Selection Helpers
// ============================================================================

int ScoreCanvas::findNoteAtPosition(const QPoint &pos) const
{
    const QVector<Note> &notes = phrase.getNotes();

    // Search in reverse order (topmost/newest notes first)
    for (int i = notes.size() - 1; i >= 0; --i) {
        const Note &note = notes[i];

        // Only select notes on a currently selected track
        int noteTrack = note.getTrackIndex();
        bool trackReachable = trackSelector
            ? trackSelector->isTrackSelected(noteTrack)
            : (noteTrack == activeTrackIndex);
        if (!trackReachable) continue;

        int x = timeToPixel(note.getStartTime());
        int width = timeToPixel(note.getEndTime()) - x;

        // Find maximum active curve value for blob height
        double maxDynamics = 0.0;
        for (int j = 0; j <= 20; ++j) {
            double t = j / 20.0;
            int activeIdx = resolveActiveCurveIndex(note);
            double value = (activeIdx <= 0)
                ? note.getDynamicsAt(t)
                : note.getExpressiveCurve(activeIdx).valueAt(t);
            if (value > maxDynamics) maxDynamics = value;
        }

        int blobHeight = static_cast<int>(20 + maxDynamics * 60);

        // Find min and max pitch across the note (for glissando notes)
        double minPitch = note.getPitchAt(0.0);
        double maxPitch = minPitch;
        for (int j = 0; j <= 20; ++j) {
            double t = j / 20.0;
            double pitch = note.getPitchAt(t);
            if (pitch < minPitch) minPitch = pitch;
            if (pitch > maxPitch) maxPitch = pitch;
        }

        // Calculate vertical bounds
        int topY = frequencyToPixel(maxPitch) - blobHeight/2;
        int bottomY = frequencyToPixel(minPitch) + blobHeight/2;
        int height = bottomY - topY;

        // Simple bounding box hit test
        QRect noteRect(x, topY, width, height);
        if (noteRect.contains(pos)) {
            return i;
        }
    }

    return -1;  // No note found
}

QVector<int> ScoreCanvas::findNotesInRectangle(const QRect &rect) const
{
    QVector<int> foundIndices;
    const QVector<Note> &notes = phrase.getNotes();

    for (int i = 0; i < notes.size(); ++i) {
        const Note &note = notes[i];

        // Only select notes on a currently selected track
        int noteTrack = note.getTrackIndex();
        bool trackReachable = trackSelector
            ? trackSelector->isTrackSelected(noteTrack)
            : (noteTrack == activeTrackIndex);
        if (!trackReachable) continue;

        QRect noteRect = getNoteRect(note);

        // Check if note rectangle intersects with selection rectangle
        if (noteRect.intersects(rect)) {
            foundIndices.append(i);
        }
    }

    return foundIndices;
}

void ScoreCanvas::selectNote(int index, bool addToSelection)
{
    const QVector<Note> &notes = phrase.getNotes();
    if (index >= 0 && index < notes.size()) {
        if (!addToSelection) {
            selectedNoteIndices.clear();
        }
        if (!selectedNoteIndices.contains(index)) {
            selectedNoteIndices.append(index);
        }
        m_selectionAnchorIndex = index;
        emit noteSelectionChanged();
    }
}

void ScoreCanvas::selectNoteRange(int toIndex)
{
    const QVector<Note> &notes = phrase.getNotes();
    if (toIndex < 0 || toIndex >= notes.size())
        return;

    int anchor = (m_selectionAnchorIndex >= 0 && m_selectionAnchorIndex < notes.size())
                 ? m_selectionAnchorIndex : toIndex;

    // Determine range by start time order
    double anchorTime = notes[anchor].getStartTime();
    double toTime     = notes[toIndex].getStartTime();
    double tMin = qMin(anchorTime, toTime);
    double tMax = qMax(anchorTime, toTime);

    selectedNoteIndices.clear();
    for (int i = 0; i < notes.size(); ++i) {
        double t = notes[i].getStartTime();
        if (t >= tMin - 1e-9 && t <= tMax + 1e-9)
            selectedNoteIndices.append(i);
    }
    // Keep anchor unchanged so repeated Shift+Clicks extend from the same anchor
    emit noteSelectionChanged();
}

void ScoreCanvas::selectNotes(const QVector<int> &indices)
{
    selectedNoteIndices = indices;
    emit noteSelectionChanged();
    update();
}

void ScoreCanvas::deselectAll()
{
    selectedNoteIndices.clear();
    emit noteSelectionChanged();
    update();
}

void ScoreCanvas::selectNextNote(int direction)
{
    const QVector<Note> &notes = phrase.getNotes();
    if (notes.isEmpty()) return;

    double refTime;
    if (direction > 0) {
        refTime = -1e9;
        for (int si : selectedNoteIndices) {
            if (si >= 0 && si < notes.size())
                refTime = qMax(refTime, notes[si].getStartTime());
        }
        int best = -1;
        double bestTime = 1e9;
        for (int i = 0; i < notes.size(); ++i) {
            double t = notes[i].getStartTime();
            if (t > refTime && t < bestTime) { bestTime = t; best = i; }
        }
        if (best >= 0) { selectNote(best, false); update(); }
    } else {
        refTime = 1e9;
        for (int si : selectedNoteIndices) {
            if (si >= 0 && si < notes.size())
                refTime = qMin(refTime, notes[si].getStartTime());
        }
        int best = -1;
        double bestTime = -1e9;
        for (int i = 0; i < notes.size(); ++i) {
            double t = notes[i].getStartTime();
            if (t < refTime && t > bestTime) { bestTime = t; best = i; }
        }
        if (best >= 0) { selectNote(best, false); update(); }
    }
}

bool ScoreCanvas::isNoteSelected(int index) const
{
    return selectedNoteIndices.contains(index);
}

// ============================================================================
// Drag Helpers
// ============================================================================

QRect ScoreCanvas::getNoteRect(const Note &note) const
{
    int x = timeToPixel(note.getStartTime());
    int width = timeToPixel(note.getEndTime()) - x;

    // Find maximum active curve value for blob height
    double maxDynamics = 0.0;
    for (int i = 0; i <= 20; ++i) {
        double t = i / 20.0;
        int activeIdx = resolveActiveCurveIndex(note);
        double value = (activeIdx <= 0)
            ? note.getDynamicsAt(t)
            : note.getExpressiveCurve(activeIdx).valueAt(t);
        if (value > maxDynamics) maxDynamics = value;
    }

    int blobHeight = static_cast<int>(20 + maxDynamics * 60);

    // Find min and max pitch across the note (for glissando notes)
    double minPitch = note.getPitchAt(0.0);
    double maxPitch = minPitch;
    for (int i = 0; i <= 20; ++i) {
        double t = i / 20.0;
        double pitch = note.getPitchAt(t);
        if (pitch < minPitch) minPitch = pitch;
        if (pitch > maxPitch) maxPitch = pitch;
    }

    // Calculate vertical bounds
    int topY = frequencyToPixel(maxPitch) - blobHeight/2 - 5;
    int bottomY = frequencyToPixel(minPitch) + blobHeight/2 + 5;
    int height = bottomY - topY;

    return QRect(x - 5, topY, width + 10, height);
}

QRect ScoreCanvas::getLeftResizeHandle(const Note &note) const
{
    QRect noteRect = getNoteRect(note);
    int handleSize = 16;
    // Center the handle vertically on the left edge
    return QRect(noteRect.left() - handleSize/2,
                 noteRect.center().y() - handleSize/2,
                 handleSize, handleSize);
}

QRect ScoreCanvas::getRightResizeHandle(const Note &note) const
{
    QRect noteRect = getNoteRect(note);
    int handleSize = 16;
    // Center the handle vertically on the right edge
    return QRect(noteRect.right() - handleSize/2,
                 noteRect.center().y() - handleSize/2,
                 handleSize, handleSize);
}

int ScoreCanvas::calculateCurveDotCount(const Note &note) const
{
    int width = timeToPixel(note.getEndTime()) - timeToPixel(note.getStartTime());

    // Adaptive dot count based on note width
    // One dot every ~50 pixels, minimum 4, no upper limit
    int dotCount = std::max(4, width / 50);

    return dotCount;
}

// ============================================================================
// Curve Lanes (see Claude-curve-lanes.md)
// ============================================================================

Track *ScoreCanvas::trackForNote(const Note &note) const
{
    return (note.getTrackIndex() == activeTrackIndex) ? m_currentTrack : nullptr;
}

const CurveLanes::Context &ScoreCanvas::laneContextFor(int variationIndex) const
{
    auto it = m_laneContextCache.find(variationIndex);
    if (it == m_laneContextCache.end())
        it = m_laneContextCache.insert(variationIndex,
                                       CurveLanes::makeContext(m_currentTrack, variationIndex));
    return it.value();
}

void ScoreCanvas::invalidateLaneCache()
{
    m_laneContextCache.clear();
}

void ScoreCanvas::setLanesAbove(int count)
{
    const int clamped = (count < 0) ? -1 : count;
    if (m_lanesAboveOverride == clamped) return;
    m_lanesAboveOverride = clamped;
    update();
    emit lanesAboveChanged(m_lanesAboveOverride);
}

QVector<CurveLanes::Lane> ScoreCanvas::resolveLanesFor(const Note &note) const
{
    if (!trackForNote(note)) return {};
    return CurveLanes::resolve(note, laneContextFor(note.getVariationIndex()));
}

int ScoreCanvas::lanesAboveCount(int laneCount) const
{
    if (laneCount <= 0) return 0;
    if (m_lanesAboveOverride >= 0)
        return std::min(m_lanesAboveOverride, laneCount);
    // Balanced split, odd counts favouring the top.
    return (laneCount + 1) / 2;
}

void ScoreCanvas::setDimOthers(bool dim)
{
    if (m_dimOthers == dim) return;
    m_dimOthers = dim;
    update();
}

QVector<CurveLanes::Lane> ScoreCanvas::resolveLanesForNote(int noteIndex) const
{
    const QVector<Note> &notes = phrase.getNotes();
    if (noteIndex < 0 || noteIndex >= notes.size()) return {};
    return resolveLanesFor(notes[noteIndex]);
}

int ScoreCanvas::lanesAboveForNote(int noteIndex) const
{
    return lanesAboveCount(resolveLanesForNote(noteIndex).size());
}

void ScoreCanvas::noteSpanX(const Note &note, int &x, int &width) const
{
    x = timeToPixel(note.getStartTime());
    width = std::max(1, timeToPixel(note.getEndTime()) - x);
}

int ScoreCanvas::expandedLaneFor(int noteIndex) const
{
    return (noteIndex >= 0 && noteIndex == m_editLaneNote) ? m_editLaneIndex : -1;
}

QRect ScoreCanvas::laneRect(const Note &note, int laneIndex, int laneCount,
                            int expandedLane) const
{
    const QRect noteRect = getNoteRect(note);
    int x = 0, w = 0;
    noteSpanX(note, x, w);
    const int above = lanesAboveCount(laneCount);

    // The opened lane is taller, so offsets accumulate rather than stride.
    auto heightOf = [expandedLane](int idx) {
        return idx == expandedLane ? EDIT_LANE_HEIGHT : LANE_HEIGHT;
    };

    if (laneIndex < above) {
        // slot 0 hugs the hull top; later slots stack upward
        int offset = LANE_GAP;
        for (int k = 0; k < laneIndex; ++k) offset += heightOf(k) + LANE_GAP;
        return QRect(x, noteRect.top() - offset - heightOf(laneIndex),
                     w, heightOf(laneIndex));
    }

    // slot 0 hugs the hull bottom; later slots stack downward
    int offset = LANE_GAP;
    for (int k = above; k < laneIndex; ++k) offset += heightOf(k) + LANE_GAP;
    return QRect(x, noteRect.bottom() + offset, w, heightOf(laneIndex));
}

void ScoreCanvas::openLaneEditor(int noteIndex, int laneIndex)
{
    if (m_editLaneNote == noteIndex && m_editLaneIndex == laneIndex) return;
    m_editLaneNote = noteIndex;
    m_editLaneIndex = laneIndex;
    // Selecting the note keeps it promoted to Lanes (D3) and gives the undo
    // command the single-selection context it expects.
    selectNote(noteIndex, false);
    update();
}

void ScoreCanvas::createCurveOnLane(int noteIndex, int laneIndex)
{
    const QVector<Note> &notes = phrase.getNotes();
    if (noteIndex < 0 || noteIndex >= notes.size()) return;

    const QVector<CurveLanes::Lane> lanes = resolveLanesFor(notes[noteIndex]);
    if (laneIndex < 0 || laneIndex >= lanes.size()) return;
    const CurveLanes::Lane &lane = lanes[laneIndex];

    // The EQ pseudo-lane stands for ten band curves at once - there is no
    // single curve to create, so it keeps going through the EQ dialog.
    if (lane.eqCollapsed) return;

    // Start flat at the row's resting value: creating a curve must not change
    // how the note sounds until it is actually shaped. (It does change what
    // the bake sends - a curve-carrying row streams, a no-curve row sends
    // nothing - but streaming a constant neutral is audibly identical.)
    const double v = qBound(0.0, lane.neutral / 127.0, 1.0);
    QVector<EnvelopePoint> flat;
    flat.append(EnvelopePoint(0.0, v));
    flat.append(EnvelopePoint(1.0, v));

    undoStack->push(new ApplyExpressiveCurveToSelectionCommand(
        &phrase, QVector<int>{noteIndex}, lane.name, flat, 1.0, false, this));

    openLaneEditor(noteIndex, laneIndex);
    emit expressiveCurveApplied(lane.name);
}

QVector<EnvelopeCurveCanvas::Ghost> ScoreCanvas::buildGhostCurves(
    const Note &note, const QString &excludeName) const
{
    QVector<EnvelopeCurveCanvas::Ghost> ghosts;
    const int kSamples = 48;
    const int kMaxGhosts = 8;

    // The editor canvas is near-black, so these are the dark-surface steps of
    // the family palette - the light-canvas ones sit too dark to read there.
    auto ghostColor = [](CurveLanes::Family f) -> QColor {
        switch (f) {
        case CurveLanes::Family::Amplitude: return QColor(0x39, 0x87, 0xe5);
        case CurveLanes::Family::Element:   return QColor(0xd9, 0x59, 0x26);
        case CurveLanes::Family::Part:      return QColor(0x19, 0x9e, 0x70);
        case CurveLanes::Family::Other:     break;
        }
        return QColor(0xc3, 0xc2, 0xb7);
    };

    auto addGhost = [&](const QString &name, const Curve &curve, const QColor &color) {
        if (ghosts.size() >= kMaxGhosts) return;
        EnvelopeCurveCanvas::Ghost g;
        g.name = name;
        g.color = color;
        g.samples.reserve(kSamples + 1);
        for (int i = 0; i <= kSamples; ++i) {
            const double t = static_cast<double>(i) / kSamples;
            g.samples.append(QPointF(t, qBound(0.0, curve.valueAt(t), 1.0)));
        }
        ghosts.append(g);
    };

    if (excludeName != QStringLiteral("Dynamics"))
        addGhost(QStringLiteral("Dynamics"), note.getDynamicsCurve(),
                 QColor(0xc3, 0xc2, 0xb7));

    for (int i = 1; i < note.getExpressiveCurveCount(); ++i) {
        const QString name = note.getExpressiveCurveName(i);
        if (name.isEmpty() || name == excludeName) continue;
        addGhost(name, note.getExpressiveCurve(i),
                 ghostColor(CurveLanes::familyFor(name)));
    }

    return ghosts;
}

void ScoreCanvas::editLaneCurveDialog(int noteIndex, int laneIndex)
{
    const QVector<Note> &notes = phrase.getNotes();
    if (noteIndex < 0 || noteIndex >= notes.size()) return;

    const QVector<CurveLanes::Lane> lanes = resolveLanesFor(notes[noteIndex]);
    if (laneIndex < 0 || laneIndex >= lanes.size()) return;
    const CurveLanes::Lane &lane = lanes[laneIndex];
    if (lane.eqCollapsed || lane.state == CurveLanes::State::GraphDriven) return;

    // An empty lane becomes a flat-at-neutral curve first, exactly like a
    // single click would - the dialog then edits it.
    if (lane.state == CurveLanes::State::Empty)
        createCurveOnLane(noteIndex, laneIndex);

    selectNote(noteIndex, false);   // dialog applies to the selection

    const QStringList curveNames = selectionCommonExpressiveCurveNames();
    if (curveNames.isEmpty()) return;

    const Note &note = phrase.getNotes()[noteIndex];
    const double duration = note.getDuration();
    ExpressiveCurveApplyDialog dialog(selectedNoteIndices.size(), duration, curveNames, this);
    dialog.setSelectedCurveName(lane.name);

    auto curveToEnvelopePoints = [](const Curve &curve) -> QVector<EnvelopePoint> {
        QVector<EnvelopePoint> pts;
        for (const Curve::Point &p : curve.getPoints())
            pts.append(EnvelopePoint(p.time, p.value, 0));
        if (pts.isEmpty()) {
            pts.append(EnvelopePoint(0.0, 0.8, 0));
            pts.append(EnvelopePoint(1.0, 0.8, 0));
        }
        return pts;
    };
    const int ci = note.findExpressiveCurveIndexByName(lane.name);
    if (ci >= 0) {
        const Curve &existing = note.getExpressiveCurve(ci);
        const QVector<Curve::Point> &pts = existing.getPoints();
        // A flat curve is unshaped - either the auto-created neutral line (a
        // click on an empty lane; note the apply command re-samples it to 30
        // points, so point count cannot be the test) or a deliberately flat
        // one. Either way there is no SHAPE to preload, and preloading it puts
        // a line glued to one edge in place of the dialog's normal default
        // preset, which reads as "my curve is gone". Leave the dialog on its
        // default instead.
        double lo = 0.0, hi = 0.0;
        if (!pts.isEmpty()) {
            lo = hi = pts[0].value;
            for (const Curve::Point &p : pts) {
                lo = std::min(lo, p.value);
                hi = std::max(hi, p.value);
            }
        }
        if (hi - lo >= 1e-9)
            dialog.setInitialCurve(curveToEnvelopePoints(existing));
    }
    dialog.setGhostCurves(buildGhostCurves(note, QString()));

    if (dialog.exec() == QDialog::Accepted) {
        const QString targetName = dialog.getSelectedCurveName();
        undoStack->push(new ApplyExpressiveCurveToSelectionCommand(
            &phrase, selectedNoteIndices, targetName, dialog.getCurve(),
            dialog.getWeight(), dialog.getPerNoteMode(), this));
        // No auto-activation of the note body - the "Show curve" combo is the
        // only thing that changes which curve shapes the note.
        emit expressiveCurveApplied(targetName);
    }
}

void ScoreCanvas::closeLaneEditor()
{
    if (m_editLaneNote < 0 && m_editLaneIndex < 0) return;
    m_editLaneNote = -1;
    m_editLaneIndex = -1;
    update();
}

QVector<QRect> ScoreCanvas::laneDotRects(const Note &note, const Curve &curve,
                                         const QRect &r) const
{
    QVector<QRect> dots;
    const int numDots = calculateCurveDotCount(note);
    const double span = r.height() - 1;
    const int size = 9;
    for (int i = 1; i <= numDots; ++i) {
        const double t = static_cast<double>(i) / (numDots + 1);
        const double v = qBound(0.0, curve.valueAt(t), 1.0);
        const int cx = r.left() + static_cast<int>(t * r.width());
        const int cy = r.bottom() - static_cast<int>(v * span);
        dots.append(QRect(cx - size / 2, cy - size / 2, size, size));
    }
    return dots;
}

int ScoreCanvas::findLaneDotAt(const QPoint &pos, const Note &note, const QRect &r) const
{
    const QVector<CurveLanes::Lane> lanes = resolveLanesFor(note);
    if (m_editLaneIndex < 0 || m_editLaneIndex >= lanes.size()) return -1;
    const CurveLanes::Lane &lane = lanes[m_editLaneIndex];
    if (lane.curveIndex < 0) return -1;

    const QVector<QRect> dots =
        laneDotRects(note, note.getExpressiveCurve(lane.curveIndex), r);
    const int tol = 5;
    for (int i = 0; i < dots.size(); ++i)
        if (dots[i].adjusted(-tol, -tol, tol, tol).contains(pos)) return i;
    return -1;
}

void ScoreCanvas::drawLaneHull(QPainter &painter, const Note &note,
                               const CurveLanes::Lane &lane, const QRect &r)
{
    painter.save();

    // Deliberately not the note hull's grey: the two hulls are different
    // things and must never be mistaken for each other.
    const QColor accent(0x2a, 0x78, 0xd6);
    painter.setPen(QPen(accent, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(r.adjusted(-2, -2, 1, 1));

    // Top-edge dots only - a lane has no pitch, so no bottom dots (D7).
    if (lane.curveIndex >= 0) {
        const QVector<QRect> dots =
            laneDotRects(note, note.getExpressiveCurve(lane.curveIndex), r);
        painter.setPen(QPen(accent.darker(130), 1));
        painter.setBrush(QColor(255, 255, 255));
        for (const QRect &d : dots) painter.drawEllipse(d);
    }

    // The parameter name, written under the hull - Nimus's original request.
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    painter.setPen(QColor(0x52, 0x51, 0x4e));
    painter.drawText(QRect(r.left(), r.bottom() + 4, std::max(r.width(), 140), 14),
                     Qt::AlignLeft | Qt::AlignVCenter, lane.name);

    painter.restore();
}

// Sample a curve into a polyline across a lane / band rect.
static QPolygonF curvePolyline(const Curve &curve, const QRect &r)
{
    const int n = std::max(6, r.width() / 5);
    const double span = r.height() - 1;
    QPolygonF poly;
    poly.reserve(n + 1);
    for (int i = 0; i <= n; ++i) {
        const double t = static_cast<double>(i) / n;
        const double v = qBound(0.0, curve.valueAt(t), 1.0);
        poly.append(QPointF(r.left() + t * r.width(), r.bottom() - v * span));
    }
    return poly;
}

void ScoreCanvas::drawSingleLane(QPainter &painter, const Note &note,
                                 const CurveLanes::Lane &lane, const QRect &r)
{
    if (r.width() < 2) return;
    const QColor family = CurveLanes::familyColor(lane.family);
    const double span = r.height() - 1;

    switch (lane.state) {
    case CurveLanes::State::Curve: {
        // A light fill under the curve so the lane reads as "occupied" at a
        // glance; the line carries the shape.
        const QPolygonF line = curvePolyline(note.getExpressiveCurve(lane.curveIndex), r);
        QPolygonF fill = line;
        fill.append(QPointF(r.right(), r.bottom()));
        fill.append(QPointF(r.left(),  r.bottom()));

        QColor fillColor = family;
        fillColor.setAlpha(38);
        painter.setPen(Qt::NoPen);
        painter.setBrush(fillColor);
        painter.drawPolygon(fill);

        painter.setPen(QPen(family, 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawPolyline(line);
        break;
    }

    case CurveLanes::State::Empty: {
        // Flat line at the row's resting value - where this parameter sits
        // when nothing is drawn on it.
        const double v = qBound(0.0, lane.neutral / 127.0, 1.0);
        const int y = r.bottom() - static_cast<int>(v * span);
        QColor c = family;
        c.setAlpha(60);
        painter.setPen(QPen(c, 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawLine(r.left(), y, r.right(), y);
        break;
    }

    case CurveLanes::State::Orphan: {
        // Stored on the note but NOT baked - its row is gone. Deliberately
        // does not wear the family colour at strength, because wearing it
        // would imply this is being sent.
        const QPolygonF line = curvePolyline(note.getExpressiveCurve(lane.curveIndex), r);
        painter.setPen(QPen(QColor(0x89, 0x87, 0x81, 90), 1, Qt::DotLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(r.adjusted(0, 0, -1, -1));
        painter.setPen(QPen(QColor(0x89, 0x87, 0x81), 1.0, Qt::DashLine));
        painter.drawPolyline(line);
        break;
    }

    case CurveLanes::State::GraphDriven: {
        // A Modifier connection drives this row, so the bake ignores any drawn
        // curve here. Must read as not-yours-to-draw rather than merely empty.
        QColor c = family;
        c.setAlpha(90);
        const int y = r.center().y();
        painter.setPen(QPen(c, 1, Qt::DotLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawLine(r.left() + 12, y, r.right(), y);

        // Small link glyph at the left edge: two bars joined by a bridge.
        painter.setPen(QPen(c, 2));
        const int gx = r.left() + 3;
        painter.drawLine(gx,     y - 3, gx + 5, y - 3);
        painter.drawLine(gx,     y + 3, gx + 5, y + 3);
        painter.drawLine(gx + 2, y - 3, gx + 2, y + 3);
        break;
    }
    }
}

void ScoreCanvas::drawNoteLanes(QPainter &painter, int noteIndex, const Note &note)
{
    const QVector<CurveLanes::Lane> lanes = resolveLanesFor(note);
    if (lanes.isEmpty()) return;
    const int expanded = expandedLaneFor(noteIndex);

    painter.save();
    for (int i = 0; i < lanes.size(); ++i) {
        const QRect r = laneRect(note, i, lanes.size(), expanded);
        drawSingleLane(painter, note, lanes[i], r);
        if (i == expanded)
            drawLaneHull(painter, note, lanes[i], r);
    }
    painter.restore();
}

int ScoreCanvas::ribbonExtentAbove(int laneCount) const
{
    const int above = lanesAboveCount(laneCount);
    if (above <= 0) return 0;
    return LANE_GAP + above * LANE_HEIGHT + (above - 1) * LANE_GAP;
}

int ScoreCanvas::ribbonExtentBelow(int laneCount) const
{
    const int below = laneCount - lanesAboveCount(laneCount);
    if (below <= 0) return 0;
    return LANE_GAP + below * LANE_HEIGHT + (below - 1) * LANE_GAP;
}

bool ScoreCanvas::hasRoomForLanes(int noteIndex, const Note &note, int laneCount) const
{
    if (laneCount <= 0) return false;

    const QRect myRect = getNoteRect(note);
    const int needAbove = ribbonExtentAbove(laneCount);
    const int needBelow = ribbonExtentBelow(laneCount);

    const QVector<Note> &notes = phrase.getNotes();
    for (int i = 0; i < notes.size(); ++i) {
        if (i == noteIndex) continue;
        const Note &other = notes[i];
        if (!trackForNote(other)) continue;

        // Only notes overlapping in time can collide vertically.
        if (other.getEndTime() <= note.getStartTime() ||
            other.getStartTime() >= note.getEndTime()) continue;

        const QRect otherRect = getNoteRect(other);
        if (otherRect.top() > myRect.bottom()) {
            if (otherRect.top() - myRect.bottom() < needBelow) return false;
        } else if (otherRect.bottom() < myRect.top()) {
            if (myRect.top() - otherRect.bottom() < needAbove) return false;
        } else {
            return false;   // hulls already overlap vertically
        }
    }
    return true;
}

ScoreCanvas::DensityTier ScoreCanvas::effectiveTierForNote(int noteIndex, const Note &note) const
{
    if (m_densityTier == DensityTier::Off) return DensityTier::Off;

    // D3: the selected note is always the detailed one, and is never demoted -
    // that is what makes chords workable without a separate mode.
    if (isNoteSelected(noteIndex)) return DensityTier::Lanes;

    if (m_densityTier == DensityTier::Overlay) return DensityTier::Overlay;

    const QVector<CurveLanes::Lane> lanes = resolveLanesFor(note);
    if (lanes.isEmpty()) return DensityTier::Off;
    return hasRoomForLanes(noteIndex, note, lanes.size())
         ? DensityTier::Lanes : DensityTier::Overlay;
}

void ScoreCanvas::setDensityTier(DensityTier tier)
{
    if (m_densityTier == tier) return;
    m_densityTier = tier;
    m_hoverNote = -1;
    m_hoverLane = -1;
    update();
    emit densityTierChanged(tier);
}

QRect ScoreCanvas::overlayBandRect(const Note &note) const
{
    const QRect noteRect = getNoteRect(note);
    int x = 0, w = 0;
    noteSpanX(note, x, w);
    return QRect(x, noteRect.top() - LANE_GAP - OVERLAY_HEIGHT, w, OVERLAY_HEIGHT);
}

// Lanes that actually have a shape to draw in the Overlay band.
// D4: Overlay reports what the bake will SEND. Empty rows would be identical
// flat lines at the same neutral (one line, no information), graph-driven rows
// have no note curve at all, and an orphan is stored but never sent - drawing
// it here would report the note as busier than it really is.
static QVector<const CurveLanes::Lane *> overlayDrawableLanes(
        const QVector<CurveLanes::Lane> &lanes)
{
    QVector<const CurveLanes::Lane *> drawable;
    for (const CurveLanes::Lane &l : lanes)
        if (l.state == CurveLanes::State::Curve && l.curveIndex >= 0)
            drawable.append(&l);
    return drawable;
}

void ScoreCanvas::drawNoteOverlay(QPainter &painter, const Note &note)
{
    const QVector<CurveLanes::Lane> lanes = resolveLanesFor(note);
    const QVector<const CurveLanes::Lane *> drawable = overlayDrawableLanes(lanes);
    if (drawable.isEmpty()) return;

    const QRect band = overlayBandRect(note);
    if (band.width() < 2) return;

    painter.save();

    // Faint baseline so the band reads as a band even when its curves hug
    // the bottom.
    painter.setPen(QPen(QColor(0xe1, 0xe0, 0xd9), 1));
    painter.drawLine(band.bottomLeft(), band.bottomRight());

    const int numPoints = std::max(10, band.width() / 5);
    const double span = band.height() - 1;

    for (const CurveLanes::Lane *lane : drawable) {
        const Curve &curve = note.getExpressiveCurve(lane->curveIndex);
        QPolygonF poly;
        poly.reserve(numPoints + 1);
        for (int i = 0; i <= numPoints; ++i) {
            const double t = static_cast<double>(i) / numPoints;
            const double v = qBound(0.0, curve.valueAt(t), 1.0);
            poly.append(QPointF(band.left() + t * band.width(),
                                band.bottom() - v * span));
        }
        painter.setPen(QPen(CurveLanes::familyColor(lane->family), 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawPolyline(poly);
    }

    painter.restore();
}

int ScoreCanvas::findOverlayBandAtPosition(const QPoint &pos) const
{
    const QVector<Note> &notes = phrase.getNotes();
    // Reverse order so the newest note wins, matching findNoteAtPosition.
    for (int i = notes.size() - 1; i >= 0; --i) {
        if (!trackForNote(notes[i])) continue;
        if (overlayBandRect(notes[i]).contains(pos)) return i;
    }
    return -1;
}

bool ScoreCanvas::findLaneAtPosition(const QPoint &pos, int &noteIndex, int &laneIndex) const
{
    const QVector<Note> &notes = phrase.getNotes();
    // Reverse order so the newest note wins, matching findNoteAtPosition.
    for (int i = notes.size() - 1; i >= 0; --i) {
        if (!trackForNote(notes[i])) continue;
        if (effectiveTierForNote(i, notes[i]) != DensityTier::Lanes) continue;
        const QVector<CurveLanes::Lane> lanes = resolveLanesFor(notes[i]);
        const int expanded = expandedLaneFor(i);
        for (int l = 0; l < lanes.size(); ++l) {
            if (laneRect(notes[i], l, lanes.size(), expanded).contains(pos)) {
                noteIndex = i;
                laneIndex = l;
                return true;
            }
        }
    }
    return false;
}

void ScoreCanvas::updateLaneHover(const QPoint &pos)
{
    int note = -1, lane = -1;
    if (!findLaneAtPosition(pos, note, lane)) {
        note = findOverlayBandAtPosition(pos);
        lane = -1;
    }
    if (note != m_hoverNote || lane != m_hoverLane) {
        m_hoverNote = note;
        m_hoverLane = lane;
        update();
    }
}

void ScoreCanvas::drawHoverLabel(QPainter &painter)
{
    if (m_hoverNote < 0) return;
    const QVector<Note> &notes = phrase.getNotes();
    if (m_hoverNote >= notes.size()) return;

    const Note &note = notes[m_hoverNote];
    const QVector<CurveLanes::Lane> allLanes = resolveLanesFor(note);

    // Hovering one lane names that lane (with its state, since "why is this
    // one greyed out" is the question a single lane raises); hovering an
    // overlay band names everything in it.
    QVector<const CurveLanes::Lane *> drawable;
    QRect anchor;
    if (m_hoverLane >= 0) {
        if (m_hoverLane >= allLanes.size()) return;
        drawable.append(&allLanes[m_hoverLane]);
        anchor = laneRect(note, m_hoverLane, allLanes.size(), expandedLaneFor(m_hoverNote));
    } else {
        drawable = overlayDrawableLanes(allLanes);
        anchor = overlayBandRect(note);
    }
    if (drawable.isEmpty()) return;

    painter.save();
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    const QFontMetrics fm(font);

    const int swatch = 8, padX = 6, padY = 4, gap = 5;
    const int lineH = fm.height() + 2;

    // A lane's state is part of its identity - an orphan or a graph-driven row
    // looks "off" and the label is where that gets explained.
    auto labelFor = [](const CurveLanes::Lane *l) {
        switch (l->state) {
        case CurveLanes::State::Empty:       return l->name + QStringLiteral("  — no curve");
        case CurveLanes::State::Orphan:      return l->name + QStringLiteral("  — orphan, not sent");
        case CurveLanes::State::GraphDriven: return l->name + QStringLiteral("  — driven by graph");
        case CurveLanes::State::Curve:       break;
        }
        return l->name;
    };

    int textW = 0;
    for (const CurveLanes::Lane *l : drawable)
        textW = std::max(textW, fm.horizontalAdvance(labelFor(l)));

    const int w = padX * 2 + swatch + gap + textW;
    const int h = padY * 2 + lineH * drawable.size();

    int x = anchor.left();
    int y = anchor.top() - h - 4;
    if (y < 0) y = anchor.bottom() + 4;               // flip below rather than clip
    if (x + w > width()) x = width() - w - 2;
    if (x < 0) x = 0;

    const QRect panel(x, y, w, h);
    painter.setPen(QPen(QColor(11, 11, 11, 26), 1));  // hairline ring
    painter.setBrush(QColor(0xfc, 0xfc, 0xfb));       // chart surface
    painter.drawRoundedRect(panel, 3, 3);

    int ty = panel.top() + padY;
    for (const CurveLanes::Lane *l : drawable) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(CurveLanes::familyColor(l->family));
        painter.drawRect(QRect(panel.left() + padX,
                               ty + (lineH - swatch) / 2, swatch, swatch));
        // Text wears ink, never the series colour.
        painter.setPen(QColor(0x52, 0x51, 0x4e));
        painter.drawText(QRect(panel.left() + padX + swatch + gap, ty, textW, lineH),
                         Qt::AlignVCenter | Qt::AlignLeft, labelFor(l));
        ty += lineH;
    }

    painter.restore();
}

Curve ScoreCanvas::quantizePitchCurveToScale(const Curve &pitchCurve, const Note &note) const
{
    // Quantize a continuous pitch curve to snap to scale degrees
    // Uses hysteresis: only switches to a new pitch when crossing halfway between scale degrees

    // Generate scale lines for the time when this note starts
    double noteStartTime = note.getStartTime();
    Scale activeScale = getScaleAtTime(noteStartTime);
    double activeBaseFreq = getBaseFrequencyAtTime(noteStartTime);
    QVector<ScaleLine> timeScaleLines = generateScaleLinesForScale(activeScale, activeBaseFreq);

    if (pitchCurve.isEmpty() || timeScaleLines.isEmpty()) {
        return pitchCurve;
    }

    Curve quantizedCurve;

    // Sample the curve at many points to detect all transitions
    const int sampleCount = 500;  // High resolution for accurate transition detection
    double currentScalePitch = 0.0;  // Track the current snapped pitch
    double previousTime = 0.0;

    for (int i = 0; i < sampleCount; ++i) {
        double t = i / static_cast<double>(sampleCount - 1);  // Normalized time 0.0-1.0
        double rawPitch = pitchCurve.valueAt(t);

        if (i == 0) {
            // Initialize to nearest scale line at note's time position
            currentScalePitch = snapToNearestScaleLineAtTime(rawPitch, noteStartTime);
            quantizedCurve.addPoint(0.0, currentScalePitch);
            previousTime = 0.0;
            continue;
        }

        // Find the two nearest scale lines to the current snapped pitch
        double lowerScalePitch = 0.0;
        double upperScalePitch = 0.0;
        bool foundLower = false;
        bool foundUpper = false;

        for (const ScaleLine &line : timeScaleLines) {
            if (line.frequencyHz < currentScalePitch) {
                if (!foundLower || line.frequencyHz > lowerScalePitch) {
                    lowerScalePitch = line.frequencyHz;
                    foundLower = true;
                }
            } else if (line.frequencyHz > currentScalePitch) {
                if (!foundUpper || line.frequencyHz < upperScalePitch) {
                    upperScalePitch = line.frequencyHz;
                    foundUpper = true;
                }
            }
        }

        // Calculate threshold for switching (halfway point between current and adjacent scale lines)
        bool shouldSwitch = false;
        double newScalePitch = currentScalePitch;

        if (rawPitch > currentScalePitch && foundUpper) {
            // Moving upward - check if we've crossed halfway to upper scale line
            double threshold = (currentScalePitch + upperScalePitch) / 2.0;
            if (rawPitch > threshold) {
                shouldSwitch = true;
                newScalePitch = upperScalePitch;
            }
        } else if (rawPitch < currentScalePitch && foundLower) {
            // Moving downward - check if we've crossed halfway to lower scale line
            double threshold = (currentScalePitch + lowerScalePitch) / 2.0;
            if (rawPitch < threshold) {
                shouldSwitch = true;
                newScalePitch = lowerScalePitch;
            }
        }

        // When switching to a new pitch, create a clean step transition
        if (shouldSwitch) {
            // Add a point just before the transition at the old pitch (creates horizontal line)
            double transitionTime = (previousTime + t) / 2.0;  // Midpoint for clean transition
            quantizedCurve.addPoint(transitionTime - 0.001, currentScalePitch);
            // Add a point at the transition at the new pitch (creates vertical step)
            quantizedCurve.addPoint(transitionTime, newScalePitch);

            currentScalePitch = newScalePitch;
        }

        previousTime = t;
    }

    // Ensure curve ends at 1.0 with the final pitch
    quantizedCurve.addPoint(1.0, currentScalePitch);

    qDebug() << "Quantized curve points:" << quantizedCurve.getPointCount();

    return quantizedCurve;
}

ScoreCanvas::DragMode ScoreCanvas::detectDragMode(const QPoint &pos, const Note &note) const
{
    QRect noteRect = getNoteRect(note);
    int dotSize = 8;
    int clickTolerance = 12;  // Larger tolerance for easier clicking

    // Transform mode: hit-test the 8 perimeter handles on the frozen rect.
    if (transformMode) {
        const int handleTol = 14;  // Larger than per-point click tolerance for these
        int leftX  = transformModeRect.left();
        int rightX = transformModeRect.right();
        int topYr  = transformModeRect.top();
        int botYr  = transformModeRect.bottom();
        int midX   = transformModeRect.left() + transformModeRect.width() / 2;
        int midY   = transformModeRect.top() + transformModeRect.height() / 2;

        auto hits = [&](int cx, int cy) {
            return QRect(cx - handleTol/2, cy - handleTol/2, handleTol, handleTol).contains(pos);
        };

        // Edge midpoints — vertical-stretch (top/bottom) or time-stretch via existing resize (left/right)
        if (hits(midX, topYr)) return TransformStretchTop;
        if (hits(midX, botYr)) return TransformStretchBottom;
        if (hits(leftX,  midY)) return ResizingLeft;
        if (hits(rightX, midY)) return ResizingRight;

        // Corners — anchored rotation: each corner's pivot is the opposite corner so the
        // grabbed corner moves and the other side stays put.
        auto setAnchor = [this](int ax, int ay) {
            const_cast<ScoreCanvas*>(this)->transformRotateAnchor = QPointF(ax, ay);
        };
        if (hits(leftX,  topYr)) { setAnchor(rightX, botYr); return TransformRotate; }
        if (hits(rightX, topYr)) { setAnchor(leftX,  botYr); return TransformRotate; }
        if (hits(leftX,  botYr)) { setAnchor(rightX, topYr); return TransformRotate; }
        if (hits(rightX, botYr)) { setAnchor(leftX,  topYr); return TransformRotate; }

        // Anywhere else inside transform mode: do nothing (no per-point edits, no note drag)
        return NoDrag;
    }

    int numDots = calculateCurveDotCount(note);
    int numSegments = numDots + 1;  // e.g., 4 dots = 5 segments

    // Check top edge dots (for editing dynamics curve)
    int topY = noteRect.top();
    for (int i = 1; i <= numDots; ++i) {
        int dotX = noteRect.left() + (noteRect.width() * i / numSegments);
        QRect dotRect(dotX - clickTolerance/2, topY - clickTolerance/2,
                     clickTolerance, clickTolerance);
        if (dotRect.contains(pos)) {
            // Store which dot was clicked (will be used in press event)
            const_cast<ScoreCanvas*>(this)->editingDotIndex = i - 1;  // 0-based index
            const_cast<ScoreCanvas*>(this)->editingDotTimePos = static_cast<double>(i) / numSegments;  // Normalized time
            return EditingTopCurve;
        }
    }

    // Check bottom edge dots (for editing pitch curve)
    int bottomY = noteRect.bottom();
    for (int i = 1; i <= numDots; ++i) {
        int dotX = noteRect.left() + (noteRect.width() * i / numSegments);
        QRect dotRect(dotX - clickTolerance/2, bottomY - clickTolerance/2,
                     clickTolerance, clickTolerance);
        if (dotRect.contains(pos)) {
            const_cast<ScoreCanvas*>(this)->editingDotIndex = i - 1;
            const_cast<ScoreCanvas*>(this)->editingDotTimePos = static_cast<double>(i) / numSegments;
            return EditingPitchCurve;
        }
    }

    // Check left edge top dot (for editing dynamics curve start value)
    int leftX = noteRect.left();
    int leftDotY = noteRect.top() + (noteRect.height() / 4);
    QRect leftDotRect(leftX - clickTolerance/2, leftDotY - clickTolerance/2,
                     clickTolerance, clickTolerance);
    if (leftDotRect.contains(pos)) {
        const_cast<ScoreCanvas*>(this)->editingDotTimePos = 0.0;  // Start of curve
        return EditingTopCurveStart;
    }

    // Check right edge top dot (for editing dynamics curve end value)
    int rightX = noteRect.right();
    int rightDotY = noteRect.top() + (noteRect.height() / 4);
    QRect rightDotRect(rightX - clickTolerance/2, rightDotY - clickTolerance/2,
                      clickTolerance, clickTolerance);
    if (rightDotRect.contains(pos)) {
        const_cast<ScoreCanvas*>(this)->editingDotTimePos = 1.0;  // End of curve
        return EditingTopCurveEnd;
    }

    // Check left edge bottom dot (for editing pitch curve start value)
    int leftPitchDotY = noteRect.top() + (noteRect.height() * 3 / 4);
    QRect leftPitchDotRect(leftX - clickTolerance/2, leftPitchDotY - clickTolerance/2,
                           clickTolerance, clickTolerance);
    if (leftPitchDotRect.contains(pos)) {
        const_cast<ScoreCanvas*>(this)->editingDotTimePos = 0.0;
        return EditingPitchCurveStart;
    }

    // Check right edge bottom dot (for editing pitch curve end value)
    int rightPitchDotY = noteRect.top() + (noteRect.height() * 3 / 4);
    QRect rightPitchDotRect(rightX - clickTolerance/2, rightPitchDotY - clickTolerance/2,
                            clickTolerance, clickTolerance);
    if (rightPitchDotRect.contains(pos)) {
        const_cast<ScoreCanvas*>(this)->editingDotTimePos = 1.0;
        return EditingPitchCurveEnd;
    }

    // Check resize handles (higher priority than note body)
    if (getLeftResizeHandle(note).contains(pos)) {
        return ResizingLeft;
    }
    if (getRightResizeHandle(note).contains(pos)) {
        return ResizingRight;
    }

    // Check if clicking on note body
    if (noteRect.contains(pos)) {
        return DraggingNote;
    }

    return NoDrag;
}

// ============================================================================
// Pen Tablet Input
// ============================================================================

void ScoreCanvas::tabletEvent(QTabletEvent *event)
{
    // Mark that we're using tablet input
    usingTablet = true;

    // Capture pen pressure (0.0-1.0)
    penPressure = event->pressure();

    // Get position
    QPointF pos = event->position();

    // Check if using eraser - delete notes on contact
    if (event->pointerType() == QPointingDevice::PointerType::Eraser) {
        if (event->type() == QEvent::TabletPress || event->type() == QEvent::TabletMove) {
            int noteIndex = findNoteAtPosition(pos.toPoint());
            if (noteIndex >= 0) {
                // Use undo command to delete the note under the eraser
                undoStack->push(new DeleteNoteCommand(&phrase, noteIndex, this));

                // Deselect if the deleted note was selected
                if (selectedNoteIndices.contains(noteIndex)) {
                    selectedNoteIndices.removeAll(noteIndex);
                }

                // Adjust selection indices if a note before them was deleted
                for (int &idx : selectedNoteIndices) {
                    if (idx > noteIndex) {
                        idx--;
                    }
                }

                update();
            }
        }
        event->accept();
        return;  // Don't process as normal pen input
    }

    switch (event->type()) {
    case QEvent::TabletPress: {
        // Ensure widget has keyboard focus for Delete key and other shortcuts
        setFocus();

        // Handle right-click from pen barrel button
        // Don't handle it here - ignore the tablet event and let it fall through
        // to the mouse event system which handles context menus properly
        if (event->button() == Qt::RightButton) {
            event->ignore();  // Let mouse event handle it
            return;
        }

        // Double-tap detection for pen (alternative to mouse double-click)
        // Check if this tap is close in time and position to the last tap
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        QPoint currentPos = pos.toPoint();
        bool isDoubleTap = false;

        if (lastTabletPressTime > 0) {
            qint64 timeDiff = currentTime - lastTabletPressTime;
            int distance = std::abs(currentPos.x() - lastTabletPressPos.x()) +
                          std::abs(currentPos.y() - lastTabletPressPos.y());

            if (timeDiff < DOUBLE_TAP_TIME_MS && distance < DOUBLE_TAP_DISTANCE) {
                isDoubleTap = true;
            }
        }

        // Update last tap info for next comparison
        lastTabletPressTime = currentTime;
        lastTabletPressPos = currentPos;

        // Handle double-tap: enter segment editing mode (same as mouseDoubleClickEvent)
        if (isDoubleTap) {
            // Check if in segment editing mode - double-tap exits
            if (segmentEditingMode) {
                exitSegmentEditingMode();
                event->accept();
                return;
            }

            int clickedNoteIndex = findNoteAtPosition(currentPos);
            if (clickedNoteIndex >= 0) {
                const Note &note = phrase.getNotes()[clickedNoteIndex];

                // Only enter segment mode for quantized continuous notes
                if (note.isQuantized() && note.hasPitchCurve()) {
                    enterSegmentEditingMode(clickedNoteIndex);

                    // Select the segment under cursor
                    int segIndex = findSegmentAtPosition(currentPos, clickedNoteIndex);
                    if (segIndex >= 0) {
                        selectedSegmentIndex = segIndex;
                    }
                    update();
                    event->accept();
                    return;
                }

                // Double-tap on a lane: same dialog as the mouse path. The
                // single-shot defers past this press so the pen isn't left
                // inert inside the modal dialog (see the note path below).
                int laneNote = -1, laneIdx = -1;
                if (m_densityTier != DensityTier::Off &&
                    findLaneAtPosition(currentPos, laneNote, laneIdx)) {
                    QTimer::singleShot(0, this, [this, laneNote, laneIdx]() {
                        editLaneCurveDialog(laneNote, laneIdx);
                    });
                    event->accept();
                    return;
                }

                // Selected note: open curve editor for the active curve.
                // Defer so the tablet press completes before the modal dialog opens —
                // otherwise the pen never gets a TabletRelease and is inert inside the dialog.
                if (selectedNoteIndices.contains(clickedNoteIndex)) {
                    QTimer::singleShot(0, this, [this, clickedNoteIndex]() {
                        editNoteCurve(clickedNoteIndex);
                    });
                    event->accept();
                    return;
                }
            }
        }

        // Mode-specific behavior (left button / pen tip)
        if (currentInputMode == SelectionMode) {
            const QVector<Note> &notes = phrase.getNotes();

            // Curve lanes (D7) - same handling as the mouse path.
            if (handleLanePress(pos.toPoint())) {
                event->accept();
                return;
            }

            // Ctrl+click on the body of an already-selected note toggles it off
            if (event->modifiers() & Qt::ControlModifier) {
                int toggleIndex = findNoteAtPosition(pos.toPoint());
                if (toggleIndex >= 0 && selectedNoteIndices.contains(toggleIndex)) {
                    selectedNoteIndices.removeAll(toggleIndex);
                    emit noteSelectionChanged();
                    update();
                    event->accept();
                    return;
                }
            }

            // FIRST: Check if clicking on handles/dots of selected note (these are outside note rect)
            if (selectedNoteIndices.size() == 1) {
                int selectedIndex = selectedNoteIndices.first();
                if (selectedIndex < notes.size()) {
                    const Note &selectedNote = notes[selectedIndex];
                    DragMode detectedMode = detectDragMode(pos.toPoint(), selectedNote);

                    if (detectedMode != NoDrag) {
                        // Clicking on a handle/dot - prepare for drag operation
                        // Use pending mode with threshold to avoid accidental drags from pen barrel buttons
                        pendingDragMode = detectedMode;
                        currentDragMode = NoDrag;  // Don't start dragging yet
                        dragThresholdExceeded = false;
                        dragStartPos = pos.toPoint();
                        dragStartTime = selectedNote.getStartTime();
                        dragStartPitch = selectedNote.getPitchHz();
                        dragStartDuration = selectedNote.getDuration();

                        // Store original curves for editing
                        if (pendingDragMode == EditingTopCurve ||
                            pendingDragMode == EditingTopCurveStart ||
                            pendingDragMode == EditingTopCurveEnd) {
                            // Use active expressive curve resolved by name for this specific note
                            int activeIdx = resolveActiveCurveIndex(selectedNote);
                            if (activeIdx > 0) {
                                dragStartCurve = selectedNote.getExpressiveCurve(activeIdx);
                            } else {
                                dragStartCurve = selectedNote.getDynamicsCurve();
                            }
                        } else if (pendingDragMode == EditingPitchCurve ||
                                   pendingDragMode == EditingPitchCurveStart ||
                                   pendingDragMode == EditingPitchCurveEnd) {
                            dragStartCurve = selectedNote.getPitchCurve();
                            if (dragStartCurve.isEmpty()) {
                                // Discrete note: seed with nominal pitch so valueAt() returns nominal
                                // for unedited points. First drag will auto-promote to continuous.
                                double nominal = selectedNote.getPitchHz();
                                dragStartCurve.addPoint(0.0, nominal);
                                dragStartCurve.addPoint(1.0, nominal);
                            }
                        } else if (pendingDragMode == TransformStretchTop ||
                                   pendingDragMode == TransformStretchBottom ||
                                   pendingDragMode == TransformRotate) {
                            dragStartCurve = selectedNote.getPitchCurve();
                            if (dragStartCurve.isEmpty()) {
                                int n = calculateCurveDotCount(selectedNote) + 2;
                                double nominal = selectedNote.getPitchHz();
                                for (int i = 0; i < n; ++i) {
                                    double t = (n == 1) ? 0.0 : double(i) / (n - 1);
                                    dragStartCurve.addPoint(t, nominal);
                                }
                            }
                        } else if (pendingDragMode == DraggingNote && selectedNote.hasPitchCurve()) {
                            // Save pitch curve for continuous notes
                            dragStartCurve = selectedNote.getPitchCurve();
                        }

                        event->accept();
                        return;  // Don't proceed with other selection logic
                    }
                }
            }

            // Check resize handles on any selected note (multi-selection resize)
            if (selectedNoteIndices.size() > 1) {
                for (int selIdx : selectedNoteIndices) {
                    if (selIdx >= 0 && selIdx < notes.size()) {
                        const Note &selNote = notes[selIdx];
                        if (getLeftResizeHandle(selNote).contains(pos.toPoint())) {
                            pendingDragMode = ResizingLeft;
                            currentDragMode = NoDrag;
                            dragThresholdExceeded = false;
                            dragStartPos = pos.toPoint();

                            multiResizeStartTimes.clear();
                            multiResizeStartDurations.clear();
                            for (int index : selectedNoteIndices) {
                                if (index >= 0 && index < notes.size()) {
                                    const Note &note = notes[index];
                                    multiResizeStartTimes.append(qMakePair(index, note.getStartTime()));
                                    multiResizeStartDurations.append(qMakePair(index, note.getDuration()));
                                }
                            }
                            event->accept();
                            return;
                        }
                        if (getRightResizeHandle(selNote).contains(pos.toPoint())) {
                            pendingDragMode = ResizingRight;
                            currentDragMode = NoDrag;
                            dragThresholdExceeded = false;
                            dragStartPos = pos.toPoint();

                            multiResizeStartTimes.clear();
                            multiResizeStartDurations.clear();
                            for (int index : selectedNoteIndices) {
                                if (index >= 0 && index < notes.size()) {
                                    const Note &note = notes[index];
                                    multiResizeStartTimes.append(qMakePair(index, note.getStartTime()));
                                    multiResizeStartDurations.append(qMakePair(index, note.getDuration()));
                                }
                            }
                            event->accept();
                            return;
                        }
                    }
                }
            }

            // SECOND: Check if clicking on a note body
            int clickedNoteIndex = findNoteAtPosition(pos.toPoint());

            if (!selectedNoteIndices.isEmpty() && selectedNoteIndices.contains(clickedNoteIndex)) {
                // Clicking on body of a selected note - prepare for drag
                if (selectedNoteIndices.size() == 1) {
                    // Already handled above with detectDragMode
                    event->accept();
                    return;
                } else {
                    // Multi-selection - allow dragging the group
                    pendingDragMode = DraggingNote;
                    currentDragMode = NoDrag;
                    dragThresholdExceeded = false;
                    dragStartPos = pos.toPoint();

                    // Store original states for all selected notes
                    multiDragStartTimes.clear();
                    multiDragStartPitches.clear();
                    multiDragStartCurves.clear();

                    for (int index : selectedNoteIndices) {
                        if (index >= 0 && index < notes.size()) {
                            const Note &note = notes[index];
                            multiDragStartTimes.append(qMakePair(index, note.getStartTime()));
                            multiDragStartPitches.append(qMakePair(index, note.getPitchHz()));
                            if (note.hasPitchCurve()) {
                                multiDragStartCurves.append(qMakePair(index, note.getPitchCurve()));
                            }
                        }
                    }

                    event->accept();
                    return;  // Don't start selecting another note
                }
            }

            // THIRD: Selecting a different note or empty space
            if (clickedNoteIndex >= 0) {
                if (event->modifiers() & Qt::ShiftModifier) {
                    selectNoteRange(clickedNoteIndex);
                } else {
                    bool addToSelection = (event->modifiers() & Qt::ControlModifier);
                    selectNote(clickedNoteIndex, addToSelection);
                }
                update();
            } else {
                // Tapped empty space - start lasso selection
                lassoAddToSelection = (event->modifiers() & Qt::ControlModifier);
                if (!lassoAddToSelection) {
                    deselectAll();
                }
                isDrawingLasso = true;
                lassoStartPos = pos.toPoint();
                lassoCurrentPos = pos.toPoint();
                m_autoScrollTimer->start();  // Run every tick; timer checks cursor vs edge
                update();
            }
        } else if (!isDrawingNote) {
            // In drawing modes (discrete or continuous), start drawing note
            isDrawingNote = true;
            noteStartPos = pos.toPoint();
            noteCurrentPos = pos.toPoint();

            // Start capturing pressure curve
            pressurePoints.clear();
            double startTime = pixelToTime(pos.x());
            pressurePoints.append(qMakePair(startTime, penPressure));

            // In continuous mode, start tracking pitch curve
            if (currentInputMode == DrawModeContinuous) {
                pitchPoints.clear();
                double startPitch = pixelToFrequency(pos.y());
                pitchPoints.append(qMakePair(startTime, startPitch));
            }

            // Emit pressure update
            emit pressureChanged(penPressure, true);

            update();
        }
        event->accept();
        break;
    }

    case QEvent::TabletMove:
        // Check if we have a pending drag and should activate it (threshold exceeded)
        if (pendingDragMode != NoDrag && !dragThresholdExceeded) {
            QPoint delta = pos.toPoint() - dragStartPos;
            int distance = std::abs(delta.x()) + std::abs(delta.y());  // Manhattan distance
            if (distance >= DRAG_THRESHOLD) {
                // Threshold exceeded - activate the drag
                currentDragMode = pendingDragMode;
                pendingDragMode = NoDrag;
                dragThresholdExceeded = true;
            }
        }

        // Handle dragging/resizing selected notes
        if (currentDragMode != NoDrag && !selectedNoteIndices.isEmpty()) {
            QVector<Note> &notes = phrase.getNotes();
            QPoint delta = pos.toPoint() - dragStartPos;
            // Slide mode: lock pitch — discard vertical component when moving note bodies
            if (slideMode && currentDragMode == DraggingNote)
                delta.setY(0);

            if (selectedNoteIndices.size() == 1) {
                // Single selection - full editing capabilities
                int selectedIndex = selectedNoteIndices.first();
                if (selectedIndex >= notes.size()) {
                    event->accept();
                    return;
                }

                Note &note = notes[selectedIndex];

                switch (currentDragMode) {
                case DraggingNote: {
                // Move note in time and pitch
                double timeDelta = pixelToTime(delta.x()) - pixelToTime(0);
                double newStartTime = dragStartTime + timeDelta;
                if (newStartTime < 0.0) newStartTime = 0.0;

                note.setStartTime(newStartTime);

                // Handle pitch change based on whether note has pitch curve
                if (note.hasPitchCurve()) {
                    // Continuous note with pitch curve: shift entire curve by pixel delta
                    Curve shiftedPitchCurve;
                    const Curve& originalCurve = dragStartCurve;  // Use curve saved at drag start
                    for (const auto& point : originalCurve.getPoints()) {
                        double t = point.time;
                        double originalPitch = point.value;
                        // Convert to pixel, apply delta, convert back
                        int originalPixelY = frequencyToPixel(originalPitch);
                        int newPixelY = originalPixelY + delta.y();
                        double newPitch = pixelToFrequency(newPixelY);
                        shiftedPitchCurve.addPoint(t, newPitch);
                    }

                    // If note is quantized, snap the shifted curve to scale (maintains legato/stepped effect)
                    if (note.isQuantized()) {
                        Curve quantizedCurve = quantizePitchCurveToScale(shiftedPitchCurve, note);
                        note.setPitchCurve(quantizedCurve);
                    } else {
                        note.setPitchCurve(shiftedPitchCurve);
                    }
                } else {
                    // Discrete note: snap to scale lines at note's time position
                    double rawPitch = pixelToFrequency(dragStartPos.y() + delta.y());
                    double snappedPitch = snapToNearestScaleLineAtTime(rawPitch, note.getStartTime());
                    note.setPitchHz(snappedPitch);
                }
                break;
            }

            case ResizingLeft: {
                double timeDelta = pixelToTime(delta.x()) - pixelToTime(0);
                double newStartTime = dragStartTime + timeDelta;
                double newDuration = dragStartDuration - timeDelta;

                if (newDuration < 100.0) {
                    newDuration = 100.0;
                    newStartTime = dragStartTime + dragStartDuration - 100.0;
                }
                if (newStartTime < 0.0) newStartTime = 0.0;

                note.setStartTime(newStartTime);
                note.setDuration(newDuration);
                break;
            }

            case ResizingRight: {
                double timeDelta = pixelToTime(delta.x()) - pixelToTime(0);
                double newDuration = dragStartDuration + timeDelta;

                if (newDuration < 100.0) newDuration = 100.0;

                note.setDuration(newDuration);
                break;
            }

            case EditingTopCurve: {
                // Edit active expressive curve by dragging dot vertically
                double dynamicsChange = -delta.y() / 50.0;  // 50 pixels = 1.0 change

                double originalValue = dragStartCurve.valueAt(editingDotTimePos);
                double newValue = qBound(0.0, originalValue + dynamicsChange, 1.0);

                int activeIdx = resolveActiveCurveIndex(note);
                // Never coarsen - same rule as the mouse path.
                note.getExpressiveCurve(activeIdx) =
                    curveAfterDotDrag(dragStartCurve, calculateCurveDotCount(note),
                                           editingDotIndex, newValue);
                break;
            }

            case EditingLaneCurve: {
                const QVector<CurveLanes::Lane> lanes = resolveLanesFor(note);
                if (m_editLaneIndex < 0 || m_editLaneIndex >= lanes.size()) break;
                const CurveLanes::Lane &lane = lanes[m_editLaneIndex];
                if (lane.curveIndex < 0) break;

                const QRect r = laneRect(note, m_editLaneIndex, lanes.size(), m_editLaneIndex);
                const double span = std::max(1, r.height() - 1);
                const double newLaneValue =
                    qBound(0.0, (r.bottom() - pos.toPoint().y()) / span, 1.0);

                note.getExpressiveCurve(lane.curveIndex) =
                    curveAfterDotDrag(dragStartCurve, calculateCurveDotCount(note),
                                           editingDotIndex, newLaneValue);
                break;
            }

            case EditingPitchCurve: {
                // Edit pitch curve by dragging dot vertically (Hz, log-y via canvas mapping)
                double startHz = dragStartCurve.valueAt(editingDotTimePos);
                int startPixelY = frequencyToPixel(startHz);
                double newHz = pixelToFrequency(startPixelY + delta.y());

                Curve &pitchCurve = note.getPitchCurve();
                pitchCurve.clearPoints();

                // Rebuild curve with adaptive number of control points
                int numDots = calculateCurveDotCount(note);
                for (int i = 0; i <= numDots + 1; ++i) {
                    double t = i / (numDots + 1.0);
                    double value;

                    if (i == editingDotIndex + 1) {
                        value = newHz;
                    } else {
                        value = dragStartCurve.valueAt(t);
                    }

                    pitchCurve.addPoint(t, value);
                }

                pitchCurve.sortPoints();
                break;
            }

            case EditingTopCurveStart: {
                // Edit active expressive curve start value (t=0.0)
                double dynamicsChange = -delta.y() / 50.0;
                double originalValue = dragStartCurve.valueAt(0.0);
                double newValue = qBound(0.0, originalValue + dynamicsChange, 1.0);

                int activeIdx = resolveActiveCurveIndex(note);
                Curve &activeCurve = note.getExpressiveCurve(activeIdx);
                activeCurve.clearPoints();

                int numDots = calculateCurveDotCount(note);
                for (int i = 0; i <= numDots + 1; ++i) {
                    double t = i / (numDots + 1.0);
                    double value = (i == 0) ? newValue : dragStartCurve.valueAt(t);
                    activeCurve.addPoint(t, value);
                }
                activeCurve.sortPoints();
                break;
            }

            case EditingTopCurveEnd: {
                // Edit active expressive curve end value (t=1.0)
                double dynamicsChange = -delta.y() / 50.0;
                double originalValue = dragStartCurve.valueAt(1.0);
                double newValue = qBound(0.0, originalValue + dynamicsChange, 1.0);

                int activeIdx = resolveActiveCurveIndex(note);
                Curve &activeCurve = note.getExpressiveCurve(activeIdx);
                activeCurve.clearPoints();

                int numDots = calculateCurveDotCount(note);
                for (int i = 0; i <= numDots + 1; ++i) {
                    double t = i / (numDots + 1.0);
                    double value = (i == numDots + 1) ? newValue : dragStartCurve.valueAt(t);
                    activeCurve.addPoint(t, value);
                }
                activeCurve.sortPoints();
                break;
            }

            case EditingPitchCurveStart: {
                // Edit pitch curve start value (t=0.0)
                double startHz = dragStartCurve.valueAt(0.0);
                int startPixelY = frequencyToPixel(startHz);
                double newHz = pixelToFrequency(startPixelY + delta.y());

                Curve &pitchCurve = note.getPitchCurve();
                pitchCurve.clearPoints();

                int numDots = calculateCurveDotCount(note);
                for (int i = 0; i <= numDots + 1; ++i) {
                    double t = i / (numDots + 1.0);
                    double value = (i == 0) ? newHz : dragStartCurve.valueAt(t);
                    pitchCurve.addPoint(t, value);
                }
                pitchCurve.sortPoints();
                break;
            }

            case EditingPitchCurveEnd: {
                // Edit pitch curve end value (t=1.0)
                double startHz = dragStartCurve.valueAt(1.0);
                int startPixelY = frequencyToPixel(startHz);
                double newHz = pixelToFrequency(startPixelY + delta.y());

                Curve &pitchCurve = note.getPitchCurve();
                pitchCurve.clearPoints();

                int numDots = calculateCurveDotCount(note);
                for (int i = 0; i <= numDots + 1; ++i) {
                    double t = i / (numDots + 1.0);
                    double value = (i == numDots + 1) ? newHz : dragStartCurve.valueAt(t);
                    pitchCurve.addPoint(t, value);
                }
                pitchCurve.sortPoints();
                break;
            }

            case TransformStretchTop:
            case TransformStretchBottom: {
                int pivotY = frequencyToPixel(note.getPitchHz());
                int origHandleY = (currentDragMode == TransformStretchTop)
                    ? transformModeRect.top()
                    : transformModeRect.bottom();
                int origOffset = origHandleY - pivotY;
                if (origOffset == 0) break;
                double scale = double(origOffset + delta.y()) / double(origOffset);

                Curve &pitchCurve = note.getPitchCurve();
                pitchCurve.clearPoints();
                for (const auto &p : dragStartCurve.getPoints()) {
                    int origPixelY = frequencyToPixel(p.value);
                    int newPixelY  = pivotY + static_cast<int>((origPixelY - pivotY) * scale);
                    pitchCurve.addPoint(p.time, pixelToFrequency(newPixelY));
                }
                pitchCurve.sortPoints();
                break;
            }

            case TransformRotate: {
                const QPointF &anchor = transformRotateAnchor;
                double dx0 = dragStartPos.x() - anchor.x();
                double dy0 = dragStartPos.y() - anchor.y();
                double dx1 = (dragStartPos.x() + delta.x()) - anchor.x();
                double dy1 = (dragStartPos.y() + delta.y()) - anchor.y();
                double angle = std::atan2(dy1, dx1) - std::atan2(dy0, dx0);
                double tanA = std::tan(angle);
                int width = transformModeRect.width();
                double tPivot = (width > 0)
                    ? double(anchor.x() - transformModeRect.left()) / double(width)
                    : 0.5;

                Curve &pitchCurve = note.getPitchCurve();
                pitchCurve.clearPoints();
                for (const auto &p : dragStartCurve.getPoints()) {
                    int origPixelY = frequencyToPixel(p.value);
                    int newPixelY  = origPixelY + static_cast<int>(tanA * (p.time - tPivot) * width);
                    pitchCurve.addPoint(p.time, pixelToFrequency(newPixelY));
                }
                pitchCurve.sortPoints();
                break;
            }

                case NoDrag:
                    break;
                }
            } else {
                // Multi-selection resize or drag
                if (currentDragMode == ResizingLeft) {
                    double timeDelta = pixelToTime(delta.x()) - pixelToTime(0);
                    for (int i = 0; i < multiResizeStartTimes.size(); ++i) {
                        int index = multiResizeStartTimes[i].first;
                        double origStart = multiResizeStartTimes[i].second;
                        double origDur = multiResizeStartDurations[i].second;
                        if (index >= 0 && index < notes.size()) {
                            double newStart = origStart + timeDelta;
                            double newDur = origDur - timeDelta;
                            if (newDur < 100.0) {
                                newDur = 100.0;
                                newStart = origStart + origDur - 100.0;
                            }
                            if (newStart < 0.0) newStart = 0.0;
                            notes[index].setStartTime(newStart);
                            notes[index].setDuration(newDur);
                        }
                    }
                } else if (currentDragMode == ResizingRight) {
                    double timeDelta = pixelToTime(delta.x()) - pixelToTime(0);
                    for (int i = 0; i < multiResizeStartDurations.size(); ++i) {
                        int index = multiResizeStartDurations[i].first;
                        double origDur = multiResizeStartDurations[i].second;
                        if (index >= 0 && index < notes.size()) {
                            double newDur = origDur + timeDelta;
                            if (newDur < 100.0) newDur = 100.0;
                            notes[index].setDuration(newDur);
                        }
                    }
                } else if (currentDragMode == DraggingNote) {
                    // Calculate time delta from original position
                    double timeDelta = pixelToTime(delta.x()) - pixelToTime(0);

                    // Apply time deltas to all selected notes
                    for (int i = 0; i < multiDragStartTimes.size(); ++i) {
                        int index = multiDragStartTimes[i].first;
                        double originalTime = multiDragStartTimes[i].second;

                        if (index >= 0 && index < notes.size()) {
                            Note &note = notes[index];

                            // Apply time movement from original position
                            double newStartTime = originalTime + timeDelta;
                            if (newStartTime < 0.0) newStartTime = 0.0;
                            note.setStartTime(newStartTime);
                        }
                    }

                    // Apply pitch movement - each note moves by same PIXEL delta
                    for (int i = 0; i < multiDragStartPitches.size(); ++i) {
                        int index = multiDragStartPitches[i].first;
                        double originalPitch = multiDragStartPitches[i].second;

                        if (index >= 0 && index < notes.size()) {
                            Note &note = notes[index];

                            if (!note.hasPitchCurve()) {
                                // Discrete note: apply pixel delta then snap to nearest scale degree
                                int originalPixelY = frequencyToPixel(originalPitch);
                                int newPixelY = originalPixelY + delta.y();
                                double rawPitch = pixelToFrequency(newPixelY);
                                // Snap to nearest scale line at note's time position
                                double snappedPitch = snapToNearestScaleLineAtTime(rawPitch, note.getStartTime());
                                note.setPitchHz(snappedPitch);
                            }
                        }
                    }

                    // Apply pitch curve shifts - each curve point moves by same PIXEL delta
                    for (int i = 0; i < multiDragStartCurves.size(); ++i) {
                        int index = multiDragStartCurves[i].first;
                        const Curve &originalCurve = multiDragStartCurves[i].second;

                        if (index >= 0 && index < notes.size()) {
                            Note &note = notes[index];

                            // Shift pitch curve by pixel delta
                            Curve shiftedPitchCurve;
                            for (const auto& point : originalCurve.getPoints()) {
                                // Convert original pitch to pixel Y
                                int originalPixelY = frequencyToPixel(point.value);
                                // Apply pixel delta
                                int newPixelY = originalPixelY + delta.y();
                                // Convert back to frequency
                                double newPitch = pixelToFrequency(newPixelY);
                                shiftedPitchCurve.addPoint(point.time, newPitch);
                            }

                            // If note is quantized, snap the shifted curve to scale (maintains legato effect)
                            if (note.isQuantized()) {
                                Curve quantizedCurve = quantizePitchCurveToScale(shiftedPitchCurve, note);
                                note.setPitchCurve(quantizedCurve);
                            } else {
                                note.setPitchCurve(shiftedPitchCurve);
                            }
                        }
                    }
                }
            }

            update();
            event->accept();
            return;
        }

        // Handle lasso selection dragging
        if (isDrawingLasso) {
            lassoCurrentPos = pos.toPoint();
            update();
            event->accept();
            return;
        }

        // Update note drawing position
        if (isDrawingNote) {
            noteCurrentPos = pos.toPoint();

            // Capture pressure point at this position
            double currentTime = pixelToTime(pos.x());
            pressurePoints.append(qMakePair(currentTime, penPressure));

            // In continuous mode, track pitch changes
            if (currentInputMode == DrawModeContinuous) {
                double currentPitch = pixelToFrequency(pos.y());
                pitchPoints.append(qMakePair(currentTime, currentPitch));
            }

            // Emit pressure update
            emit pressureChanged(penPressure, true);

            update();
        }
        event->accept();
        break;

    case QEvent::TabletRelease:
        // Reset pending drag state if threshold was never exceeded
        // This happens when user lifts pen before moving enough to start drag
        // (e.g., when using barrel button for right-click)
        if (pendingDragMode != NoDrag && !dragThresholdExceeded) {
            pendingDragMode = NoDrag;
            // Don't return - allow other release handling to continue
        }

        // End drag/resize operation
        if (currentDragMode != NoDrag) {
            if (selectedNoteIndices.size() == 1) {
                // Single selection - full editing capabilities
                int selectedIndex = selectedNoteIndices.first();
                if (selectedIndex >= 0 && selectedIndex < phrase.getNotes().size()) {
                    const Note &note = phrase.getNotes()[selectedIndex];

                    switch (currentDragMode) {
                    case DraggingNote: {
                        // Push move command with old and new states
                        double newStartTime = note.getStartTime();
                        double newPitch = note.getPitchHz();
                        Curve newPitchCurve;
                        bool hasCurve = note.hasPitchCurve();
                        if (hasCurve) {
                            newPitchCurve = note.getPitchCurve();
                        }

                        undoStack->push(new MoveNoteCommand(&phrase, selectedIndex,
                                                            dragStartTime, dragStartPitch,
                                                            newStartTime, newPitch,
                                                            dragStartCurve, newPitchCurve,
                                                            hasCurve, this));
                        break;
                    }

                    case ResizingLeft:
                    case ResizingRight: {
                        // Push resize command
                        double newStartTime = note.getStartTime();
                        double newDuration = note.getDuration();

                        undoStack->push(new ResizeNoteCommand(&phrase, selectedIndex,
                                                              dragStartTime, dragStartDuration,
                                                              newStartTime, newDuration,
                                                              this));
                        break;
                    }

                    case EditingTopCurve:
                    case EditingTopCurveStart:
                    case EditingTopCurveEnd: {
                        // Push curve edit command for active expressive curve
                        int activeIdx = resolveActiveCurveIndex(note);
                        Curve newCurve = note.getExpressiveCurve(activeIdx);
                        if (activeIdx == 0) {
                            undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                                 EditCurveCommand::DynamicsCurve,
                                                                 dragStartCurve, newCurve,
                                                                 this));
                        } else {
                            undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                                 EditCurveCommand::ExpressiveCurveN,
                                                                 activeIdx,
                                                                 dragStartCurve, newCurve,
                                                                 this));
                        }
                        break;
                    }

                    case EditingLaneCurve: {
                        const QVector<CurveLanes::Lane> lanes = resolveLanesFor(note);
                        if (m_editLaneIndex >= 0 && m_editLaneIndex < lanes.size() &&
                            lanes[m_editLaneIndex].curveIndex >= 0) {
                            const int ci = lanes[m_editLaneIndex].curveIndex;
                            undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                                 EditCurveCommand::ExpressiveCurveN,
                                                                 ci, dragStartCurve,
                                                                 note.getExpressiveCurve(ci),
                                                                 this));
                        }
                        break;
                    }

                    case EditingPitchCurve: {
                        // Push curve edit command for pitch curve
                        Curve newCurve = note.getPitchCurve();
                        undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                             EditCurveCommand::PitchCurve,
                                                             dragStartCurve, newCurve,
                                                             this));
                        break;
                    }

                    case EditingPitchCurveStart:
                    case EditingPitchCurveEnd: {
                        // Push curve edit command for pitch curve (start/end)
                        Curve newCurve = note.getPitchCurve();
                        undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                             EditCurveCommand::PitchCurve,
                                                             dragStartCurve, newCurve,
                                                             this));
                        break;
                    }

                    case TransformStretchTop:
                    case TransformStretchBottom:
                    case TransformRotate: {
                        Curve newCurve = note.getPitchCurve();
                        undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                             EditCurveCommand::PitchCurve,
                                                             dragStartCurve, newCurve,
                                                             this));
                        break;
                    }

                    case NoDrag:
                        break;
                    default:
                        break;
                    }
                }
            } else if (selectedNoteIndices.size() > 1 && currentDragMode == DraggingNote) {
                // Multi-selection drag - create batch move command
                QVector<MoveMultipleNotesCommand::NoteState> oldStates;
                QVector<MoveMultipleNotesCommand::NoteState> newStates;

                const QVector<Note> &notes = phrase.getNotes();

                // Build old states from stored drag start data
                for (int i = 0; i < multiDragStartTimes.size(); ++i) {
                    int index = multiDragStartTimes[i].first;
                    MoveMultipleNotesCommand::NoteState state;
                    state.index = index;
                    state.startTime = multiDragStartTimes[i].second;

                    // Find matching pitch
                    for (const auto &pair : multiDragStartPitches) {
                        if (pair.first == index) {
                            state.pitch = pair.second;
                            break;
                        }
                    }

                    // Find matching curve if it exists
                    state.hasPitchCurve = false;
                    for (const auto &pair : multiDragStartCurves) {
                        if (pair.first == index) {
                            state.pitchCurve = pair.second;
                            state.hasPitchCurve = true;
                            break;
                        }
                    }

                    oldStates.append(state);
                }

                // Build new states from current note positions
                for (int index : selectedNoteIndices) {
                    if (index >= 0 && index < notes.size()) {
                        const Note &note = notes[index];
                        MoveMultipleNotesCommand::NoteState state;
                        state.index = index;
                        state.startTime = note.getStartTime();
                        state.pitch = note.getPitchHz();
                        state.hasPitchCurve = note.hasPitchCurve();
                        if (state.hasPitchCurve) {
                            state.pitchCurve = note.getPitchCurve();
                        }
                        newStates.append(state);
                    }
                }

                undoStack->push(new MoveMultipleNotesCommand(&phrase, selectedNoteIndices,
                                                             oldStates, newStates, this));
            } else if (selectedNoteIndices.size() > 1 &&
                       (currentDragMode == ResizingLeft || currentDragMode == ResizingRight)) {
                // Multi-selection resize - create batch resize command
                QVector<ResizeMultipleNotesCommand::NoteState> oldStates;
                QVector<ResizeMultipleNotesCommand::NoteState> newStates;

                const QVector<Note> &notes = phrase.getNotes();

                for (int i = 0; i < multiResizeStartTimes.size(); ++i) {
                    int index = multiResizeStartTimes[i].first;
                    ResizeMultipleNotesCommand::NoteState oldState;
                    oldState.index = index;
                    oldState.startTime = multiResizeStartTimes[i].second;
                    oldState.duration = multiResizeStartDurations[i].second;
                    oldStates.append(oldState);

                    if (index >= 0 && index < notes.size()) {
                        ResizeMultipleNotesCommand::NoteState newState;
                        newState.index = index;
                        newState.startTime = notes[index].getStartTime();
                        newState.duration = notes[index].getDuration();
                        newStates.append(newState);
                    }
                }

                undoStack->push(new ResizeMultipleNotesCommand(&phrase, oldStates, newStates, this));
            }

            currentDragMode = NoDrag;
            pendingDragMode = NoDrag;
            dragThresholdExceeded = false;
            update();
            event->accept();
            return;
        }

        // Complete lasso selection
        if (isDrawingLasso) {
            isDrawingLasso = false;
            m_autoScrollTimer->stop();

            // Calculate lasso rectangle
            int x = std::min(lassoStartPos.x(), lassoCurrentPos.x());
            int y = std::min(lassoStartPos.y(), lassoCurrentPos.y());
            int width = std::abs(lassoCurrentPos.x() - lassoStartPos.x());
            int height = std::abs(lassoCurrentPos.y() - lassoStartPos.y());
            QRect lassoRect(x, y, width, height);

            // Find all notes within the lasso rectangle
            QVector<int> notesInRect = findNotesInRectangle(lassoRect);
            if (lassoAddToSelection) {
                for (int idx : notesInRect) {
                    if (!selectedNoteIndices.contains(idx)) {
                        selectedNoteIndices.append(idx);
                    }
                }
                emit noteSelectionChanged();
            } else {
                selectNotes(notesInRect);
            }

            update();
            event->accept();
            return;
        }

        // Capture final pressure point and handle note creation or selection
        if (isDrawingNote) {
            double endTime = pixelToTime(pos.x());
            pressurePoints.append(qMakePair(endTime, penPressure));

            // Create the note directly here (don't wait for mouseReleaseEvent)
            isDrawingNote = false;

            // Calculate note parameters
            double startTime = pixelToTime(std::min(noteStartPos.x(), noteCurrentPos.x()));
            double endTimeCalc = pixelToTime(std::max(noteStartPos.x(), noteCurrentPos.x()));
            double duration = endTimeCalc - startTime;

            // Use pixel distance to distinguish taps from intentional drags
            double pixelDist = QLineF(noteStartPos, noteCurrentPos).length();
            if (pixelDist < 3.0) {
                // This was a tap, not a drag — use default duration
                duration = 500.0;
                endTimeCalc = startTime + duration;
            }

            // Create a new note (we're in drawing mode)
            // Get pitch from vertical position
            double rawPitch = pixelToFrequency(noteStartPos.y());
            double finalPitch;

            // Apply pitch snapping based on input mode
            if (currentInputMode == DrawModeDiscrete) {
                // Discrete mode: snap to nearest scale line at note's time position
                finalPitch = snapToNearestScaleLineAtTime(rawPitch, startTime);
            } else {
                // Continuous mode: use literal pitch (no snapping)
                finalPitch = rawPitch;
            }

            // Deselect when creating a new note
            deselectAll();

            // Create note with default dynamics first
            Note newNote(startTime, duration, finalPitch, 0.7);

            // Set the note to use the active track
            newNote.setTrackIndex(activeTrackIndex);

            // Create a default bottom curve with some variation (for visual interest)
            Curve defaultBottomCurve;
            defaultBottomCurve.addPoint(0.0, 0.5);
            defaultBottomCurve.addPoint(0.25, 0.6);
            defaultBottomCurve.addPoint(0.5, 0.65);
            defaultBottomCurve.addPoint(0.75, 0.6);
            defaultBottomCurve.addPoint(1.0, 0.55);
            newNote.setBottomCurve(defaultBottomCurve);

            // Create dynamics curve from captured pressure points
            if (pressurePoints.size() >= 2) {
                Curve dynamicsCurve;

                // Convert absolute time points to normalized time (0.0-1.0 within the note)
                for (const auto &point : pressurePoints) {
                    double absoluteTime = point.first;
                    double pressure = point.second;

                    // Normalize time within the note duration
                    double normalizedTime = (absoluteTime - startTime) / duration;
                    normalizedTime = qBound(0.0, normalizedTime, 1.0);

                    dynamicsCurve.addPoint(normalizedTime, pressure);
                }

                // Sort points by time to ensure proper interpolation
                dynamicsCurve.sortPoints();

                // Set the curve on the note
                newNote.setDynamicsCurve(dynamicsCurve);
            }

            // If in continuous mode and we have pitch points, create a pitch curve for glissando
            if (currentInputMode == DrawModeContinuous && pitchPoints.size() >= 2) {
                Curve pitchCurve;

                // Convert absolute time points to normalized time (0.0-1.0 within the note)
                for (const auto &point : pitchPoints) {
                    double absoluteTime = point.first;
                    double pitchHz = point.second;

                    // Normalize time within the note duration
                    double normalizedTime = (absoluteTime - startTime) / duration;
                    normalizedTime = qBound(0.0, normalizedTime, 1.0);

                    pitchCurve.addPoint(normalizedTime, pitchHz);
                }

                // Sort points by time to ensure proper interpolation
                pitchCurve.sortPoints();

                // Set the pitch curve on the note
                newNote.setPitchCurve(pitchCurve);
            }

            // Stamp current toolbar variation
            newNote.setVariationIndex(m_activeVariationIndex);

            // Use undo command to add note
            undoStack->push(new AddNoteCommand(&phrase, newNote, this));

            // Reset state for next input
            usingTablet = false;
            pressurePoints.clear();
            pitchPoints.clear();

            // Signal that pressure monitoring is inactive
            emit pressureChanged(0.0, false);

            update();
        }
        event->accept();
        break;

    default:
        event->ignore();
        break;
    }
}

// ============================================================================
// Keyboard Input
// ============================================================================

void ScoreCanvas::keyPressEvent(QKeyEvent *event)
{
    // Handle segment editing mode keys first
    if (segmentEditingMode) {
        // Escape exits segment mode
        if (event->key() == Qt::Key_Escape) {
            exitSegmentEditingMode();
            event->accept();
            return;
        }

        // Delete removes selected segment
        if (event->key() == Qt::Key_Delete && selectedSegmentIndex >= 0) {
            deleteSelectedSegment();
            event->accept();
            return;
        }

        // Arrow keys to navigate between segments
        if (event->key() == Qt::Key_Left && selectedSegmentIndex > 0) {
            selectedSegmentIndex--;
            update();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_Right) {
            const QVector<Note> &notes = phrase.getNotes();
            if (segmentEditingNoteIndex >= 0 && segmentEditingNoteIndex < notes.size()) {
                int segCount = notes[segmentEditingNoteIndex].getSegmentCount();
                if (selectedSegmentIndex < segCount - 1) {
                    selectedSegmentIndex++;
                    update();
                    event->accept();
                    return;
                }
            }
        }
    }

    // Escape closes an open curve-lane hull first
    if (event->key() == Qt::Key_Escape && m_editLaneNote >= 0) {
        closeLaneEditor();
        event->accept();
        return;
    }

    // Escape exits transform mode (when active and not in segment mode)
    if (event->key() == Qt::Key_Escape && transformMode) {
        setTransformMode(false);
        event->accept();
        return;
    }

    // T: toggle transform mode (only meaningful with a single note selected)
    if (event->key() == Qt::Key_T && event->modifiers() == Qt::NoModifier) {
        toggleTransformMode();
        event->accept();
        return;
    }

    // Ctrl+L lives in ScoreCanvasWindow now (Phase F): an application-wide
    // QAction shortcut, so the tier cycles even while a toolbar combo holds
    // focus. Handled there, never here.

    // Ctrl+Shift+Up / Down: move the split point one lane. Phase F backs this
    // with the toolbar spin + Auto checkbox, but the keyboard path stays -
    // Nimus called it "a game changer". Ctrl+Shift+Home restores auto.
    if ((event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) &&
        event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
        int current = m_lanesAboveOverride;
        if (current < 0) {
            // Materialize the auto split for the note under the cursor before
            // nudging, so the first press moves by one rather than jumping.
            int count = 0;
            if (m_hoverNote >= 0 && m_hoverNote < phrase.getNotes().size())
                count = resolveLanesFor(phrase.getNotes()[m_hoverNote]).size();
            current = lanesAboveCount(count);
        }
        setLanesAbove(std::max(0, current + (event->key() == Qt::Key_Up ? 1 : -1)));
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Home &&
        event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
        setLanesAbove(-1);
        event->accept();
        return;
    }

    // Enter key enters segment editing mode (alternative to double-click)
    // Works when a single quantized continuous note is selected
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (selectedNoteIndices.size() == 1) {
            int noteIndex = selectedNoteIndices.first();
            const QVector<Note> &notes = phrase.getNotes();
            if (noteIndex >= 0 && noteIndex < notes.size()) {
                const Note &note = notes[noteIndex];
                // Only enter segment mode for quantized continuous notes
                if (note.isQuantized() && note.hasPitchCurve()) {
                    enterSegmentEditingMode(noteIndex);
                    update();
                    event->accept();
                    return;
                }
            }
        }
    }

    // Menu key or Shift+F10 opens context menu (alternative to right-click)
    if (event->key() == Qt::Key_Menu ||
        (event->key() == Qt::Key_F10 && (event->modifiers() & Qt::ShiftModifier))) {
        if (!selectedNoteIndices.isEmpty()) {
            // Get position of first selected note for menu placement
            const QVector<Note> &notes = phrase.getNotes();
            int noteIndex = selectedNoteIndices.first();
            if (noteIndex >= 0 && noteIndex < notes.size()) {
                QRect noteRect = getNoteRect(notes[noteIndex]);
                QPoint menuPos = mapToGlobal(noteRect.center());
                showNoteContextMenu(menuPos);
                event->accept();
                return;
            }
        }
    }

    // Shift+Up / Shift+Down: transpose selected notes by scale degrees
    if ((event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) &&
        (event->modifiers() & Qt::ShiftModifier) &&
        !selectedNoteIndices.isEmpty()) {
        int steps = (event->key() == Qt::Key_Up) ? 1 : -1;
        transposeSelectedNotesByDegrees(steps);
        event->accept();
        return;
    }

    // Shift+Left / Shift+Right: navigate to previous/next note
    if ((event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) &&
        (event->modifiers() & Qt::ShiftModifier)) {
        selectNextNote((event->key() == Qt::Key_Right) ? 1 : -1);
        event->accept();
        return;
    }

    // Delete key removes selected notes
    if (event->key() == Qt::Key_Delete && !selectedNoteIndices.isEmpty()) {
        performDelete();
        event->accept();
        return;
    }

    // Ctrl+Z for undo
    if (event->matches(QKeySequence::Undo)) {
        undoStack->undo();
        emit undoRedoPerformed();
        event->accept();
        return;
    }

    // Ctrl+Y or Ctrl+Shift+Z for redo
    if (event->matches(QKeySequence::Redo)) {
        undoStack->redo();
        emit undoRedoPerformed();
        event->accept();
        return;
    }

    // Ctrl+C for copy
    if (event->matches(QKeySequence::Copy)) {
        performCopy();
        event->accept();
        return;
    }

    // Ctrl+X for cut
    if (event->matches(QKeySequence::Cut)) {
        performCut();
        event->accept();
        return;
    }

    // Ctrl+Shift+V: paste at end of active track (building mode)
    if (event->key() == Qt::Key_V && event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
        performPaste(true);
        event->accept();
        return;
    }

    // Ctrl+V: paste at now marker (editing mode)
    if (event->matches(QKeySequence::Paste)) {
        performPaste();
        event->accept();
        return;
    }

    // Ctrl+A for select all notes in active track
    if (event->matches(QKeySequence::SelectAll)) {
        performSelectAll();
        event->accept();
        return;
    }

    // E: extend selection from earliest selected note to the last note on the active track
    if (event->key() == Qt::Key_E && event->modifiers() == Qt::NoModifier) {
        performSelectToEnd();
        event->accept();
        return;
    }

    // A: toggle slide mode (time-axis-only movement)
    if (event->key() == Qt::Key_A && event->modifiers() == Qt::NoModifier) {
        toggleSlideMode();
        event->accept();
        return;
    }

    // R: retrograde selection at now marker
    if (event->key() == Qt::Key_R && event->modifiers() == Qt::NoModifier) {
        performRetrograde();
        event->accept();
        return;
    }

    // Shift+L: link selected notes as legato (matches right-click "Link as Legato").
    // Plain L is reserved for loop-mode at the window level.
    if (event->key() == Qt::Key_L && event->modifiers() == Qt::ShiftModifier) {
        if (selectedNoteIndices.size() >= 2) {
            undoStack->push(new LinkAsLegatoCommand(&phrase, selectedNoteIndices, this));
        }
        event->accept();
        return;
    }

    // Let parent window handle other keys (including shortcuts)
    event->ignore();
}

void ScoreCanvas::toggleSlideMode()
{
    slideMode = !slideMode;
    emit slideModeChanged(slideMode);
}

void ScoreCanvas::toggleTransformMode()
{
    setTransformMode(!transformMode);
}

void ScoreCanvas::setTransformMode(bool active)
{
    // Transform mode requires a single-note selection.
    if (active && selectedNoteIndices.size() != 1) {
        active = false;
    }
    if (transformMode == active) {
        return;
    }
    transformMode = active;
    if (transformMode) {
        // Freeze the selection rect at entry so it doesn't pulse as the curve grows
        // while transforming. Live recomputation reads min/max pitch from the curve,
        // which would make all four handles drift mid-drag.
        const QVector<Note> &notes = phrase.getNotes();
        int idx = selectedNoteIndices.first();
        if (idx >= 0 && idx < notes.size()) {
            transformModeRect = getNoteRect(notes[idx]);
        }
    }
    emit transformModeChanged(transformMode);
    update();
}

// ============================================================================
// Public Edit Operations (called from menu or keyPressEvent)
// ============================================================================

void ScoreCanvas::performCopy()
{
    if (!selectedNoteIndices.isEmpty()) {
        clipboard.clear();
        const QVector<Note> &notes = phrase.getNotes();
        for (int index : selectedNoteIndices) {
            if (index >= 0 && index < notes.size()) {
                clipboard.append(notes[index]);
            }
        }
        qDebug() << "Copied" << clipboard.size() << "notes to clipboard";
    }
}

void ScoreCanvas::performCut()
{
    if (!selectedNoteIndices.isEmpty()) {
        clipboard.clear();
        const QVector<Note> &notes = phrase.getNotes();
        for (int index : selectedNoteIndices) {
            if (index >= 0 && index < notes.size()) {
                clipboard.append(notes[index]);
            }
        }
        qDebug() << "Cut" << clipboard.size() << "notes to clipboard";

        undoStack->push(new DeleteMultipleNotesCommand(&phrase, selectedNoteIndices, this));
        deselectAll();
        update();
    }
}

void ScoreCanvas::performPaste(bool atEndOfTrack)
{
    if (!clipboard.isEmpty()) {
        double targetTime;
        if (atEndOfTrack) {
            // Ctrl+Shift+V: paste 200ms after the end of the active track (building mode).
            double trackEnd = 0.0;
            bool hasTrackNotes = false;
            const QVector<Note> &existing = phrase.getNotes();
            for (const Note &n : existing) {
                if (n.getTrackIndex() != activeTrackIndex) continue;
                hasTrackNotes = true;
                if (n.getEndTime() > trackEnd) trackEnd = n.getEndTime();
            }
            targetTime = hasTrackNotes ? trackEnd + 200.0 : 0.0;
        } else {
            // Ctrl+V: paste at now marker (editing mode).
            targetTime = pasteTargetTime;
        }

        PasteNotesCommand *pasteCmd = new PasteNotesCommand(&phrase, clipboard, targetTime, activeTrackIndex, this);
        undoStack->push(pasteCmd);

        selectNotes(pasteCmd->getPastedIndices());
        update();
        qDebug() << "Pasted" << clipboard.size() << "notes at time" << targetTime;
    }
}

void ScoreCanvas::performDelete()
{
    if (!selectedNoteIndices.isEmpty()) {
        undoStack->push(new DeleteMultipleNotesCommand(&phrase, selectedNoteIndices, this));
        deselectAll();
        update();
    }
}

void ScoreCanvas::performSelectAll()
{
    const QVector<Note> &notes = phrase.getNotes();
    QList<int> trackNoteIndices;

    for (int i = 0; i < notes.size(); ++i) {
        if (notes[i].getTrackIndex() == activeTrackIndex) {
            trackNoteIndices.append(i);
        }
    }

    if (!trackNoteIndices.isEmpty()) {
        selectNotes(trackNoteIndices);
        update();
        qDebug() << "Selected all" << trackNoteIndices.size() << "notes in track" << activeTrackIndex;
    }
}

void ScoreCanvas::performSelectToEnd()
{
    if (selectedNoteIndices.isEmpty()) return;

    const QVector<Note> &notes = phrase.getNotes();

    // Find the earliest start time among currently selected notes (anchor)
    double anchorTime = std::numeric_limits<double>::max();
    for (int idx : selectedNoteIndices) {
        if (idx >= 0 && idx < notes.size())
            anchorTime = qMin(anchorTime, notes[idx].getStartTime());
    }

    // Select all notes on the active track at or after the anchor time
    QVector<int> newSelection;
    for (int i = 0; i < notes.size(); ++i) {
        if (notes[i].getTrackIndex() == activeTrackIndex &&
            notes[i].getStartTime() >= anchorTime) {
            newSelection.append(i);
        }
    }

    if (!newSelection.isEmpty()) {
        selectNotes(newSelection);
        update();
    }
}

void ScoreCanvas::performRetrograde()
{
    if (selectedNoteIndices.isEmpty()) return;

    const QVector<Note> &notes = phrase.getNotes();
    QVector<Note> selectedNotes;
    for (int index : selectedNoteIndices) {
        if (index >= 0 && index < notes.size()) {
            selectedNotes.append(notes[index]);
        }
    }

    RetrogradeNotesCommand *cmd = new RetrogradeNotesCommand(
        &phrase, selectedNotes, pasteTargetTime, activeTrackIndex, this);
    undoStack->push(cmd);

    selectNotes(cmd->getInsertedIndices());
    update();
    qDebug() << "Retrograde:" << selectedNotes.size() << "notes at time" << pasteTargetTime;
}

void ScoreCanvas::transposeSelectedNotesByDegrees(int steps)
{
    if (selectedNoteIndices.isEmpty() || steps == 0)
        return;

    const QVector<Note> &notes = phrase.getNotes();
    QUndoCommand *macro = new QUndoCommand(
        steps > 0 ? "Transpose Up" : "Transpose Down");
    int count = 0;

    for (int idx : selectedNoteIndices) {
        if (idx < 0 || idx >= notes.size()) continue;
        const Note &n = notes[idx];

        const Scale scale    = getScaleAtTime(n.getStartTime());
        const double baseFreq = getBaseFrequencyAtTime(n.getStartTime());
        const int degCount = scale.getDegreeCount();
        if (degCount <= 0 || baseFreq <= 0.0 || n.getPitchHz() <= 0.0) continue;

        // Find nearest (octave, scale-degree) by minimizing log2 distance
        const double logPitch = std::log2(n.getPitchHz() / baseFreq);
        double minDist = 1e9;
        int bestDeg = 0, bestOct = 0;
        for (int oct = -5; oct <= 8; ++oct) {
            for (int d = 0; d < degCount; ++d) {
                double ratio = scale.getRatio(d);
                if (ratio <= 0.0) continue;
                double logRatio = std::log2(ratio) + oct;
                double dist = std::abs(logPitch - logRatio);
                if (dist < minDist) { minDist = dist; bestDeg = d; bestOct = oct; }
            }
        }

        // Walk N steps from (bestDeg, bestOct)
        int total = bestDeg + steps;
        int newOct = bestOct + total / degCount;
        int newDeg = total % degCount;
        if (newDeg < 0) { newDeg += degCount; newOct--; }
        double newRatio = scale.getRatio(newDeg);
        if (newRatio <= 0.0) continue;
        double newPitch = baseFreq * newRatio * std::pow(2.0, newOct);

        // Scale pitch curve Hz values if present
        Curve newPitchCurve = n.getPitchCurve();
        if (n.hasPitchCurve()) {
            newPitchCurve.clearPoints();
            const double factor = newPitch / n.getPitchHz();
            for (const Curve::Point &pt : n.getPitchCurve().getPoints())
                newPitchCurve.addPoint(pt.time, pt.value * factor, pt.pressure);
        }

        new MoveNoteCommand(&phrase, idx,
                            n.getStartTime(), n.getPitchHz(),
                            n.getStartTime(), newPitch,
                            n.getPitchCurve(), newPitchCurve,
                            n.hasPitchCurve(), this, macro);
        ++count;
    }

    if (count > 0) {
        undoStack->push(macro);
        update();
        qDebug() << "Transposed" << count << "note(s) by" << steps << "scale degree(s)";
    } else {
        delete macro;
    }
}

// ============================================================================
// Context Menu and Variation Assignment
// ============================================================================

void ScoreCanvas::showNoteContextMenu(const QPoint &globalPos)
{
    QMenu menu;

    // Quantize to Scale action (existing functionality)
    QAction *quantizeAction = menu.addAction("Quantize to Scale");
    connect(quantizeAction, &QAction::triggered, this, &ScoreCanvas::snapSelectedNotesToScale);

    // Make Continuous action (convert discrete notes to free-pitch continuous)
    QAction *makeContinuousAction = menu.addAction("Make Continuous");
    connect(makeContinuousAction, &QAction::triggered, this, &ScoreCanvas::makeSelectedNotesContinuous);

    // Make Discrete action (convert continuous notes back to scale-snapped discrete)
    QAction *makeDiscreteAction = menu.addAction("Make Discrete");
    connect(makeDiscreteAction, &QAction::triggered, this, &ScoreCanvas::makeSelectedNotesDiscrete);

    menu.addSeparator();

    // Apply Variation submenu
    QMenu *varMenu = menu.addMenu("Apply Variation");

    // Base sounit (variation index 0)
    QAction *baseAction = varMenu->addAction("Base Sounit");
    connect(baseAction, &QAction::triggered, this, [this]() {
        applyVariationToSelection(0);
    });

    // Add variations if we have a current track
    if (m_currentTrack && m_currentTrack->getVariationCount() > 0) {
        varMenu->addSeparator();

        for (int i = 1; i <= m_currentTrack->getVariationCount(); ++i) {
            QString name = m_currentTrack->getVariationName(i);
            QAction *varAction = varMenu->addAction(QString("%1: %2").arg(i).arg(name));
            int varIndex = i;  // Capture by value
            connect(varAction, &QAction::triggered, this, [this, varIndex]() {
                applyVariationToSelection(varIndex);
            });
        }
    }

    menu.addSeparator();

    // Rhythmic Easing action (only meaningful with 2+ notes)
    QAction *easingAction = menu.addAction("Easing...");
    easingAction->setEnabled(selectedNoteIndices.size() >= 2);
    connect(easingAction, &QAction::triggered, this, [this]() {
        if (selectedNoteIndices.size() < 2) return;

        EasingDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            Easing easing = dialog.getEasing();
            // Convert dialog anchor mode to command anchor mode
            auto anchorMode = static_cast<ApplyRhythmicEasingCommand::AnchorMode>(
                dialog.getAnchorMode());
            double weight = dialog.getWeight();
            undoStack->push(new ApplyRhythmicEasingCommand(
                &phrase, selectedNoteIndices, easing, anchorMode, weight, this));
        }
    });

    // Dynamics Curve action (enabled with 1+ notes)
    QAction *dynamicsAction = menu.addAction("Apply Dynamics Curve...");
    dynamicsAction->setEnabled(selectedNoteIndices.size() >= 1);
    connect(dynamicsAction, &QAction::triggered, this, &ScoreCanvas::showDynamicsCurveDialog);

    // Scale Dynamics action (enabled with 1+ notes)
    QAction *scaleDynamicsAction = menu.addAction("Scale Dynamics...");
    scaleDynamicsAction->setEnabled(selectedNoteIndices.size() >= 1);
    connect(scaleDynamicsAction, &QAction::triggered, this, &ScoreCanvas::showScaleDynamicsDialog);

    // Expressive Curve action — enabled when every selected note's variation
    // (or the base sounit for notes on the base) declares at least one curve
    // name in common. The dialog's dropdown is populated from that intersection,
    // so mixed-variation selections only expose curves all of them share.
    {
        QStringList commonNames = selectionCommonExpressiveCurveNames();
        QAction *expressiveAction = menu.addAction("Apply Expressive Curve...");
        expressiveAction->setEnabled(!commonNames.isEmpty());
        connect(expressiveAction, &QAction::triggered,
                this, &ScoreCanvas::showExpressiveCurveApplyDialog);
    }

    // EQ Curve action — enabled when every selected note's variation has
    // all 10 "band 1".."band 10" expressive curves wired up (for the 10-Band EQ).
    {
        QStringList bandNames = selectionMatchingBandCurveNames();
        QAction *eqAction = menu.addAction("Apply EQ Curve...");
        eqAction->setEnabled(!bandNames.isEmpty());
        connect(eqAction, &QAction::triggered,
                this, &ScoreCanvas::showEqCurveDialog);
    }

    menu.addSeparator();

    // Legato articulation (only meaningful with 2+ notes)
    QAction *legatoAction = menu.addAction("Link as Legato");
    legatoAction->setEnabled(selectedNoteIndices.size() >= 2);
    connect(legatoAction, &QAction::triggered, this, [this]() {
        if (selectedNoteIndices.size() >= 2) {
            undoStack->push(new LinkAsLegatoCommand(&phrase, selectedNoteIndices, this));
        }
    });

    QAction *staccatoAction = menu.addAction("Unlink (Staccato)");
    // Enable if any selected note is a segmented (merged legato) note
    const QVector<Note> &allNotes = phrase.getNotes();
    bool hasSegmentedNote = false;
    for (int idx : selectedNoteIndices) {
        if (idx >= 0 && idx < allNotes.size() &&
            (allNotes[idx].isQuantized() || allNotes[idx].isLinkedAsLegato()) && allNotes[idx].getSegmentCount() > 1) {
            hasSegmentedNote = true;
            break;
        }
    }
    staccatoAction->setEnabled(hasSegmentedNote);
    connect(staccatoAction, &QAction::triggered, this, [this]() {
        if (!selectedNoteIndices.isEmpty()) {
            undoStack->push(new UnlinkLegatoCommand(&phrase, selectedNoteIndices, this));
        }
    });

    menu.addSeparator();

    // Scale Timing action (tempo change for selected notes)
    QAction *scaleTimingAction = menu.addAction("Scale Timing...");
    scaleTimingAction->setEnabled(selectedNoteIndices.size() >= 1);
    connect(scaleTimingAction, &QAction::triggered, this, [this]() {
        if (selectedNoteIndices.isEmpty()) return;

        // Get the current tempo at the earliest selected note's position
        const QVector<Note> &notes = phrase.getNotes();
        double earliestTime = std::numeric_limits<double>::max();
        for (int idx : selectedNoteIndices) {
            if (idx >= 0 && idx < notes.size()) {
                earliestTime = qMin(earliestTime, notes[idx].getStartTime());
            }
        }

        double currentTempo = getTempoAtTime(earliestTime);

        TempoChangeDialog dialog(currentTempo, this);
        if (dialog.exec() == QDialog::Accepted) {
            double proportion = dialog.getProportion();
            if (qFuzzyCompare(proportion, 1.0)) {
                // No change needed
                return;
            }
            undoStack->push(new ScaleTimingCommand(&phrase, selectedNoteIndices, proportion, this));
        }
    });

    menu.exec(globalPos);
}

void ScoreCanvas::applyVariationToSelection(int variationIndex)
{
    if (selectedNoteIndices.isEmpty())
        return;

    const QVector<Note> &notes = phrase.getNotes();

    QVector<EditNotePropertyCommand::NoteChange> changes;
    changes.reserve(selectedNoteIndices.size());
    for (int idx : selectedNoteIndices) {
        if (idx < 0 || idx >= notes.size())
            continue;
        const int oldIdx = notes[idx].getVariationIndex();
        if (oldIdx == variationIndex)
            continue;
        EditNotePropertyCommand::NoteChange c;
        c.index    = idx;
        c.oldValue = static_cast<double>(oldIdx);
        c.newValue = static_cast<double>(variationIndex);
        changes.append(c);
    }

    if (changes.isEmpty())
        return;

    auto *cmd = new EditNotePropertyCommand(
        &phrase, changes, EditNotePropertyCommand::VariationIndex, this);
    cmd->setText(QString("Apply variation %1").arg(variationIndex));
    undoStack->push(cmd);

    update();
}

void ScoreCanvas::showDynamicsCurveDialog()
{
    if (selectedNoteIndices.isEmpty()) {
        qDebug() << "ScoreCanvas::showDynamicsCurveDialog - No notes selected";
        return;
    }

    const QVector<Note> &notes = phrase.getNotes();

    // Calculate selection time span
    double startTime = std::numeric_limits<double>::max();
    double endTime = std::numeric_limits<double>::lowest();

    for (int idx : selectedNoteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            double noteStart = notes[idx].getStartTime();
            double noteEnd = noteStart + notes[idx].getDuration();
            startTime = qMin(startTime, noteStart);
            endTime = qMax(endTime, noteEnd);
        }
    }

    double timeSpanMs = endTime - startTime;

    // Show the dialog
    DynamicsCurveDialog dialog(selectedNoteIndices.size(), timeSpanMs, this);

    if (dialog.exec() == QDialog::Accepted) {
        QVector<EnvelopePoint> curve = dialog.getCurve();

        // Create and push the undo command
        double weight = dialog.getWeight();
        bool perNote = dialog.getPerNoteMode();
        undoStack->push(new ApplyDynamicsCurveCommand(
            &phrase, selectedNoteIndices, curve, weight, perNote, this));

        qDebug() << "ScoreCanvas: Applied dynamics curve to"
                 << selectedNoteIndices.size() << "notes";
    }
}

void ScoreCanvas::showScaleDynamicsDialog()
{
    if (selectedNoteIndices.isEmpty()) return;

    // Build a minimal dialog: a single slider (10%–200%, default 100%)
    QDialog dialog(this);
    dialog.setWindowTitle("Scale Dynamics");
    dialog.setFixedWidth(320);

    QVBoxLayout *vbox = new QVBoxLayout(&dialog);
    vbox->setContentsMargins(16, 16, 16, 16);
    vbox->setSpacing(10);

    QLabel *titleLabel = new QLabel(
        QString("Scale dynamics of %1 note%2:")
            .arg(selectedNoteIndices.size())
            .arg(selectedNoteIndices.size() == 1 ? "" : "s"),
        &dialog);
    vbox->addWidget(titleLabel);

    QHBoxLayout *sliderRow = new QHBoxLayout();
    QSlider *slider = new QSlider(Qt::Horizontal, &dialog);
    slider->setRange(10, 200);   // 10% to 200%
    slider->setValue(100);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setTickInterval(10);

    QLabel *valueLabel = new QLabel("100%", &dialog);
    valueLabel->setMinimumWidth(42);
    valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    sliderRow->addWidget(slider);
    sliderRow->addWidget(valueLabel);
    vbox->addLayout(sliderRow);

    QLabel *hintLabel = new QLabel("< 100%: quieter   |   > 100%: louder (capped at max)", &dialog);
    hintLabel->setStyleSheet("color: #666; font-size: 10px;");
    hintLabel->setAlignment(Qt::AlignCenter);
    vbox->addWidget(hintLabel);

    QObject::connect(slider, &QSlider::valueChanged, [valueLabel](int v) {
        valueLabel->setText(QString::number(v) + "%");
    });

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    vbox->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    double factor = slider->value() / 100.0;
    undoStack->push(new ScaleDynamicsCommand(&phrase, selectedNoteIndices, factor, this));

    qDebug() << "ScoreCanvas: Scaled dynamics of" << selectedNoteIndices.size()
             << "notes by" << factor;
}

QStringList ScoreCanvas::selectionCommonExpressiveCurveNames() const
{
    if (selectedNoteIndices.isEmpty() || !m_currentTrack) return {};

    const QVector<Note> &notes = phrase.getNotes();

    // Collect the set of distinct variation indices under the selection.
    QSet<int> variationIndices;
    for (int idx : selectedNoteIndices) {
        if (idx < 0 || idx >= notes.size()) continue;
        variationIndices.insert(notes[idx].getVariationIndex());
    }
    if (variationIndices.isEmpty()) return {};

    // Use the canvas of *some* variation as the reference for ordering, then
    // intersect with the curve name list of every other variation in play.
    QStringList result;
    bool first = true;
    for (int varIdx : variationIndices) {
        Canvas *c = m_currentTrack->getCanvasForVariation(varIdx);
        QStringList names = c ? c->getExpressiveCurveNames() : QStringList{};
        if (first) {
            result = names;
            first = false;
        } else {
            QSet<QString> keep(names.begin(), names.end());
            result.erase(std::remove_if(result.begin(), result.end(),
                                        [&keep](const QString &n) { return !keep.contains(n); }),
                         result.end());
        }
        if (result.isEmpty()) break;
    }
    return result;
}

QStringList ScoreCanvas::selectionMatchingBandCurveNames() const
{
    QStringList common = selectionCommonExpressiveCurveNames();
    if (common.isEmpty()) return {};

    // For each band index 1..10, find a case/whitespace-insensitive match of
    // "band N" in the shared names. If any band is missing, return empty.
    QStringList result;
    for (int b = 1; b <= 10; ++b) {
        QString expected = QString("band %1").arg(b);
        QString match;
        for (const QString &n : common) {
            QString normalized = n.simplified().toLower();
            if (normalized == expected) { match = n; break; }
        }
        if (match.isEmpty()) return {};
        result.append(match);
    }
    return result;
}

void ScoreCanvas::showEqCurveDialog()
{
    if (selectedNoteIndices.isEmpty()) return;

    QStringList bandNames = selectionMatchingBandCurveNames();
    if (bandNames.size() != 10) {
        QMessageBox::information(this, "Apply EQ Curve",
            "This feature needs every selected note's sounit (or variation) to "
            "expose expressive curves named \"band 1\" through \"band 10\" — "
            "one per EQ band.\n\n"
            "Add those names on the Envelope container(s) driving the 10-Band EQ.");
        return;
    }

    const QVector<Note> &notes = phrase.getNotes();
    double startTime = std::numeric_limits<double>::max();
    double endTime   = std::numeric_limits<double>::lowest();
    for (int idx : selectedNoteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            double noteStart = notes[idx].getStartTime();
            double noteEnd   = noteStart + notes[idx].getDuration();
            startTime = qMin(startTime, noteStart);
            endTime   = qMax(endTime,   noteEnd);
        }
    }
    double timeSpanMs = endTime - startTime;

    EqCurveDialog dialog(selectedNoteIndices.size(), timeSpanMs, this);

    if (dialog.exec() == QDialog::Accepted) {
        QVector<EnvelopePoint> shape     = dialog.getShapeCurve();
        QVector<EnvelopePoint> intensity = dialog.getIntensityCurve();
        double weight  = dialog.getWeight();
        bool   perNote = dialog.getPerNoteMode();

        undoStack->push(new ApplyEqCurveCommand(
            &phrase, selectedNoteIndices, bandNames,
            shape, intensity, weight, perNote, this));

        // Activate the EQ pseudo-entry so the score canvas shows the freshly
        // applied curves. Emitting expressiveCurveApplied drives the toolbar
        // dropdown to switch to "EQ Curve", which in turn re-triggers
        // setActiveExpressiveCurveIndex with the proper combo index.
        emit expressiveCurveApplied(QStringLiteral("EQ Curve"));

        qDebug() << "ScoreCanvas: Applied EQ curve to"
                 << selectedNoteIndices.size() << "notes";
    }
}

void ScoreCanvas::showExpressiveCurveApplyDialog()
{
    if (selectedNoteIndices.isEmpty()) return;

    // Curve names come from the sounit (base or variation) each selected note
    // is attached to. Only names shared by *all* selected notes' sounits are
    // offered, so the chosen curve can be applied to every note in one go.
    QStringList curveNames = selectionCommonExpressiveCurveNames();

    if (curveNames.isEmpty()) {
        QMessageBox::information(this, "Apply Expressive Curve",
            "No expressive curve is shared by every selected note's sounit.\n"
            "Add curve names in the Envelope container inspector of each "
            "variation (or narrow the selection).");
        return;
    }

    const QVector<Note> &notes = phrase.getNotes();

    // Selection time span (for dialog's display only — same shape applies to every note regardless)
    double startTime = std::numeric_limits<double>::max();
    double endTime   = std::numeric_limits<double>::lowest();
    for (int idx : selectedNoteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            double noteStart = notes[idx].getStartTime();
            double noteEnd   = noteStart + notes[idx].getDuration();
            startTime = qMin(startTime, noteStart);
            endTime   = qMax(endTime,   noteEnd);
        }
    }
    double timeSpanMs = endTime - startTime;

    ExpressiveCurveApplyDialog dialog(selectedNoteIndices.size(), timeSpanMs, curveNames, this);
    // Ghost context (R6): shape a curve AGAINST the note's other curves.
    if (!selectedNoteIndices.isEmpty() &&
        selectedNoteIndices.first() < phrase.getNotes().size()) {
        dialog.setGhostCurves(buildGhostCurves(phrase.getNotes()[selectedNoteIndices.first()],
                                               QString()));
    }

    if (dialog.exec() == QDialog::Accepted) {
        QString targetName = dialog.getSelectedCurveName();
        QVector<EnvelopePoint> curve = dialog.getCurve();
        double weight = dialog.getWeight();
        bool perNote = dialog.getPerNoteMode();

        undoStack->push(new ApplyExpressiveCurveToSelectionCommand(
            &phrase, selectedNoteIndices, targetName, curve, weight, perNote, this));

        // Deliberately NOT activating the applied curve on the note body
        // (reverted 2026-08-26): the "Show curve" combo is the only thing that
        // decides the body shape. Auto-switching made the dialog feel like it
        // stole the note's silhouette. The signal below still lets KalaMain
        // refresh the note-inspector dropdown.
        emit expressiveCurveApplied(targetName);

        qDebug() << "ScoreCanvas: Applied expressive curve '" << targetName
                 << "' to" << selectedNoteIndices.size() << "notes";
    }
}

void ScoreCanvas::editNoteCurve(int noteIndex)
{
    const QVector<Note> &notes = phrase.getNotes();
    if (noteIndex < 0 || noteIndex >= notes.size()) return;

    const Note &note = notes[noteIndex];
    int activeCurveIndex = resolveActiveCurveIndex(note);

    // Fallback: convert a Note's sampled Curve to EnvelopePoints for the dialog canvas
    auto curveToEnvelopePoints = [](const Curve &curve) -> QVector<EnvelopePoint> {
        QVector<EnvelopePoint> points;
        for (const Curve::Point &pt : curve.getPoints()) {
            points.append(EnvelopePoint(pt.time, pt.value, 0));
        }
        if (points.isEmpty()) {
            points.append(EnvelopePoint(0.0, 0.8, 0));
            points.append(EnvelopePoint(1.0, 0.8, 0));
        }
        return points;
    };

    double duration = note.getDuration();

    // ---- Edit EQ curve ----
    // When the "EQ Curve" pseudo-entry is active, route double-click to the
    // EqCurveDialog with shape + intensity reconstructed from the note's 10
    // band envelopes. All bands share the same time-shape scaled by their own
    // delta from 0.5, so the intensity curve can be recovered from the band
    // with the largest peak deviation.
    if (m_activeExpressiveCurveName == QStringLiteral("EQ Curve")) {
        QStringList bandNames = selectionMatchingBandCurveNames();
        if (bandNames.size() != 10) {
            QMessageBox::information(this, "Edit EQ Curve",
                "This requires every selected note's sounit (or variation) to "
                "expose \"band 1\" through \"band 10\" as expressive curves.");
            return;
        }

        QVector<EnvelopePoint> shapeCurve;
        QVector<EnvelopePoint> intensityCurve;

        // Preferred path: the note stored the original author shape/intensity
        // when the EQ curve was applied, so editing loads the exact 2–3 point
        // curves instead of the dense per-band samples.
        const bool hasStoredShape =
            note.hasEnvelopeControlPoints(QStringLiteral("EQ Shape"));
        const bool hasStoredIntensity =
            note.hasEnvelopeControlPoints(QStringLiteral("EQ Intensity"));

        if (hasStoredShape && hasStoredIntensity) {
            shapeCurve     = note.getEnvelopeControlPoints(QStringLiteral("EQ Shape"));
            intensityCurve = note.getEnvelopeControlPoints(QStringLiteral("EQ Intensity"));
        } else {
            // Legacy fallback: reconstruct shape + intensity from the per-band
            // envelope points (older EQ curves applied before we persisted the
            // originals). Evaluates a stored envelope at normalised time t.
            auto evalEnv = [](const QVector<EnvelopePoint> &env, double t) -> double {
                if (env.isEmpty()) return 0.5;
                if (env.size() == 1) return env[0].value;
                if (t <= env.first().time) return env.first().value;
                if (t >= env.last().time)  return env.last().value;
                for (int i = 0; i < env.size() - 1; ++i) {
                    if (t >= env[i].time && t <= env[i + 1].time) {
                        double dt = env[i + 1].time - env[i].time;
                        if (dt <= 0.0) return env[i + 1].value;
                        double segT = (t - env[i].time) / dt;
                        if (env[i].curveType == 1) {
                            double s = (1.0 - std::cos(segT * M_PI)) * 0.5;
                            return env[i].value + s * (env[i + 1].value - env[i].value);
                        } else if (env[i].curveType == 2) {
                            return env[i].value;
                        } else {
                            return env[i].value + segT * (env[i + 1].value - env[i].value);
                        }
                    }
                }
                return env.last().value;
            };

            // Find the band with the largest |peak - 0.5| across its control
            // points, and the time where that peak occurs.
            int bestBand = -1;
            double bestAbsDelta = 0.0;
            double bestSignedDelta = 0.0;
            double bestTime = 0.0;
            for (int b = 0; b < 10; ++b) {
                if (!note.hasEnvelopeControlPoints(bandNames[b])) continue;
                QVector<EnvelopePoint> eps = note.getEnvelopeControlPoints(bandNames[b]);
                for (const EnvelopePoint &ep : eps) {
                    double d = ep.value - 0.5;
                    if (std::abs(d) > bestAbsDelta) {
                        bestAbsDelta = std::abs(d);
                        bestSignedDelta = d;
                        bestBand = b;
                        bestTime = ep.time;
                    }
                }
            }

            auto bandValueAt = [&](int b, double t) -> double {
                if (!note.hasEnvelopeControlPoints(bandNames[b])) return 0.5;
                return evalEnv(note.getEnvelopeControlPoints(bandNames[b]), t);
            };
            shapeCurve.append(EnvelopePoint(0.0, bandValueAt(0, bestTime), 0));
            for (int b = 0; b < 10; ++b) {
                double t = (b + 0.5) / 10.0;
                shapeCurve.append(EnvelopePoint(t, bandValueAt(b, bestTime), 1));
            }
            shapeCurve.append(EnvelopePoint(1.0, bandValueAt(9, bestTime), 0));

            if (bestBand < 0 || bestAbsDelta < 1e-6) {
                intensityCurve.append(EnvelopePoint(0.0, 0.0, 0));
                intensityCurve.append(EnvelopePoint(1.0, 0.0, 0));
            } else {
                QVector<EnvelopePoint> bestEnv = note.getEnvelopeControlPoints(bandNames[bestBand]);
                for (const EnvelopePoint &ep : bestEnv) {
                    double intensity = (ep.value - 0.5) / bestSignedDelta;
                    intensity = qBound(0.0, intensity, 1.0);
                    intensityCurve.append(EnvelopePoint(ep.time, intensity, ep.curveType));
                }
                if (intensityCurve.isEmpty()) {
                    intensityCurve.append(EnvelopePoint(0.0, 0.0, 0));
                    intensityCurve.append(EnvelopePoint(1.0, 0.0, 0));
                }
            }
        }

        EqCurveDialog dialog(selectedNoteIndices.size(), duration, this);
        dialog.setWindowTitle("Edit EQ Curve");
        dialog.setInitialShapeCurve(shapeCurve);
        dialog.setInitialIntensityCurve(intensityCurve);

        if (dialog.exec() == QDialog::Accepted) {
            QVector<EnvelopePoint> shape     = dialog.getShapeCurve();
            QVector<EnvelopePoint> intensity = dialog.getIntensityCurve();
            double weight  = dialog.getWeight();
            bool   perNote = dialog.getPerNoteMode();

            undoStack->push(new ApplyEqCurveCommand(
                &phrase, selectedNoteIndices, bandNames,
                shape, intensity, weight, perNote, this));

            qDebug() << "ScoreCanvas: Edited EQ curve on"
                     << selectedNoteIndices.size() << "notes (double-click)";
        }
        return;
    }

    if (activeCurveIndex == 0) {
        // ---- Edit dynamics curve ----
        DynamicsCurveDialog dialog(selectedNoteIndices.size(), duration, this);
        dialog.setWindowTitle("Edit Dynamics Curve");

        // Prefer original envelope control points; fall back to sampled curve
        QVector<EnvelopePoint> initialPoints;
        if (note.hasEnvelopeControlPoints(QStringLiteral("Dynamics")))
            initialPoints = note.getEnvelopeControlPoints(QStringLiteral("Dynamics"));
        else
            initialPoints = curveToEnvelopePoints(note.getDynamicsCurve());
        dialog.setInitialCurve(initialPoints);
        dialog.setGhostCurves(buildGhostCurves(note, QStringLiteral("Dynamics")));

        if (dialog.exec() == QDialog::Accepted) {
            QVector<EnvelopePoint> curve = dialog.getCurve();
            double weight = dialog.getWeight();
            bool perNote = dialog.getPerNoteMode();
            undoStack->push(new ApplyDynamicsCurveCommand(
                &phrase, selectedNoteIndices, curve, weight, perNote, this));

            qDebug() << "ScoreCanvas: Edited dynamics curve on"
                     << selectedNoteIndices.size() << "notes (double-click)";
        }
    } else {
        // ---- Edit expressive curve ----
        QString curveName = note.getExpressiveCurveName(activeCurveIndex);
        QStringList curveNames = selectionCommonExpressiveCurveNames();

        if (curveNames.isEmpty()) {
            QMessageBox::information(this, "Edit Expressive Curve",
                "No expressive curve is shared by every selected note's sounit.");
            return;
        }

        ExpressiveCurveApplyDialog dialog(selectedNoteIndices.size(), duration,
                                          curveNames, this);
        dialog.setWindowTitle("Edit Expressive Curve");
        dialog.setSelectedCurveName(curveName);

        // Prefer original envelope control points; fall back to sampled curve
        QVector<EnvelopePoint> initialPoints;
        if (note.hasEnvelopeControlPoints(curveName))
            initialPoints = note.getEnvelopeControlPoints(curveName);
        else
            initialPoints = curveToEnvelopePoints(note.getExpressiveCurve(activeCurveIndex));
        dialog.setInitialCurve(initialPoints);
        dialog.setGhostCurves(buildGhostCurves(note, QString()));

        if (dialog.exec() == QDialog::Accepted) {
            QString targetName = dialog.getSelectedCurveName();
            QVector<EnvelopePoint> curve = dialog.getCurve();
            double weight = dialog.getWeight();
            bool perNote = dialog.getPerNoteMode();

            undoStack->push(new ApplyExpressiveCurveToSelectionCommand(
                &phrase, selectedNoteIndices, targetName, curve, weight, perNote, this));

            // No auto-activation - see showExpressiveCurveApplyDialog.
            emit expressiveCurveApplied(targetName);

            qDebug() << "ScoreCanvas: Edited expressive curve '" << targetName
                     << "' on" << selectedNoteIndices.size() << "notes (double-click)";
        }
    }
}

// ============================================================================
// Expressive Curve Management
// ============================================================================

void ScoreCanvas::setActiveExpressiveCurveIndex(int index, const QString &name)
{
    m_activeExpressiveCurveIndex = index;
    // When a name is supplied use it; when resetting to 0 without a name, clear to "Dynamics"
    if (!name.isEmpty())
        m_activeExpressiveCurveName = name;
    else
        m_activeExpressiveCurveName = (index == 0) ? QStringLiteral("Dynamics") : QString();
    update();
}

int ScoreCanvas::resolveActiveCurveIndex(const Note &note) const
{
    // Index 0 / "Dynamics" / unset → always dynamics
    if (m_activeExpressiveCurveName.isEmpty() ||
        m_activeExpressiveCurveName == QStringLiteral("Dynamics")) {
        return 0;
    }
    // "EQ Curve" pseudo-entry: render the first band curve the note actually
    // carries. All 10 bands share the same time-shape (intensity curve) scaled
    // by their own delta, so any band visually conveys the intensity over time.
    // Notes without EQ applied fall back to dynamics rendering.
    if (m_activeExpressiveCurveName == QStringLiteral("EQ Curve")) {
        for (int i = 1; i < note.getExpressiveCurveCount(); ++i) {
            QString n = note.getExpressiveCurveName(i).simplified().toLower();
            if (n.startsWith(QStringLiteral("band "))) {
                int num = n.mid(5).toInt();
                if (num >= 1 && num <= 10) return i;
            }
        }
        return 0;
    }
    // Search the note for a curve that matches the active name
    for (int i = 1; i < note.getExpressiveCurveCount(); ++i) {
        if (note.getExpressiveCurveName(i) == m_activeExpressiveCurveName)
            return i;
    }
    // Note doesn't carry this curve — fall back to dynamics
    return 0;
}

void ScoreCanvas::removeExpressiveCurveFromSelection(int curveIndex)
{
    if (selectedNoteIndices.isEmpty() || curveIndex < 1) return;
    undoStack->push(new RemoveExpressiveCurveCommand(
        &phrase, selectedNoteIndices, curveIndex, this));
}

void ScoreCanvas::removeExpressiveCurveByName(const QString &name)
{
    if (name.isEmpty()) return;
    // Scope to the current selection so the inspector's Delete button only strips
    // the curve from the notes the user has selected, not every note in the phrase.
    undoStack->push(new RemoveNamedExpressiveCurveCommand(
        &phrase, name, selectedNoteIndices, this));
}

void ScoreCanvas::applyNamedCurveToSelection(const QString &name, const Curve &curve)
{
    if (selectedNoteIndices.isEmpty()) return;
    undoStack->push(new ApplyNamedCurveCommand(
        &phrase, selectedNoteIndices, name, curve, this));
}

// ============================================================================
// Segment Editing Helpers
// ============================================================================

int ScoreCanvas::findSegmentAtPosition(const QPoint &pos, int noteIndex) const
{
    const QVector<Note> &notes = phrase.getNotes();
    if (noteIndex < 0 || noteIndex >= notes.size()) return -1;

    const Note &note = notes[noteIndex];
    const auto &segments = note.getSegments();

    for (int i = 0; i < segments.size(); ++i) {
        QRect segRect = getSegmentRect(note, i);
        if (segRect.contains(pos)) {
            return i;
        }
    }
    return -1;
}

QRect ScoreCanvas::getSegmentRect(const Note &note, int segmentIndex) const
{
    const auto &segments = note.getSegments();
    if (segmentIndex < 0 || segmentIndex >= segments.size()) return QRect();

    const Segment &seg = segments[segmentIndex];
    double noteDuration = note.getDuration();

    // Convert segment times to absolute times
    double segStartMs = note.getStartTime() + seg.startTime * noteDuration;
    double segEndMs = note.getStartTime() + seg.endTime * noteDuration;

    int x1 = timeToPixel(segStartMs);
    int x2 = timeToPixel(segEndMs);
    int y = frequencyToPixel(seg.pitchHz);

    // Find maximum dynamics value in curve for blob height (similar to drawNote)
    const Curve &dynamicsCurve = note.getDynamicsCurve();
    double maxDynamics = 0.0;
    for (int i = 0; i <= 20; ++i) {
        double t = i / 20.0;
        double value = dynamicsCurve.valueAt(t);
        if (value > maxDynamics) maxDynamics = value;
    }

    int blobHeight = static_cast<int>(20 + maxDynamics * 60);

    return QRect(x1, y - blobHeight/2, x2 - x1, blobHeight);
}

void ScoreCanvas::drawSegmentOverlay(QPainter &painter, const Note &note)
{
    const auto &segments = note.getSegments();

    for (int i = 0; i < segments.size(); ++i) {
        QRect segRect = getSegmentRect(note, i);

        // Draw segment boundary lines (vertical dashed lines)
        painter.setPen(QPen(Qt::white, 2, Qt::DashLine));

        // Draw left boundary (except for first segment)
        if (i > 0) {
            painter.drawLine(segRect.left(), segRect.top() - 5, segRect.left(), segRect.bottom() + 5);
        }

        // Highlight selected segment
        if (i == selectedSegmentIndex) {
            painter.setBrush(QColor(255, 255, 255, 50));
            painter.setPen(QPen(Qt::yellow, 3));
            painter.drawRect(segRect.adjusted(-2, -2, 2, 2));

            // Draw segment pitch label
            painter.setPen(Qt::white);
            QFont font = painter.font();
            font.setPointSize(9);
            font.setBold(true);
            painter.setFont(font);
            QString pitchLabel = QString::number(segments[i].pitchHz, 'f', 1) + " Hz";
            painter.drawText(segRect.left() + 5, segRect.top() - 8, pitchLabel);
        } else {
            // Non-selected segments get a subtle outline
            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(QColor(255, 255, 255, 100), 1, Qt::DotLine));
            painter.drawRect(segRect);
        }
    }

    // Draw mode indicator at top of note
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    int noteX = timeToPixel(note.getStartTime());
    int noteY = frequencyToPixel(note.getPitchAt(0.5));
    painter.drawText(noteX, noteY - 50, "Segment Mode (Esc to exit)");
}

void ScoreCanvas::deleteSelectedSegment()
{
    if (!segmentEditingMode || selectedSegmentIndex < 0) return;

    QVector<Note> &notes = phrase.getNotes();
    if (segmentEditingNoteIndex < 0 || segmentEditingNoteIndex >= notes.size()) return;

    Note &note = notes[segmentEditingNoteIndex];
    const auto &segments = note.getSegments();

    if (segments.size() <= 1) {
        // Only one segment - delete the whole note
        undoStack->push(new DeleteNoteCommand(&phrase, segmentEditingNoteIndex, this));
        exitSegmentEditingMode();
        return;
    }

    // Split the note into two notes (before and after deleted segment)
    undoStack->push(new SplitNoteAtSegmentCommand(&phrase, segmentEditingNoteIndex,
                                                   selectedSegmentIndex, this));
    exitSegmentEditingMode();
}

void ScoreCanvas::showSegmentContextMenu(const QPoint &globalPos)
{
    if (!segmentEditingMode || selectedSegmentIndex < 0) return;

    QVector<Note> &notes = phrase.getNotes();
    if (segmentEditingNoteIndex < 0 || segmentEditingNoteIndex >= notes.size()) return;

    Note &note = notes[segmentEditingNoteIndex];
    const auto &segments = note.getSegments();
    if (selectedSegmentIndex >= segments.size()) return;

    const Segment &seg = segments[selectedSegmentIndex];

    QMenu menu;

    // Show current pitch
    QAction *pitchLabel = menu.addAction(QString("Pitch: %1 Hz").arg(seg.pitchHz, 0, 'f', 1));
    pitchLabel->setEnabled(false);

    menu.addSeparator();

    // Change pitch submenu - offer nearby scale degrees
    QMenu *pitchMenu = menu.addMenu("Change Pitch");

    // Get scale lines at the note's time position
    double noteStartTime = note.getStartTime() + seg.startTime * note.getDuration();
    Scale activeScale = getScaleAtTime(noteStartTime);
    double activeBaseFreq = getBaseFrequencyAtTime(noteStartTime);
    QVector<ScaleLine> scaleLines = generateScaleLinesForScale(activeScale, activeBaseFreq);

    // Find nearby scale degrees (within 2 octaves)
    for (const ScaleLine &line : scaleLines) {
        // Only show pitches within reasonable range of current pitch
        if (line.frequencyHz > seg.pitchHz * 0.25 && line.frequencyHz < seg.pitchHz * 4.0) {
            QString label = QString("%1 (%2 Hz)").arg(line.noteName).arg(line.frequencyHz, 0, 'f', 1);
            QAction *pitchAction = pitchMenu->addAction(label);
            double newPitch = line.frequencyHz;
            connect(pitchAction, &QAction::triggered, this, [this, newPitch]() {
                if (segmentEditingNoteIndex >= 0 && selectedSegmentIndex >= 0) {
                    QVector<Note> &notes = phrase.getNotes();
                    if (segmentEditingNoteIndex < notes.size()) {
                        undoStack->push(new ChangeSegmentPitchCommand(
                            &phrase, segmentEditingNoteIndex, selectedSegmentIndex, newPitch, this));
                        // Re-detect segments after pitch change
                        notes[segmentEditingNoteIndex].detectSegments();
                        update();
                    }
                }
            });
        }
    }

    menu.addSeparator();

    // Detach as discrete note action
    QAction *detachAction = menu.addAction("Detach as Discrete Note");
    connect(detachAction, &QAction::triggered, this, [this]() {
        if (segmentEditingNoteIndex >= 0 && selectedSegmentIndex >= 0) {
            undoStack->push(new DetachSegmentCommand(
                &phrase, segmentEditingNoteIndex, selectedSegmentIndex, this));
            exitSegmentEditingMode();
        }
    });

    // Delete segment action
    QAction *deleteAction = menu.addAction("Delete Segment");
    connect(deleteAction, &QAction::triggered, this, &ScoreCanvas::deleteSelectedSegment);

    menu.addSeparator();

    // Exit segment mode action
    QAction *exitAction = menu.addAction("Exit Segment Mode");
    connect(exitAction, &QAction::triggered, this, &ScoreCanvas::exitSegmentEditingMode);

    menu.exec(globalPos);
}

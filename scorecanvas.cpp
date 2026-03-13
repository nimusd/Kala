#include "scorecanvas.h"
#include "scorecanvascommands.h"
#include "curve.h"
#include "trackselect.h"
#include "track.h"
#include "easingdialog.h"
#include "dynamicscurvedialog.h"
#include "expressivecurveapplydialog.h"
#include "tempochangedialog.h"
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QMouseEvent>
#include <QTabletEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSet>
#include <QDateTime>
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
    , baseFrequency(25.0)       // Default base frequency: 25 Hz
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
                line.color = getScaleDegreeColor(degree + 1);
                line.noteName = activeScale.getNoteName(degree) + QString::number(octave);
                line.isThicker = (degree == 0 || (degreeCount >= 5 && degree == 4));  // Tonic and Fifth (if scale has 5+ notes)

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
    // Convert Hz to pixel position using logarithmic spacing
    // Each octave = PIXELS_PER_OCTAVE (100 pixels)
    // Higher frequencies = lower Y values (top of screen)

    if (hz <= 0 || baseFrequency <= 0) {
        return 0;
    }

    // Calculate octave number from base frequency (logarithmic)
    double octaveNumber = std::log2(hz / baseFrequency);

    // Convert to pixels from bottom
    double pixelsFromBottom = octaveNumber * PIXELS_PER_OCTAVE;

    // Calculate visible octave range
    double minOctave = std::log2(visibleMinHz / baseFrequency);
    double maxOctave = std::log2(visibleMaxHz / baseFrequency);
    double visibleOctaveRange = maxOctave - minOctave;

    // Normalize position within visible range
    double normalizedPos = (octaveNumber - minOctave) / visibleOctaveRange;
    // Don't clamp - allow frequencies outside visible range to be positioned off-screen
    // The paintEvent will filter them out

    // Convert to pixel position (flip for top-down coordinate system)
    // Return canvas-widget coordinates; the QScrollArea handles physical scrolling.
    int pixel = height() - static_cast<int>(normalizedPos * height());
    return pixel;
}

double ScoreCanvas::pixelToFrequency(int pixel) const
{
    // Convert pixel position to Hz using logarithmic spacing.
    // 'pixel' is already in canvas-widget coordinates (from mouse events),
    // so no scroll offset adjustment is needed here.
    double normalizedPos = 1.0 - (static_cast<double>(pixel) / height());
    normalizedPos = qBound(0.0, normalizedPos, 1.0);

    // Calculate visible octave range
    double minOctave = std::log2(visibleMinHz / baseFrequency);
    double maxOctave = std::log2(visibleMaxHz / baseFrequency);
    double visibleOctaveRange = maxOctave - minOctave;

    // Calculate octave number at this position
    double octaveNumber = minOctave + (normalizedPos * visibleOctaveRange);

    // Convert back to Hz
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
                line.color = getScaleDegreeColor(degree + 1);
                line.noteName = scale.getNoteName(degree) + QString::number(octave);
                line.isThicker = (degree == 0 || (degreeCount >= 5 && degree == 4));  // Tonic and Fifth (if scale has 5+ notes)

                lines.append(line);
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
    // Quantize all selected continuous notes to scale degrees
    if (selectedNoteIndices.isEmpty()) {
        qDebug() << "ScoreCanvas::snapSelectedNotesToScale - No notes selected";
        return;
    }

    QVector<Note> &notes = phrase.getNotes();
    int quantizedCount = 0;

    for (int index : selectedNoteIndices) {
        if (index >= 0 && index < notes.size()) {
            Note &note = notes[index];

            // Only quantize notes that have pitch curves (continuous notes)
            if (note.hasPitchCurve()) {
                Curve quantizedCurve = quantizePitchCurveToScale(note.getPitchCurve(), note);
                note.setPitchCurve(quantizedCurve);
                note.setQuantized(true);  // Mark as quantized so it snaps when dragged
                note.detectSegments();    // Detect segments for segment editing mode
                quantizedCount++;
                qDebug() << "Quantized note" << index << "- original points:"
                         << note.getPitchCurve().getPointCount()
                         << "quantized points:" << quantizedCurve.getPointCount()
                         << "segments:" << note.getSegmentCount();
            }
        }
    }

    if (quantizedCount > 0) {
        update();
        qDebug() << "ScoreCanvas::snapSelectedNotesToScale - Quantized" << quantizedCount << "notes";
    } else {
        qDebug() << "ScoreCanvas::snapSelectedNotesToScale - No continuous notes in selection";
    }
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
    if (bpm >= 20.0 && bpm <= 300.0) {
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
        if (timeSigBottom == 0) {
            // Simple mode: beat = 60000/tempo
            beatDurationMs = 60000.0 / tempo;
            barDurationMs = beatDurationMs * timeSigTop;
        } else {
            // Scaled mode: bar = (60000/tempo) * (top/bottom)
            barDurationMs = (60000.0 / tempo) * (static_cast<double>(timeSigTop) / timeSigBottom);
            beatDurationMs = barDurationMs / timeSigTop;
        }
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

    // Draw pitch indicator (small circle on left at starting pitch)
    double startPitch = note.getPitchAt(0.0);
    int startCenterY = frequencyToPixel(startPitch);
    painter.setBrush(noteColor.darker(140));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(x - 3, startCenterY - 3, 6, 6);

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

        int clickedNoteIndex = findNoteAtPosition(event->pos());

        if (clickedNoteIndex >= 0) {
            const Note &note = phrase.getNotes()[clickedNoteIndex];

            // Only enter segment mode for quantized continuous notes
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
        }
    }

    QWidget::mouseDoubleClickEvent(event);
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
                        } else if (pendingDragMode == EditingBottomCurve ||
                                   pendingDragMode == EditingBottomCurveStart ||
                                   pendingDragMode == EditingBottomCurveEnd) {
                            dragStartCurve = selectedNote.getBottomCurve();
                        } else if (pendingDragMode == DraggingNote && selectedNote.hasPitchCurve()) {
                            // Save pitch curve for continuous notes
                            dragStartCurve = selectedNote.getPitchCurve();
                        }

                        return;  // Don't proceed with other selection logic
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
                    // Multi-selection - only allow dragging the group (no resize/curve edit)
                    // Use pending mode with threshold to avoid accidental drags from pen barrel buttons
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

                // Then select the note (Ctrl adds to selection)
                bool addToSelection = (event->modifiers() & Qt::ControlModifier);
                selectNote(clickedNoteIndex, addToSelection);

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
            showSegmentContextMenu(event->globalPos());
            return;
        }

        int clickedNoteIndex = findNoteAtPosition(event->pos());

        // If clicked on unselected note, select it
        if (clickedNoteIndex >= 0 && !isNoteSelected(clickedNoteIndex)) {
            selectNote(clickedNoteIndex, false);
        }

        // Only show menu if we have selection
        if (!selectedNoteIndices.isEmpty()) {
            showNoteContextMenu(event->globalPos());
        }
    }
}

void ScoreCanvas::mouseMoveEvent(QMouseEvent *event)
{
    // Emit cursor position for status bar (always update, regardless of what else is happening)
    double cursorTime = pixelToTime(event->pos().x());
    double cursorPitch = pixelToFrequency(event->pos().y());
    emit cursorPositionChanged(cursorTime, cursorPitch);

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
            Curve &activeCurve = note.getExpressiveCurve(activeIdx);
            activeCurve.clearPoints();

            // Rebuild curve with adaptive number of control points
            int numDots = calculateCurveDotCount(note);
            for (int i = 0; i <= numDots + 1; ++i) {
                double t = i / (numDots + 1.0);
                double value;

                if (i == editingDotIndex + 1) {
                    // This is the dot being edited - use new value
                    value = newValue;
                } else {
                    // Keep original value from dragStartCurve
                    value = dragStartCurve.valueAt(t);
                }

                activeCurve.addPoint(t, value);
            }

            activeCurve.sortPoints();
            break;
        }

        case EditingBottomCurve: {
            // Edit bottom curve by dragging dot vertically
            double curveChange = delta.y() / 50.0;  // Positive because down = more for bottom

            double originalValue = dragStartCurve.valueAt(editingDotTimePos);
            double newValue = qBound(0.0, originalValue + curveChange, 1.0);

            // Get current bottom curve
            Curve &bottomCurve = note.getBottomCurve();
            bottomCurve.clearPoints();

            // Rebuild curve with adaptive number of control points
            int numDots = calculateCurveDotCount(note);
            for (int i = 0; i <= numDots + 1; ++i) {
                double t = i / (numDots + 1.0);
                double value;

                if (i == editingDotIndex + 1) {
                    value = newValue;
                } else {
                    value = dragStartCurve.valueAt(t);
                }

                bottomCurve.addPoint(t, value);
            }

            bottomCurve.sortPoints();
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

        case EditingBottomCurveStart: {
            // Edit bottom curve start value (t=0.0)
            double curveChange = delta.y() / 50.0;
            double originalValue = dragStartCurve.valueAt(0.0);
            double newValue = qBound(0.0, originalValue + curveChange, 1.0);

            Curve &bottomCurve = note.getBottomCurve();
            bottomCurve.clearPoints();

            // Rebuild curve, changing only the start point
            int numDots = calculateCurveDotCount(note);
            for (int i = 0; i <= numDots + 1; ++i) {
                double t = i / (numDots + 1.0);
                double value = (i == 0) ? newValue : dragStartCurve.valueAt(t);
                bottomCurve.addPoint(t, value);
            }
            bottomCurve.sortPoints();
            break;
        }

        case EditingBottomCurveEnd: {
            // Edit bottom curve end value (t=1.0)
            double curveChange = delta.y() / 50.0;
            double originalValue = dragStartCurve.valueAt(1.0);
            double newValue = qBound(0.0, originalValue + curveChange, 1.0);

            Curve &bottomCurve = note.getBottomCurve();
            bottomCurve.clearPoints();

            // Rebuild curve, changing only the end point
            int numDots = calculateCurveDotCount(note);
            for (int i = 0; i <= numDots + 1; ++i) {
                double t = i / (numDots + 1.0);
                double value = (i == numDots + 1) ? newValue : dragStartCurve.valueAt(t);
                bottomCurve.addPoint(t, value);
            }
            bottomCurve.sortPoints();
            break;
        }

            case NoDrag:
                break;
            }
        } else {
            // Multi-selection - only allow dragging the group
            if (currentDragMode == DraggingNote) {
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

    // Update cursor based on hover position over selected note (only for single selection)
    if (selectedNoteIndices.size() == 1 && currentDragMode == NoDrag && !isDrawingNote) {
        const QVector<Note> &notes = phrase.getNotes();
        int selectedIndex = selectedNoteIndices.first();
        if (selectedIndex < notes.size()) {
            const Note &selectedNote = notes[selectedIndex];
            DragMode hoverMode = detectDragMode(event->pos(), selectedNote);

            switch (hoverMode) {
            case ResizingLeft:
            case ResizingRight:
                setCursor(Qt::SizeHorCursor);  // Horizontal resize cursor
                break;
            case EditingTopCurve:
            case EditingBottomCurve:
            case EditingTopCurveStart:
            case EditingTopCurveEnd:
            case EditingBottomCurveStart:
            case EditingBottomCurveEnd:
                setCursor(Qt::SizeVerCursor);  // Vertical resize cursor for curve editing
                break;
            case DraggingNote:
                setCursor(Qt::SizeAllCursor);  // Move cursor
                break;
            case NoDrag:
                setCursor(Qt::ArrowCursor);  // Default cursor
                break;
            default:
                break;
            }
        }
    } else if (!isDrawingNote) {
        setCursor(Qt::ArrowCursor);  // Default cursor when not over note
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

                case EditingBottomCurve: {
                    // Push curve edit command for bottom curve
                    Curve newCurve = note.getBottomCurve();
                    undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                         EditCurveCommand::BottomCurve,
                                                         dragStartCurve, newCurve,
                                                         this));
                    break;
                }

                case EditingBottomCurveStart:
                case EditingBottomCurveEnd: {
                    // Push curve edit command for bottom curve (start/end)
                    Curve newCurve = note.getBottomCurve();
                    undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                         EditCurveCommand::BottomCurve,
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

    // Draw selection rectangle (gray outline)
    QRect selectionRect(x - 5, topY, width + 10, height);
    painter.setPen(QPen(QColor(128, 128, 128), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(selectionRect);

    // Draw manipulation dots (round circles)
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

    // Left edge dots (2 dots - top for dynamics curve start, middle is square for resize)
    int leftX = selectionRect.left();
    for (int i = 1; i <= 2; ++i) {
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

    // Right edge dots (2 dots - top for dynamics curve end, middle is square for resize)
    int rightX = selectionRect.right();
    for (int i = 1; i <= 2; ++i) {
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
        emit noteSelectionChanged();
    }
}

void ScoreCanvas::selectNotes(const QVector<int> &indices)
{
    selectedNoteIndices = indices;
    emit noteSelectionChanged();
}

void ScoreCanvas::deselectAll()
{
    selectedNoteIndices.clear();
    emit noteSelectionChanged();
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
            }
        }

        // Mode-specific behavior (left button / pen tip)
        if (currentInputMode == SelectionMode) {
            const QVector<Note> &notes = phrase.getNotes();

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
                        } else if (pendingDragMode == EditingBottomCurve ||
                                   pendingDragMode == EditingBottomCurveStart ||
                                   pendingDragMode == EditingBottomCurveEnd) {
                            dragStartCurve = selectedNote.getBottomCurve();
                        } else if (pendingDragMode == DraggingNote && selectedNote.hasPitchCurve()) {
                            // Save pitch curve for continuous notes
                            dragStartCurve = selectedNote.getPitchCurve();
                        }

                        event->accept();
                        return;  // Don't proceed with other selection logic
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
                    // Multi-selection - only allow dragging the group (no resize/curve edit)
                    // Use pending mode with threshold to avoid accidental drags from pen barrel buttons
                    pendingDragMode = DraggingNote;
                    currentDragMode = NoDrag;  // Don't start dragging yet
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
                bool addToSelection = (event->modifiers() & Qt::ControlModifier);
                selectNote(clickedNoteIndex, addToSelection);
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
                Curve &activeCurve = note.getExpressiveCurve(activeIdx);
                activeCurve.clearPoints();

                // Rebuild curve with adaptive number of control points
                int numDots = calculateCurveDotCount(note);
                for (int i = 0; i <= numDots + 1; ++i) {
                    double t = i / (numDots + 1.0);
                    double value;

                    if (i == editingDotIndex + 1) {
                        value = newValue;
                    } else {
                        value = dragStartCurve.valueAt(t);
                    }

                    activeCurve.addPoint(t, value);
                }

                activeCurve.sortPoints();
                break;
            }

            case EditingBottomCurve: {
                // Edit bottom curve by dragging dot vertically
                double curveChange = delta.y() / 50.0;  // Positive because down = more for bottom

                double originalValue = dragStartCurve.valueAt(editingDotTimePos);
                double newValue = qBound(0.0, originalValue + curveChange, 1.0);

                Curve &bottomCurve = note.getBottomCurve();
                bottomCurve.clearPoints();

                // Rebuild curve with adaptive number of control points
                int numDots = calculateCurveDotCount(note);
                for (int i = 0; i <= numDots + 1; ++i) {
                    double t = i / (numDots + 1.0);
                    double value;

                    if (i == editingDotIndex + 1) {
                        value = newValue;
                    } else {
                        value = dragStartCurve.valueAt(t);
                    }

                    bottomCurve.addPoint(t, value);
                }

                bottomCurve.sortPoints();
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

            case EditingBottomCurveStart: {
                // Edit bottom curve start value (t=0.0)
                double curveChange = delta.y() / 50.0;
                double originalValue = dragStartCurve.valueAt(0.0);
                double newValue = qBound(0.0, originalValue + curveChange, 1.0);

                Curve &bottomCurve = note.getBottomCurve();
                bottomCurve.clearPoints();

                int numDots = calculateCurveDotCount(note);
                for (int i = 0; i <= numDots + 1; ++i) {
                    double t = i / (numDots + 1.0);
                    double value = (i == 0) ? newValue : dragStartCurve.valueAt(t);
                    bottomCurve.addPoint(t, value);
                }
                bottomCurve.sortPoints();
                break;
            }

            case EditingBottomCurveEnd: {
                // Edit bottom curve end value (t=1.0)
                double curveChange = delta.y() / 50.0;
                double originalValue = dragStartCurve.valueAt(1.0);
                double newValue = qBound(0.0, originalValue + curveChange, 1.0);

                Curve &bottomCurve = note.getBottomCurve();
                bottomCurve.clearPoints();

                int numDots = calculateCurveDotCount(note);
                for (int i = 0; i <= numDots + 1; ++i) {
                    double t = i / (numDots + 1.0);
                    double value = (i == numDots + 1) ? newValue : dragStartCurve.valueAt(t);
                    bottomCurve.addPoint(t, value);
                }
                bottomCurve.sortPoints();
                break;
            }

                case NoDrag:
                    break;
                }
            } else {
                // Multi-selection - only allow dragging the group
                if (currentDragMode == DraggingNote) {
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

                    case EditingBottomCurve: {
                        // Push curve edit command for bottom curve
                        Curve newCurve = note.getBottomCurve();
                        undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                             EditCurveCommand::BottomCurve,
                                                             dragStartCurve, newCurve,
                                                             this));
                        break;
                    }

                    case EditingBottomCurveStart:
                    case EditingBottomCurveEnd: {
                        // Push curve edit command for bottom curve (start/end)
                        Curve newCurve = note.getBottomCurve();
                        undoStack->push(new EditCurveCommand(&phrase, selectedIndex,
                                                             EditCurveCommand::BottomCurve,
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

    // Delete key removes selected notes
    if (event->key() == Qt::Key_Delete && !selectedNoteIndices.isEmpty()) {
        performDelete();
        event->accept();
        return;
    }

    // Ctrl+Z for undo
    if (event->matches(QKeySequence::Undo)) {
        undoStack->undo();
        event->accept();
        return;
    }

    // Ctrl+Y or Ctrl+Shift+Z for redo
    if (event->matches(QKeySequence::Redo)) {
        undoStack->redo();
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

    // Ctrl+V for paste
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

    // Let parent window handle other keys (including shortcuts)
    event->ignore();
}

void ScoreCanvas::toggleSlideMode()
{
    slideMode = !slideMode;
    emit slideModeChanged(slideMode);
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

void ScoreCanvas::performPaste()
{
    if (!clipboard.isEmpty()) {
        double viewCenterTime = (horizontalScrollOffset + 400) / pixelsPerSecond * 1000.0;
        PasteNotesCommand *pasteCmd = new PasteNotesCommand(&phrase, clipboard, viewCenterTime, activeTrackIndex, this);
        undoStack->push(pasteCmd);

        selectNotes(pasteCmd->getPastedIndices());
        update();
        qDebug() << "Pasted" << clipboard.size() << "notes at time" << viewCenterTime;
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

    // Expressive Curve Shape action — enabled only when selection has named curves
    {
        bool hasExpressiveCurves = false;
        const QVector<Note> &notesRef = phrase.getNotes();
        for (int idx : selectedNoteIndices) {
            if (idx >= 0 && idx < notesRef.size() &&
                notesRef[idx].getExpressiveCurveCount() > 1) {
                hasExpressiveCurves = true;
                break;
            }
        }
        QAction *expressiveAction = menu.addAction("Apply Expressive Curve...");
        expressiveAction->setEnabled(hasExpressiveCurves);
        connect(expressiveAction, &QAction::triggered,
                this, &ScoreCanvas::showExpressiveCurveApplyDialog);
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
    QVector<Note> &notes = phrase.getNotes();

    int appliedCount = 0;
    for (int idx : selectedNoteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            qDebug() << "ScoreCanvas: Setting note" << notes[idx].getId()
                     << "from variation" << notes[idx].getVariationIndex()
                     << "to variation" << variationIndex;
            notes[idx].setVariationIndex(variationIndex);
            notes[idx].setRenderDirty(true);
            appliedCount++;
        }
    }

    // Debug: Show current variation state of all notes
    qDebug() << "ScoreCanvas: After applying, note variation state:";
    for (int i = 0; i < notes.size(); ++i) {
        qDebug() << "  Note" << i << "id=" << notes[i].getId()
                 << "variationIndex=" << notes[i].getVariationIndex();
    }

    update();
    qDebug() << "ScoreCanvas: Applied variation" << variationIndex
             << "to" << appliedCount << "notes";
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

void ScoreCanvas::showExpressiveCurveApplyDialog()
{
    if (selectedNoteIndices.isEmpty()) return;

    const QVector<Note> &notes = phrase.getNotes();

    // Collect unique named expressive curve names from the selection
    QSet<QString> namesSet;
    for (int idx : selectedNoteIndices) {
        if (idx >= 0 && idx < notes.size()) {
            const Note &note = notes[idx];
            for (int i = 1; i < note.getExpressiveCurveCount(); ++i) {
                namesSet.insert(note.getExpressiveCurveName(i));
            }
        }
    }

    if (namesSet.isEmpty()) {
        QMessageBox::information(this, "Apply Expressive Curve",
            "No named expressive curves found in the selection.\n"
            "Add expressive curves to notes first using the inspector.");
        return;
    }

    QStringList curveNames = QStringList(namesSet.begin(), namesSet.end());
    curveNames.sort();

    // Calculate selection time span
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

    if (dialog.exec() == QDialog::Accepted) {
        QString targetName = dialog.getSelectedCurveName();
        QVector<EnvelopePoint> curve = dialog.getCurve();
        double weight = dialog.getWeight();

        undoStack->push(new ApplyExpressiveCurveShapeCommand(
            &phrase, selectedNoteIndices, targetName, curve, weight, this));

        qDebug() << "ScoreCanvas: Applied expressive curve shape to '" << targetName
                 << "' on" << selectedNoteIndices.size() << "notes";
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
    // Search the note for a curve that matches the active name
    for (int i = 1; i < note.getExpressiveCurveCount(); ++i) {
        if (note.getExpressiveCurveName(i) == m_activeExpressiveCurveName)
            return i;
    }
    // Note doesn't carry this curve — fall back to dynamics
    return 0;
}

void ScoreCanvas::addExpressiveCurveToSelection(const QString &name,
                                                 const QVector<EnvelopePoint> &curve,
                                                 double weight, bool perNote)
{
    if (selectedNoteIndices.isEmpty()) return;
    undoStack->push(new AddExpressiveCurveCommand(
        &phrase, selectedNoteIndices, name, curve, weight, perNote, this));
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
    undoStack->push(new RemoveNamedExpressiveCurveCommand(&phrase, name, this));
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

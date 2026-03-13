#include "trackselect.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QMenu>
#include <cmath>

TrackSelector::TrackSelector(QWidget *parent)
    : QWidget(parent)
    , activeTrackIndex(-1)
    , hoveredTrackIndex(-1)
    , visibleMinHz(20.0)
    , visibleMaxHz(20000.0)
    , pixelsPerHz(0.1)
    , verticalScrollOffset(0)
{
    setMinimumWidth(TRACK_BAR_WIDTH);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
}

void TrackSelector::addTrack(const QString &name, const QColor &color,
                             double minHz, double maxHz)
{
    Track track;
    track.name = name;
    track.color = color;
    track.minFreqHz = minHz;
    track.maxFreqHz = maxHz;
    track.isActive = false;
    track.isSelected = true;  // New tracks are visible by default
    track.trackIndex = tracks.size();

    tracks.append(track);

    // If this is the first track, make it active
    if (tracks.size() == 1) {
        setActiveTrack(0);
    }

    updateGeometry();
    update();
}

void TrackSelector::removeTrack(int index)
{
    if (index >= 0 && index < tracks.size()) {
        tracks.remove(index);

        // Update track indices
        for (int i = 0; i < tracks.size(); ++i) {
            tracks[i].trackIndex = i;
        }

        // Adjust active track if needed
        if (activeTrackIndex == index) {
            activeTrackIndex = -1;
            if (!tracks.isEmpty()) {
                setActiveTrack(0);
            }
        } else if (activeTrackIndex > index) {
            activeTrackIndex--;
        }

        updateGeometry();
        update();
    }
}

void TrackSelector::setActiveTrack(int index)
{
    if (index >= 0 && index < tracks.size()) {
        // Deactivate previous active track
        if (activeTrackIndex >= 0 && activeTrackIndex < tracks.size()) {
            tracks[activeTrackIndex].isActive = false;
        }

        // Activate new track — collapse to single selection
        for (auto &t : tracks) t.isSelected = false;
        activeTrackIndex = index;
        tracks[index].isActive = true;
        tracks[index].isSelected = true;

        update();
        emit trackSelected(index);
        emit multiSelectionChanged(getSelectedIndices());
    }
}

int TrackSelector::getActiveTrack() const
{
    return activeTrackIndex;
}

void TrackSelector::addToSelection(int index)
{
    if (index < 0 || index >= tracks.size()) return;

    if (tracks[index].isSelected) {
        // Deselect — but never leave the selection empty
        if (getSelectedIndices().size() > 1) {
            tracks[index].isSelected = false;
            // If we just deselected the active track, move active to first remaining
            if (index == activeTrackIndex) {
                for (int i = 0; i < tracks.size(); ++i) {
                    if (tracks[i].isSelected) {
                        activeTrackIndex = i;
                        tracks[i].isActive = true;
                        emit trackSelected(i);
                        break;
                    }
                }
            }
        }
    } else {
        // Add to selection; the newly clicked track becomes primary
        if (activeTrackIndex >= 0 && activeTrackIndex < tracks.size())
            tracks[activeTrackIndex].isActive = false;
        tracks[index].isSelected = true;
        tracks[index].isActive = true;
        activeTrackIndex = index;
        emit trackSelected(index);
    }

    update();
    emit multiSelectionChanged(getSelectedIndices());
    qDebug() << "TrackSelector: multi-selection is now" << getSelectedIndices();
}

bool TrackSelector::isTrackSelected(int index) const
{
    if (index >= 0 && index < tracks.size()) {
        return tracks[index].isSelected;
    }
    return false;
}

QVector<int> TrackSelector::getSelectedIndices() const
{
    QVector<int> result;
    for (int i = 0; i < tracks.size(); ++i) {
        if (tracks[i].isSelected)
            result.append(i);
    }
    return result;
}

void TrackSelector::clearTracks()
{
    tracks.clear();
    activeTrackIndex = -1;
    hoveredTrackIndex = -1;
    updateGeometry();
    update();
}

void TrackSelector::updateTrack(int index, const QString &name, const QColor &color)
{
    if (index >= 0 && index < tracks.size()) {
        tracks[index].name = name;
        tracks[index].color = color;
        update();
    }
}

void TrackSelector::setFrequencyRange(double minHz, double maxHz)
{
    visibleMinHz = minHz;
    visibleMaxHz = maxHz;
    update();
}

void TrackSelector::setPixelsPerHz(double ratio)
{
    pixelsPerHz = ratio;
    update();
}

void TrackSelector::setVerticalOffset(int offsetPixels)
{
    verticalScrollOffset = offsetPixels;
    update();
}

QSize TrackSelector::sizeHint() const
{
    int width = tracks.size() * (TRACK_BAR_WIDTH + TRACK_SPACING);
    if (width == 0) {
        width = TRACK_BAR_WIDTH;  // Minimum width even with no tracks
    }
    return QSize(width, 600);
}

QSize TrackSelector::minimumSizeHint() const
{
    return QSize(TRACK_BAR_WIDTH, 400);
}

void TrackSelector::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw background - light golden color
    painter.fillRect(rect(), QColor(255, 235, 205));  // Light golden (peach puff)

    // Draw each track bar
    for (int i = 0; i < tracks.size(); ++i) {
        const Track &track = tracks[i];
        QRect trackRect = getTrackRect(i);

        // Skip if not visible
        if (!trackRect.intersects(event->rect())) {
            continue;
        }

        // Full opacity for active or selected; dimmed for unselected
        QColor color = track.color;
        color.setAlpha((track.isActive || track.isSelected) ? ACTIVE_OPACITY : DIMMED_OPACITY);

        // Draw track bar
        painter.fillRect(trackRect, color);

        // Draw border: thick white for active, thinner white for other selected, subtle for unselected
        if (track.isActive) {
            painter.setPen(QPen(Qt::white, 3));
            painter.drawRect(trackRect.adjusted(2, 2, -2, -2));
        } else if (track.isSelected) {
            painter.setPen(QPen(Qt::white, 2, Qt::DashLine));
            painter.drawRect(trackRect.adjusted(1, 1, -1, -1));
        } else {
            painter.setPen(QPen(QColor(200, 200, 200), 1));
            painter.drawRect(trackRect);
        }

        // Draw track name vertically
        if (trackRect.height() > 50) {
            painter.save();
            painter.setPen(Qt::white);
            QFont font = painter.font();
            font.setPointSize(10);
            font.setBold(true);
            painter.setFont(font);

            // Calculate text metrics
            QFontMetrics fm(font);
            int textWidth = fm.horizontalAdvance(track.name);

            // Rotate and draw text vertically
            // Move to bottom-left of track rect, then rotate -90 degrees
            int centerX = trackRect.center().x();
            int centerY = trackRect.center().y();

            painter.translate(centerX, centerY);
            painter.rotate(-90);

            // Draw centered at origin (which is now rotated)
            painter.drawText(-textWidth / 2, fm.ascent() / 2, track.name);

            painter.restore();
        }
    }
}

void TrackSelector::mousePressEvent(QMouseEvent *event)
{
    int index = trackIndexAtPosition(event->pos());

    if (event->button() == Qt::RightButton) {
        if (index >= 0) {
            QMenu menu(window());
            QAction *deleteAction = menu.addAction("Delete Track");
            QAction *chosen = menu.exec(event->globalPosition().toPoint());
            if (chosen == deleteAction) {
                emit deleteTrackRequested(index);
            }
        }
        return;
    }

    if (index >= 0) {
        // Ctrl+click: add/remove track from multi-selection
        if (event->modifiers() & Qt::ControlModifier) {
            addToSelection(index);
        }
        // Regular click: make track active (and selected)
        else {
            setActiveTrack(index);
        }
        emit trackClicked(index);
    }
    QWidget::mousePressEvent(event);
}

void TrackSelector::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}

int TrackSelector::frequencyToPixel(double hz) const
{
    // Convert Hz to pixel position using logarithmic spacing
    // Each octave = PIXELS_PER_OCTAVE (100 pixels)
    // Higher frequencies = lower Y values (top of screen)
    // MUST MATCH ScoreCanvas implementation

    if (hz <= 0 || BASE_FREQUENCY <= 0) {
        return 0;
    }

    // Calculate octave number from base frequency (logarithmic)
    double octaveNumber = std::log2(hz / BASE_FREQUENCY);

    // Calculate visible octave range
    double minOctave = std::log2(visibleMinHz / BASE_FREQUENCY);
    double maxOctave = std::log2(visibleMaxHz / BASE_FREQUENCY);
    double visibleOctaveRange = maxOctave - minOctave;

    // Normalize position within visible range
    double normalizedPos = (octaveNumber - minOctave) / visibleOctaveRange;
    // Don't clamp - allow frequencies outside visible range to be positioned off-screen
    // MUST MATCH ScoreCanvas implementation

    // Convert to pixel position (flip for top-down coordinate system)
    int pixel = height() - static_cast<int>(normalizedPos * height());
    return pixel;
}

double TrackSelector::pixelToFrequency(int pixel) const
{
    // Convert pixel position to Hz using logarithmic spacing
    // MUST MATCH ScoreCanvas implementation
    int adjustedPixel = pixel;
    double normalizedPos = 1.0 - (static_cast<double>(adjustedPixel) / height());
    normalizedPos = qBound(0.0, normalizedPos, 1.0);

    // Calculate visible octave range
    double minOctave = std::log2(visibleMinHz / BASE_FREQUENCY);
    double maxOctave = std::log2(visibleMaxHz / BASE_FREQUENCY);
    double visibleOctaveRange = maxOctave - minOctave;

    // Calculate octave number at this position
    double octaveNumber = minOctave + (normalizedPos * visibleOctaveRange);

    // Convert back to Hz
    return BASE_FREQUENCY * std::pow(2.0, octaveNumber);
}

int TrackSelector::trackIndexAtPosition(const QPoint &pos) const
{
    for (int i = 0; i < tracks.size(); ++i) {
        if (getTrackRect(i).contains(pos)) {
            return i;
        }
    }
    return -1;  // No track at position
}

QRect TrackSelector::getTrackRect(int trackIndex) const
{
    if (trackIndex < 0 || trackIndex >= tracks.size()) {
        return QRect();
    }

    const Track &track = tracks[trackIndex];

    int x = trackIndex * (TRACK_BAR_WIDTH + TRACK_SPACING);
    int yTop = frequencyToPixel(track.maxFreqHz);     // Top of bar (higher freq)
    int yBottom = frequencyToPixel(track.minFreqHz);  // Bottom of bar (lower freq)
    int height = yBottom - yTop;

    // Ensure minimum height of 10 pixels for visibility
    if (height < 10) {
        height = 10;
    }

    return QRect(x, yTop, TRACK_BAR_WIDTH, height);
}

void TrackSelector::updateGeometry()
{
    // Resize to fit all tracks (enables scrollbar in scroll area when > 4 tracks)
    QSize hint = sizeHint();
    resize(hint.width(), height());  // Keep current height, update width
    QWidget::updateGeometry();
}

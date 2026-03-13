#ifndef TRACKSELECT_H
#define TRACKSELECT_H

#include <QWidget>
#include <QVector>
#include <QColor>
#include <QString>

class TrackSelector : public QWidget
{
    Q_OBJECT

public:
    struct Track {
        QString name;
        QColor color;
        double minFreqHz;      // Bottom of register range
        double maxFreqHz;      // Top of register range
        bool isActive;         // The track that receives new notes
        bool isSelected;       // Track is visible (notes shown)
        int trackIndex;
    };

    explicit TrackSelector(QWidget *parent = nullptr);

    // Track management
    void addTrack(const QString &name, const QColor &color,
                  double minHz, double maxHz);
    void removeTrack(int index);
    void setActiveTrack(int index);
    int getActiveTrack() const;
    void addToSelection(int index);        // Ctrl+click: add/remove from multi-selection
    bool isTrackSelected(int index) const;
    QVector<int> getSelectedIndices() const;
    void clearTracks();
    void updateTrack(int index, const QString &name, const QColor &color);
    const QVector<Track>& getTracks() const { return tracks; }

    // Synchronization with score canvas
    void setFrequencyRange(double minHz, double maxHz);  // Visible range
    void setPixelsPerHz(double ratio);                    // Scaling factor
    void setVerticalOffset(int offsetPixels);             // Scroll sync

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void trackClicked(int trackIndex);
    void trackSelected(int trackIndex);
    void multiSelectionChanged(QVector<int> selectedIndices);
    void deleteTrackRequested(int trackIndex);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QVector<Track> tracks;
    int activeTrackIndex;
    int hoveredTrackIndex;

    // Frequency mapping
    double visibleMinHz;
    double visibleMaxHz;
    double pixelsPerHz;
    int verticalScrollOffset;

    // Visual constants
    static constexpr int TRACK_BAR_WIDTH = 40;
    static constexpr int TRACK_SPACING = 2;
    static constexpr int DIMMED_OPACITY = 102;      // 40% of 255
    static constexpr int ACTIVE_OPACITY = 255;       // 100%
    static constexpr double BASE_FREQUENCY = 25.0;   // Hz - base frequency for just intonation
    static constexpr int PIXELS_PER_OCTAVE = 100;    // Fixed vertical size for each octave

    // Helper methods
    int frequencyToPixel(double hz) const;
    double pixelToFrequency(int pixel) const;
    int trackIndexAtPosition(const QPoint &pos) const;
    QRect getTrackRect(int trackIndex) const;
    void updateGeometry();
};

#endif // TRACKSELECT_H

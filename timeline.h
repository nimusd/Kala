#ifndef TIMELINE_H
#define TIMELINE_H

#include <QWidget>
#include <QPoint>

class Timeline : public QWidget
{
    Q_OBJECT

public:
    enum TimeMode {
        Absolute,  // Min:Sec:Ms format
        Musical    // Bars:Beats:Ms format
    };

    explicit Timeline(QWidget *parent = nullptr);

    // Setters
    void setTimeMode(TimeMode mode);
    void setTempo(double bpm);
    void setTimeSignature(int numerator, int denominator);
    void setNowMarker(double timeMs);
    void setLoopModeActive(bool active);
    void setLoopEnd(double timeMs);
    void clearLoop();
    void setHorizontalOffset(int offset);
    void setPixelsPerSecond(double pps);
    void setScaleChanges(const QMap<double, QString> &changes);  // time -> scale name
    void setTempoChanges(const QMap<double, QString> &changes);  // time -> tempo description

    // Getters
    double getNowMarker() const { return nowMarkerTime; }
    bool hasLoop() const { return loopEndTime >= 0.0; }
    double getLoopStart() const { return loopStartTime; }
    double getLoopEnd() const { return loopEndTime; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

signals:
    void nowMarkerChanged(double timeMs);
    void loopEndSet(double timeMs);
    void loopCleared();

private:
    // Time display
    TimeMode timeMode;
    double tempo;  // BPM
    int timeSigNumerator;
    int timeSigDenominator;

    // Timeline state
    double nowMarkerTime;     // in milliseconds
    double loopStartTime;     // in milliseconds (-1 if no loop)
    double loopEndTime;       // in milliseconds (-1 if no loop)
    bool loopModeActive;      // true when waiting for loop end click

    // View state
    int horizontalOffset;     // Horizontal scroll offset in pixels
    double pixelsPerSecond;   // Zoom level for time axis

    // Scale modulations
    QMap<double, QString> scaleChanges;  // time (ms) -> scale name

    // Tempo changes
    QMap<double, QString> tempoChanges;  // time (ms) -> tempo description

    // Interaction state
    bool hoveringLoopEnd;     // true when hovering over loop end triangle
    QPoint loopEndTrianglePos; // Position of loop end triangle
    bool mouseOnTimeline;     // true when mouse is over timeline
    int mouseCursorX;         // Mouse cursor X position

    // Helper methods
    QString formatTime(double timeMs) const;
    double pixelToTime(int x) const;
    int timeToPixel(double timeMs) const;
    void drawTickMarks(QPainter &painter);
    void drawLoopRegion(QPainter &painter);
    void drawNowMarker(QPainter &painter);
    void drawScaleChangeMarkers(QPainter &painter);
    void drawTempoChangeMarkers(QPainter &painter);
    QPolygon getLoopEndTriangle(int x) const;
    bool isPointInLoopEndTriangle(const QPoint &pos) const;
};

#endif // TIMELINE_H

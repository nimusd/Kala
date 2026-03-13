#ifndef FREQUENCYLABELS_H
#define FREQUENCYLABELS_H

#include <QWidget>
#include <QVector>
#include <QColor>
#include <QString>

class FrequencyLabels : public QWidget
{
    Q_OBJECT

public:
    struct ScaleLine {
        double frequencyHz;
        int scaleDegree;
        QColor color;
        QString noteName;
        bool isThicker;
    };

    explicit FrequencyLabels(QWidget *parent = nullptr);

    void setFrequencyRange(double minHz, double maxHz);
    void setVerticalOffset(int offsetPixels);
    void setScaleLines(const QVector<ScaleLine> &lines);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<ScaleLine> scaleLines;
    double visibleMinHz;
    double visibleMaxHz;
    int verticalScrollOffset;
    int canvasHeight;  // Shared canvas height for calculations

    static constexpr double BASE_FREQUENCY = 25.0;
    static constexpr int PIXELS_PER_OCTAVE = 100;
    static constexpr int LABEL_WIDTH = 60;

    int frequencyToPixel(double hz) const;
    void generateScaleLines();

public slots:
    void setCanvasHeight(int height);
};

#endif // FREQUENCYLABELS_H

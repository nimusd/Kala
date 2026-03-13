#ifndef EASINGPREVIEWWIDGET_H
#define EASINGPREVIEWWIDGET_H

#include <QWidget>
#include "easing.h"

/**
 * EasingPreviewWidget - Visual preview of an easing curve
 *
 * Displays a graph showing the easing function's output (y-axis)
 * against normalized input time (x-axis). Updates in real-time
 * as easing type or parameters change.
 */
class EasingPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EasingPreviewWidget(QWidget *parent = nullptr);

    // Set the easing to preview
    void setEasing(const Easing &easing);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Easing currentEasing;

    // Drawing constants
    static constexpr int MARGIN = 25;
    static constexpr int SAMPLE_COUNT = 100;
};

#endif // EASINGPREVIEWWIDGET_H

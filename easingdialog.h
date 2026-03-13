#ifndef EASINGDIALOG_H
#define EASINGDIALOG_H

#include <QDialog>
#include "easing.h"

class EasingPreviewWidget;

namespace Ui {
class EasingDialog;
}

/**
 * EasingDialog - Select easing type and parameters for rhythmic timing modification
 *
 * Used by ScoreCanvas to apply easing functions to redistribute note positions
 * within a time span. The dialog shows categorized easing types and parameter
 * sliders for parametric easings (Back, Elastic, Wobble).
 */
class EasingDialog : public QDialog
{
    Q_OBJECT

public:
    // Anchor mode determines which notes stay fixed at their original positions
    enum AnchorMode {
        AnchorNone,   // All notes can move (default)
        AnchorFirst,  // First note stays fixed
        AnchorLast,   // Last note stays fixed
        AnchorBoth    // First and last notes stay fixed
    };

    explicit EasingDialog(QWidget *parent = nullptr);
    ~EasingDialog();

    // Get the configured easing
    Easing getEasing() const;

    // Get the selected anchor mode
    AnchorMode getAnchorMode() const;

    // Get the overall effect weight (0-2, where 1 = normal)
    double getWeight() const;

private slots:
    void onEasingTypeChanged(int index);
    void updatePreview();

private:
    Ui::EasingDialog *ui;
    EasingPreviewWidget *previewWidget;

    void populateEasingTypes();
    void updateParameterVisibility();
};

#endif // EASINGDIALOG_H

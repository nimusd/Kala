#ifndef GOTODIALOG_H
#define GOTODIALOG_H

#include <QDialog>
#include "compositionsettings.h"

class QSpinBox;
class QLabel;
class QStackedWidget;

/**
 * GotoDialog - Navigate to specific time position
 *
 * Modal dialog triggered by 'g' keyboard shortcut
 * Supports both Absolute (MM:SS:MS) and Musical (Measure:Beat:MS) modes
 */
class GotoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GotoDialog(CompositionSettings::TimeMode mode,
                       int tempo,
                       int timeSigTop,
                       int timeSigBottom,
                       QWidget *parent = nullptr);

    // Get target time in milliseconds (call after exec() returns Accepted)
    double getTargetTimeMs() const;

private slots:
    void onAbsoluteTimeChanged();
    void onMusicalTimeChanged();

private:
    void setupUI();
    double calculateBarDuration() const;

    // UI Components
    QStackedWidget *inputStack;

    // Absolute mode inputs (MM:SS:MS)
    QSpinBox *absMinutes;
    QSpinBox *absSeconds;
    QSpinBox *absMilliseconds;

    // Musical mode inputs (Measure:Beat:MS)
    QSpinBox *musicalMeasure;
    QSpinBox *musicalBeat;
    QSpinBox *musicalMilliseconds;

    // Info label
    QLabel *infoLabel;

    // Settings
    CompositionSettings::TimeMode timeMode;
    int currentTempo;
    int currentTimeSigTop;
    int currentTimeSigBottom;

    // Result
    double targetTimeMs;
};

#endif // GOTODIALOG_H

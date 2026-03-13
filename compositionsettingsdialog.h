#ifndef COMPOSITIONSETTINGSDIALOG_H
#define COMPOSITIONSETTINGSDIALOG_H

#include <QDialog>
#include "compositionsettings.h"

class QLineEdit;
class QRadioButton;
class QSpinBox;
class QTimeEdit;
class QButtonGroup;
class QStackedWidget;
class QLabel;

/**
 * CompositionSettingsDialog - Edit composition metadata
 *
 * Modal dialog accessible via File > Composition Settings
 * Syncs with ScoreCanvasWindow toolbar controls
 */
class CompositionSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompositionSettingsDialog(const CompositionSettings &current,
                                      QWidget *parent = nullptr);

    // Get edited settings (call after exec() returns Accepted)
    CompositionSettings getSettings() const;

private slots:
    void onTimeModeChanged(int buttonId);
    void onTempoChanged(int bpm);
    void onTimeSignatureChanged();
    void onDurationChanged();
    void onBarsChanged(int bars);

private:
    void setupUI();
    void updateLengthConversion();
    QString formatDuration(double ms) const;

    // UI Components
    QLineEdit *nameEdit;
    QRadioButton *radioAbsolute;
    QRadioButton *radioMusical;
    QButtonGroup *timeModeGroup;
    QSpinBox *tempoSpinBox;
    QSpinBox *timeSigNumerator;
    QSpinBox *timeSigDenominator;

    // Length inputs (stacked, shown/hidden based on time mode)
    QStackedWidget *lengthStack;
    QTimeEdit *durationEdit;     // HH:MM:SS (for Absolute)
    QSpinBox *barsSpinBox;       // Number of bars (for Musical)

    // Calculated info labels
    QLabel *infoBarDuration;
    QLabel *infoTotalDuration;

    // Data
    CompositionSettings settings;
};

#endif // COMPOSITIONSETTINGSDIALOG_H

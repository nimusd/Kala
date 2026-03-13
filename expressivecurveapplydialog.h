#ifndef EXPRESSIVECURVEAPPLYDIALOG_H
#define EXPRESSIVECURVEAPPLYDIALOG_H

#include <QDialog>
#include <QVector>
#include <QStringList>
#include "envelopelibraryDialog.h"

class QComboBox;
class QLabel;
class QPushButton;
class QSlider;
class EnvelopeCurveCanvas;

/**
 * ExpressiveCurveApplyDialog - Apply a shape curve progressively across selected
 * notes for a chosen named expressive curve.
 *
 * Similar to DynamicsCurveDialog but:
 *   - A dropdown selects which named expressive curve to target.
 *   - No "Per note" toggle — always spans the whole selection (progressive mode).
 */
class ExpressiveCurveApplyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExpressiveCurveApplyDialog(int noteCount, double timeSpanMs,
                                        const QStringList &curveNames,
                                        QWidget *parent = nullptr);

    // The envelope shape drawn by the user
    QVector<EnvelopePoint> getCurve() const;

    // Weight (0.0 – 2.0, 1.0 = neutral)
    double getWeight() const;

    // Which named expressive curve to apply to
    QString getSelectedCurveName() const;

private slots:
    void onPresetChanged(int index);
    void onSaveClicked();
    void onLoadClicked();
    void onWeightChanged(int value);

private:
    void setupUi(const QStringList &curveNames);
    void setupPresets();
    void loadPreset(int index);

    QComboBox *curveNameCombo;
    QComboBox *presetCombo;
    EnvelopeCurveCanvas *curveCanvas;
    QLabel *selectionLabel;
    QSlider *weightSlider;
    QLabel *weightValueLabel;
    QPushButton *saveButton;
    QPushButton *loadButton;
    QPushButton *applyButton;
    QPushButton *cancelButton;

    int m_noteCount;
    double m_timeSpanMs;

    struct Preset {
        QString name;
        QVector<EnvelopePoint> curve;
    };
    QVector<Preset> presets;
};

#endif // EXPRESSIVECURVEAPPLYDIALOG_H

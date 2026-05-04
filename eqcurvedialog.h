#ifndef EQCURVEDIALOG_H
#define EQCURVEDIALOG_H

#include <QDialog>
#include <QVector>
#include "envelopelibraryDialog.h"

class QComboBox;
class QLabel;
class QPushButton;
class QSlider;
class EnvelopeCurveCanvas;

/**
 * EqCurveDialog - Apply a 10-band EQ curve to selected notes.
 *
 * The dialog authors two curves:
 *   - Shape curve : X = Band 1 -> Band 10, Y = band gain (0.5 = flat).
 *                   Sampled into 10 band values by averaging over each band's
 *                   1/10th slice of the X axis.
 *   - Intensity curve : X = time (normalized to note or selection),
 *                       Y = 0..1. Scales the shape toward flat (0 = flat,
 *                       1 = full shape).
 *
 * The apply command writes per-band envelopes to each selected note under
 * the curve names "band 1" ... "band 10" (the names the user has set on
 * the Envelope container hooked to the 10-Band EQ).
 */
class EqCurveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EqCurveDialog(int noteCount, double timeSpanMs, QWidget *parent = nullptr);

    QVector<EnvelopePoint> getShapeCurve() const;
    QVector<EnvelopePoint> getIntensityCurve() const;
    double getWeight() const;
    bool getPerNoteMode() const;

    void setInitialShapeCurve(const QVector<EnvelopePoint> &points);
    void setInitialIntensityCurve(const QVector<EnvelopePoint> &points);

private slots:
    void onShapePresetChanged(int index);
    void onIntensityPresetChanged(int index);
    void onShapeSaveClicked();
    void onShapeLoadClicked();
    void onIntensitySaveClicked();
    void onIntensityLoadClicked();
    void onWeightChanged(int value);
    void onShapeFlatResetClicked();

private:
    struct Preset {
        QString name;
        QVector<EnvelopePoint> curve;
    };

    void setupUi();
    void setupShapePresets();
    void setupIntensityPresets();
    void loadShapePreset(int index);
    void loadIntensityPreset(int index);

    void saveCurveToFile(const QVector<EnvelopePoint> &points,
                         const QString &defaultBaseName,
                         const QString &dialogTitle,
                         const QString &nameTag);
    QVector<EnvelopePoint> loadCurveFromFile(const QString &dialogTitle, bool *ok);

    // UI
    QComboBox *shapePresetCombo;
    QComboBox *intensityPresetCombo;
    EnvelopeCurveCanvas *shapeCanvas;
    EnvelopeCurveCanvas *intensityCanvas;
    QLabel *selectionLabel;
    QSlider *weightSlider;
    QLabel *weightValueLabel;
    QPushButton *perNoteToggle;
    QPushButton *shapeSaveButton;
    QPushButton *shapeLoadButton;
    QPushButton *shapeFlatButton;
    QPushButton *intensitySaveButton;
    QPushButton *intensityLoadButton;
    QPushButton *applyButton;
    QPushButton *cancelButton;

    int m_noteCount;
    double m_timeSpanMs;

    QVector<Preset> shapePresets;
    QVector<Preset> intensityPresets;
};

#endif // EQCURVEDIALOG_H

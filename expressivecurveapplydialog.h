#ifndef EXPRESSIVECURVEAPPLYDIALOG_H
#define EXPRESSIVECURVEAPPLYDIALOG_H

#include <QDialog>
#include <QVector>
#include <QStringList>
#include "envelopelibraryDialog.h"
#include "envelopecurvecanvas.h"

class QComboBox;
class QLabel;
class QPushButton;
class QSlider;

/**
 * ExpressiveCurveApplyDialog - Choose an expressive curve name from the sounit
 * and draw a shape; the same shape is applied to every selected note under that
 * name (creating the curve on notes that don't have it yet).
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

    // Per-note mode toggle state
    bool getPerNoteMode() const;

    // Which named expressive curve to apply to
    QString getSelectedCurveName() const;

    // Pre-load an existing curve into the editor (switches preset to "Custom")
    void setInitialCurve(const QVector<EnvelopePoint> &points);

    // Pre-select a curve name in the target dropdown
    void setSelectedCurveName(const QString &name);

    // Draw the note's other curves dimmed behind the editable one. Pass ALL
    // of the note's curves including the target: the dialog filters out
    // whichever one the dropdown currently points at, so switching target
    // re-filters instead of leaving a curve ghosting itself.
    void setGhostCurves(const QVector<EnvelopeCurveCanvas::Ghost> &ghosts);

private slots:
    void onPresetChanged(int index);
    void onSaveClicked();
    void onLoadClicked();
    void onWeightChanged(int value);

private:
    void applyGhostFilter();
    QVector<EnvelopeCurveCanvas::Ghost> m_allGhosts;

    void setupUi(const QStringList &curveNames);
    void setupPresets();
    void loadPreset(int index);

    QComboBox *curveNameCombo;
    QComboBox *presetCombo;
    EnvelopeCurveCanvas *curveCanvas;
    QLabel *selectionLabel;
    QSlider *weightSlider;
    QLabel *weightValueLabel;
    QPushButton *perNoteToggle;
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

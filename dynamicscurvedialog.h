#ifndef DYNAMICSCURVEDIALOG_H
#define DYNAMICSCURVEDIALOG_H

#include <QDialog>
#include <QVector>
#include "envelopelibraryDialog.h"
#include "envelopecurvecanvas.h"

class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QSlider;

/**
 * DynamicsCurveDialog - Apply dynamics curve to selected notes
 *
 * Features:
 * - Factory presets (Crescendo, Decrescendo, Swell, etc.)
 * - Interactive EnvelopeCurveCanvas for custom editing
 * - Save/Load curves compatible with envelope library format
 * - Weight control (0-2) for subtle to boosted effects
 * - Shows selection info (note count and time span)
 * - Non-destructive with undo support
 */
class DynamicsCurveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DynamicsCurveDialog(int noteCount, double timeSpanMs, QWidget *parent = nullptr);

    // Get the curve to apply
    QVector<EnvelopePoint> getCurve() const;

    // Get the weight (0.0 to 2.0, where 1.0 is neutral)
    double getWeight() const;

    // Get per-note mode toggle state
    bool getPerNoteMode() const;

    // Pre-load an existing curve into the editor (switches preset to "Custom")
    void setInitialCurve(const QVector<EnvelopePoint> &points);

    // Configure as the legato-phrase curve editor: the curve spans a whole
    // legato phrase, so the per-note toggle (meaningless here) is hidden and
    // the canvas marks the note boundaries. `noteBoundaries` are normalized
    // 0..1 phrase positions of the note starts/ends.
    void setLegatoMode(const QVector<double> &noteBoundaries);

    // Draw the note's other curves dimmed behind the editable one.
    void setGhostCurves(const QVector<EnvelopeCurveCanvas::Ghost> &ghosts);
private slots:
    void onPresetChanged(int index);
    void onSaveClicked();
    void onLoadClicked();
    void onWeightChanged(int value);

private:
    void setupUi();
    void setupPresets();
    void loadPreset(int index);

    // UI components
    QComboBox *presetCombo;
    EnvelopeCurveCanvas *curveCanvas;
    QGroupBox *curveGroup = nullptr;
    QLabel *selectionLabel;
    QSlider *weightSlider;
    QLabel *weightValueLabel;
    QPushButton *perNoteToggle;
    QPushButton *saveButton;
    QPushButton *loadButton;
    QPushButton *applyButton;
    QPushButton *cancelButton;

    // Selection info
    int m_noteCount;
    double m_timeSpanMs;

    // Preset curves
    struct Preset {
        QString name;
        QVector<EnvelopePoint> curve;
    };
    QVector<Preset> presets;
};

#endif // DYNAMICSCURVEDIALOG_H

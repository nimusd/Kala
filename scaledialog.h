#ifndef SCALEDIALOG_H
#define SCALEDIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include "scale.h"

namespace Ui {
class ScaleDialog;
}

class ScaleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScaleDialog(const Scale &currentScale, double currentBaseFreq, bool isAtTimeZero, QWidget *parent = nullptr);
    ~ScaleDialog();

    // Get the selected scale
    Scale getSelectedScale() const;

    // Get the selected base frequency
    double getBaseFrequency() const;

    // Get the selected tonic index for ET (0-11, -1 if not ET or C)
    int getTonicIndex() const { return selectedTonicIndex; }

    // Check if delete was requested
    bool isDeleteRequested() const { return deleteRequested; }

private slots:
    void onDeleteClicked();
    void onScaleChanged(int index);
    void onKeyButtonClicked(int tonicIndex);

private:
    Ui::ScaleDialog *ui;
    bool deleteRequested = false;
    int selectedTonicIndex = -1;  // -1 = default (C for non-ET, or use scale's default)
    int currentScaleId = -1;
    QVector<QPushButton*> keyButtons;

    // Inflection UI
    QGroupBox *inflectionGroup = nullptr;
    QVBoxLayout *inflectionLayout = nullptr;
    QVector<QSlider*> inflectionSliders;
    QVector<QLabel*> inflectionCentsLabels;

    // Degree frequency display
    QGroupBox *degreeFreqGroup = nullptr;
    QHBoxLayout *degreeFreqLayout = nullptr;
    QDoubleSpinBox *degreeRefFreqSpin = nullptr;
    QVector<QLabel*> degreeFreqLabels;

    void updateKeyButtonStyles();
    void rebuildInflectionRows(const Scale &scale);
    void updateDegreeFrequencies();
    QVector<double> getCurrentCentOffsets() const;
};

#endif // SCALEDIALOG_H

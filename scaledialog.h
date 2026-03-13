#ifndef SCALEDIALOG_H
#define SCALEDIALOG_H

#include <QDialog>
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

    // Check if delete was requested
    bool isDeleteRequested() const { return deleteRequested; }

private slots:
    void onDeleteClicked();
    void onScaleChanged(int index);

private:
    Ui::ScaleDialog *ui;
    bool deleteRequested = false;
};

#endif // SCALEDIALOG_H

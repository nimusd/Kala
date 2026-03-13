#ifndef TEMPODIALOG_H
#define TEMPODIALOG_H

#include <QDialog>
#include "tempotimesignature.h"

namespace Ui {
class TempoDialog;
}

class TempoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TempoDialog(const TempoTimeSignature &current, bool isAtTimeZero, QWidget *parent = nullptr);
    ~TempoDialog();

    // Get the configured tempo/time signature
    TempoTimeSignature getTempoTimeSignature() const;

    // Check if delete was requested
    bool isDeleteRequested() const { return deleteRequested; }

private slots:
    void onDeleteClicked();

private:
    Ui::TempoDialog *ui;
    bool deleteRequested = false;
};

#endif // TEMPODIALOG_H

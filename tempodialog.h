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
    explicit TempoDialog(const TempoTimeSignature &current, bool isAtTimeZero,
                         bool isKalaMode, QWidget *parent = nullptr);
    ~TempoDialog();

    TempoTimeSignature getTempoTimeSignature() const;
    bool isDeleteRequested() const { return deleteRequested; }

private slots:
    void onDeleteClicked();

private:
    Ui::TempoDialog *ui;
    bool deleteRequested = false;
    bool m_isKalaMode = false;
};

#endif // TEMPODIALOG_H

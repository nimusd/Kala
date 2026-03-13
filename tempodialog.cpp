#include "tempodialog.h"
#include "ui_tempodialog.h"

TempoDialog::TempoDialog(const TempoTimeSignature &current, bool isAtTimeZero, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TempoDialog)
{
    ui->setupUi(this);

    // Set current values
    ui->spinTempo->setValue(static_cast<int>(current.bpm));
    ui->spinTimeSigNum->setValue(current.timeSigNumerator);
    ui->spinTimeSigDenom->setValue(current.timeSigDenominator);
    ui->checkGradual->setChecked(current.gradualTransition);

    // Disable delete button at time 0 (can't delete the default)
    ui->btnDelete->setEnabled(!isAtTimeZero);

    // Connect delete button
    connect(ui->btnDelete, &QPushButton::clicked, this, &TempoDialog::onDeleteClicked);
}

TempoDialog::~TempoDialog()
{
    delete ui;
}

TempoTimeSignature TempoDialog::getTempoTimeSignature() const
{
    TempoTimeSignature tts;
    tts.bpm = ui->spinTempo->value();
    tts.timeSigNumerator = ui->spinTimeSigNum->value();
    tts.timeSigDenominator = ui->spinTimeSigDenom->value();
    tts.gradualTransition = ui->checkGradual->isChecked();
    return tts;
}

void TempoDialog::onDeleteClicked()
{
    deleteRequested = true;
    accept();  // Close dialog with accepted state
}

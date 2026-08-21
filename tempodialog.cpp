#include "tempodialog.h"
#include "ui_tempodialog.h"

TempoDialog::TempoDialog(const TempoTimeSignature &current, bool isAtTimeZero,
                         bool isKalaMode, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TempoDialog)
    , m_isKalaMode(isKalaMode)
{
    ui->setupUi(this);

    if (isKalaMode) {
        ui->stackedMode->setCurrentWidget(ui->pageKala);
        ui->spinPulsesPerBar->setValue(current.timeSigNumerator);
        int pulseMs = static_cast<int>(60000.0 / current.bpm);
        if (pulseMs < 1) pulseMs = 1;
        if (pulseMs > 60000) pulseMs = 60000;
        ui->spinPulseDurationMs->setValue(pulseMs);
    } else {
        ui->stackedMode->setCurrentWidget(ui->pageWestern);
        ui->spinTempo->setValue(static_cast<int>(current.bpm));
        ui->spinTimeSigNum->setValue(current.timeSigNumerator);
        int denom = current.timeSigDenominator;
        if (denom < 1) denom = 4;  // Western mode needs a valid denominator
        ui->spinTimeSigDenom->setValue(denom);
    }

    ui->checkGradual->setChecked(current.gradualTransition);
    ui->btnDelete->setEnabled(!isAtTimeZero);

    connect(ui->btnDelete, &QPushButton::clicked, this, &TempoDialog::onDeleteClicked);
}

TempoDialog::~TempoDialog()
{
    delete ui;
}

TempoTimeSignature TempoDialog::getTempoTimeSignature() const
{
    TempoTimeSignature tts;
    if (m_isKalaMode) {
        int pulseMs = ui->spinPulseDurationMs->value();
        tts.bpm = 60000.0 / pulseMs;
        tts.timeSigNumerator = ui->spinPulsesPerBar->value();
        tts.timeSigDenominator = 0;
    } else {
        tts.bpm = ui->spinTempo->value();
        tts.timeSigNumerator = ui->spinTimeSigNum->value();
        tts.timeSigDenominator = ui->spinTimeSigDenom->value();
    }
    tts.gradualTransition = ui->checkGradual->isChecked();
    return tts;
}

void TempoDialog::onDeleteClicked()
{
    deleteRequested = true;
    accept();
}

#include "compositionsettingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QTimeEdit>
#include <QTime>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDebug>

CompositionSettingsDialog::CompositionSettingsDialog(const CompositionSettings &current, QWidget *parent)
    : QDialog(parent)
    , settings(current)
{
    qDebug() << "CompositionSettingsDialog: Constructor called";
    setupUI();
    setWindowTitle("Composition Settings");
    resize(400, 450);
    qDebug() << "CompositionSettingsDialog: Setup complete";
}

void CompositionSettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Form layout for settings
    QFormLayout *formLayout = new QFormLayout();

    // Name
    nameEdit = new QLineEdit(settings.compositionName, this);
    formLayout->addRow("Name:", nameEdit);

    // Time Mode (Radio buttons)
    QHBoxLayout *timeModeLayout = new QHBoxLayout();
    radioAbsolute = new QRadioButton("Absolute", this);
    radioMusical = new QRadioButton("Musical", this);
    timeModeGroup = new QButtonGroup(this);
    timeModeGroup->addButton(radioAbsolute, 0);
    timeModeGroup->addButton(radioMusical, 1);
    timeModeLayout->addWidget(radioAbsolute);
    timeModeLayout->addWidget(radioMusical);
    timeModeLayout->addStretch();

    if (settings.timeMode == CompositionSettings::Musical) {
        radioMusical->setChecked(true);
    } else {
        radioAbsolute->setChecked(true);
    }

    formLayout->addRow("Time Mode:", timeModeLayout);

    // Tempo
    tempoSpinBox = new QSpinBox(this);
    tempoSpinBox->setRange(20, 300);
    tempoSpinBox->setSuffix(" BPM");
    tempoSpinBox->setValue(settings.tempo);
    formLayout->addRow("Tempo:", tempoSpinBox);

    // Time Signature
    QHBoxLayout *timeSigLayout = new QHBoxLayout();
    timeSigNumerator = new QSpinBox(this);
    timeSigNumerator->setRange(1, 99);
    timeSigNumerator->setValue(settings.timeSigTop);
    timeSigDenominator = new QSpinBox(this);
    timeSigDenominator->setRange(0, 99);
    timeSigDenominator->setValue(settings.timeSigBottom);
    QLabel *slashLabel = new QLabel("/", this);

    timeSigLayout->addWidget(timeSigNumerator);
    timeSigLayout->addWidget(slashLabel);
    timeSigLayout->addWidget(timeSigDenominator);
    timeSigLayout->addStretch();

    formLayout->addRow("Time Signature:", timeSigLayout);

    // Length (Stacked: duration for Absolute, bars for Musical)
    lengthStack = new QStackedWidget(this);

    // Duration widget (for Absolute mode)
    QWidget *durationWidget = new QWidget(this);
    QHBoxLayout *durationLayout = new QHBoxLayout(durationWidget);
    durationEdit = new QTimeEdit(this);
    durationEdit->setDisplayFormat("HH:mm:ss");
    // Convert lengthMs to time
    int totalSeconds = static_cast<int>(settings.lengthMs / 1000.0);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;
    durationEdit->setTime(QTime(hours, minutes, seconds));
    durationLayout->addWidget(durationEdit);
    durationLayout->addStretch();

    // Bars widget (for Musical mode)
    QWidget *barsWidget = new QWidget(this);
    QHBoxLayout *barsLayout = new QHBoxLayout(barsWidget);
    barsSpinBox = new QSpinBox(this);
    barsSpinBox->setRange(1, 9999);
    barsSpinBox->setSuffix(" bars");
    barsSpinBox->setValue(settings.lengthBars);
    barsLayout->addWidget(barsSpinBox);
    barsLayout->addStretch();

    lengthStack->addWidget(durationWidget);  // Index 0: Absolute
    lengthStack->addWidget(barsWidget);      // Index 1: Musical

    // Set current stack page based on time mode
    lengthStack->setCurrentIndex(settings.timeMode == CompositionSettings::Musical ? 1 : 0);

    formLayout->addRow("Length:", lengthStack);

    mainLayout->addLayout(formLayout);

    // Calculated info group box
    QGroupBox *infoBox = new QGroupBox("Calculated Info", this);
    QVBoxLayout *infoLayout = new QVBoxLayout(infoBox);

    infoBarDuration = new QLabel(this);
    infoTotalDuration = new QLabel(this);

    infoLayout->addWidget(infoBarDuration);
    infoLayout->addWidget(infoTotalDuration);

    mainLayout->addWidget(infoBox);

    // Dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    mainLayout->addWidget(buttonBox);

    // Connect signals
    connect(timeModeGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &CompositionSettingsDialog::onTimeModeChanged);
    connect(tempoSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CompositionSettingsDialog::onTempoChanged);
    connect(timeSigNumerator, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CompositionSettingsDialog::onTimeSignatureChanged);
    connect(timeSigDenominator, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CompositionSettingsDialog::onTimeSignatureChanged);
    connect(durationEdit, &QTimeEdit::timeChanged,
            this, &CompositionSettingsDialog::onDurationChanged);
    connect(barsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CompositionSettingsDialog::onBarsChanged);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Initial calculation update
    updateLengthConversion();
}

void CompositionSettingsDialog::onTimeModeChanged(int buttonId)
{
    settings.timeMode = (buttonId == 1) ? CompositionSettings::Musical : CompositionSettings::Absolute;

    // Switch length input widget
    lengthStack->setCurrentIndex(buttonId);

    updateLengthConversion();
}

void CompositionSettingsDialog::onTempoChanged(int bpm)
{
    settings.tempo = bpm;
    updateLengthConversion();
}

void CompositionSettingsDialog::onTimeSignatureChanged()
{
    settings.timeSigTop = timeSigNumerator->value();
    settings.timeSigBottom = timeSigDenominator->value();
    updateLengthConversion();
}

void CompositionSettingsDialog::onDurationChanged()
{
    // User edited duration in Absolute mode
    QTime time = durationEdit->time();
    settings.lengthMs = (time.hour() * 3600000.0) + (time.minute() * 60000.0) + (time.second() * 1000.0);
    settings.syncLengthFromMs();

    // Update bars spinbox without triggering its signal
    barsSpinBox->blockSignals(true);
    barsSpinBox->setValue(settings.lengthBars);
    barsSpinBox->blockSignals(false);

    updateLengthConversion();
}

void CompositionSettingsDialog::onBarsChanged(int bars)
{
    // User edited bars in Musical mode
    settings.lengthBars = bars;
    settings.syncLengthFromBars();

    // Update duration edit without triggering its signal
    int totalSeconds = static_cast<int>(settings.lengthMs / 1000.0);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    durationEdit->blockSignals(true);
    durationEdit->setTime(QTime(hours, minutes, seconds));
    durationEdit->blockSignals(false);

    updateLengthConversion();
}

void CompositionSettingsDialog::updateLengthConversion()
{
    double barDuration = settings.calculateBarDuration();

    // Update info labels
    infoBarDuration->setText(QString("Bar duration: %1 ms").arg(barDuration, 0, 'f', 2));
    infoTotalDuration->setText(QString("Total: %1 bars = %2")
        .arg(settings.lengthBars)
        .arg(formatDuration(settings.lengthMs)));
}

QString CompositionSettingsDialog::formatDuration(double ms) const
{
    int totalSeconds = static_cast<int>(ms / 1000.0);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;
    int milliseconds = static_cast<int>(ms) % 1000;

    return QString("%1:%2.%3")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
        .arg(milliseconds, 3, 10, QChar('0'));
}

CompositionSettings CompositionSettingsDialog::getSettings() const
{
    // Read all values from dialog widgets
    CompositionSettings s;
    s.compositionName = nameEdit->text();
    s.timeMode = radioMusical->isChecked() ? CompositionSettings::Musical : CompositionSettings::Absolute;
    s.tempo = tempoSpinBox->value();
    s.timeSigTop = timeSigNumerator->value();
    s.timeSigBottom = timeSigDenominator->value();

    if (s.timeMode == CompositionSettings::Musical) {
        s.lengthBars = barsSpinBox->value();
        s.syncLengthFromBars();  // Calculate lengthMs from bars
    } else {
        // Get time from QTimeEdit (returns QTime, convert to milliseconds)
        QTime t = durationEdit->time();
        s.lengthMs = (t.hour() * 3600 + t.minute() * 60 + t.second()) * 1000.0 + t.msec();
        s.syncLengthFromMs();  // Calculate lengthBars from ms
    }

    return s;
}

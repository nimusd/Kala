#include "gotodialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDebug>

GotoDialog::GotoDialog(CompositionSettings::TimeMode mode,
                       int tempo,
                       int timeSigTop,
                       int timeSigBottom,
                       QWidget *parent)
    : QDialog(parent)
    , timeMode(mode)
    , currentTempo(tempo)
    , currentTimeSigTop(timeSigTop)
    , currentTimeSigBottom(timeSigBottom)
    , targetTimeMs(0.0)
{
    qDebug() << "GotoDialog: Constructor called with mode=" << mode;
    setupUI();
    setWindowTitle("Go To");
    resize(350, 200);
    qDebug() << "GotoDialog: Setup complete";
}

void GotoDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Mode label
    QLabel *modeLabel = new QLabel(this);
    if (timeMode == CompositionSettings::Musical) {
        modeLabel->setText("Go to position (Measure:Beat:Milliseconds):");
    } else {
        modeLabel->setText("Go to position (Minutes:Seconds:Milliseconds):");
    }
    mainLayout->addWidget(modeLabel);

    // Stacked widget for different input modes
    inputStack = new QStackedWidget(this);

    // === Absolute mode widget (MM:SS:MS) ===
    QWidget *absoluteWidget = new QWidget(this);
    QHBoxLayout *absoluteLayout = new QHBoxLayout(absoluteWidget);

    absMinutes = new QSpinBox(this);
    absMinutes->setRange(0, 9999);
    absMinutes->setSuffix(" min");
    absMinutes->setValue(0);

    QLabel *absColon1 = new QLabel(":", this);

    absSeconds = new QSpinBox(this);
    absSeconds->setRange(0, 59);
    absSeconds->setSuffix(" sec");
    absSeconds->setValue(0);

    QLabel *absDot = new QLabel(".", this);

    absMilliseconds = new QSpinBox(this);
    absMilliseconds->setRange(0, 999);
    absMilliseconds->setSuffix(" ms");
    absMilliseconds->setValue(0);

    absoluteLayout->addWidget(absMinutes);
    absoluteLayout->addWidget(absColon1);
    absoluteLayout->addWidget(absSeconds);
    absoluteLayout->addWidget(absDot);
    absoluteLayout->addWidget(absMilliseconds);
    absoluteLayout->addStretch();

    // === Musical mode widget (Measure:Beat:MS) ===
    QWidget *musicalWidget = new QWidget(this);
    QHBoxLayout *musicalLayout = new QHBoxLayout(musicalWidget);

    musicalMeasure = new QSpinBox(this);
    musicalMeasure->setRange(1, 9999);
    musicalMeasure->setPrefix("Bar ");
    musicalMeasure->setValue(1);

    QLabel *musicalColon = new QLabel(":", this);

    musicalBeat = new QSpinBox(this);
    musicalBeat->setRange(1, currentTimeSigTop);
    musicalBeat->setPrefix("Beat ");
    musicalBeat->setValue(1);

    QLabel *musicalDot = new QLabel(".", this);

    musicalMilliseconds = new QSpinBox(this);
    musicalMilliseconds->setRange(0, 999);
    musicalMilliseconds->setSuffix(" ms");
    musicalMilliseconds->setValue(0);

    musicalLayout->addWidget(musicalMeasure);
    musicalLayout->addWidget(musicalColon);
    musicalLayout->addWidget(musicalBeat);
    musicalLayout->addWidget(musicalDot);
    musicalLayout->addWidget(musicalMilliseconds);
    musicalLayout->addStretch();

    // Add widgets to stack
    inputStack->addWidget(absoluteWidget);  // Index 0: Absolute
    inputStack->addWidget(musicalWidget);   // Index 1: Musical

    // Set current stack page based on time mode
    inputStack->setCurrentIndex(timeMode == CompositionSettings::Musical ? 1 : 0);

    mainLayout->addWidget(inputStack);

    // Info label for calculated time
    infoLabel = new QLabel(this);
    infoLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");
    mainLayout->addWidget(infoLabel);

    // Dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    mainLayout->addWidget(buttonBox);

    // Connect signals
    connect(absMinutes, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GotoDialog::onAbsoluteTimeChanged);
    connect(absSeconds, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GotoDialog::onAbsoluteTimeChanged);
    connect(absMilliseconds, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GotoDialog::onAbsoluteTimeChanged);

    connect(musicalMeasure, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GotoDialog::onMusicalTimeChanged);
    connect(musicalBeat, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GotoDialog::onMusicalTimeChanged);
    connect(musicalMilliseconds, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GotoDialog::onMusicalTimeChanged);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Initial calculation
    if (timeMode == CompositionSettings::Musical) {
        onMusicalTimeChanged();
    } else {
        onAbsoluteTimeChanged();
    }

    // Set focus to first input field
    if (timeMode == CompositionSettings::Musical) {
        musicalMeasure->setFocus();
        musicalMeasure->selectAll();
    } else {
        absMinutes->setFocus();
        absMinutes->selectAll();
    }
}

void GotoDialog::onAbsoluteTimeChanged()
{
    // Calculate target time in milliseconds
    targetTimeMs = (absMinutes->value() * 60.0 * 1000.0) +
                   (absSeconds->value() * 1000.0) +
                   absMilliseconds->value();

    infoLabel->setText(QString("Target: %1 ms").arg(targetTimeMs, 0, 'f', 0));
}

void GotoDialog::onMusicalTimeChanged()
{
    // Calculate bar duration
    double barDuration = calculateBarDuration();
    double beatDuration = barDuration / currentTimeSigTop;

    // Calculate target time
    // Measures are 1-based, so subtract 1 to get 0-based bar index
    int barIndex = musicalMeasure->value() - 1;
    // Beats are 1-based, so subtract 1 to get 0-based beat index
    int beatIndex = musicalBeat->value() - 1;

    targetTimeMs = (barIndex * barDuration) +
                   (beatIndex * beatDuration) +
                   musicalMilliseconds->value();

    // Format time display
    int totalSeconds = static_cast<int>(targetTimeMs / 1000.0);
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    int ms = static_cast<int>(targetTimeMs) % 1000;

    infoLabel->setText(QString("Target: %1:%2.%3 (%4 ms)")
                          .arg(minutes, 2, 10, QChar('0'))
                          .arg(seconds, 2, 10, QChar('0'))
                          .arg(ms, 3, 10, QChar('0'))
                          .arg(targetTimeMs, 0, 'f', 0));
}

double GotoDialog::calculateBarDuration() const
{
    // Bar duration in milliseconds
    // BPM is beats per minute, where beat = 1/bottom note value
    // For 4/4 at 120 BPM: 4 beats per bar, each beat is 500ms, so bar = 2000ms
    double beatDurationMs = 60000.0 / currentTempo;  // Duration of one bottom-note beat
    double barDurationMs = beatDurationMs * currentTimeSigTop;
    return barDurationMs;
}

double GotoDialog::getTargetTimeMs() const
{
    return targetTimeMs;
}

#include "tempochangedialog.h"
#include "ui_tempochangedialog.h"

TempoChangeDialog::TempoChangeDialog(double currentTempo, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TempoChangeDialog)
    , m_updating(false)
{
    ui->setupUi(this);

    // Set initial values
    ui->spinOriginalTempo->setValue(currentTempo);
    ui->spinNewTempo->setValue(currentTempo);
    ui->spinProportion->setValue(1.0);  // No change initially

    // Connect signals for synchronization
    connect(ui->spinOriginalTempo, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TempoChangeDialog::onOriginalTempoChanged);
    connect(ui->spinNewTempo, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TempoChangeDialog::onNewTempoChanged);
    connect(ui->spinProportion, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TempoChangeDialog::onProportionChanged);
}

TempoChangeDialog::~TempoChangeDialog()
{
    delete ui;
}

double TempoChangeDialog::getProportion() const
{
    return ui->spinProportion->value();
}

double TempoChangeDialog::getOriginalTempo() const
{
    return ui->spinOriginalTempo->value();
}

double TempoChangeDialog::getNewTempo() const
{
    return ui->spinNewTempo->value();
}

void TempoChangeDialog::onOriginalTempoChanged(double value)
{
    if (m_updating) return;
    m_updating = true;

    // Recalculate proportion: original / new
    double newTempo = ui->spinNewTempo->value();
    if (newTempo > 0.0) {
        double proportion = value / newTempo;
        ui->spinProportion->setValue(proportion);
    }

    m_updating = false;
}

void TempoChangeDialog::onNewTempoChanged(double value)
{
    if (m_updating) return;
    m_updating = true;

    // Recalculate proportion: original / new
    double originalTempo = ui->spinOriginalTempo->value();
    if (value > 0.0) {
        double proportion = originalTempo / value;
        ui->spinProportion->setValue(proportion);
    }

    m_updating = false;
}

void TempoChangeDialog::onProportionChanged(double value)
{
    if (m_updating) return;
    m_updating = true;

    // Recalculate new tempo: original / proportion
    double originalTempo = ui->spinOriginalTempo->value();
    if (value > 0.0) {
        double newTempo = originalTempo / value;
        ui->spinNewTempo->setValue(newTempo);
    }

    m_updating = false;
}

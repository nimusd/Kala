#ifndef TEMPOCHANGEDIALOG_H
#define TEMPOCHANGEDIALOG_H

#include <QDialog>

namespace Ui {
class TempoChangeDialog;
}

/**
 * TempoChangeDialog - Scale note timing based on tempo proportion
 *
 * Provides 3 synchronized spinboxes:
 * - Original tempo (BPM at current timeline position)
 * - New tempo (target BPM)
 * - Proportion (original/new, editable directly or calculated)
 *
 * When the user changes tempo values, proportion is recalculated.
 * When the user changes proportion, new tempo is recalculated.
 */
class TempoChangeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TempoChangeDialog(double currentTempo, QWidget *parent = nullptr);
    ~TempoChangeDialog();

    // Get the calculated proportion (original/new)
    double getProportion() const;

    // Get tempo values
    double getOriginalTempo() const;
    double getNewTempo() const;

private slots:
    void onOriginalTempoChanged(double value);
    void onNewTempoChanged(double value);
    void onProportionChanged(double value);

private:
    Ui::TempoChangeDialog *ui;
    bool m_updating;  // Prevent recursive updates during synchronization
};

#endif // TEMPOCHANGEDIALOG_H

#ifndef ENVELOPEVISUALIZER_H
#define ENVELOPEVISUALIZER_H

#include <QWidget>
#include <QVector>
#include "envelopelibraryDialog.h"

/**
 * EnvelopeVisualizer - Visual display of envelope shape
 * 
 * Shows the envelope curve as a line graph over time (0.0 to 1.0)
 */
class EnvelopeVisualizer : public QWidget
{
    Q_OBJECT

public:
    explicit EnvelopeVisualizer(QWidget *parent = nullptr);

    // Set envelope type and parameters
    void setEnvelope(int envelopeType, double attack, double decay,
                    double sustain, double release, double fadeTime);

    // Set custom envelope data
    void setCustomEnvelope(const QVector<EnvelopePoint> &points);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int envelopeType = 0;      // 0=Linear, 1=Attack-Decay, 2=ADSR, 3=Fade In, 4=Fade Out, 5=Custom
    double attack = 0.1;
    double decay = 0.1;
    double sustain = 0.7;
    double release = 0.2;
    double fadeTime = 0.5;
    QVector<EnvelopePoint> customPoints;  // Custom envelope points

    // Calculate envelope value at normalized time (0.0-1.0)
    double calculateEnvelopeValue(double normalizedTime);
    double calculateCustomEnvelopeValue(double normalizedTime);
};

#endif // ENVELOPEVISUALIZER_H

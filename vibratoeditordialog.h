#ifndef VIBRATOEDITORDIALOG_H
#define VIBRATOEDITORDIALOG_H

#include <QDialog>
#include "vibrato.h"

class QDoubleSpinBox;
class QSlider;
class QLabel;
class QComboBox;
class QPushButton;
class EnvelopeCurveCanvas;

/**
 * VibratoEditorDialog - Edit vibrato parameters for a note
 *
 * Provides controls for:
 * - Rate (oscillation speed in Hz)
 * - Pitch Depth (how much pitch varies)
 * - Amplitude Depth (how much loudness varies)
 * - Onset (when vibrato starts, % into note)
 * - Regularity (0 = mechanical, 1 = organic)
 * - Envelope (intensity curve over note lifetime)
 */
class VibratoEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VibratoEditorDialog(QWidget *parent = nullptr);

    // Set/get vibrato data
    void setVibrato(const Vibrato &v);
    Vibrato getVibrato() const;

private slots:
    void onRateChanged(double value);
    void onPitchDepthChanged(double value);
    void onAmplitudeDepthChanged(double value);
    void onOnsetChanged(double value);
    void onRegularitySliderChanged(int value);
    void onEnvelopeChanged();
    void onLoadEnvelopeClicked();
    void onVibratoPresetChanged(int index);
    void onSaveVibratoPresetClicked();

private:
    void setupUI();
    void updateRegularityLabel();
    void loadFactoryPresets();
    void loadUserPresets();
    void saveUserPresets();
    void updatePresetCombo();
    void applyPreset(const Vibrato &preset);

    // Controls
    QDoubleSpinBox *spinRate;
    QDoubleSpinBox *spinPitchDepth;
    QDoubleSpinBox *spinAmplitudeDepth;
    QDoubleSpinBox *spinOnset;
    QSlider *sliderRegularity;
    QLabel *labelRegularityValue;
    EnvelopeCurveCanvas *envelopeCanvas;
    QPushButton *btnLoadEnvelope;
    QComboBox *comboVibratoPreset;
    QPushButton *btnSavePreset;

    // Current vibrato data
    Vibrato currentVibrato;

    // Presets
    struct VibratoPreset {
        QString name;
        Vibrato vibrato;
        bool isFactory;
    };
    QVector<VibratoPreset> factoryPresets;
    QVector<VibratoPreset> userPresets;
    bool updatingFromPreset = false;
};

#endif // VIBRATOEDITORDIALOG_H

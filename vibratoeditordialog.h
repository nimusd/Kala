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

    // Returns the user's default vibrato preset. Returns true if a default is configured.
    static bool getDefaultPreset(Vibrato &out);

private slots:
    void onRateChanged(double value);
    void onPitchDepthChanged(double value);
    void onAmplitudeDepthChanged(double value);
    void onOnsetChanged(double value);
    void onRegularitySliderChanged(int value);
    void onEnvelopeChanged();
    void onRateEnvelopeChanged();
    void onLoadEnvelopeClicked();
    void onVibratoPresetChanged(int index);
    void onSaveVibratoPresetClicked();
    void onDeleteVibratoPresetClicked();
    void onSetDefaultPresetClicked();

private:
    void setupUI();
    void updateRegularityLabel();
    void loadFactoryPresets();
    void loadUserPresets();
    void saveUserPresets();
    void updatePresetCombo();
    void updatePresetButtons();
    void applyPreset(const Vibrato &preset);
    bool isUserPresetSelected(int *userIndex = nullptr) const;
    int findMatchingPreset(const Vibrato &v) const;

    // Controls
    QDoubleSpinBox *spinRate;
    QDoubleSpinBox *spinPitchDepth;
    QDoubleSpinBox *spinAmplitudeDepth;
    QDoubleSpinBox *spinOnset;
    QSlider *sliderRegularity;
    QLabel *labelRegularityValue;
    EnvelopeCurveCanvas *envelopeCanvas;
    QPushButton *btnLoadEnvelope;
    EnvelopeCurveCanvas *rateEnvelopeCanvas;
    QLabel *labelRateEnvMax;
    QComboBox *comboVibratoPreset;
    QPushButton *btnSavePreset;
    QPushButton *btnDeletePreset;
    QPushButton *btnSetDefault;

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
    QString defaultPresetName;
    bool updatingFromPreset = false;
};

#endif // VIBRATOEDITORDIALOG_H

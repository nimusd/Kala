#ifndef SPECTRUMVISUALIZER_H
#define SPECTRUMVISUALIZER_H

#include <QWidget>
#include <QPainter>
#include <QVector>

/**
 * SpectrumVisualizer - Displays harmonic spectrum as vertical bars
 *
 * Shows the relative amplitudes of harmonics in a visual form.
 * Used in the Harmonic Generator inspector to preview the spectrum.
 *
 * Can be switched to editable mode for custom DNA pattern creation.
 */
class SpectrumVisualizer : public QWidget
{
    Q_OBJECT

public:
    explicit SpectrumVisualizer(QWidget *parent = nullptr);

    // Set the spectrum data to display
    void setSpectrum(const QVector<double> &amplitudes);

    // Set number of harmonics and rolloff for preview
    void setParameters(int numHarmonics, double rolloffPower);

    // Set DNA preset (0-4, or -1 for custom) with purity blend
    void setDnaPreset(int dnaPreset, int numHarmonics, double purity = 0.0);

    // Enable/disable editing mode
    void setEditable(bool editable);
    bool isEditable() const { return editMode; }

    // Get current spectrum (for saving custom patterns)
    QVector<double> getSpectrum() const { return harmonicAmplitudes; }

signals:
    // Emitted when spectrum is edited by user
    void spectrumChanged(const QVector<double> &amplitudes);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QVector<double> harmonicAmplitudes;
    int maxHarmonicsToShow = 64;  // Limit display for performance

    // Edit mode state
    bool editMode = false;
    int selectedHarmonic = -1;  // Index of currently selected harmonic (-1 = none)
    bool dragging = false;      // Currently dragging a bar
    int dragStartY = 0;         // Y position where drag started
    double dragStartAmp = 0.0;  // Amplitude when drag started

    // Generate spectrum from parameters (for preview without full calculation)
    void generatePreviewSpectrum(int numHarmonics, double rolloffPower);

    // Helper: Get harmonic index at given x position (-1 if none)
    int getHarmonicAtX(int x) const;

    // Helper: Check if mouse is over the draggable top area of a bar
    bool isOverBarTop(int x, int y, int harmonicIndex) const;

    // Helper: Set amplitude for a harmonic (0.0 to 1.0 normalized)
    void setHarmonicAmplitude(int index, double amplitude);
};

#endif // SPECTRUMVISUALIZER_H

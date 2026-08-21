#ifndef GUITARMODEL_H
#define GUITARMODEL_H

#include <QHash>
#include <cmath>
#include <QString>
#include <QVector>

// Pull in the full guitardsp class definition (via guitardsp.h →
// guitar_oud.cpp). This is the pattern Kala uses.
#include "guitardsp.h"

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

/// Wrapper around the Faust-generated 6-string nylon guitar model
/// (guitar_oud.cpp) with modal body resonator.
///
/// 0 audio inputs, 1 audio output (mono).
/// Exposes per-string pluck position, wound damping, body shape/scale,
/// and an active-string selector for the test app.
class GuitarModel
{
public:
    explicit GuitarModel(double sampleRate = 44100.0);
    ~GuitarModel();
    GuitarModel(const GuitarModel&);            // deep copy via guitardsp::clone()
    GuitarModel& operator=(const GuitarModel&);

    void reset(bool isLegato = false);

    double tick(double pitch, double noteProgress,
                bool isLegato, bool tailMode,
                double currentDynamics = 1.0);

    // Per-sample setters (become Kala container ports)
    void setPluckPosition(int string, double pos);   // 0–5, 0.0–1.0
    void setWoundDamping(double d);                   // 0.0–1.0
    void setSympatheticGain(double g);                // 0.0–0.05
    void setAirResonance(double freq);                // 80.0–150.0 Hz
    void setTopResonance(double freq);                // 150.0–300.0 Hz
    void setPluckHardness(double h);                  // 0.0–1.0 (soft→hard)
    void setNailFleshRatio(double r);                 // 0.0–1.0 (flesh→nail)
    void setOutputGain(double g);                     // 0.0–1.0
    void setActiveStringMask(unsigned int mask);      // Bitmask for strings 0-5 (0x3F = all)
    void setActiveString(int s);                      // Convenience: 0–5, or -1 for all
    void setActiveStrings(const QVector<bool>& strings); // Vector of 6 bools
    void setPitchMultiplier(double m) { pitchMultiplier = m; }

    // Amplitude-dependent pitch glide (Legge & Fletcher nonlinear string effect)
    void setPitchGlideAmount(double cents);  // 0–50 cents, max detune at full dynamics
    void setJitterAmount(double cents);      // 0–20 cents, irregular micro-detuning

private:
    double sampleRate;
    guitardsp* dspInstance = nullptr;
    QHash<QString, FAUSTFLOAT*> paramMap;
    double pitchMultiplier = 1.0;
    unsigned int activeStringMask = 0x3F;  // All 6 strings (bits 0-5 set)

    // Pitch glide state
    double pitchGlideAmount = 8.0;   // cents
    double jitterAmount = 3.0;       // cents

    // Slow irregular noise state (multi-sine with irrational ratios)
    double noisePhase1 = 0.0;
    double noisePhase2 = 0.0;
    double noisePhase3 = 0.0;
    double noisePhase4 = 0.0;

    FAUSTFLOAT getFaustParam(const QString& name) const;
    void setFaustParam(const QString& name, FAUSTFLOAT value);
    void applyDefaults();
    void setAllGates(FAUSTFLOAT value);

    /// Generate a single sample of slow irregular noise in [-1, 1].
    /// Uses four sine waves with irrational frequency ratios so the
    /// waveform never exactly repeats — no audible periodicity.
    double generateIrregularNoise();
};

#endif // GUITARMODEL_H

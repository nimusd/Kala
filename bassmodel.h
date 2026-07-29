#ifndef BASSMODEL_H
#define BASSMODEL_H

#include <QHash>
#include <QString>

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

// Forward declaration — bass.cpp defines bassdsp : public dsp
class bassdsp;

/// Wrapper around the Faust-generated Nonlinear WaveGuide Acoustic Bass model (bass.cpp).
///
/// 0 audio inputs, 2 audio outputs (stereo → averaged to mono).
/// Exposes 4 modulatable parameters; hardcodes reverb/spatial to neutral defaults.
class BassModel
{
public:
    explicit BassModel(double sampleRate = 44100.0);
    ~BassModel();
    BassModel(const BassModel&);            // deep copy via bassdsp::clone()
    BassModel& operator=(const BassModel&);

    void reset(bool isLegato = false);

    double tick(double pitch, double noteProgress,
                bool isLegato, bool tailMode,
                double currentDynamics = 1.0);

    // Per-sample setters (safe to call every sample for modulation)
    void setNonlinearity(double n);
    void setModulationFrequency(double f);
    void setModulationType(int t);
    void setTouchLength(double t);
    void setPitchMultiplier(double m) { pitchMultiplier = m; }

private:
    double sampleRate;
    bassdsp* dspInstance = nullptr;
    QHash<QString, FAUSTFLOAT*> paramMap;  // populated once at construction
    double pitchMultiplier = 1.0;

    FAUSTFLOAT getFaustParam(const QString& name) const;
    void setFaustParam(const QString& name, FAUSTFLOAT value);
    void applyDefaults();
};

#endif // BASSMODEL_H

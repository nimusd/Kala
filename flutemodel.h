#ifndef FLUTEMODEL_H
#define FLUTEMODEL_H

#include <QHash>
#include <QString>

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

// Forward declaration — flute.cpp defines flutedsp : public dsp
class flutedsp;

/// Wrapper around the Faust-generated Nonlinear Waveguide Flute model (flute.cpp).
///
/// 0 audio inputs, 2 audio outputs (stereo → averaged to mono).
/// Exposes 9 modulatable parameters; hardcodes ~8 others to sensible defaults.
class FluteModel
{
public:
    explicit FluteModel(double sampleRate = 44100.0);
    ~FluteModel();
    FluteModel(const FluteModel&);            // deep copy via flutedsp::clone()
    FluteModel& operator=(const FluteModel&);

    void reset(bool isLegato = false);

    double tick(double pitch, double noteProgress,
                bool isLegato, bool tailMode,
                double currentDynamics = 1.0);

    // Per-sample setters (safe to call every sample for modulation)
    void setBreathPressure(double p);
    void setNoiseGain(double g);
    void setNonlinearity(double n);
    void setNlAttack(double a);
    void setModFrequency(double f);
    void setModType(int t);
    void setVibratoFreq(double f);
    void setVibratoGain(double g);
    void setPitchMultiplier(double m) { pitchMultiplier = m; }

private:
    double sampleRate;
    flutedsp* dspInstance = nullptr;
    QHash<QString, FAUSTFLOAT*> paramMap;  // populated once at construction
    double pitchMultiplier = 1.0;

    FAUSTFLOAT getFaustParam(const QString& name) const;
    void setFaustParam(const QString& name, FAUSTFLOAT value);

    // Apply hardcoded defaults for non-exposed Faust params
    // Must be called after every init() (both constructors) since
    // instanceResetUserInterface() restores Faust defaults.
    void applyDefaults();
};

#endif // FLUTEMODEL_H

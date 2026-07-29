#ifndef PIANOMODEL_H
#define PIANOMODEL_H

#include <QHash>
#include <QString>

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

// Forward declaration — piano.cpp defines pianodsp : public dsp
class pianodsp;

/// Wrapper around the Faust-generated Nonlinear Piano model (piano.cpp).
///
/// 0 audio inputs, 2 audio outputs (stereo → averaged to mono).
/// Exposes 5 modulatable parameters; hardcodes reverb/spatial to neutral defaults.
class PianoModel
{
public:
    explicit PianoModel(double sampleRate = 44100.0);
    ~PianoModel();
    PianoModel(const PianoModel&);            // deep copy via pianodsp::clone()
    PianoModel& operator=(const PianoModel&);

    void reset(bool isLegato = false);

    double tick(double pitch, double noteProgress,
                bool isLegato, bool tailMode,
                double currentDynamics = 1.0);

    // Per-sample setters (safe to call every sample for modulation)
    void setBrightness(double b);
    void setDetuning(double d);
    void setHammerHardness(double h);
    void setStiffness(double s);
    void setSoftness(double s);
    void setPitchMultiplier(double m) { pitchMultiplier = m; }

private:
    double sampleRate;
    pianodsp* dspInstance = nullptr;
    QHash<QString, FAUSTFLOAT*> paramMap;  // populated once at construction
    double pitchMultiplier = 1.0;
    double m_softness = 1.0;

    FAUSTFLOAT getFaustParam(const QString& name) const;
    void setFaustParam(const QString& name, FAUSTFLOAT value);
    void applyDefaults();
};

#endif // PIANOMODEL_H

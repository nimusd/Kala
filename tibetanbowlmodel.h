#ifndef TIBETANBOWLMODEL_H
#define TIBETANBOWLMODEL_H

#include <QHash>
#include <QString>

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

class tibetanbowldsp;

/// Wrapper around the Faust-generated Tibetan Bowl model (tibetanBowl.cpp).
///
/// 0 audio inputs, 2 audio outputs (stereo → averaged to mono).
/// Exposes 7 modulatable parameters; hardcodes reverb/spatial to neutral defaults.
class TibetanBowlModel
{
public:
    explicit TibetanBowlModel(double sampleRate = 44100.0);
    ~TibetanBowlModel();
    TibetanBowlModel(const TibetanBowlModel&);
    TibetanBowlModel& operator=(const TibetanBowlModel&);

    void reset(bool isLegato = false);

    double tick(double pitch, double noteProgress,
                bool isLegato, bool tailMode,
                double currentDynamics = 1.0);

    void setNonlinearity(double n);
    void setModulationFrequency(double f);
    void setModulationType(int t);
    void setBaseGain(double g);
    void setBowPressure(double p);
    void setExcitationSelector(int s);
    void setIntegrationConstant(double c);
    void setPitchMultiplier(double m) { pitchMultiplier = m; }

private:
    double sampleRate;
    tibetanbowldsp* dspInstance = nullptr;
    QHash<QString, FAUSTFLOAT*> paramMap;
    double pitchMultiplier = 1.0;

    FAUSTFLOAT getFaustParam(const QString& name) const;
    void setFaustParam(const QString& name, FAUSTFLOAT value);
    void applyDefaults();
};

#endif // TIBETANBOWLMODEL_H

#ifndef INSTRUMENTMODEL_H
#define INSTRUMENTMODEL_H

/// Abstract interface for a physically-modeled instrument DSP core.
///
/// AudioEngine depends on this interface so it can drive any instrument
/// model (GuitarModel, OudModel, ...) without knowing the concrete type.
/// The interface mirrors the tick()/reset() contract that GuitarModel
/// already exposed — concrete models just inherit and override.
class InstrumentModel
{
public:
    virtual ~InstrumentModel() = default;

    virtual void reset(bool isLegato = false) = 0;

    virtual double tick(double pitch, double noteProgress,
                        bool isLegato, bool tailMode,
                        double currentDynamics = 1.0) = 0;
};

#endif // INSTRUMENTMODEL_H

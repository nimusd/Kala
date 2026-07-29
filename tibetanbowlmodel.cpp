#include "tibetanbowlmodel.h"
#include "faust_dummy.h"
#include "tibetanBowl.cpp"
#include <algorithm>

TibetanBowlModel::TibetanBowlModel(double sampleRate)
    : sampleRate(sampleRate)
{
    dspInstance = new tibetanbowldsp();
    dspInstance->init(static_cast<int>(sampleRate));

    ParameterMapUI ui;
    dspInstance->buildUserInterface(&ui);
    paramMap = ui.paramMap;

    applyDefaults();
}

TibetanBowlModel::~TibetanBowlModel()
{
    delete dspInstance;
}

TibetanBowlModel::TibetanBowlModel(const TibetanBowlModel& other)
    : sampleRate(other.sampleRate)
    , pitchMultiplier(other.pitchMultiplier)
{
    dspInstance = static_cast<tibetanbowldsp*>(other.dspInstance->clone());
    dspInstance->init(static_cast<int>(sampleRate));

    ParameterMapUI ui;
    dspInstance->buildUserInterface(&ui);
    paramMap = ui.paramMap;

    applyDefaults();
}

TibetanBowlModel& TibetanBowlModel::operator=(const TibetanBowlModel& other)
{
    if (this != &other) {
        delete dspInstance;
        sampleRate = other.sampleRate;
        pitchMultiplier = other.pitchMultiplier;
        dspInstance = static_cast<tibetanbowldsp*>(other.dspInstance->clone());
        dspInstance->init(static_cast<int>(sampleRate));

        ParameterMapUI ui;
        dspInstance->buildUserInterface(&ui);
        paramMap = ui.paramMap;

        applyDefaults();
    }
    return *this;
}

void TibetanBowlModel::applyDefaults()
{
    setFaustParam("reverbGain",   0.0f);
    setFaustParam("roomSize",     0.72f);
    setFaustParam("pan angle",     0.5f);
    setFaustParam("spatial width", 0.0f);

    setFaustParam("Nonlinearity",          0.0f);
    setFaustParam("Modulation_Frequency",  220.0f);
    setFaustParam("Modulation_Type",       0.0f);
    setFaustParam("Base_Gain",            1.0f);
    setFaustParam("Bow_Pressure",         0.2f);
    setFaustParam("Excitation_Selector",   0.0f);
    setFaustParam("Integration_Constant",  0.0f);
}

void TibetanBowlModel::reset(bool isLegato)
{
    if (!isLegato)
        dspInstance->instanceClear();
}

double TibetanBowlModel::tick(double pitch, double noteProgress,
                              bool isLegato, bool tailMode,
                              double currentDynamics)
{
    (void)noteProgress;
    (void)isLegato;

    double freqVal = pitch * pitchMultiplier;
    setFaustParam("freq", static_cast<FAUSTFLOAT>(freqVal));
    setFaustParam("gain", static_cast<FAUSTFLOAT>(currentDynamics));
    setFaustParam("gate", tailMode ? 0.0f : 1.0f);

    FAUSTFLOAT* inputs[1] = { nullptr };
    FAUSTFLOAT output0 = 0.0f;
    FAUSTFLOAT output1 = 0.0f;
    FAUSTFLOAT* outputs[2] = { &output0, &output1 };

    dspInstance->compute(1, inputs, outputs);

    return static_cast<double>((output0 + output1) * 0.5f);
}

FAUSTFLOAT TibetanBowlModel::getFaustParam(const QString& name) const
{
    auto it = paramMap.constFind(name);
    if (it != paramMap.constEnd())
        return *(it.value());
    return 0.0f;
}

void TibetanBowlModel::setFaustParam(const QString& name, FAUSTFLOAT value)
{
    auto it = paramMap.find(name);
    if (it != paramMap.end())
        *(it.value()) = value;
}

void TibetanBowlModel::setNonlinearity(double n)
{
    setFaustParam("Nonlinearity", static_cast<FAUSTFLOAT>(std::clamp(n, 0.0, 1.0)));
}

void TibetanBowlModel::setModulationFrequency(double f)
{
    setFaustParam("Modulation_Frequency", static_cast<FAUSTFLOAT>(std::clamp(f, 20.0, 1000.0)));
}

void TibetanBowlModel::setModulationType(int t)
{
    setFaustParam("Modulation_Type", static_cast<FAUSTFLOAT>(std::clamp(t, 0, 4)));
}

void TibetanBowlModel::setBaseGain(double g)
{
    setFaustParam("Base_Gain", static_cast<FAUSTFLOAT>(std::clamp(g, 0.0, 1.0)));
}

void TibetanBowlModel::setBowPressure(double p)
{
    setFaustParam("Bow_Pressure", static_cast<FAUSTFLOAT>(std::clamp(p, 0.0, 1.0)));
}

void TibetanBowlModel::setExcitationSelector(int s)
{
    setFaustParam("Excitation_Selector", static_cast<FAUSTFLOAT>(std::clamp(s, 0, 1)));
}

void TibetanBowlModel::setIntegrationConstant(double c)
{
    setFaustParam("Integration_Constant", static_cast<FAUSTFLOAT>(std::clamp(c, 0.0, 1.0)));
}

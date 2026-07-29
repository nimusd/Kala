#include "bassmodel.h"
#include "faust_dummy.h"
#include "bass.cpp"   // bassdsp full definition
#include <algorithm>

// ─────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────

BassModel::BassModel(double sampleRate)
    : sampleRate(sampleRate)
{
    dspInstance = new bassdsp();
    dspInstance->init(static_cast<int>(sampleRate));

    // Capture all Faust parameter pointers keyed by name
    ParameterMapUI ui;
    dspInstance->buildUserInterface(&ui);
    paramMap = ui.paramMap;

    applyDefaults();
}

BassModel::~BassModel()
{
    delete dspInstance;
}

BassModel::BassModel(const BassModel& other)
    : sampleRate(other.sampleRate)
    , pitchMultiplier(other.pitchMultiplier)
{
    dspInstance = static_cast<bassdsp*>(other.dspInstance->clone());
    dspInstance->init(static_cast<int>(sampleRate));

    ParameterMapUI ui;
    dspInstance->buildUserInterface(&ui);
    paramMap = ui.paramMap;

    applyDefaults();
}

BassModel& BassModel::operator=(const BassModel& other)
{
    if (this != &other) {
        delete dspInstance;
        sampleRate = other.sampleRate;
        pitchMultiplier = other.pitchMultiplier;
        dspInstance = static_cast<bassdsp*>(other.dspInstance->clone());
        dspInstance->init(static_cast<int>(sampleRate));

        ParameterMapUI ui;
        dspInstance->buildUserInterface(&ui);
        paramMap = ui.paramMap;

        applyDefaults();
    }
    return *this;
}

// ─────────────────────────────────────────────
//  Hardcoded defaults (called after every init)
// ─────────────────────────────────────────────

void BassModel::applyDefaults()
{
    // Reverb — neutralized (Kala has IR Convolution)
    setFaustParam("reverbGain",   0.0f);
    setFaustParam("roomSize",     0.72f);

    // Spatial — center, mono (Kala has its own Pan container)
    setFaustParam("pan angle",     0.5f);
    setFaustParam("spatial width", 0.0f);

    // Exposed params — set starting defaults
    setFaustParam("Nonlinearity",          0.0f);
    setFaustParam("Modulation_Frequency",  220.0f);
    setFaustParam("Modulation_Type",       0.0f);
    setFaustParam("Touch_Length",          0.15f);
}

// ─────────────────────────────────────────────
//  Reset
// ─────────────────────────────────────────────

void BassModel::reset(bool isLegato)
{
    if (!isLegato)
        dspInstance->instanceClear();
}

// ─────────────────────────────────────────────
//  Main sample generator
// ─────────────────────────────────────────────

double BassModel::tick(double pitch, double noteProgress,
                       bool isLegato, bool tailMode,
                       double currentDynamics)
{
    (void)noteProgress;
    (void)isLegato;

    // Set per-tick parameters
    double freqVal = pitch * pitchMultiplier;
    setFaustParam("freq", static_cast<FAUSTFLOAT>(freqVal));
    setFaustParam("gain", static_cast<FAUSTFLOAT>(currentDynamics));
    setFaustParam("gate", tailMode ? 0.0f : 1.0f);

    // Faust model: 0 inputs, 2 outputs
    FAUSTFLOAT* inputs[1] = { nullptr };
    FAUSTFLOAT output0 = 0.0f;
    FAUSTFLOAT output1 = 0.0f;
    FAUSTFLOAT* outputs[2] = { &output0, &output1 };

    dspInstance->compute(1, inputs, outputs);

    double result = static_cast<double>((output0 + output1) * 0.5f);
    return result;
}

// ─────────────────────────────────────────────
//  Parameter helpers
// ─────────────────────────────────────────────

FAUSTFLOAT BassModel::getFaustParam(const QString& name) const
{
    auto it = paramMap.constFind(name);
    if (it != paramMap.constEnd())
        return *(it.value());
    return 0.0f;
}

void BassModel::setFaustParam(const QString& name, FAUSTFLOAT value)
{
    auto it = paramMap.find(name);
    if (it != paramMap.end())
        *(it.value()) = value;
}

// ─────────────────────────────────────────────
//  Per-sample parameter setters
// ─────────────────────────────────────────────

void BassModel::setNonlinearity(double n)
{
    setFaustParam("Nonlinearity", static_cast<FAUSTFLOAT>(std::clamp(n, 0.0, 1.0)));
}

void BassModel::setModulationFrequency(double f)
{
    setFaustParam("Modulation_Frequency", static_cast<FAUSTFLOAT>(std::clamp(f, 20.0, 1000.0)));
}

void BassModel::setModulationType(int t)
{
    setFaustParam("Modulation_Type", static_cast<FAUSTFLOAT>(std::clamp(t, 0, 4)));
}

void BassModel::setTouchLength(double t)
{
    setFaustParam("Touch_Length", static_cast<FAUSTFLOAT>(std::clamp(t, 0.0, 1.0)));
}

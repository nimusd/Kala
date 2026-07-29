#include "flutemodel.h"
#include "faust_dummy.h"
#include "flute.cpp"   // flutedsp full definition
#include <algorithm>

// ─────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────

FluteModel::FluteModel(double sampleRate)
    : sampleRate(sampleRate)
{
    dspInstance = new flutedsp();
    dspInstance->init(static_cast<int>(sampleRate));

    // Capture all Faust parameter pointers keyed by name
    ParameterMapUI ui;
    dspInstance->buildUserInterface(&ui);
    paramMap = ui.paramMap;

    applyDefaults();
}

FluteModel::~FluteModel()
{
    delete dspInstance;
}

FluteModel::FluteModel(const FluteModel& other)
    : sampleRate(other.sampleRate)
    , pitchMultiplier(other.pitchMultiplier)
{
    dspInstance = static_cast<flutedsp*>(other.dspInstance->clone());
    dspInstance->init(static_cast<int>(sampleRate));

    // Re-capture param pointers from the cloned instance
    ParameterMapUI ui;
    dspInstance->buildUserInterface(&ui);
    paramMap = ui.paramMap;

    applyDefaults();
}

FluteModel& FluteModel::operator=(const FluteModel& other)
{
    if (this != &other) {
        delete dspInstance;
        sampleRate = other.sampleRate;
        pitchMultiplier = other.pitchMultiplier;
        dspInstance = static_cast<flutedsp*>(other.dspInstance->clone());
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

void FluteModel::applyDefaults()
{
    // Internal envelopes — minimal, Kala's Envelope Engine handles note shaping
    setFaustParam("Glob_Env_Attack",   0.01f);
    setFaustParam("Glob_Env_Release",  0.05f);
    setFaustParam("Press_Env_Attack",  0.01f);
    setFaustParam("Press_Env_Decay",   0.01f);
    setFaustParam("Press_Env_Release", 0.05f);
    setFaustParam("Pressure_Env",      1.0f);   // always enabled

    // Vibrato envelope defaults (start silent — set via inspector or port)
    setFaustParam("Vibrato_Gain",    0.0f);
    setFaustParam("Vibrato_Freq",    5.0f);
    setFaustParam("Vibrato_Attack",  0.5f);
    setFaustParam("Vibrato_Begin",   0.1f);
    setFaustParam("Vibrato_Release", 0.2f);

    // Reverb — neutralized (Kala has IR Convolution)
    setFaustParam("reverbGain", 0.0f);
    setFaustParam("roomSize",   0.72f);

    // Spatial — center, mono (Kala has its own Pan container)
    setFaustParam("pan angle",     0.5f);
    setFaustParam("spatial width", 0.0f);

    // Exposed params — set starting defaults
    setFaustParam("Pressure",               0.9f);
    setFaustParam("Noise Gain",            0.1f);
    setFaustParam("Nonlinearity",          0.0f);
    setFaustParam("Nonlinearity Attack",   0.1f);
    setFaustParam("Modulation_Frequency", 220.0f);
    setFaustParam("Modulation_Type",       0.0f);
}

// ─────────────────────────────────────────────
//  Reset
// ─────────────────────────────────────────────

void FluteModel::reset(bool isLegato)
{
    if (!isLegato)
        dspInstance->instanceClear();
}

// ─────────────────────────────────────────────
//  Main sample generator
// ─────────────────────────────────────────────

double FluteModel::tick(double pitch, double noteProgress,
                         bool isLegato, bool tailMode,
                         double currentDynamics)
{
    (void)noteProgress;
    (void)isLegato;

    // Set per-tick parameters
    setFaustParam("freq", static_cast<FAUSTFLOAT>(pitch * pitchMultiplier));
    setFaustParam("gain", static_cast<FAUSTFLOAT>(currentDynamics));
    setFaustParam("gate", tailMode ? 0.0f : 1.0f);

    // Faust model: 0 inputs, 2 outputs
    FAUSTFLOAT* inputs[1] = { nullptr };
    FAUSTFLOAT output0 = 0.0f;
    FAUSTFLOAT output1 = 0.0f;
    FAUSTFLOAT* outputs[2] = { &output0, &output1 };

    dspInstance->compute(1, inputs, outputs);

    return static_cast<double>((output0 + output1) * 0.5f);
}

// ─────────────────────────────────────────────
//  Parameter helpers
// ─────────────────────────────────────────────

FAUSTFLOAT FluteModel::getFaustParam(const QString& name) const
{
    auto it = paramMap.constFind(name);
    if (it != paramMap.constEnd())
        return *(it.value());
    return 0.0f;
}

void FluteModel::setFaustParam(const QString& name, FAUSTFLOAT value)
{
    auto it = paramMap.find(name);
    if (it != paramMap.end())
        *(it.value()) = value;
}

// ─────────────────────────────────────────────
//  Per-sample parameter setters
// ─────────────────────────────────────────────

void FluteModel::setBreathPressure(double p)
{
    setFaustParam("Pressure", static_cast<FAUSTFLOAT>(std::clamp(p, 0.0, 1.0)));
}

void FluteModel::setNoiseGain(double g)
{
    setFaustParam("Noise Gain", static_cast<FAUSTFLOAT>(std::clamp(g, 0.0, 1.0)));
}

void FluteModel::setNonlinearity(double n)
{
    setFaustParam("Nonlinearity", static_cast<FAUSTFLOAT>(std::clamp(n, 0.0, 1.0)));
}

void FluteModel::setNlAttack(double a)
{
    setFaustParam("Nonlinearity Attack", static_cast<FAUSTFLOAT>(std::clamp(a, 0.0, 2.0)));
}

void FluteModel::setModFrequency(double f)
{
    setFaustParam("Modulation_Frequency", static_cast<FAUSTFLOAT>(std::clamp(f, 20.0, 1000.0)));
}

void FluteModel::setModType(int t)
{
    setFaustParam("Modulation_Type", static_cast<FAUSTFLOAT>(std::clamp(t, 0, 4)));
}

void FluteModel::setVibratoFreq(double f)
{
    setFaustParam("Vibrato_Freq", static_cast<FAUSTFLOAT>(std::clamp(f, 1.0, 15.0)));
}

void FluteModel::setVibratoGain(double g)
{
    setFaustParam("Vibrato_Gain", static_cast<FAUSTFLOAT>(std::clamp(g, 0.0, 1.0)));
}

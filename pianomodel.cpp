#include "pianomodel.h"
#include "faust_dummy.h"
#include "piano.cpp"   // pianodsp full definition
#include <algorithm>

// ─────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────

PianoModel::PianoModel(double sampleRate)
    : sampleRate(sampleRate)
{
    dspInstance = new pianodsp();
    dspInstance->init(static_cast<int>(sampleRate));

    // Capture all Faust parameter pointers keyed by name
    ParameterMapUI ui;
    dspInstance->buildUserInterface(&ui);
    paramMap = ui.paramMap;

    applyDefaults();
}

PianoModel::~PianoModel()
{
    delete dspInstance;
}

PianoModel::PianoModel(const PianoModel& other)
    : sampleRate(other.sampleRate)
    , pitchMultiplier(other.pitchMultiplier)
{
    dspInstance = static_cast<pianodsp*>(other.dspInstance->clone());
    dspInstance->init(static_cast<int>(sampleRate));

    ParameterMapUI ui;
    dspInstance->buildUserInterface(&ui);
    paramMap = ui.paramMap;

    applyDefaults();
}

PianoModel& PianoModel::operator=(const PianoModel& other)
{
    if (this != &other) {
        delete dspInstance;
        sampleRate = other.sampleRate;
        pitchMultiplier = other.pitchMultiplier;
        dspInstance = static_cast<pianodsp*>(other.dspInstance->clone());
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

void PianoModel::applyDefaults()
{
    // Reverb — neutralized (Kala has IR Convolution)
    setFaustParam("reverbGain",   0.0f);
    setFaustParam("roomSize",     0.72f);

    // Spatial — center, mono (Kala has its own Pan container)
    setFaustParam("pan angle",     0.5f);
    setFaustParam("spatial width", 0.0f);

    // Exposed params — set starting defaults
    setFaustParam("Brightness_Factor", 0.0f);
    setFaustParam("Detuning_Factor",   0.1f);
    setFaustParam("Hammer_Hardness",   0.1f);
    setFaustParam("Stiffness_Factor",  0.28f);
}

// ─────────────────────────────────────────────
//  Reset
// ─────────────────────────────────────────────

void PianoModel::reset(bool isLegato)
{
    if (!isLegato)
        dspInstance->instanceClear();
}

// ─────────────────────────────────────────────
//  Main sample generator
// ─────────────────────────────────────────────

double PianoModel::tick(double pitch, double noteProgress,
                         bool isLegato, bool tailMode,
                         double currentDynamics)
{
    (void)noteProgress;
    (void)isLegato;

    // Set per-tick parameters
    double freqVal = pitch * pitchMultiplier;
    setFaustParam("freq", static_cast<FAUSTFLOAT>(freqVal));
    setFaustParam("gain", static_cast<FAUSTFLOAT>(currentDynamics * m_softness));
    setFaustParam("gate", tailMode ? 0.0f : 1.0f);

    // Faust model: 0 inputs, 2 outputs
    FAUSTFLOAT* inputs[1] = { nullptr };
    FAUSTFLOAT output0 = 0.0f;
    FAUSTFLOAT output1 = 0.0f;
    FAUSTFLOAT* outputs[2] = { &output0, &output1 };

    dspInstance->compute(1, inputs, outputs);

    double result = static_cast<double>((output0 + output1) * 0.5f);
    // Apply dynamics continuously so the curve acts as a volume envelope.
    // (The Faust "gain" parameter above only affects the hammer strike.)
    return result * currentDynamics;
}

// ─────────────────────────────────────────────
//  Parameter helpers
// ─────────────────────────────────────────────

FAUSTFLOAT PianoModel::getFaustParam(const QString& name) const
{
    auto it = paramMap.constFind(name);
    if (it != paramMap.constEnd())
        return *(it.value());
    return 0.0f;
}

void PianoModel::setFaustParam(const QString& name, FAUSTFLOAT value)
{
    auto it = paramMap.find(name);
    if (it != paramMap.end())
        *(it.value()) = value;
}

// ─────────────────────────────────────────────
//  Per-sample parameter setters
// ─────────────────────────────────────────────

void PianoModel::setBrightness(double b)
{
    setFaustParam("Brightness_Factor", static_cast<FAUSTFLOAT>(std::clamp(b, 0.0, 1.0)));
}

void PianoModel::setDetuning(double d)
{
    setFaustParam("Detuning_Factor", static_cast<FAUSTFLOAT>(std::clamp(d, 0.0, 1.0)));
}

void PianoModel::setHammerHardness(double h)
{
    setFaustParam("Hammer_Hardness", static_cast<FAUSTFLOAT>(std::clamp(h, 0.0, 1.0)));
}

void PianoModel::setStiffness(double s)
{
    setFaustParam("Stiffness_Factor", static_cast<FAUSTFLOAT>(std::clamp(s, 0.0, 1.0)));
}

void PianoModel::setSoftness(double s)
{
    m_softness = std::clamp(s, 0.0, 1.0);
}

#include "oudmodel.h"
#include <algorithm>

// ─────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────

OudModel::OudModel(double sampleRate)
    : sampleRate(sampleRate)
{
    dspInstance = new ouddsp();
    dspInstance->init(static_cast<int>(sampleRate));

    ParameterMapUI ui;
    dspInstance->buildUserInterface(&ui);
    paramMap = ui.paramMap;

    applyDefaults();
}

OudModel::~OudModel()
{
    delete dspInstance;
}

OudModel::OudModel(const OudModel& other)
    : sampleRate(other.sampleRate)
    , pitchMultiplier(other.pitchMultiplier)
    , activeCourseMask(other.activeCourseMask)
{
    dspInstance = static_cast<ouddsp*>(other.dspInstance->clone());
    dspInstance->init(static_cast<int>(sampleRate));

    ParameterMapUI ui;
    dspInstance->buildUserInterface(&ui);
    paramMap = ui.paramMap;

    applyDefaults();
}

OudModel& OudModel::operator=(const OudModel& other)
{
    if (this != &other) {
        delete dspInstance;
        sampleRate = other.sampleRate;
        pitchMultiplier = other.pitchMultiplier;
        activeCourseMask = other.activeCourseMask;
        dspInstance = static_cast<ouddsp*>(other.dspInstance->clone());
        dspInstance->init(static_cast<int>(sampleRate));

        ParameterMapUI ui;
        dspInstance->buildUserInterface(&ui);
        paramMap = ui.paramMap;

        applyDefaults();
    }
    return *this;
}

// ─────────────────────────────────────────────
//  Hardcoded defaults
// ─────────────────────────────────────────────

void OudModel::applyDefaults()
{
    // Output
    setFaustParam("outGain", 1.0f);

    // Sympathetic coupling (oud bridge couples strongly)
    setFaustParam("SympatheticGain", 0.05f);

    // Body — oud-tuned modal resonator
    setFaustParam("AirResonance", 140.0f);
    setFaustParam("TopResonance", 300.0f);

    // Plectrum excitation
    setFaustParam("PlectrumHardness", 0.7f);
    setFaustParam("PlectrumBrightness", 0.6f);

    // Course detune (chorus shimmer on doubled courses)
    setFaustParam("CourseDetune", 0.003f);

    // Per-course pluck positions (middle of string — oud default)
    for (int i = 0; i < 6; i++) {
        QString name = QString("PluckPos%1").arg(i);
        setFaustParam(name, 0.5f);
    }

    // All gates off
    setAllGates(0.0f);
}

// ─────────────────────────────────────────────
//  Gate helpers
// ─────────────────────────────────────────────

void OudModel::setAllGates(FAUSTFLOAT value)
{
    for (int i = 0; i < 6; i++) {
        QString name = QString("Gate%1").arg(i);
        setFaustParam(name, value);
    }
}

// ─────────────────────────────────────────────
//  Reset
// ─────────────────────────────────────────────

void OudModel::reset(bool isLegato)
{
    if (!isLegato)
        dspInstance->instanceClear();
}

// ─────────────────────────────────────────────
//  Main sample generator
// ─────────────────────────────────────────────

double OudModel::tick(double pitch, double noteProgress,
                      bool isLegato, bool tailMode,
                      double currentDynamics)
{
    (void)noteProgress;
    (void)isLegato;

    // Set frequency and gain
    double freqVal = pitch * pitchMultiplier;
    setFaustParam("freq", static_cast<FAUSTFLOAT>(freqVal));
    setFaustParam("gain", static_cast<FAUSTFLOAT>(currentDynamics));

    // Route gate to the selected course(s).
    // tailMode=true → all gates off → courses + body ring down naturally.
    // Doubled courses fire both voices via a single gate (gate per course).
    float gateVal = tailMode ? 0.0f : 1.0f;
    for (int i = 0; i < 6; i++) {
        QString gateName = QString("Gate%1").arg(i);
        bool courseActive = (activeCourseMask & (1 << i)) != 0;
        setFaustParam(gateName, courseActive ? gateVal : 0.0f);
    }

    // Faust model: 1 input (self-contained DSP), 1 output (mono).
    FAUSTFLOAT dummyIn = 0.0f;
    FAUSTFLOAT* inputs[1] = { &dummyIn };
    FAUSTFLOAT output0 = 0.0f;
    FAUSTFLOAT* outputs[1] = { &output0 };

    dspInstance->compute(1, inputs, outputs);

    // Apply dynamics as a continuous output gain so the note's dynamics
    // curve acts as a volume envelope for every sample.  (The Faust "gain"
    // parameter above only affects the 2‑sample excitation burst — without
    // this multiply the curve has no effect after the initial pluck.)
    return static_cast<double>(output0) * currentDynamics;
}

// ─────────────────────────────────────────────
//  Parameter helpers
// ─────────────────────────────────────────────

FAUSTFLOAT OudModel::getFaustParam(const QString& name) const
{
    auto it = paramMap.constFind(name);
    if (it != paramMap.constEnd())
        return *(it.value());
    return 0.0f;
}

void OudModel::setFaustParam(const QString& name, FAUSTFLOAT value)
{
    auto it = paramMap.find(name);
    if (it != paramMap.end())
        *(it.value()) = value;
}

// ─────────────────────────────────────────────
//  Per-sample parameter setters
// ─────────────────────────────────────────────

void OudModel::setPluckPosition(int course, double pos)
{
    if (course < 0 || course > 5) return;
    QString name = QString("PluckPos%1").arg(course);
    setFaustParam(name, static_cast<FAUSTFLOAT>(std::clamp(pos, 0.0, 1.0)));
}

void OudModel::setPlectrumHardness(double h)
{
    setFaustParam("PlectrumHardness", static_cast<FAUSTFLOAT>(std::clamp(h, 0.0, 1.0)));
}

void OudModel::setPlectrumBrightness(double b)
{
    setFaustParam("PlectrumBrightness", static_cast<FAUSTFLOAT>(std::clamp(b, 0.0, 1.0)));
}

void OudModel::setCourseDetune(double d)
{
    setFaustParam("CourseDetune", static_cast<FAUSTFLOAT>(std::clamp(d, 0.0, 0.01)));
}

void OudModel::setSympatheticGain(double g)
{
    setFaustParam("SympatheticGain", static_cast<FAUSTFLOAT>(std::clamp(g, 0.0, 0.05)));
}

void OudModel::setAirResonance(double freq)
{
    setFaustParam("AirResonance", static_cast<FAUSTFLOAT>(std::clamp(freq, 120.0, 180.0)));
}

void OudModel::setTopResonance(double freq)
{
    setFaustParam("TopResonance", static_cast<FAUSTFLOAT>(std::clamp(freq, 250.0, 400.0)));
}

void OudModel::setOutputGain(double g)
{
    setFaustParam("outGain", static_cast<FAUSTFLOAT>(std::clamp(g, 0.0, 1.0)));
}

// ─────────────────────────────────────────────
//  Course Selection Methods
// ─────────────────────────────────────────────

void OudModel::setActiveCourseMask(unsigned int mask)
{
    // Only use lower 6 bits
    activeCourseMask = mask & 0x3F;
}

void OudModel::setActiveCourse(int c)
{
    if (c < 0) {
        // All courses
        activeCourseMask = 0x3F;
    } else if (c >= 0 && c <= 5) {
        // Single course
        activeCourseMask = (1 << c);
    }
}

void OudModel::setActiveCourses(const QVector<bool>& courses)
{
    activeCourseMask = 0;
    for (int i = 0; i < 6 && i < courses.size(); ++i) {
        if (courses[i]) {
            activeCourseMask |= (1 << i);
        }
    }
}

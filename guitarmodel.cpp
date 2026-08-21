#include "guitarmodel.h"
#include <algorithm>

// ─────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────

GuitarModel::GuitarModel(double sampleRate)
    : sampleRate(sampleRate)
{
    dspInstance = new guitardsp();
    dspInstance->init(static_cast<int>(sampleRate));

    ParameterMapUI ui;
    dspInstance->buildUserInterface(&ui);
    paramMap = ui.paramMap;

    applyDefaults();
}

GuitarModel::~GuitarModel()
{
    delete dspInstance;
}

GuitarModel::GuitarModel(const GuitarModel& other)
    : sampleRate(other.sampleRate)
    , pitchMultiplier(other.pitchMultiplier)
    , activeStringMask(other.activeStringMask)
    , pitchGlideAmount(other.pitchGlideAmount)
    , jitterAmount(other.jitterAmount)
{
    dspInstance = static_cast<guitardsp*>(other.dspInstance->clone());
    dspInstance->init(static_cast<int>(sampleRate));

    ParameterMapUI ui;
    dspInstance->buildUserInterface(&ui);
    paramMap = ui.paramMap;

    applyDefaults();
}

GuitarModel& GuitarModel::operator=(const GuitarModel& other)
{
    if (this != &other) {
        delete dspInstance;
        sampleRate = other.sampleRate;
        pitchMultiplier = other.pitchMultiplier;
        activeStringMask = other.activeStringMask;
        pitchGlideAmount = other.pitchGlideAmount;
        jitterAmount = other.jitterAmount;
        dspInstance = static_cast<guitardsp*>(other.dspInstance->clone());
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

void GuitarModel::applyDefaults()
{
    // Output
    setFaustParam("outGain", 0.5f);

    // Sympathetic coupling
    setFaustParam("SympatheticGain", 0.05f);

    // Body — guitar-tuned modal resonator
    setFaustParam("AirResonance", 100.0f);
    setFaustParam("TopResonance", 200.0f);

    // Excitation
    setFaustParam("PluckHardness", 0.8f);
    setFaustParam("NailFleshRatio", 0.6f);

    // Wound damping
    setFaustParam("WoundDamping", 0.3f);

    // Per-string pluck positions
    for (int i = 0; i < 6; i++) {
        QString name = QString("PluckPos%1").arg(i);
        setFaustParam(name, 0.8f);
    }

    // All gates off
    setAllGates(0.0f);
}

// ─────────────────────────────────────────────
//  Gate helpers
// ─────────────────────────────────────────────

void GuitarModel::setAllGates(FAUSTFLOAT value)
{
    for (int i = 0; i < 6; i++) {
        QString name = QString("Gate%1").arg(i);
        setFaustParam(name, value);
    }
}

// ─────────────────────────────────────────────
//  Amplitude-dependent pitch glide
// ─────────────────────────────────────────────

void GuitarModel::setPitchGlideAmount(double cents)
{
    pitchGlideAmount = std::clamp(cents, 0.0, 50.0);
}

void GuitarModel::setJitterAmount(double cents)
{
    jitterAmount = std::clamp(cents, 0.0, 20.0);
}

double GuitarModel::generateIrregularNoise()
{
    // Four sine waves at irrational frequency ratios ensure the
    // combined waveform never exactly repeats — avoids the "mechanical"
    // quality of a single-rate vibrato LFO.
    // Base rate ~3 Hz gives slow, breathing irregularity.
    constexpr double baseRate = 3.0;
    double s1 = std::sin(noisePhase1);
    double s2 = std::sin(noisePhase2) * 0.5;
    double s3 = std::sin(noisePhase3) * 0.25;
    double s4 = std::sin(noisePhase4) * 0.125;

    double sample = (s1 + s2 + s3 + s4) / 1.875;

    // Advance phases — irrational multipliers prevent periodicity
    double step = 2.0 * M_PI * baseRate / sampleRate;
    noisePhase1 += step;
    noisePhase2 += step * std::sqrt(2.0);      // 1.41421…
    noisePhase3 += step * std::sqrt(3.0);      // 1.73205…
    noisePhase4 += step * std::sqrt(5.0);      // 2.23607…

    // Wrap to prevent eventual precision loss
    constexpr double twoPiThousand = 2.0 * M_PI * 1000.0;
    if (noisePhase1 > twoPiThousand) noisePhase1 -= twoPiThousand;
    if (noisePhase2 > twoPiThousand) noisePhase2 -= twoPiThousand;
    if (noisePhase3 > twoPiThousand) noisePhase3 -= twoPiThousand;
    if (noisePhase4 > twoPiThousand) noisePhase4 -= twoPiThousand;

    return sample;
}

// ─────────────────────────────────────────────
//  Reset
// ─────────────────────────────────────────────

void GuitarModel::reset(bool isLegato)
{
    if (!isLegato) {
        dspInstance->instanceClear();
        noisePhase1 = 0.0;
        noisePhase2 = 0.0;
        noisePhase3 = 0.0;
        noisePhase4 = 0.0;
    }
}

// ─────────────────────────────────────────────
//  Main sample generator
// ─────────────────────────────────────────────

double GuitarModel::tick(double pitch, double noteProgress,
                         bool isLegato, bool tailMode,
                         double currentDynamics)
{
    (void)noteProgress;
    (void)isLegato;

    // ── Amplitude-dependent pitch glide (Legge & Fletcher effect) ──
    // Real strings: large-amplitude vibration stretches the string more
    // than resting tension accounts for, so pitch starts sharp and
    // relaxes as amplitude decays.  The coupling is ~dynamics² because
    // the incremental tension from stretching scales with displacement².
    double dynamicsClamped = std::clamp(currentDynamics, 0.0, 1.0);
    double glideCents = pitchGlideAmount * dynamicsClamped * dynamicsClamped;

    // Irregular jitter coupled to amplitude — not a clean LFO vibrato.
    // At low dynamics the noise vanishes, at high dynamics it breathes.
    double jitterCents = jitterAmount * dynamicsClamped * generateIrregularNoise();

    double totalCents = glideCents + jitterCents;
    double freqMultiplier = std::pow(2.0, totalCents / 1200.0);

    // Set frequency and gain
    double freqVal = pitch * pitchMultiplier * freqMultiplier;
    setFaustParam("freq", static_cast<FAUSTFLOAT>(freqVal));
    setFaustParam("gain", static_cast<FAUSTFLOAT>(currentDynamics));

    // Route gate to the selected string(s)
    // tailMode=true → all gates off → strings + body ring down naturally
    float gateVal = tailMode ? 0.0f : 1.0f;
    for (int i = 0; i < 6; i++) {
        QString gateName = QString("Gate%1").arg(i);
        // Check if string i is active in the bitmask
        bool stringActive = (activeStringMask & (1 << i)) != 0;
        setFaustParam(gateName, stringActive ? gateVal : 0.0f);
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

FAUSTFLOAT GuitarModel::getFaustParam(const QString& name) const
{
    auto it = paramMap.constFind(name);
    if (it != paramMap.constEnd())
        return *(it.value());
    return 0.0f;
}

void GuitarModel::setFaustParam(const QString& name, FAUSTFLOAT value)
{
    auto it = paramMap.find(name);
    if (it != paramMap.end())
        *(it.value()) = value;
}

// ─────────────────────────────────────────────
//  Per-sample parameter setters
// ─────────────────────────────────────────────

void GuitarModel::setPluckPosition(int string, double pos)
{
    if (string < 0 || string > 5) return;
    QString name = QString("PluckPos%1").arg(string);
    setFaustParam(name, static_cast<FAUSTFLOAT>(std::clamp(pos, 0.0, 1.0)));
}

void GuitarModel::setWoundDamping(double d)
{
    setFaustParam("WoundDamping", static_cast<FAUSTFLOAT>(std::clamp(d, 0.0, 1.0)));
}

void GuitarModel::setSympatheticGain(double g)
{
    setFaustParam("SympatheticGain", static_cast<FAUSTFLOAT>(std::clamp(g, 0.0, 0.05)));
}

void GuitarModel::setAirResonance(double freq)
{
    setFaustParam("AirResonance", static_cast<FAUSTFLOAT>(std::clamp(freq, 80.0, 150.0)));
}

void GuitarModel::setTopResonance(double freq)
{
    setFaustParam("TopResonance", static_cast<FAUSTFLOAT>(std::clamp(freq, 150.0, 300.0)));
}

void GuitarModel::setPluckHardness(double h)
{
    setFaustParam("PluckHardness", static_cast<FAUSTFLOAT>(std::clamp(h, 0.0, 1.0)));
}

void GuitarModel::setNailFleshRatio(double r)
{
    setFaustParam("NailFleshRatio", static_cast<FAUSTFLOAT>(std::clamp(r, 0.0, 1.0)));
}

void GuitarModel::setOutputGain(double g)
{
    setFaustParam("outGain", static_cast<FAUSTFLOAT>(std::clamp(g, 0.0, 1.0)));
}

// ─────────────────────────────────────────────
//  String Selection Methods
// ─────────────────────────────────────────────

void GuitarModel::setActiveStringMask(unsigned int mask)
{
    // Only use lower 6 bits
    activeStringMask = mask & 0x3F;
}

void GuitarModel::setActiveString(int s)
{
    if (s < 0) {
        // All strings
        activeStringMask = 0x3F;
    } else if (s >= 0 && s <= 5) {
        // Single string
        activeStringMask = (1 << s);
    }
}

void GuitarModel::setActiveStrings(const QVector<bool>& strings)
{
    activeStringMask = 0;
    for (int i = 0; i < 6 && i < strings.size(); ++i) {
        if (strings[i]) {
            activeStringMask |= (1 << i);
        }
    }
}

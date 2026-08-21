#ifndef OUDMODEL_H
#define OUDMODEL_H

#include <QHash>
#include <QString>
#include <QVector>

#include "ouddsp.h"

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

/// Wrapper around the Faust-generated 6-course oud model (oud.cpp)
/// with oud-tuned modal body resonator and plectrum excitation.
///
/// 0 audio inputs, 1 audio output (mono).
/// Courses 1-5 are doubled (detuned second voice) for chorus shimmer.
/// Exposes per-course pluck position, plectrum controls, body shape,
/// and an active-course selector for the test app.
class OudModel
{
public:
    explicit OudModel(double sampleRate = 44100.0);
    ~OudModel();
    OudModel(const OudModel&);            // deep copy via ouddsp::clone()
    OudModel& operator=(const OudModel&);

    void reset(bool isLegato = false);

    double tick(double pitch, double noteProgress,
                bool isLegato, bool tailMode,
                double currentDynamics = 1.0);

    // Per-sample setters (become Kala container ports)
    void setPluckPosition(int course, double pos);    // 0–5, 0.0–1.0
    void setPlectrumHardness(double h);               // 0.0–1.0 (soft→hard)
    void setPlectrumBrightness(double b);             // 0.0–1.0 (dark→bright)
    void setCourseDetune(double d);                   // 0.0–0.01
    void setSympatheticGain(double g);                // 0.0–0.05
    void setAirResonance(double freq);                // 120.0–180.0 Hz
    void setTopResonance(double freq);                // 250.0–400.0 Hz
    void setOutputGain(double g);                     // 0.0–1.0
    void setActiveCourseMask(unsigned int mask);      // Bitmask for courses 0-5 (0x3F = all)
    void setActiveCourse(int c);                      // Convenience: 0–5, or -1 for all
    void setActiveCourses(const QVector<bool>& courses); // Vector of 6 bools
    void setPitchMultiplier(double m) { pitchMultiplier = m; }

private:
    double sampleRate;
    ouddsp* dspInstance = nullptr;
    QHash<QString, FAUSTFLOAT*> paramMap;
    double pitchMultiplier = 1.0;
    unsigned int activeCourseMask = 0x3F;  // All 6 courses (bits 0-5 set)

    FAUSTFLOAT getFaustParam(const QString& name) const;
    void setFaustParam(const QString& name, FAUSTFLOAT value);
    void applyDefaults();
    void setAllGates(FAUSTFLOAT value);
};

#endif // OUDMODEL_H

#ifndef FAUST_DUMMY_H
#define FAUST_DUMMY_H

// Minimal Faust base classes required by Faust-generated DSP code.
// Provides just enough interface for mydsp (flute.cpp) to compile and for
// FluteModel to capture parameter pointers via ParameterMapUI.

#include <QHash>
#include <QString>

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

// Stub for Faust's Meta class (used by metadata() in generated DSP)
class Meta
{
public:
    void declare(const char*, const char*) {}
};

class UI
{
public:
    virtual ~UI() = default;

    virtual void openVerticalBox(const char*) {}
    virtual void openHorizontalBox(const char*) {}
    virtual void closeBox() {}

    virtual void addHorizontalSlider(const char*, FAUSTFLOAT*, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT) {}
    virtual void addNumEntry(const char*, FAUSTFLOAT*, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT) {}
    virtual void addButton(const char*, FAUSTFLOAT*) {}
    virtual void addCheckButton(const char*, FAUSTFLOAT*) {}

    virtual void declare(FAUSTFLOAT*, const char*, const char*) {}
};

class dsp
{
public:
    virtual ~dsp() = default;

    virtual int getNumInputs() = 0;
    virtual int getNumOutputs() = 0;
    virtual void init(int sampleRate) = 0;
    virtual void instanceInit(int sampleRate) = 0;
    virtual void instanceConstants(int sampleRate) = 0;
    virtual void instanceClear() = 0;
    virtual void buildUserInterface(UI* ui) = 0;
    virtual dsp* clone() = 0;

    int getSampleRate() const { return fSampleRate; }

    virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) = 0;

protected:
    int fSampleRate = 44100;
};

// Captures Faust parameter pointers keyed by name during buildUserInterface.
// Used once during FluteModel construction, then discarded.
class ParameterMapUI : public UI
{
public:
    QHash<QString, FAUSTFLOAT*> paramMap;

    void addHorizontalSlider(const char* label, FAUSTFLOAT* zone,
                             FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT) override
    {
        paramMap[QString::fromUtf8(label)] = zone;
    }

    void addNumEntry(const char* label, FAUSTFLOAT* zone,
                     FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT) override
    {
        paramMap[QString::fromUtf8(label)] = zone;
    }

    void addButton(const char* label, FAUSTFLOAT* zone) override
    {
        paramMap[QString::fromUtf8(label)] = zone;
    }

    void addCheckButton(const char* label, FAUSTFLOAT* zone) override
    {
        paramMap[QString::fromUtf8(label)] = zone;
    }
};

#endif // FAUST_DUMMY_H

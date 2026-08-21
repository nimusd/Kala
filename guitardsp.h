#ifndef GUITARDSP_H
#define GUITARDSP_H

// Minimal header for the Faust-generated guitar DSP class.
// The full definition lives in guitar_oud.cpp, which is compiled
// as a separate translation unit.

#include "faust_dummy.h"

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

// Force the class name BEFORE including the generated code
#ifndef FAUSTCLASS
#define FAUSTCLASS guitardsp
#endif

// Include the Faust-generated implementation inline.
// This is the same pattern Kala uses: the generated .cpp is
// #include'd from a header so static lookup tables don't cause
// duplicate symbols across translation units.
#include "guitar_oud.cpp"

#endif // GUITARDSP_H

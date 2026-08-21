#ifndef OUDDSP_H
#define OUDDSP_H

// Minimal header for the Faust-generated oud DSP class.
// The full definition lives in oud.cpp, which is #include'd inline
// (same pattern as guitardsp.h) to avoid duplicate symbols from
// static lookup tables in the Faust-generated code.

#include "faust_dummy.h"

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

// Force the class name BEFORE including the generated code
#ifndef FAUSTCLASS
#define FAUSTCLASS ouddsp
#endif

#include "oud.cpp"

#endif // OUDDSP_H

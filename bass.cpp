/* ------------------------------------------------------------
author: "Romain Michon"
copyright: "Romain Michon (rmichon@ccrma.stanford.edu)"
name: "Bass"
version: "1.0"
Code generated with Faust 2.81.10 (https://faust.grame.fr)
Compilation options: -lang cpp -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __bassdsp_H__
#define  __bassdsp_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

/* link with : "" */
#include <algorithm>
#include "faust_dummy.h"
#include "bass_lookup.h"
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS bassdsp
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

#if defined(_WIN32)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

class bassdspSIG0 {
	
  private:
	
	int iVec2[2];
	int iRec13[2];
	
  public:
	
	int getNumInputsbassdspSIG0() {
		return 0;
	}
	int getNumOutputsbassdspSIG0() {
		return 1;
	}
	
	void instanceInitbassdspSIG0(int sample_rate) {
		for (int l13 = 0; l13 < 2; l13 = l13 + 1) {
			iVec2[l13] = 0;
		}
		for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
			iRec13[l14] = 0;
		}
	}
	
	void fillbassdspSIG0(int count, float* table) {
		for (int i1 = 0; i1 < count; i1 = i1 + 1) {
			iVec2[0] = 1;
			iRec13[0] = (iVec2[1] + iRec13[1]) % 65536;
			table[i1] = std::sin(9.58738e-05f * static_cast<float>(iRec13[0]));
			iVec2[1] = iVec2[0];
			iRec13[1] = iRec13[0];
		}
	}

};

static bassdspSIG0* newbassdspSIG0() { return (bassdspSIG0*)new bassdspSIG0(); }
static void deletebassdspSIG0(bassdspSIG0* dsp) { delete dsp; }

static float bassdsp_faustpower2_f(float value) {
	return value * value;
}
static float ftbl0bassdspSIG0[65536];

class bassdsp : public dsp {
	
 private:
	
	int iVec0[2];
	FAUSTFLOAT fButton0;
	int iRec2[2];
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fRec1[2];
	FAUSTFLOAT fEntry0;
	int IOTA0;
	float fRec5[2];
	float fConst2;
	float fRec4[2];
	float fVec1[2];
	float fConst3;
	float fConst4;
	FAUSTFLOAT fHslider0;
	float fRec6[2];
	FAUSTFLOAT fEntry1;
	float fRec12[2];
	float fRec11[2];
	float fRec10[2];
	float fRec9[2];
	float fRec8[2];
	float fRec7[2];
	FAUSTFLOAT fHslider1;
	float fRec15[2];
	float fConst5;
	float fRec14[2];
	float fRec21[2];
	float fRec20[2];
	float fRec19[2];
	float fRec18[2];
	float fRec17[2];
	float fRec16[2];
	float fVec3[2];
	float fRec3[2];
	int iRec24[2];
	FAUSTFLOAT fHslider2;
	float fConst6;
	FAUSTFLOAT fEntry2;
	float fRec25[2];
	float fVec4[2];
	float fConst7;
	float fRec26[2];
	float fRec23[2];
	float fRec22[2];
	float fRec0[8192];
	float fConst8;
	float fRec27[3];
	float fVec5[8192];
	FAUSTFLOAT fHslider3;
	float fRec28[2];
	FAUSTFLOAT fHslider4;
	FAUSTFLOAT fHslider5;
	float fConst9;
	float fConst10;
	float fConst11;
	float fConst12;
	float fConst13;
	float fConst14;
	float fConst15;
	float fRec40[2];
	float fConst16;
	float fRec39[2];
	float fVec6[16384];
	float fConst17;
	int iConst18;
	float fVec7[4096];
	int iConst19;
	float fVec8[4096];
	int iConst20;
	float fRec37[2];
	float fConst21;
	float fConst22;
	float fConst23;
	float fRec44[2];
	float fConst24;
	float fRec43[2];
	float fVec9[16384];
	float fConst25;
	int iConst26;
	float fVec10[2048];
	int iConst27;
	float fRec41[2];
	float fConst28;
	float fConst29;
	float fConst30;
	float fRec48[2];
	float fConst31;
	float fRec47[2];
	float fVec11[16384];
	float fConst32;
	int iConst33;
	float fVec12[4096];
	int iConst34;
	float fRec45[2];
	float fConst35;
	float fConst36;
	float fConst37;
	float fRec52[2];
	float fConst38;
	float fRec51[2];
	float fVec13[16384];
	float fConst39;
	int iConst40;
	float fVec14[2048];
	int iConst41;
	float fRec49[2];
	float fConst42;
	float fConst43;
	float fConst44;
	float fRec56[2];
	float fConst45;
	float fRec55[2];
	float fVec15[32768];
	float fConst46;
	int iConst47;
	FAUSTFLOAT fHslider6;
	float fConst48;
	float fVec16[4096];
	float fVec17[4096];
	int iConst49;
	float fRec53[2];
	float fConst50;
	float fConst51;
	float fConst52;
	float fRec60[2];
	float fConst53;
	float fRec59[2];
	float fVec18[16384];
	float fConst54;
	int iConst55;
	float fVec19[4096];
	int iConst56;
	float fRec57[2];
	float fConst57;
	float fConst58;
	float fConst59;
	float fRec64[2];
	float fConst60;
	float fRec63[2];
	float fVec20[32768];
	float fConst61;
	int iConst62;
	float fVec21[4096];
	int iConst63;
	float fRec61[2];
	float fConst64;
	float fConst65;
	float fConst66;
	float fRec68[2];
	float fConst67;
	float fRec67[2];
	float fVec22[32768];
	float fConst68;
	int iConst69;
	float fVec23[2048];
	int iConst70;
	float fRec65[2];
	float fRec29[3];
	float fRec30[3];
	float fRec31[3];
	float fRec32[3];
	float fRec33[3];
	float fRec34[3];
	float fRec35[3];
	float fRec36[3];
	
 public:
	bassdsp() {
	}
	
	void metadata(Meta* m) { 
		m->declare("author", "Romain Michon");
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "1.22.0");
		m->declare("compile_options", "-lang cpp -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("copyright", "Romain Michon (rmichon@ccrma.stanford.edu)");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.2.0");
		m->declare("description", "Nonlinear WaveGuide Acoustic Bass");
		m->declare("filename", "untitled.dsp");
		m->declare("filters.lib/allpass_comb:author", "Julius O. Smith III");
		m->declare("filters.lib/allpass_comb:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/allpass_comb:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/allpassnn:author", "Julius O. Smith III");
		m->declare("filters.lib/allpassnn:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/allpassnn:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass0_highpass1:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/tf1:author", "Julius O. Smith III");
		m->declare("filters.lib/tf1:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf1:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf1s:author", "Julius O. Smith III");
		m->declare("filters.lib/tf1s:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf1s:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/version", "1.7.1");
		m->declare("instruments.lib/author", "Romain Michon (rmichon@ccrma.stanford.edu)");
		m->declare("instruments.lib/copyright", "Romain Michon");
		m->declare("instruments.lib/licence", "STK-4.3");
		m->declare("instruments.lib/name", "Faust-STK Tools Library");
		m->declare("instruments.lib/version", "1.0.0");
		m->declare("licence", "STK-4.3");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.9.0");
		m->declare("name", "untitled");
		m->declare("noises.lib/name", "Faust Noise Generator Library");
		m->declare("noises.lib/version", "1.5.0");
		m->declare("oscillators.lib/name", "Faust Oscillator Library");
		m->declare("oscillators.lib/version", "1.6.0");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("reverbs.lib/name", "Faust Reverb Library");
		m->declare("reverbs.lib/version", "1.4.0");
		m->declare("routes.lib/hadamard:author", "Remy Muller, revised by Romain Michon");
		m->declare("routes.lib/name", "Faust Signal Routing Library");
		m->declare("routes.lib/version", "1.2.0");
		m->declare("signals.lib/name", "Faust Signal Routing Library");
		m->declare("signals.lib/version", "1.6.0");
		m->declare("version", "1.0");
	}

	virtual int getNumInputs() {
		return 0;
	}
	virtual int getNumOutputs() {
		return 2;
	}
	
	static void classInit(int sample_rate) {
		bassdspSIG0* sig0 = newbassdspSIG0();
		sig0->instanceInitbassdspSIG0(sample_rate);
		sig0->fillbassdspSIG0(65536, ftbl0bassdspSIG0);
		deletebassdspSIG0(sig0);
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, static_cast<float>(fSampleRate)));
		fConst1 = std::exp(-(1.4e+02f / fConst0));
		fConst2 = std::exp(-(7e+02f / fConst0));
		fConst3 = 44.1f / fConst0;
		fConst4 = 1.0f - fConst3;
		fConst5 = 1.0f / fConst0;
		fConst6 = 3.5f / fConst0;
		fConst7 = std::exp(-(3.5e+02f / fConst0));
		fConst8 = 1.994f * std::cos(678.584f / fConst0);
		fConst9 = std::floor(0.174713f * fConst0 + 0.5f);
		fConst10 = fConst9 / fConst0;
		fConst11 = 3.4538777f * fConst10;
		fConst12 = std::cos(37699.113f / fConst0);
		fConst13 = 1.0f / std::tan(628.31854f / fConst0);
		fConst14 = 1.0f - fConst13;
		fConst15 = 1.0f / (fConst13 + 1.0f);
		fConst16 = 2.3025851f * fConst10;
		fConst17 = std::floor(0.022904f * fConst0 + 0.5f);
		iConst18 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst9 - fConst17)));
		iConst19 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, 0.02f * fConst0)));
		iConst20 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst17 + -1.0f)));
		fConst21 = std::floor(0.153129f * fConst0 + 0.5f);
		fConst22 = fConst21 / fConst0;
		fConst23 = 3.4538777f * fConst22;
		fConst24 = 2.3025851f * fConst22;
		fConst25 = std::floor(0.020346f * fConst0 + 0.5f);
		iConst26 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst21 - fConst25)));
		iConst27 = static_cast<int>(std::min<float>(1024.0f, std::max<float>(0.0f, fConst25 + -1.0f)));
		fConst28 = std::floor(0.127837f * fConst0 + 0.5f);
		fConst29 = fConst28 / fConst0;
		fConst30 = 3.4538777f * fConst29;
		fConst31 = 2.3025851f * fConst29;
		fConst32 = std::floor(0.031604f * fConst0 + 0.5f);
		iConst33 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst28 - fConst32)));
		iConst34 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst32 + -1.0f)));
		fConst35 = std::floor(0.125f * fConst0 + 0.5f);
		fConst36 = fConst35 / fConst0;
		fConst37 = 3.4538777f * fConst36;
		fConst38 = 2.3025851f * fConst36;
		fConst39 = std::floor(0.013458f * fConst0 + 0.5f);
		iConst40 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst35 - fConst39)));
		iConst41 = static_cast<int>(std::min<float>(1024.0f, std::max<float>(0.0f, fConst39 + -1.0f)));
		fConst42 = std::floor(0.210389f * fConst0 + 0.5f);
		fConst43 = fConst42 / fConst0;
		fConst44 = 3.4538777f * fConst43;
		fConst45 = 2.3025851f * fConst43;
		fConst46 = std::floor(0.024421f * fConst0 + 0.5f);
		iConst47 = static_cast<int>(std::min<float>(16384.0f, std::max<float>(0.0f, fConst42 - fConst46)));
		fConst48 = 0.5f * fConst0;
		iConst49 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst46 + -1.0f)));
		fConst50 = std::floor(0.192303f * fConst0 + 0.5f);
		fConst51 = fConst50 / fConst0;
		fConst52 = 3.4538777f * fConst51;
		fConst53 = 2.3025851f * fConst51;
		fConst54 = std::floor(0.029291f * fConst0 + 0.5f);
		iConst55 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst50 - fConst54)));
		iConst56 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst54 + -1.0f)));
		fConst57 = std::floor(0.256891f * fConst0 + 0.5f);
		fConst58 = fConst57 / fConst0;
		fConst59 = 3.4538777f * fConst58;
		fConst60 = 2.3025851f * fConst58;
		fConst61 = std::floor(0.027333f * fConst0 + 0.5f);
		iConst62 = static_cast<int>(std::min<float>(16384.0f, std::max<float>(0.0f, fConst57 - fConst61)));
		iConst63 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst61 + -1.0f)));
		fConst64 = std::floor(0.219991f * fConst0 + 0.5f);
		fConst65 = fConst64 / fConst0;
		fConst66 = 3.4538777f * fConst65;
		fConst67 = 2.3025851f * fConst65;
		fConst68 = std::floor(0.019123f * fConst0 + 0.5f);
		iConst69 = static_cast<int>(std::min<float>(16384.0f, std::max<float>(0.0f, fConst64 - fConst68)));
		iConst70 = static_cast<int>(std::min<float>(1024.0f, std::max<float>(0.0f, fConst68 + -1.0f)));
	}
	
	virtual void instanceResetUserInterface() {
		fButton0 = static_cast<FAUSTFLOAT>(0.0f);
		fEntry0 = static_cast<FAUSTFLOAT>(1.2e+02f);
		fHslider0 = static_cast<FAUSTFLOAT>(0.0f);
		fEntry1 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider1 = static_cast<FAUSTFLOAT>(2.2e+02f);
		fHslider2 = static_cast<FAUSTFLOAT>(0.15f);
		fEntry2 = static_cast<FAUSTFLOAT>(1.0f);
		fHslider3 = static_cast<FAUSTFLOAT>(0.137f);
		fHslider4 = static_cast<FAUSTFLOAT>(0.6f);
		fHslider5 = static_cast<FAUSTFLOAT>(0.72f);
		fHslider6 = static_cast<FAUSTFLOAT>(0.5f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iVec0[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			iRec2[l1] = 0;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec1[l2] = 0.0f;
		}
		IOTA0 = 0;
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec5[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fRec4[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fVec1[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fRec6[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			fRec12[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
			fRec11[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
			fRec10[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			fRec9[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fRec8[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			fRec7[l12] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fRec15[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 2; l16 = l16 + 1) {
			fRec14[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2; l17 = l17 + 1) {
			fRec21[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec20[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 2; l19 = l19 + 1) {
			fRec19[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 2; l20 = l20 + 1) {
			fRec18[l20] = 0.0f;
		}
		for (int l21 = 0; l21 < 2; l21 = l21 + 1) {
			fRec17[l21] = 0.0f;
		}
		for (int l22 = 0; l22 < 2; l22 = l22 + 1) {
			fRec16[l22] = 0.0f;
		}
		for (int l23 = 0; l23 < 2; l23 = l23 + 1) {
			fVec3[l23] = 0.0f;
		}
		for (int l24 = 0; l24 < 2; l24 = l24 + 1) {
			fRec3[l24] = 0.0f;
		}
		for (int l25 = 0; l25 < 2; l25 = l25 + 1) {
			iRec24[l25] = 0;
		}
		for (int l26 = 0; l26 < 2; l26 = l26 + 1) {
			fRec25[l26] = 0.0f;
		}
		for (int l27 = 0; l27 < 2; l27 = l27 + 1) {
			fVec4[l27] = 0.0f;
		}
		for (int l28 = 0; l28 < 2; l28 = l28 + 1) {
			fRec26[l28] = 0.0f;
		}
		for (int l29 = 0; l29 < 2; l29 = l29 + 1) {
			fRec23[l29] = 0.0f;
		}
		for (int l30 = 0; l30 < 2; l30 = l30 + 1) {
			fRec22[l30] = 0.0f;
		}
		for (int l31 = 0; l31 < 8192; l31 = l31 + 1) {
			fRec0[l31] = 0.0f;
		}
		for (int l32 = 0; l32 < 3; l32 = l32 + 1) {
			fRec27[l32] = 0.0f;
		}
		for (int l33 = 0; l33 < 8192; l33 = l33 + 1) {
			fVec5[l33] = 0.0f;
		}
		for (int l34 = 0; l34 < 2; l34 = l34 + 1) {
			fRec28[l34] = 0.0f;
		}
		for (int l35 = 0; l35 < 2; l35 = l35 + 1) {
			fRec40[l35] = 0.0f;
		}
		for (int l36 = 0; l36 < 2; l36 = l36 + 1) {
			fRec39[l36] = 0.0f;
		}
		for (int l37 = 0; l37 < 16384; l37 = l37 + 1) {
			fVec6[l37] = 0.0f;
		}
		for (int l38 = 0; l38 < 4096; l38 = l38 + 1) {
			fVec7[l38] = 0.0f;
		}
		for (int l39 = 0; l39 < 4096; l39 = l39 + 1) {
			fVec8[l39] = 0.0f;
		}
		for (int l40 = 0; l40 < 2; l40 = l40 + 1) {
			fRec37[l40] = 0.0f;
		}
		for (int l41 = 0; l41 < 2; l41 = l41 + 1) {
			fRec44[l41] = 0.0f;
		}
		for (int l42 = 0; l42 < 2; l42 = l42 + 1) {
			fRec43[l42] = 0.0f;
		}
		for (int l43 = 0; l43 < 16384; l43 = l43 + 1) {
			fVec9[l43] = 0.0f;
		}
		for (int l44 = 0; l44 < 2048; l44 = l44 + 1) {
			fVec10[l44] = 0.0f;
		}
		for (int l45 = 0; l45 < 2; l45 = l45 + 1) {
			fRec41[l45] = 0.0f;
		}
		for (int l46 = 0; l46 < 2; l46 = l46 + 1) {
			fRec48[l46] = 0.0f;
		}
		for (int l47 = 0; l47 < 2; l47 = l47 + 1) {
			fRec47[l47] = 0.0f;
		}
		for (int l48 = 0; l48 < 16384; l48 = l48 + 1) {
			fVec11[l48] = 0.0f;
		}
		for (int l49 = 0; l49 < 4096; l49 = l49 + 1) {
			fVec12[l49] = 0.0f;
		}
		for (int l50 = 0; l50 < 2; l50 = l50 + 1) {
			fRec45[l50] = 0.0f;
		}
		for (int l51 = 0; l51 < 2; l51 = l51 + 1) {
			fRec52[l51] = 0.0f;
		}
		for (int l52 = 0; l52 < 2; l52 = l52 + 1) {
			fRec51[l52] = 0.0f;
		}
		for (int l53 = 0; l53 < 16384; l53 = l53 + 1) {
			fVec13[l53] = 0.0f;
		}
		for (int l54 = 0; l54 < 2048; l54 = l54 + 1) {
			fVec14[l54] = 0.0f;
		}
		for (int l55 = 0; l55 < 2; l55 = l55 + 1) {
			fRec49[l55] = 0.0f;
		}
		for (int l56 = 0; l56 < 2; l56 = l56 + 1) {
			fRec56[l56] = 0.0f;
		}
		for (int l57 = 0; l57 < 2; l57 = l57 + 1) {
			fRec55[l57] = 0.0f;
		}
		for (int l58 = 0; l58 < 32768; l58 = l58 + 1) {
			fVec15[l58] = 0.0f;
		}
		for (int l59 = 0; l59 < 4096; l59 = l59 + 1) {
			fVec16[l59] = 0.0f;
		}
		for (int l60 = 0; l60 < 4096; l60 = l60 + 1) {
			fVec17[l60] = 0.0f;
		}
		for (int l61 = 0; l61 < 2; l61 = l61 + 1) {
			fRec53[l61] = 0.0f;
		}
		for (int l62 = 0; l62 < 2; l62 = l62 + 1) {
			fRec60[l62] = 0.0f;
		}
		for (int l63 = 0; l63 < 2; l63 = l63 + 1) {
			fRec59[l63] = 0.0f;
		}
		for (int l64 = 0; l64 < 16384; l64 = l64 + 1) {
			fVec18[l64] = 0.0f;
		}
		for (int l65 = 0; l65 < 4096; l65 = l65 + 1) {
			fVec19[l65] = 0.0f;
		}
		for (int l66 = 0; l66 < 2; l66 = l66 + 1) {
			fRec57[l66] = 0.0f;
		}
		for (int l67 = 0; l67 < 2; l67 = l67 + 1) {
			fRec64[l67] = 0.0f;
		}
		for (int l68 = 0; l68 < 2; l68 = l68 + 1) {
			fRec63[l68] = 0.0f;
		}
		for (int l69 = 0; l69 < 32768; l69 = l69 + 1) {
			fVec20[l69] = 0.0f;
		}
		for (int l70 = 0; l70 < 4096; l70 = l70 + 1) {
			fVec21[l70] = 0.0f;
		}
		for (int l71 = 0; l71 < 2; l71 = l71 + 1) {
			fRec61[l71] = 0.0f;
		}
		for (int l72 = 0; l72 < 2; l72 = l72 + 1) {
			fRec68[l72] = 0.0f;
		}
		for (int l73 = 0; l73 < 2; l73 = l73 + 1) {
			fRec67[l73] = 0.0f;
		}
		for (int l74 = 0; l74 < 32768; l74 = l74 + 1) {
			fVec22[l74] = 0.0f;
		}
		for (int l75 = 0; l75 < 2048; l75 = l75 + 1) {
			fVec23[l75] = 0.0f;
		}
		for (int l76 = 0; l76 < 2; l76 = l76 + 1) {
			fRec65[l76] = 0.0f;
		}
		for (int l77 = 0; l77 < 3; l77 = l77 + 1) {
			fRec29[l77] = 0.0f;
		}
		for (int l78 = 0; l78 < 3; l78 = l78 + 1) {
			fRec30[l78] = 0.0f;
		}
		for (int l79 = 0; l79 < 3; l79 = l79 + 1) {
			fRec31[l79] = 0.0f;
		}
		for (int l80 = 0; l80 < 3; l80 = l80 + 1) {
			fRec32[l80] = 0.0f;
		}
		for (int l81 = 0; l81 < 3; l81 = l81 + 1) {
			fRec33[l81] = 0.0f;
		}
		for (int l82 = 0; l82 < 3; l82 = l82 + 1) {
			fRec34[l82] = 0.0f;
		}
		for (int l83 = 0; l83 < 3; l83 = l83 + 1) {
			fRec35[l83] = 0.0f;
		}
		for (int l84 = 0; l84 < 3; l84 = l84 + 1) {
			fRec36[l84] = 0.0f;
		}
	}
	
	virtual void init(int sample_rate) {
		classInit(sample_rate);
		instanceInit(sample_rate);
	}
	
	virtual void instanceInit(int sample_rate) {
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
	}
	
	virtual bassdsp* clone() {
		return new bassdsp();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("untitled");
		ui_interface->openHorizontalBox("Basic_Parameters");
		ui_interface->declare(&fEntry0, "1", "");
		ui_interface->declare(&fEntry0, "tooltip", "Tone frequency");
		ui_interface->declare(&fEntry0, "unit", "Hz");
		ui_interface->addNumEntry("freq", &fEntry0, FAUSTFLOAT(1.2e+02f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(2e+04f), FAUSTFLOAT(1.0f));
		ui_interface->declare(&fEntry2, "1", "");
		ui_interface->declare(&fEntry2, "tooltip", "Gain (value between 0 and 1)");
		ui_interface->addNumEntry("gain", &fEntry2, FAUSTFLOAT(1.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fButton0, "1", "");
		ui_interface->declare(&fButton0, "tooltip", "noteOn = 1, noteOff = 0");
		ui_interface->addButton("gate", &fButton0);
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Nonlinear_Filter_Parameters");
		ui_interface->declare(&fHslider1, "3", "");
		ui_interface->declare(&fHslider1, "tooltip", "Frequency of the sine wave for the modulation of theta (works if Modulation Type=3)");
		ui_interface->declare(&fHslider1, "unit", "Hz");
		ui_interface->addHorizontalSlider("Modulation_Frequency", &fHslider1, FAUSTFLOAT(2.2e+02f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(1e+03f), FAUSTFLOAT(0.1f));
		ui_interface->declare(&fEntry1, "3", "");
		ui_interface->declare(&fEntry1, "tooltip", "0=theta is modulated by the incoming signal; 1=theta is modulated by the averaged incoming signal;  2=theta is modulated by the squared incoming signal; 3=theta is modulated by a sine wave of frequency freqMod;  4=theta is modulated by a sine wave of frequency freq;");
		ui_interface->addNumEntry("Modulation_Type", &fEntry1, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(4.0f), FAUSTFLOAT(1.0f));
		ui_interface->declare(&fHslider0, "3", "");
		ui_interface->declare(&fHslider0, "tooltip", "Nonlinearity factor (value between 0 and 1)");
		ui_interface->addHorizontalSlider("Nonlinearity", &fHslider0, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Physical_Parameters");
		ui_interface->declare(&fHslider2, "2", "");
		ui_interface->declare(&fHslider2, "tooltip", "A value between 0 and 1");
		ui_interface->addHorizontalSlider("Touch_Length", &fHslider2, FAUSTFLOAT(0.15f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Reverb");
		ui_interface->addHorizontalSlider("reverbGain", &fHslider3, FAUSTFLOAT(0.137f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("roomSize", &fHslider5, FAUSTFLOAT(0.72f), FAUSTFLOAT(0.01f), FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Spat");
		ui_interface->addHorizontalSlider("pan angle", &fHslider4, FAUSTFLOAT(0.6f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("spatial width", &fHslider6, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = static_cast<float>(fButton0);
		int iSlow1 = fSlow0 < 1.0f;
		int iSlow2 = iSlow1 > 0;
		int iSlow3 = iSlow1 < 1;
		float fSlow4 = static_cast<float>(iSlow1);
		float fSlow5 = static_cast<float>(fEntry0);
		float fSlow6 = static_cast<float>(static_cast<int>(17.31234f * (std::log(fSlow5) + -6.086775f) + 69.5f));
		float fSlow7 = getValueBassLoopFiltera1(fSlow6);
		int iSlow8 = fSlow0 > 0.0f;
		float fSlow9 = fConst0 / fSlow5;
		float fSlow10 = fConst3 * static_cast<float>(fHslider0);
		float fSlow11 = static_cast<float>(fEntry1);
		float fSlow12 = 3.1415927f * static_cast<float>(fSlow11 == 2.0f);
		float fSlow13 = 1.5707964f * static_cast<float>(fSlow11 == 1.0f);
		float fSlow14 = 3.1415927f * static_cast<float>(fSlow11 == 0.0f);
		float fSlow15 = static_cast<float>(fSlow11 < 3.0f);
		float fSlow16 = fConst3 * static_cast<float>(fHslider1);
		float fSlow17 = static_cast<float>(fSlow11 != 4.0f);
		float fSlow18 = fSlow5 * static_cast<float>(fSlow11 == 4.0f);
		float fSlow19 = static_cast<float>(fSlow11 >= 3.0f);
		float fSlow20 = getValueBassLoopFilterb1(fSlow6);
		float fSlow21 = getValueBassLoopFilterb0(fSlow6);
		float fSlow22 = std::exp(-(fConst6 / static_cast<float>(fHslider2)));
		float fSlow23 = static_cast<float>(fEntry2);
		float fSlow24 = fConst3 * static_cast<float>(fHslider3);
		float fSlow25 = static_cast<float>(fHslider4);
		float fSlow26 = 4.0f * (1.0f - fSlow25);
		float fSlow27 = static_cast<float>(fHslider5);
		float fSlow28 = std::exp(-(fConst11 / fSlow27));
		float fSlow29 = bassdsp_faustpower2_f(fSlow28);
		float fSlow30 = 1.0f - fSlow29;
		float fSlow31 = 1.0f - fConst12 * fSlow29;
		float fSlow32 = std::sqrt(std::max<float>(0.0f, bassdsp_faustpower2_f(fSlow31) / bassdsp_faustpower2_f(fSlow30) + -1.0f));
		float fSlow33 = fSlow31 / fSlow30;
		float fSlow34 = fSlow33 - fSlow32;
		float fSlow35 = std::exp(-(fConst16 / fSlow27)) / fSlow28 + -1.0f;
		float fSlow36 = fSlow28 * (fSlow32 + (1.0f - fSlow33));
		float fSlow37 = std::exp(-(fConst23 / fSlow27));
		float fSlow38 = bassdsp_faustpower2_f(fSlow37);
		float fSlow39 = 1.0f - fSlow38;
		float fSlow40 = 1.0f - fConst12 * fSlow38;
		float fSlow41 = std::sqrt(std::max<float>(0.0f, bassdsp_faustpower2_f(fSlow40) / bassdsp_faustpower2_f(fSlow39) + -1.0f));
		float fSlow42 = fSlow40 / fSlow39;
		float fSlow43 = fSlow42 - fSlow41;
		float fSlow44 = std::exp(-(fConst24 / fSlow27)) / fSlow37 + -1.0f;
		float fSlow45 = fSlow37 * (fSlow41 + (1.0f - fSlow42));
		float fSlow46 = std::exp(-(fConst30 / fSlow27));
		float fSlow47 = bassdsp_faustpower2_f(fSlow46);
		float fSlow48 = 1.0f - fSlow47;
		float fSlow49 = 1.0f - fConst12 * fSlow47;
		float fSlow50 = std::sqrt(std::max<float>(0.0f, bassdsp_faustpower2_f(fSlow49) / bassdsp_faustpower2_f(fSlow48) + -1.0f));
		float fSlow51 = fSlow49 / fSlow48;
		float fSlow52 = fSlow51 - fSlow50;
		float fSlow53 = std::exp(-(fConst31 / fSlow27)) / fSlow46 + -1.0f;
		float fSlow54 = fSlow46 * (fSlow50 + (1.0f - fSlow51));
		float fSlow55 = std::exp(-(fConst37 / fSlow27));
		float fSlow56 = bassdsp_faustpower2_f(fSlow55);
		float fSlow57 = 1.0f - fSlow56;
		float fSlow58 = 1.0f - fConst12 * fSlow56;
		float fSlow59 = std::sqrt(std::max<float>(0.0f, bassdsp_faustpower2_f(fSlow58) / bassdsp_faustpower2_f(fSlow57) + -1.0f));
		float fSlow60 = fSlow58 / fSlow57;
		float fSlow61 = fSlow60 - fSlow59;
		float fSlow62 = std::exp(-(fConst38 / fSlow27)) / fSlow55 + -1.0f;
		float fSlow63 = fSlow55 * (fSlow59 + (1.0f - fSlow60));
		float fSlow64 = std::exp(-(fConst44 / fSlow27));
		float fSlow65 = bassdsp_faustpower2_f(fSlow64);
		float fSlow66 = 1.0f - fSlow65;
		float fSlow67 = 1.0f - fConst12 * fSlow65;
		float fSlow68 = std::sqrt(std::max<float>(0.0f, bassdsp_faustpower2_f(fSlow67) / bassdsp_faustpower2_f(fSlow66) + -1.0f));
		float fSlow69 = fSlow67 / fSlow66;
		float fSlow70 = fSlow69 - fSlow68;
		float fSlow71 = std::exp(-(fConst45 / fSlow27)) / fSlow64 + -1.0f;
		float fSlow72 = fSlow64 * (fSlow68 + (1.0f - fSlow69));
		int iSlow73 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst48 * (static_cast<float>(fHslider6) / fSlow5))));
		float fSlow74 = 4.0f * fSlow25;
		float fSlow75 = std::exp(-(fConst52 / fSlow27));
		float fSlow76 = bassdsp_faustpower2_f(fSlow75);
		float fSlow77 = 1.0f - fSlow76;
		float fSlow78 = 1.0f - fConst12 * fSlow76;
		float fSlow79 = std::sqrt(std::max<float>(0.0f, bassdsp_faustpower2_f(fSlow78) / bassdsp_faustpower2_f(fSlow77) + -1.0f));
		float fSlow80 = fSlow78 / fSlow77;
		float fSlow81 = fSlow80 - fSlow79;
		float fSlow82 = std::exp(-(fConst53 / fSlow27)) / fSlow75 + -1.0f;
		float fSlow83 = fSlow75 * (fSlow79 + (1.0f - fSlow80));
		float fSlow84 = std::exp(-(fConst59 / fSlow27));
		float fSlow85 = bassdsp_faustpower2_f(fSlow84);
		float fSlow86 = 1.0f - fSlow85;
		float fSlow87 = 1.0f - fConst12 * fSlow85;
		float fSlow88 = std::sqrt(std::max<float>(0.0f, bassdsp_faustpower2_f(fSlow87) / bassdsp_faustpower2_f(fSlow86) + -1.0f));
		float fSlow89 = fSlow87 / fSlow86;
		float fSlow90 = fSlow89 - fSlow88;
		float fSlow91 = std::exp(-(fConst60 / fSlow27)) / fSlow84 + -1.0f;
		float fSlow92 = fSlow84 * (fSlow88 + (1.0f - fSlow89));
		float fSlow93 = std::exp(-(fConst66 / fSlow27));
		float fSlow94 = bassdsp_faustpower2_f(fSlow93);
		float fSlow95 = 1.0f - fSlow94;
		float fSlow96 = 1.0f - fConst12 * fSlow94;
		float fSlow97 = std::sqrt(std::max<float>(0.0f, bassdsp_faustpower2_f(fSlow96) / bassdsp_faustpower2_f(fSlow95) + -1.0f));
		float fSlow98 = fSlow96 / fSlow95;
		float fSlow99 = fSlow98 - fSlow97;
		float fSlow100 = std::exp(-(fConst67 / fSlow27)) / fSlow93 + -1.0f;
		float fSlow101 = fSlow93 * (fSlow97 + (1.0f - fSlow98));
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iVec0[0] = 1;
			iRec2[0] = iSlow1 * iRec2[1] + 1;
			float fTemp0 = static_cast<float>(iRec2[0] + -1);
			int iTemp1 = (fTemp0 < 2.0f) & iSlow2;
			float fTemp2 = static_cast<float>(iTemp1);
			float fTemp3 = 0.030197384f * fTemp2 + fConst1 * static_cast<float>((fTemp0 >= 2.0f) | iSlow3);
			fRec1[0] = fRec1[1] * fTemp3 + (1.0f - fTemp3) * (fTemp2 + 0.9f * static_cast<float>(iTemp1 < 1));
			fRec5[0] = fSlow0 * fRec5[1] + 1.0f;
			float fTemp4 = fRec5[0] + -1.0f;
			int iTemp5 = (fTemp4 < 2.0f) & iSlow8;
			float fTemp6 = static_cast<float>(iTemp5 < 1);
			float fTemp7 = static_cast<float>((fTemp4 >= 2.0f) | iSlow1);
			float fTemp8 = static_cast<float>(iTemp5);
			float fTemp9 = 0.030197384f * fTemp8;
			float fTemp10 = fTemp9 + fConst2 * fTemp7;
			fRec4[0] = fRec4[1] * fTemp10 + fSlow9 * (1.0f - fTemp10) * fTemp6;
			int iTemp11 = static_cast<int>(fRec4[0]);
			float fTemp12 = std::floor(fRec4[0]);
			float fTemp13 = fRec0[(IOTA0 - (std::min<int>(4097, std::max<int>(0, iTemp11)) + 1)) & 8191] * (fTemp12 + (1.0f - fRec4[0])) + (fRec4[0] - fTemp12) * fRec0[(IOTA0 - (std::min<int>(4097, std::max<int>(0, iTemp11 + 1)) + 1)) & 8191];
			fVec1[0] = fTemp13;
			fRec6[0] = fSlow10 + fConst4 * fRec6[1];
			float fTemp14 = fRec6[0] * (fSlow14 * fTemp13 + fSlow13 * (fTemp13 + fVec1[1]) + fSlow12 * bassdsp_faustpower2_f(fTemp13));
			float fTemp15 = std::cos(fTemp14);
			float fTemp16 = std::sin(fTemp14);
			float fTemp17 = fTemp13 * fTemp15 - fTemp16 * fRec7[1];
			float fTemp18 = fTemp15 * fTemp17 - fTemp16 * fRec8[1];
			float fTemp19 = fTemp15 * fTemp18 - fTemp16 * fRec9[1];
			float fTemp20 = fTemp15 * fTemp19 - fTemp16 * fRec10[1];
			float fTemp21 = fTemp15 * fTemp20 - fTemp16 * fRec11[1];
			fRec12[0] = fTemp15 * fTemp21 - fTemp16 * fRec12[1];
			fRec11[0] = fTemp16 * fTemp21 + fTemp15 * fRec12[1];
			fRec10[0] = fTemp16 * fTemp20 + fTemp15 * fRec11[1];
			fRec9[0] = fTemp16 * fTemp19 + fTemp15 * fRec10[1];
			fRec8[0] = fTemp16 * fTemp18 + fTemp15 * fRec9[1];
			fRec7[0] = fTemp16 * fTemp17 + fTemp15 * fRec8[1];
			fRec15[0] = fSlow16 + fConst4 * fRec15[1];
			float fTemp22 = ((1 - iVec0[1]) ? 0.0f : fRec14[1] + fConst5 * (fSlow18 + fSlow17 * fRec15[0]));
			fRec14[0] = fTemp22 - std::floor(fTemp22);
			float fTemp23 = 3.1415927f * fRec6[0] * ftbl0bassdspSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec14[0]), 65535))];
			float fTemp24 = std::cos(fTemp23);
			float fTemp25 = std::sin(fTemp23);
			float fTemp26 = fTemp13 * fTemp24 - fTemp25 * fRec16[1];
			float fTemp27 = fTemp24 * fTemp26 - fTemp25 * fRec17[1];
			float fTemp28 = fTemp24 * fTemp27 - fTemp25 * fRec18[1];
			float fTemp29 = fTemp24 * fTemp28 - fTemp25 * fRec19[1];
			float fTemp30 = fTemp24 * fTemp29 - fTemp25 * fRec20[1];
			fRec21[0] = fTemp24 * fTemp30 - fTemp25 * fRec21[1];
			fRec20[0] = fTemp25 * fTemp30 + fTemp24 * fRec21[1];
			fRec19[0] = fTemp25 * fTemp29 + fTemp24 * fRec20[1];
			fRec18[0] = fTemp25 * fTemp28 + fTemp24 * fRec19[1];
			fRec17[0] = fTemp25 * fTemp27 + fTemp24 * fRec18[1];
			fRec16[0] = fTemp25 * fTemp26 + fTemp24 * fRec17[1];
			float fTemp31 = fSlow19 * (fTemp13 * fTemp25 + fRec16[1] * fTemp24) + fSlow15 * (fRec6[0] * (fTemp13 * fTemp16 + fRec7[1] * fTemp15) + (1.0f - fRec6[0]) * fTemp13);
			fVec3[0] = fTemp31;
			fRec3[0] = fSlow21 * fTemp31 + fSlow20 * fVec3[1] - fSlow7 * fRec3[1];
			iRec24[0] = 1103515245 * iRec24[1] + 12345;
			float fTemp32 = fTemp9 + fSlow22 * fTemp7;
			fRec25[0] = fRec25[1] * fTemp32 + fSlow23 * fTemp8 * (1.0f - fTemp32);
			float fTemp33 = fRec25[0] * static_cast<float>(iRec24[0]);
			fVec4[0] = fTemp33;
			float fTemp34 = fTemp9 + fConst7 * fTemp7;
			fRec26[0] = fRec26[1] * fTemp34 - (1.0f - fTemp34) * (0.5f * fTemp8 + 0.985f * fTemp6);
			fRec23[0] = 1.6298145e-11f * (fTemp33 * (fRec26[0] + 1.0f) - fRec26[0] * fVec4[1]) + 0.965f * fRec23[1];
			fRec22[0] = 0.035f * fRec23[0] + 0.965f * fRec22[1];
			fRec0[IOTA0 & 8191] = fRec22[0] + fRec3[0] * (fSlow0 + fSlow4 * fRec1[0]);
			float fTemp35 = fRec0[IOTA0 & 8191];
			fRec27[0] = fTemp35 + fConst8 * fRec27[1] - 0.994009f * fRec27[2];
			float fTemp36 = 0.00449325f * (fRec27[0] - fRec27[2]) + 0.5f * fTemp35;
			fVec5[IOTA0 & 8191] = fTemp36;
			fRec28[0] = fSlow24 + fConst4 * fRec28[1];
			float fTemp37 = 1.0f - fRec28[0];
			fRec40[0] = -(fConst15 * (fConst14 * fRec40[1] - (fRec33[1] + fRec33[2])));
			fRec39[0] = fSlow36 * (fRec33[1] + fSlow35 * fRec40[0]) + fSlow34 * fRec39[1];
			fVec6[IOTA0 & 16383] = 0.35355338f * fRec39[0] + 1e-20f;
			fVec7[IOTA0 & 4095] = fSlow26 * fRec28[0] * fTemp36;
			float fTemp38 = 0.3f * fVec7[(IOTA0 - iConst19) & 4095];
			float fTemp39 = fTemp38 + fVec6[(IOTA0 - iConst18) & 16383] - 0.6f * fRec37[1];
			fVec8[IOTA0 & 4095] = fTemp39;
			fRec37[0] = fVec8[(IOTA0 - iConst20) & 4095];
			float fRec38 = 0.6f * fTemp39;
			fRec44[0] = -(fConst15 * (fConst14 * fRec44[1] - (fRec29[1] + fRec29[2])));
			fRec43[0] = fSlow45 * (fRec29[1] + fSlow44 * fRec44[0]) + fSlow43 * fRec43[1];
			fVec9[IOTA0 & 16383] = 0.35355338f * fRec43[0] + 1e-20f;
			float fTemp40 = fVec9[(IOTA0 - iConst26) & 16383] + fTemp38 - 0.6f * fRec41[1];
			fVec10[IOTA0 & 2047] = fTemp40;
			fRec41[0] = fVec10[(IOTA0 - iConst27) & 2047];
			float fRec42 = 0.6f * fTemp40;
			float fTemp41 = fRec42 + fRec38;
			fRec48[0] = -(fConst15 * (fConst14 * fRec48[1] - (fRec31[1] + fRec31[2])));
			fRec47[0] = fSlow54 * (fRec31[1] + fSlow53 * fRec48[0]) + fSlow52 * fRec47[1];
			fVec11[IOTA0 & 16383] = 0.35355338f * fRec47[0] + 1e-20f;
			float fTemp42 = fVec11[(IOTA0 - iConst33) & 16383] - (fTemp38 + 0.6f * fRec45[1]);
			fVec12[IOTA0 & 4095] = fTemp42;
			fRec45[0] = fVec12[(IOTA0 - iConst34) & 4095];
			float fRec46 = 0.6f * fTemp42;
			fRec52[0] = -(fConst15 * (fConst14 * fRec52[1] - (fRec35[1] + fRec35[2])));
			fRec51[0] = fSlow63 * (fRec35[1] + fSlow62 * fRec52[0]) + fSlow61 * fRec51[1];
			fVec13[IOTA0 & 16383] = 0.35355338f * fRec51[0] + 1e-20f;
			float fTemp43 = fVec13[(IOTA0 - iConst40) & 16383] - (fTemp38 + 0.6f * fRec49[1]);
			fVec14[IOTA0 & 2047] = fTemp43;
			fRec49[0] = fVec14[(IOTA0 - iConst41) & 2047];
			float fRec50 = 0.6f * fTemp43;
			float fTemp44 = fRec50 + fRec46 + fTemp41;
			fRec56[0] = -(fConst15 * (fConst14 * fRec56[1] - (fRec30[1] + fRec30[2])));
			fRec55[0] = fSlow72 * (fRec30[1] + fSlow71 * fRec56[0]) + fSlow70 * fRec55[1];
			fVec15[IOTA0 & 32767] = 0.35355338f * fRec55[0] + 1e-20f;
			float fTemp45 = fVec5[(IOTA0 - iSlow73) & 8191];
			fVec16[IOTA0 & 4095] = fSlow74 * fRec28[0] * fTemp45;
			float fTemp46 = 0.3f * fVec16[(IOTA0 - iConst19) & 4095];
			float fTemp47 = fTemp46 + 0.6f * fRec53[1] + fVec15[(IOTA0 - iConst47) & 32767];
			fVec17[IOTA0 & 4095] = fTemp47;
			fRec53[0] = fVec17[(IOTA0 - iConst49) & 4095];
			float fRec54 = -(0.6f * fTemp47);
			fRec60[0] = -(fConst15 * (fConst14 * fRec60[1] - (fRec34[1] + fRec34[2])));
			fRec59[0] = fSlow83 * (fRec34[1] + fSlow82 * fRec60[0]) + fSlow81 * fRec59[1];
			fVec18[IOTA0 & 16383] = 0.35355338f * fRec59[0] + 1e-20f;
			float fTemp48 = fVec18[(IOTA0 - iConst55) & 16383] + fTemp46 + 0.6f * fRec57[1];
			fVec19[IOTA0 & 4095] = fTemp48;
			fRec57[0] = fVec19[(IOTA0 - iConst56) & 4095];
			float fRec58 = -(0.6f * fTemp48);
			fRec64[0] = -(fConst15 * (fConst14 * fRec64[1] - (fRec32[1] + fRec32[2])));
			fRec63[0] = fSlow92 * (fRec32[1] + fSlow91 * fRec64[0]) + fSlow90 * fRec63[1];
			fVec20[IOTA0 & 32767] = 0.35355338f * fRec63[0] + 1e-20f;
			float fTemp49 = 0.6f * fRec61[1] + fVec20[(IOTA0 - iConst62) & 32767];
			fVec21[IOTA0 & 4095] = fTemp49 - fTemp46;
			fRec61[0] = fVec21[(IOTA0 - iConst63) & 4095];
			float fRec62 = 0.6f * (fTemp46 - fTemp49);
			fRec68[0] = -(fConst15 * (fConst14 * fRec68[1] - (fRec36[1] + fRec36[2])));
			fRec67[0] = fSlow101 * (fRec36[1] + fSlow100 * fRec68[0]) + fSlow99 * fRec67[1];
			fVec22[IOTA0 & 32767] = 0.35355338f * fRec67[0] + 1e-20f;
			float fTemp50 = 0.6f * fRec65[1] + fVec22[(IOTA0 - iConst69) & 32767];
			fVec23[IOTA0 & 2047] = fTemp50 - fTemp46;
			fRec65[0] = fVec23[(IOTA0 - iConst70) & 2047];
			float fRec66 = 0.6f * (fTemp46 - fTemp50);
			fRec29[0] = fRec65[1] + fRec61[1] + fRec57[1] + fRec53[1] + fRec49[1] + fRec45[1] + fRec37[1] + fRec41[1] + fRec66 + fRec62 + fRec58 + fRec54 + fTemp44;
			fRec30[0] = fRec49[1] + fRec45[1] + fRec37[1] + fRec41[1] + fTemp44 - (fRec65[1] + fRec61[1] + fRec57[1] + fRec53[1] + fRec66 + fRec62 + fRec54 + fRec58);
			float fTemp51 = fRec46 + fRec50;
			fRec31[0] = fRec57[1] + fRec53[1] + fRec37[1] + fRec41[1] + fRec58 + fRec54 + fTemp41 - (fRec65[1] + fRec61[1] + fRec49[1] + fRec45[1] + fRec66 + fRec62 + fTemp51);
			fRec32[0] = fRec65[1] + fRec61[1] + fRec37[1] + fRec41[1] + fRec66 + fRec62 + fTemp41 - (fRec57[1] + fRec53[1] + fRec49[1] + fRec45[1] + fRec58 + fRec54 + fTemp51);
			float fTemp52 = fRec38 + fRec50;
			float fTemp53 = fRec42 + fRec46;
			fRec33[0] = fRec61[1] + fRec53[1] + fRec45[1] + fRec41[1] + fRec62 + fRec54 + fTemp53 - (fRec65[1] + fRec57[1] + fRec49[1] + fRec37[1] + fRec66 + fRec58 + fTemp52);
			fRec34[0] = fRec65[1] + fRec57[1] + fRec45[1] + fRec41[1] + fRec66 + fRec58 + fTemp53 - (fRec61[1] + fRec53[1] + fRec49[1] + fRec37[1] + fRec62 + fRec54 + fTemp52);
			float fTemp54 = fRec38 + fRec46;
			float fTemp55 = fRec42 + fRec50;
			fRec35[0] = fRec65[1] + fRec53[1] + fRec49[1] + fRec41[1] + fRec66 + fRec54 + fTemp55 - (fRec61[1] + fRec57[1] + fRec45[1] + fRec37[1] + fRec62 + fRec58 + fTemp54);
			fRec36[0] = fRec61[1] + fRec57[1] + fRec49[1] + fRec41[1] + fRec62 + fRec58 + fTemp55 - (fRec65[1] + fRec53[1] + fRec45[1] + fRec37[1] + fRec66 + fRec54 + fTemp54);
			output0[i0] = static_cast<FAUSTFLOAT>(0.37f * (fRec30[0] + fRec31[0]) + fSlow26 * fTemp37 * fTemp36);
			output1[i0] = static_cast<FAUSTFLOAT>(0.37f * (fRec30[0] - fRec31[0]) + fSlow74 * fTemp37 * fTemp45);
			iVec0[1] = iVec0[0];
			iRec2[1] = iRec2[0];
			fRec1[1] = fRec1[0];
			IOTA0 = IOTA0 + 1;
			fRec5[1] = fRec5[0];
			fRec4[1] = fRec4[0];
			fVec1[1] = fVec1[0];
			fRec6[1] = fRec6[0];
			fRec12[1] = fRec12[0];
			fRec11[1] = fRec11[0];
			fRec10[1] = fRec10[0];
			fRec9[1] = fRec9[0];
			fRec8[1] = fRec8[0];
			fRec7[1] = fRec7[0];
			fRec15[1] = fRec15[0];
			fRec14[1] = fRec14[0];
			fRec21[1] = fRec21[0];
			fRec20[1] = fRec20[0];
			fRec19[1] = fRec19[0];
			fRec18[1] = fRec18[0];
			fRec17[1] = fRec17[0];
			fRec16[1] = fRec16[0];
			fVec3[1] = fVec3[0];
			fRec3[1] = fRec3[0];
			iRec24[1] = iRec24[0];
			fRec25[1] = fRec25[0];
			fVec4[1] = fVec4[0];
			fRec26[1] = fRec26[0];
			fRec23[1] = fRec23[0];
			fRec22[1] = fRec22[0];
			fRec27[2] = fRec27[1];
			fRec27[1] = fRec27[0];
			fRec28[1] = fRec28[0];
			fRec40[1] = fRec40[0];
			fRec39[1] = fRec39[0];
			fRec37[1] = fRec37[0];
			fRec44[1] = fRec44[0];
			fRec43[1] = fRec43[0];
			fRec41[1] = fRec41[0];
			fRec48[1] = fRec48[0];
			fRec47[1] = fRec47[0];
			fRec45[1] = fRec45[0];
			fRec52[1] = fRec52[0];
			fRec51[1] = fRec51[0];
			fRec49[1] = fRec49[0];
			fRec56[1] = fRec56[0];
			fRec55[1] = fRec55[0];
			fRec53[1] = fRec53[0];
			fRec60[1] = fRec60[0];
			fRec59[1] = fRec59[0];
			fRec57[1] = fRec57[0];
			fRec64[1] = fRec64[0];
			fRec63[1] = fRec63[0];
			fRec61[1] = fRec61[0];
			fRec68[1] = fRec68[0];
			fRec67[1] = fRec67[0];
			fRec65[1] = fRec65[0];
			fRec29[2] = fRec29[1];
			fRec29[1] = fRec29[0];
			fRec30[2] = fRec30[1];
			fRec30[1] = fRec30[0];
			fRec31[2] = fRec31[1];
			fRec31[1] = fRec31[0];
			fRec32[2] = fRec32[1];
			fRec32[1] = fRec32[0];
			fRec33[2] = fRec33[1];
			fRec33[1] = fRec33[0];
			fRec34[2] = fRec34[1];
			fRec34[1] = fRec34[0];
			fRec35[2] = fRec35[1];
			fRec35[1] = fRec35[0];
			fRec36[2] = fRec36[1];
			fRec36[1] = fRec36[0];
		}
	}

};

#endif

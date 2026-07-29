/* ------------------------------------------------------------
author: "Romain Michon (rmichon@ccrma.stanford.edu)"
copyright: "Romain Michon"
name: "piano"
version: "1.0"
Code generated with Faust 2.81.10 (https://faust.grame.fr)
Compilation options: -lang cpp -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __pianodsp_H__
#define  __pianodsp_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

/* link with : "" */
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>
#include "faust_dummy.h"
#include "piano.h"

#ifndef FAUSTCLASS 
#define FAUSTCLASS pianodsp
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

static float pianodsp_faustpower2_f(float value) {
	return value * value;
}

class pianodsp : public dsp {
	
 private:
	
	FAUSTFLOAT fEntry0;
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	FAUSTFLOAT fHslider0;
	FAUSTFLOAT fButton0;
	FAUSTFLOAT fHslider1;
	float fConst3;
	float fRec11[2];
	float fConst4;
	float fConst5;
	float fConst6;
	float fRec10[2];
	FAUSTFLOAT fEntry1;
	float fConst7;
	float fRec12[2];
	int iRec13[2];
	float fVec0[2];
	float fVec1[2];
	float fRec9[2];
	float fRec8[2];
	float fRec7[2];
	float fRec6[2];
	float fRec5[2];
	float fRec4[3];
	float fRec3[3];
	float fRec2[3];
	float fRec1[3];
	float fRec0[2];
	int IOTA0;
	float fRec24[2];
	float fRec23[2];
	float fRec22[2];
	float fRec21[2];
	float fRec20[2];
	float fConst8;
	float fConst9;
	float fRec25[2];
	float fVec2[2];
	FAUSTFLOAT fHslider2;
	float fRec19[2];
	float fRec18[2];
	float fRec17[8192];
	FAUSTFLOAT fHslider3;
	float fConst10;
	float fVec3[2];
	float fRec29[2];
	float fRec28[2];
	float fRec27[8192];
	float fVec4[2];
	float fRec26[2];
	float fRec14[2];
	float fRec15[2];
	float fConst11;
	float fRec30[3];
	float fConst12;
	float fVec5[8192];
	FAUSTFLOAT fHslider4;
	float fRec31[2];
	FAUSTFLOAT fHslider5;
	FAUSTFLOAT fHslider6;
	float fConst13;
	float fConst14;
	float fConst15;
	float fConst16;
	float fConst17;
	float fConst18;
	float fConst19;
	float fRec43[2];
	float fConst20;
	float fRec42[2];
	float fVec6[16384];
	float fConst21;
	int iConst22;
	float fVec7[4096];
	int iConst23;
	float fVec8[4096];
	int iConst24;
	float fRec40[2];
	float fConst25;
	float fConst26;
	float fConst27;
	float fRec47[2];
	float fConst28;
	float fRec46[2];
	float fVec9[16384];
	float fConst29;
	int iConst30;
	float fVec10[2048];
	int iConst31;
	float fRec44[2];
	float fConst32;
	float fConst33;
	float fConst34;
	float fRec51[2];
	float fConst35;
	float fRec50[2];
	float fVec11[16384];
	float fConst36;
	int iConst37;
	float fVec12[4096];
	int iConst38;
	float fRec48[2];
	float fConst39;
	float fConst40;
	float fConst41;
	float fRec55[2];
	float fConst42;
	float fRec54[2];
	float fVec13[16384];
	float fConst43;
	int iConst44;
	float fVec14[2048];
	int iConst45;
	float fRec52[2];
	float fConst46;
	float fConst47;
	float fConst48;
	float fRec59[2];
	float fConst49;
	float fRec58[2];
	float fVec15[32768];
	float fConst50;
	int iConst51;
	FAUSTFLOAT fHslider7;
	float fConst52;
	float fVec16[4096];
	float fVec17[4096];
	int iConst53;
	float fRec56[2];
	float fConst54;
	float fConst55;
	float fConst56;
	float fRec63[2];
	float fConst57;
	float fRec62[2];
	float fVec18[16384];
	float fConst58;
	int iConst59;
	float fVec19[4096];
	int iConst60;
	float fRec60[2];
	float fConst61;
	float fConst62;
	float fConst63;
	float fRec67[2];
	float fConst64;
	float fRec66[2];
	float fVec20[32768];
	float fConst65;
	int iConst66;
	float fVec21[4096];
	int iConst67;
	float fRec64[2];
	float fConst68;
	float fConst69;
	float fConst70;
	float fRec71[2];
	float fConst71;
	float fRec70[2];
	float fVec22[32768];
	float fConst72;
	int iConst73;
	float fVec23[2048];
	int iConst74;
	float fRec68[2];
	float fRec32[3];
	float fRec33[3];
	float fRec34[3];
	float fRec35[3];
	float fRec36[3];
	float fRec37[3];
	float fRec38[3];
	float fRec39[3];
	
 public:
	pianodsp() {
	}
	
	void metadata(Meta* m) { 
		m->declare("author", "Romain Michon (rmichon@ccrma.stanford.edu)");
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "1.22.0");
		m->declare("compile_options", "-lang cpp -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("copyright", "Romain Michon");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.2.0");
		m->declare("description", "WaveGuide Commuted Piano");
		m->declare("filename", "piano.dsp");
		m->declare("filters.lib/allpass_comb:author", "Julius O. Smith III");
		m->declare("filters.lib/allpass_comb:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/allpass_comb:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "MIT-style STK-4.3 license");
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
		m->declare("name", "piano");
		m->declare("noises.lib/name", "Faust Noise Generator Library");
		m->declare("noises.lib/version", "1.5.0");
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
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, static_cast<float>(fSampleRate)));
		fConst1 = 6.2831855f / fConst0;
		fConst2 = 0.05f / fConst0;
		fConst3 = 0.1f * fConst0;
		fConst4 = std::exp(-(0.5f / fConst0));
		fConst5 = 1e+01f / fConst0;
		fConst6 = std::exp(-(5.0f / fConst0));
		fConst7 = 7.0f / fConst0;
		fConst8 = 44.1f / fConst0;
		fConst9 = 1.0f - fConst8;
		fConst10 = 0.15915494f * fConst0;
		fConst11 = 1.0f / fConst0;
		fConst12 = 1.0f / pianodsp_faustpower2_f(fConst0);
		fConst13 = std::floor(0.174713f * fConst0 + 0.5f);
		fConst14 = fConst13 / fConst0;
		fConst15 = 3.4538777f * fConst14;
		fConst16 = std::cos(37699.113f / fConst0);
		fConst17 = 1.0f / std::tan(628.31854f / fConst0);
		fConst18 = 1.0f - fConst17;
		fConst19 = 1.0f / (fConst17 + 1.0f);
		fConst20 = 2.3025851f * fConst14;
		fConst21 = std::floor(0.022904f * fConst0 + 0.5f);
		iConst22 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst13 - fConst21)));
		iConst23 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, 0.02f * fConst0)));
		iConst24 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst21 + -1.0f)));
		fConst25 = std::floor(0.153129f * fConst0 + 0.5f);
		fConst26 = fConst25 / fConst0;
		fConst27 = 3.4538777f * fConst26;
		fConst28 = 2.3025851f * fConst26;
		fConst29 = std::floor(0.020346f * fConst0 + 0.5f);
		iConst30 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst25 - fConst29)));
		iConst31 = static_cast<int>(std::min<float>(1024.0f, std::max<float>(0.0f, fConst29 + -1.0f)));
		fConst32 = std::floor(0.127837f * fConst0 + 0.5f);
		fConst33 = fConst32 / fConst0;
		fConst34 = 3.4538777f * fConst33;
		fConst35 = 2.3025851f * fConst33;
		fConst36 = std::floor(0.031604f * fConst0 + 0.5f);
		iConst37 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst32 - fConst36)));
		iConst38 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst36 + -1.0f)));
		fConst39 = std::floor(0.125f * fConst0 + 0.5f);
		fConst40 = fConst39 / fConst0;
		fConst41 = 3.4538777f * fConst40;
		fConst42 = 2.3025851f * fConst40;
		fConst43 = std::floor(0.013458f * fConst0 + 0.5f);
		iConst44 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst39 - fConst43)));
		iConst45 = static_cast<int>(std::min<float>(1024.0f, std::max<float>(0.0f, fConst43 + -1.0f)));
		fConst46 = std::floor(0.210389f * fConst0 + 0.5f);
		fConst47 = fConst46 / fConst0;
		fConst48 = 3.4538777f * fConst47;
		fConst49 = 2.3025851f * fConst47;
		fConst50 = std::floor(0.024421f * fConst0 + 0.5f);
		iConst51 = static_cast<int>(std::min<float>(16384.0f, std::max<float>(0.0f, fConst46 - fConst50)));
		fConst52 = 0.5f * fConst0;
		iConst53 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst50 + -1.0f)));
		fConst54 = std::floor(0.192303f * fConst0 + 0.5f);
		fConst55 = fConst54 / fConst0;
		fConst56 = 3.4538777f * fConst55;
		fConst57 = 2.3025851f * fConst55;
		fConst58 = std::floor(0.029291f * fConst0 + 0.5f);
		iConst59 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst54 - fConst58)));
		iConst60 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst58 + -1.0f)));
		fConst61 = std::floor(0.256891f * fConst0 + 0.5f);
		fConst62 = fConst61 / fConst0;
		fConst63 = 3.4538777f * fConst62;
		fConst64 = 2.3025851f * fConst62;
		fConst65 = std::floor(0.027333f * fConst0 + 0.5f);
		iConst66 = static_cast<int>(std::min<float>(16384.0f, std::max<float>(0.0f, fConst61 - fConst65)));
		iConst67 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst65 + -1.0f)));
		fConst68 = std::floor(0.219991f * fConst0 + 0.5f);
		fConst69 = fConst68 / fConst0;
		fConst70 = 3.4538777f * fConst69;
		fConst71 = 2.3025851f * fConst69;
		fConst72 = std::floor(0.019123f * fConst0 + 0.5f);
		iConst73 = static_cast<int>(std::min<float>(16384.0f, std::max<float>(0.0f, fConst68 - fConst72)));
		iConst74 = static_cast<int>(std::min<float>(1024.0f, std::max<float>(0.0f, fConst72 + -1.0f)));
	}
	
	virtual void instanceResetUserInterface() {
		fEntry0 = static_cast<FAUSTFLOAT>(4.4e+02f);
		fHslider0 = static_cast<FAUSTFLOAT>(0.0f);
		fButton0 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider1 = static_cast<FAUSTFLOAT>(0.1f);
		fEntry1 = static_cast<FAUSTFLOAT>(1.0f);
		fHslider2 = static_cast<FAUSTFLOAT>(0.28f);
		fHslider3 = static_cast<FAUSTFLOAT>(0.1f);
		fHslider4 = static_cast<FAUSTFLOAT>(0.137f);
		fHslider5 = static_cast<FAUSTFLOAT>(0.6f);
		fHslider6 = static_cast<FAUSTFLOAT>(0.72f);
		fHslider7 = static_cast<FAUSTFLOAT>(0.5f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fRec11[l0] = 0.0f;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fRec10[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec12[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			iRec13[l3] = 0;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fVec0[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fVec1[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fRec9[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			fRec8[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
			fRec7[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
			fRec6[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			fRec5[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 3; l11 = l11 + 1) {
			fRec4[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 3; l12 = l12 + 1) {
			fRec3[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 3; l13 = l13 + 1) {
			fRec2[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 3; l14 = l14 + 1) {
			fRec1[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fRec0[l15] = 0.0f;
		}
		IOTA0 = 0;
		for (int l16 = 0; l16 < 2; l16 = l16 + 1) {
			fRec24[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2; l17 = l17 + 1) {
			fRec23[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec22[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 2; l19 = l19 + 1) {
			fRec21[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 2; l20 = l20 + 1) {
			fRec20[l20] = 0.0f;
		}
		for (int l21 = 0; l21 < 2; l21 = l21 + 1) {
			fRec25[l21] = 0.0f;
		}
		for (int l22 = 0; l22 < 2; l22 = l22 + 1) {
			fVec2[l22] = 0.0f;
		}
		for (int l23 = 0; l23 < 2; l23 = l23 + 1) {
			fRec19[l23] = 0.0f;
		}
		for (int l24 = 0; l24 < 2; l24 = l24 + 1) {
			fRec18[l24] = 0.0f;
		}
		for (int l25 = 0; l25 < 8192; l25 = l25 + 1) {
			fRec17[l25] = 0.0f;
		}
		for (int l26 = 0; l26 < 2; l26 = l26 + 1) {
			fVec3[l26] = 0.0f;
		}
		for (int l27 = 0; l27 < 2; l27 = l27 + 1) {
			fRec29[l27] = 0.0f;
		}
		for (int l28 = 0; l28 < 2; l28 = l28 + 1) {
			fRec28[l28] = 0.0f;
		}
		for (int l29 = 0; l29 < 8192; l29 = l29 + 1) {
			fRec27[l29] = 0.0f;
		}
		for (int l30 = 0; l30 < 2; l30 = l30 + 1) {
			fVec4[l30] = 0.0f;
		}
		for (int l31 = 0; l31 < 2; l31 = l31 + 1) {
			fRec26[l31] = 0.0f;
		}
		for (int l32 = 0; l32 < 2; l32 = l32 + 1) {
			fRec14[l32] = 0.0f;
		}
		for (int l33 = 0; l33 < 2; l33 = l33 + 1) {
			fRec15[l33] = 0.0f;
		}
		for (int l34 = 0; l34 < 3; l34 = l34 + 1) {
			fRec30[l34] = 0.0f;
		}
		for (int l35 = 0; l35 < 8192; l35 = l35 + 1) {
			fVec5[l35] = 0.0f;
		}
		for (int l36 = 0; l36 < 2; l36 = l36 + 1) {
			fRec31[l36] = 0.0f;
		}
		for (int l37 = 0; l37 < 2; l37 = l37 + 1) {
			fRec43[l37] = 0.0f;
		}
		for (int l38 = 0; l38 < 2; l38 = l38 + 1) {
			fRec42[l38] = 0.0f;
		}
		for (int l39 = 0; l39 < 16384; l39 = l39 + 1) {
			fVec6[l39] = 0.0f;
		}
		for (int l40 = 0; l40 < 4096; l40 = l40 + 1) {
			fVec7[l40] = 0.0f;
		}
		for (int l41 = 0; l41 < 4096; l41 = l41 + 1) {
			fVec8[l41] = 0.0f;
		}
		for (int l42 = 0; l42 < 2; l42 = l42 + 1) {
			fRec40[l42] = 0.0f;
		}
		for (int l43 = 0; l43 < 2; l43 = l43 + 1) {
			fRec47[l43] = 0.0f;
		}
		for (int l44 = 0; l44 < 2; l44 = l44 + 1) {
			fRec46[l44] = 0.0f;
		}
		for (int l45 = 0; l45 < 16384; l45 = l45 + 1) {
			fVec9[l45] = 0.0f;
		}
		for (int l46 = 0; l46 < 2048; l46 = l46 + 1) {
			fVec10[l46] = 0.0f;
		}
		for (int l47 = 0; l47 < 2; l47 = l47 + 1) {
			fRec44[l47] = 0.0f;
		}
		for (int l48 = 0; l48 < 2; l48 = l48 + 1) {
			fRec51[l48] = 0.0f;
		}
		for (int l49 = 0; l49 < 2; l49 = l49 + 1) {
			fRec50[l49] = 0.0f;
		}
		for (int l50 = 0; l50 < 16384; l50 = l50 + 1) {
			fVec11[l50] = 0.0f;
		}
		for (int l51 = 0; l51 < 4096; l51 = l51 + 1) {
			fVec12[l51] = 0.0f;
		}
		for (int l52 = 0; l52 < 2; l52 = l52 + 1) {
			fRec48[l52] = 0.0f;
		}
		for (int l53 = 0; l53 < 2; l53 = l53 + 1) {
			fRec55[l53] = 0.0f;
		}
		for (int l54 = 0; l54 < 2; l54 = l54 + 1) {
			fRec54[l54] = 0.0f;
		}
		for (int l55 = 0; l55 < 16384; l55 = l55 + 1) {
			fVec13[l55] = 0.0f;
		}
		for (int l56 = 0; l56 < 2048; l56 = l56 + 1) {
			fVec14[l56] = 0.0f;
		}
		for (int l57 = 0; l57 < 2; l57 = l57 + 1) {
			fRec52[l57] = 0.0f;
		}
		for (int l58 = 0; l58 < 2; l58 = l58 + 1) {
			fRec59[l58] = 0.0f;
		}
		for (int l59 = 0; l59 < 2; l59 = l59 + 1) {
			fRec58[l59] = 0.0f;
		}
		for (int l60 = 0; l60 < 32768; l60 = l60 + 1) {
			fVec15[l60] = 0.0f;
		}
		for (int l61 = 0; l61 < 4096; l61 = l61 + 1) {
			fVec16[l61] = 0.0f;
		}
		for (int l62 = 0; l62 < 4096; l62 = l62 + 1) {
			fVec17[l62] = 0.0f;
		}
		for (int l63 = 0; l63 < 2; l63 = l63 + 1) {
			fRec56[l63] = 0.0f;
		}
		for (int l64 = 0; l64 < 2; l64 = l64 + 1) {
			fRec63[l64] = 0.0f;
		}
		for (int l65 = 0; l65 < 2; l65 = l65 + 1) {
			fRec62[l65] = 0.0f;
		}
		for (int l66 = 0; l66 < 16384; l66 = l66 + 1) {
			fVec18[l66] = 0.0f;
		}
		for (int l67 = 0; l67 < 4096; l67 = l67 + 1) {
			fVec19[l67] = 0.0f;
		}
		for (int l68 = 0; l68 < 2; l68 = l68 + 1) {
			fRec60[l68] = 0.0f;
		}
		for (int l69 = 0; l69 < 2; l69 = l69 + 1) {
			fRec67[l69] = 0.0f;
		}
		for (int l70 = 0; l70 < 2; l70 = l70 + 1) {
			fRec66[l70] = 0.0f;
		}
		for (int l71 = 0; l71 < 32768; l71 = l71 + 1) {
			fVec20[l71] = 0.0f;
		}
		for (int l72 = 0; l72 < 4096; l72 = l72 + 1) {
			fVec21[l72] = 0.0f;
		}
		for (int l73 = 0; l73 < 2; l73 = l73 + 1) {
			fRec64[l73] = 0.0f;
		}
		for (int l74 = 0; l74 < 2; l74 = l74 + 1) {
			fRec71[l74] = 0.0f;
		}
		for (int l75 = 0; l75 < 2; l75 = l75 + 1) {
			fRec70[l75] = 0.0f;
		}
		for (int l76 = 0; l76 < 32768; l76 = l76 + 1) {
			fVec22[l76] = 0.0f;
		}
		for (int l77 = 0; l77 < 2048; l77 = l77 + 1) {
			fVec23[l77] = 0.0f;
		}
		for (int l78 = 0; l78 < 2; l78 = l78 + 1) {
			fRec68[l78] = 0.0f;
		}
		for (int l79 = 0; l79 < 3; l79 = l79 + 1) {
			fRec32[l79] = 0.0f;
		}
		for (int l80 = 0; l80 < 3; l80 = l80 + 1) {
			fRec33[l80] = 0.0f;
		}
		for (int l81 = 0; l81 < 3; l81 = l81 + 1) {
			fRec34[l81] = 0.0f;
		}
		for (int l82 = 0; l82 < 3; l82 = l82 + 1) {
			fRec35[l82] = 0.0f;
		}
		for (int l83 = 0; l83 < 3; l83 = l83 + 1) {
			fRec36[l83] = 0.0f;
		}
		for (int l84 = 0; l84 < 3; l84 = l84 + 1) {
			fRec37[l84] = 0.0f;
		}
		for (int l85 = 0; l85 < 3; l85 = l85 + 1) {
			fRec38[l85] = 0.0f;
		}
		for (int l86 = 0; l86 < 3; l86 = l86 + 1) {
			fRec39[l86] = 0.0f;
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
	
	virtual pianodsp* clone() {
		return new pianodsp();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("piano");
		ui_interface->openHorizontalBox("Basic_Parameters");
		ui_interface->declare(&fEntry0, "1", "");
		ui_interface->declare(&fEntry0, "tooltip", "Tone frequency");
		ui_interface->declare(&fEntry0, "unit", "Hz");
		ui_interface->addNumEntry("freq", &fEntry0, FAUSTFLOAT(4.4e+02f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(2e+04f), FAUSTFLOAT(1.0f));
		ui_interface->declare(&fEntry1, "1", "");
		ui_interface->declare(&fEntry1, "tooltip", "Gain (value between 0 and 1)");
		ui_interface->addNumEntry("gain", &fEntry1, FAUSTFLOAT(1.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fButton0, "1", "");
		ui_interface->declare(&fButton0, "tooltip", "noteOn = 1, noteOff = 0");
		ui_interface->addButton("gate", &fButton0);
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Physical_Parameters");
		ui_interface->declare(&fHslider0, "2", "");
		ui_interface->declare(&fHslider0, "tooltip", "A value between 0 and 1");
		ui_interface->addHorizontalSlider("Brightness_Factor", &fHslider0, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider3, "2", "");
		ui_interface->declare(&fHslider3, "tooltip", "A value between 0 and 1");
		ui_interface->addHorizontalSlider("Detuning_Factor", &fHslider3, FAUSTFLOAT(0.1f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider1, "2", "");
		ui_interface->declare(&fHslider1, "tooltip", "A value between 0 and 1");
		ui_interface->addHorizontalSlider("Hammer_Hardness", &fHslider1, FAUSTFLOAT(0.1f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider2, "2", "");
		ui_interface->declare(&fHslider2, "tooltip", "A value between 0 and 1");
		ui_interface->addHorizontalSlider("Stiffness_Factor", &fHslider2, FAUSTFLOAT(0.28f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Reverb");
		ui_interface->addHorizontalSlider("reverbGain", &fHslider4, FAUSTFLOAT(0.137f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("roomSize", &fHslider6, FAUSTFLOAT(0.72f), FAUSTFLOAT(0.01f), FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Spat");
		ui_interface->addHorizontalSlider("pan angle", &fHslider5, FAUSTFLOAT(0.6f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("spatial width", &fHslider7, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = static_cast<float>(fEntry0);
		int iSlow1 = static_cast<int>(17.31234f * (std::log(fSlow0) + -6.086775f) + 69.5f);
		float fSlow2 = static_cast<float>(iSlow1);
		float fSlow3 = getValueDCBa1(fSlow2);
		float fSlow4 = std::cos(fConst1 * fSlow0);
		float fSlow5 = 2.0f * fSlow4;
		float fSlow6 = std::pow(1e+01f, fConst2 * getValuer1_2db(fSlow2));
		float fSlow7 = std::pow(1e+01f, fConst2 * getValuer1_1db(fSlow2));
		float fSlow8 = 2.0f * std::cos(fConst1 * fSlow0 * getValueSecondPartialFactor(fSlow2));
		float fSlow9 = std::pow(1e+01f, fConst2 * getValuer2db(fSlow2));
		float fSlow10 = 2.0f * std::cos(fConst1 * fSlow0 * getValueThirdPartialFactor(fSlow2));
		float fSlow11 = std::pow(1e+01f, fConst2 * getValuer3db(fSlow2));
		float fSlow12 = 0.25f * static_cast<float>(fHslider0);
		float fSlow13 = getValueLoudPole(fSlow2);
		float fSlow14 = fSlow13 + (0.02f - fSlow12);
		float fSlow15 = static_cast<float>(fButton0);
		int iSlow16 = fSlow15 > 0.0f;
		float fSlow17 = static_cast<float>(fHslider1);
		float fSlow18 = fConst3 * fSlow17;
		float fSlow19 = std::exp(-(fConst5 / fSlow17));
		float fSlow20 = fSlow15 + -1.0f;
		float fSlow21 = fConst6 * fSlow20;
		float fSlow22 = 0.2f * getValueSustainPedalLevel(fSlow2);
		int iSlow23 = fSlow15 < 1.0f;
		float fSlow24 = std::exp(-(fConst7 / (static_cast<float>(fEntry1) * getValueDryTapAmpT60(fSlow2))));
		float fSlow25 = static_cast<float>(iSlow1 >= 88);
		float fSlow26 = 1.1641532e-10f * fSlow25;
		float fSlow27 = 2.3283064e-10f * fSlow25;
		float fSlow28 = fSlow3 - 1.0f;
		float fSlow29 = 1.0f - fSlow3;
		float fSlow30 = (fSlow12 + (0.98f - fSlow13)) * getValueLoudGain(fSlow2);
		float fSlow31 = getValueBq4_gEarBalled(fSlow2);
		float fSlow32 = 2.0f * fSlow31;
		float fSlow33 = std::pow(1e+01f, 0.05f * getValueSecondStageAmpRatio(fSlow2));
		float fSlow34 = 1.0f - fSlow33;
		float fSlow35 = 2.0f * fSlow4 * (fSlow7 * fSlow33 + fSlow34 * fSlow6);
		float fSlow36 = pianodsp_faustpower2_f(fSlow7) * fSlow33 + fSlow34 * pianodsp_faustpower2_f(fSlow6);
		float fSlow37 = 1.3969839e-09f * fSlow30 * static_cast<float>(iSlow1 < 88);
		float fSlow38 = fConst8 * (0.9996f * fSlow15 - 0.9f * fSlow20 * getValueReleaseLoopGain(fSlow2));
		float fSlow39 = getValueStiffnessCoefficient(fSlow2);
		float fSlow40 = static_cast<float>(fHslider2);
		float fSlow41 = fSlow40 * fSlow39;
		float fSlow42 = 3.7f * fSlow41;
		float fSlow43 = 5.0f * static_cast<float>(fHslider3) * getValueDetuningHz(fSlow2);
		float fSlow44 = fSlow0 + fSlow43;
		float fSlow45 = getValueSingleStringPole(fSlow2);
		float fSlow46 = std::pow(1e+01f, 0.05f * (getValueSingleStringDecayRate(fSlow2) / fSlow0)) * (1.0f - fSlow45);
		float fSlow47 = getValueSingleStringZero(fSlow2);
		float fSlow48 = 1.0f - fSlow47;
		float fSlow49 = 3.0f * fSlow48 - fSlow46;
		float fSlow50 = fConst1 * fSlow44;
		float fSlow51 = std::cos(fSlow50);
		float fSlow52 = fSlow45 * fSlow48;
		float fSlow53 = 3.0f * fSlow52;
		float fSlow54 = fSlow46 * fSlow47;
		float fSlow55 = fSlow52 - fSlow54;
		float fSlow56 = 4.0f * fSlow55;
		float fSlow57 = fSlow54 + fSlow56 - fSlow53;
		float fSlow58 = fSlow46 + fSlow47 + -1.0f;
		float fSlow59 = 4.0f * fSlow58;
		float fSlow60 = (fSlow59 + fSlow57 * fSlow51) / fSlow49 + 1.0f;
		float fSlow61 = fSlow54 - fSlow53;
		float fSlow62 = fSlow61 * fSlow51 / fSlow49 + 1.0f;
		float fSlow63 = std::sin(fSlow50);
		float fSlow64 = pianodsp_faustpower2_f(fSlow49);
		float fSlow65 = pianodsp_faustpower2_f(fSlow63);
		float fSlow66 = 13.69f * pianodsp_faustpower2_f(fSlow40) * pianodsp_faustpower2_f(fSlow39);
		float fSlow67 = fSlow66 + -1.0f;
		float fSlow68 = fSlow66 + 1.0f;
		float fSlow69 = 7.4f * fSlow41;
		float fSlow70 = 3.0f * std::atan2(fSlow67 * fSlow63, fSlow69 + fSlow68 * fSlow51);
		int iSlow71 = static_cast<int>(fConst10 * ((fSlow70 + std::atan2(-(fSlow63 * (fSlow57 * fSlow62 - fSlow61 * fSlow60) / fSlow49), fSlow62 * fSlow60 + fSlow61 * fSlow57 * fSlow65 / fSlow64) + 6.2831855f) / fSlow44));
		int iSlow72 = std::min<int>(4097, std::max<int>(0, iSlow71));
		float fSlow73 = fSlow61 + fSlow56;
		float fSlow74 = (fSlow59 + fSlow73 * fSlow51) / fSlow49 + 1.0f;
		float fSlow75 = fConst10 * ((fSlow70 + std::atan2(-(fSlow63 * (fSlow73 * fSlow62 - fSlow61 * fSlow74) / fSlow49), fSlow62 * fSlow74 + fSlow61 * fSlow73 * fSlow65 / fSlow64) + 6.2831855f) / fSlow44);
		float fSlow76 = std::floor(fSlow75);
		float fSlow77 = fSlow76 + (1.0f - fSlow75);
		float fSlow78 = fSlow0 - fSlow43;
		float fSlow79 = fConst1 * fSlow78;
		float fSlow80 = std::cos(fSlow79);
		float fSlow81 = (fSlow59 + fSlow80 * fSlow57) / fSlow49 + 1.0f;
		float fSlow82 = fSlow80 * fSlow61 / fSlow49 + 1.0f;
		float fSlow83 = std::sin(fSlow79);
		float fSlow84 = pianodsp_faustpower2_f(fSlow83) * fSlow61;
		float fSlow85 = 3.0f * std::atan2(fSlow67 * fSlow83, fSlow69 + fSlow68 * fSlow80);
		int iSlow86 = static_cast<int>(fConst10 * ((fSlow85 + std::atan2(-(fSlow83 * (fSlow82 * fSlow57 - fSlow61 * fSlow81) / fSlow49), fSlow82 * fSlow81 + fSlow84 * fSlow57 / fSlow64) + 6.2831855f) / fSlow78));
		int iSlow87 = std::min<int>(4097, std::max<int>(0, iSlow86 + 1));
		float fSlow88 = (fSlow80 * fSlow73 + fSlow59) / fSlow49 + 1.0f;
		float fSlow89 = fConst10 * ((fSlow85 + std::atan2(-(fSlow83 * (fSlow73 * fSlow82 - fSlow61 * fSlow88) / fSlow49), fSlow82 * fSlow88 + fSlow84 * fSlow73 / fSlow64) + 6.2831855f) / fSlow78);
		float fSlow90 = std::floor(fSlow89);
		float fSlow91 = fSlow89 - fSlow90;
		int iSlow92 = std::min<int>(4097, std::max<int>(0, iSlow86));
		float fSlow93 = fSlow90 + (1.0f - fSlow89);
		int iSlow94 = std::min<int>(4097, std::max<int>(0, iSlow71 + 1));
		float fSlow95 = fSlow75 - fSlow76;
		float fSlow96 = 1.0f / fSlow49;
		float fSlow97 = 2.0f * std::cos(fConst1 * (fSlow0 / getValueStrikePosition(fSlow2)));
		float fSlow98 = getValueEQBandWidthFactor(fSlow2);
		float fSlow99 = fConst11 * fSlow0 * fSlow98;
		float fSlow100 = getValueEQGain(fSlow2);
		float fSlow101 = 0.5f * (1.0f - fConst12 * pianodsp_faustpower2_f(fSlow0) * pianodsp_faustpower2_f(fSlow98));
		float fSlow102 = fConst8 * static_cast<float>(fHslider4);
		float fSlow103 = static_cast<float>(fHslider5);
		float fSlow104 = 12.0f * (1.0f - fSlow103);
		float fSlow105 = static_cast<float>(fHslider6);
		float fSlow106 = std::exp(-(fConst15 / fSlow105));
		float fSlow107 = pianodsp_faustpower2_f(fSlow106);
		float fSlow108 = 1.0f - fSlow107;
		float fSlow109 = 1.0f - fConst16 * fSlow107;
		float fSlow110 = std::sqrt(std::max<float>(0.0f, pianodsp_faustpower2_f(fSlow109) / pianodsp_faustpower2_f(fSlow108) + -1.0f));
		float fSlow111 = fSlow109 / fSlow108;
		float fSlow112 = fSlow111 - fSlow110;
		float fSlow113 = std::exp(-(fConst20 / fSlow105)) / fSlow106 + -1.0f;
		float fSlow114 = fSlow106 * (fSlow110 + (1.0f - fSlow111));
		float fSlow115 = std::exp(-(fConst27 / fSlow105));
		float fSlow116 = pianodsp_faustpower2_f(fSlow115);
		float fSlow117 = 1.0f - fSlow116;
		float fSlow118 = 1.0f - fConst16 * fSlow116;
		float fSlow119 = std::sqrt(std::max<float>(0.0f, pianodsp_faustpower2_f(fSlow118) / pianodsp_faustpower2_f(fSlow117) + -1.0f));
		float fSlow120 = fSlow118 / fSlow117;
		float fSlow121 = fSlow120 - fSlow119;
		float fSlow122 = std::exp(-(fConst28 / fSlow105)) / fSlow115 + -1.0f;
		float fSlow123 = fSlow115 * (fSlow119 + (1.0f - fSlow120));
		float fSlow124 = std::exp(-(fConst34 / fSlow105));
		float fSlow125 = pianodsp_faustpower2_f(fSlow124);
		float fSlow126 = 1.0f - fSlow125;
		float fSlow127 = 1.0f - fConst16 * fSlow125;
		float fSlow128 = std::sqrt(std::max<float>(0.0f, pianodsp_faustpower2_f(fSlow127) / pianodsp_faustpower2_f(fSlow126) + -1.0f));
		float fSlow129 = fSlow127 / fSlow126;
		float fSlow130 = fSlow129 - fSlow128;
		float fSlow131 = std::exp(-(fConst35 / fSlow105)) / fSlow124 + -1.0f;
		float fSlow132 = fSlow124 * (fSlow128 + (1.0f - fSlow129));
		float fSlow133 = std::exp(-(fConst41 / fSlow105));
		float fSlow134 = pianodsp_faustpower2_f(fSlow133);
		float fSlow135 = 1.0f - fSlow134;
		float fSlow136 = 1.0f - fConst16 * fSlow134;
		float fSlow137 = std::sqrt(std::max<float>(0.0f, pianodsp_faustpower2_f(fSlow136) / pianodsp_faustpower2_f(fSlow135) + -1.0f));
		float fSlow138 = fSlow136 / fSlow135;
		float fSlow139 = fSlow138 - fSlow137;
		float fSlow140 = std::exp(-(fConst42 / fSlow105)) / fSlow133 + -1.0f;
		float fSlow141 = fSlow133 * (fSlow137 + (1.0f - fSlow138));
		float fSlow142 = std::exp(-(fConst48 / fSlow105));
		float fSlow143 = pianodsp_faustpower2_f(fSlow142);
		float fSlow144 = 1.0f - fSlow143;
		float fSlow145 = 1.0f - fConst16 * fSlow143;
		float fSlow146 = std::sqrt(std::max<float>(0.0f, pianodsp_faustpower2_f(fSlow145) / pianodsp_faustpower2_f(fSlow144) + -1.0f));
		float fSlow147 = fSlow145 / fSlow144;
		float fSlow148 = fSlow147 - fSlow146;
		float fSlow149 = std::exp(-(fConst49 / fSlow105)) / fSlow142 + -1.0f;
		float fSlow150 = fSlow142 * (fSlow146 + (1.0f - fSlow147));
		int iSlow151 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst52 * (static_cast<float>(fHslider7) / fSlow0))));
		float fSlow152 = 12.0f * fSlow103;
		float fSlow153 = std::exp(-(fConst56 / fSlow105));
		float fSlow154 = pianodsp_faustpower2_f(fSlow153);
		float fSlow155 = 1.0f - fSlow154;
		float fSlow156 = 1.0f - fConst16 * fSlow154;
		float fSlow157 = std::sqrt(std::max<float>(0.0f, pianodsp_faustpower2_f(fSlow156) / pianodsp_faustpower2_f(fSlow155) + -1.0f));
		float fSlow158 = fSlow156 / fSlow155;
		float fSlow159 = fSlow158 - fSlow157;
		float fSlow160 = std::exp(-(fConst57 / fSlow105)) / fSlow153 + -1.0f;
		float fSlow161 = fSlow153 * (fSlow157 + (1.0f - fSlow158));
		float fSlow162 = std::exp(-(fConst63 / fSlow105));
		float fSlow163 = pianodsp_faustpower2_f(fSlow162);
		float fSlow164 = 1.0f - fSlow163;
		float fSlow165 = 1.0f - fConst16 * fSlow163;
		float fSlow166 = std::sqrt(std::max<float>(0.0f, pianodsp_faustpower2_f(fSlow165) / pianodsp_faustpower2_f(fSlow164) + -1.0f));
		float fSlow167 = fSlow165 / fSlow164;
		float fSlow168 = fSlow167 - fSlow166;
		float fSlow169 = std::exp(-(fConst64 / fSlow105)) / fSlow162 + -1.0f;
		float fSlow170 = fSlow162 * (fSlow166 + (1.0f - fSlow167));
		float fSlow171 = std::exp(-(fConst70 / fSlow105));
		float fSlow172 = pianodsp_faustpower2_f(fSlow171);
		float fSlow173 = 1.0f - fSlow172;
		float fSlow174 = 1.0f - fConst16 * fSlow172;
		float fSlow175 = std::sqrt(std::max<float>(0.0f, pianodsp_faustpower2_f(fSlow174) / pianodsp_faustpower2_f(fSlow173) + -1.0f));
		float fSlow176 = fSlow174 / fSlow173;
		float fSlow177 = fSlow176 - fSlow175;
		float fSlow178 = std::exp(-(fConst71 / fSlow105)) / fSlow171 + -1.0f;
		float fSlow179 = fSlow171 * (fSlow175 + (1.0f - fSlow176));
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			fRec11[0] = fSlow15 * fRec11[1] + 1.0f;
			float fTemp0 = fRec11[0] + -1.0f;
			int iTemp1 = fTemp0 < fSlow18;
			float fTemp2 = fSlow15 * (fSlow19 * static_cast<float>(iTemp1) + fConst4 * static_cast<float>(fTemp0 >= fSlow18));
			fRec10[0] = fRec10[1] * (fTemp2 - fSlow21) + fSlow22 * (fSlow21 + (1.0f - fTemp2)) * static_cast<float>(iTemp1 & iSlow16);
			float fTemp3 = static_cast<float>((fTemp0 < 2.0f) & iSlow16);
			float fTemp4 = 0.030197384f * fTemp3 + fSlow24 * static_cast<float>((fTemp0 >= 2.0f) | iSlow23);
			fRec12[0] = fRec12[1] * fTemp4 + 0.15f * fTemp3 * (1.0f - fTemp4);
			iRec13[0] = 1103515245 * iRec13[1] + 12345;
			float fTemp5 = static_cast<float>(iRec13[0]) * (fRec12[0] + fRec10[0]);
			fVec0[0] = fSlow27 * fTemp5;
			float fTemp6 = 0.5f * fVec0[1] + fSlow26 * fTemp5;
			fVec1[0] = fTemp6;
			fRec9[0] = -(0.5f * (fSlow29 * fTemp6 + fSlow28 * fVec1[1]) + fSlow3 * fRec9[1]);
			fRec8[0] = fSlow30 * fRec9[0] + fSlow14 * fRec8[1];
			fRec7[0] = fSlow30 * fRec8[0] + fSlow14 * fRec7[1];
			fRec6[0] = fSlow30 * fRec7[0] + fSlow14 * fRec6[1];
			fRec5[0] = fSlow30 * fRec6[0] + fSlow14 * fRec5[1];
			fRec4[0] = fSlow31 * (fRec5[0] - fRec5[1]) - fSlow11 * (fSlow11 * fRec4[2] - fSlow10 * fRec4[1]);
			fRec3[0] = fSlow32 * fRec4[0] - fSlow9 * (fSlow9 * fRec3[2] - fSlow8 * fRec3[1]);
			fRec2[0] = fRec3[0] - fSlow7 * (fSlow7 * fRec2[2] - fSlow5 * fRec2[1]);
			fRec1[0] = fRec2[0] + fSlow36 * fRec2[2] - (fSlow35 * fRec2[1] + fSlow6 * (fSlow6 * fRec1[2] - fSlow5 * fRec1[1]));
			fRec0[0] = fSlow29 * fRec1[0] - fSlow3 * fRec0[1];
			fRec24[0] = fSlow37 * fTemp5 + fSlow14 * fRec24[1];
			fRec23[0] = fSlow30 * fRec24[0] + fSlow14 * fRec23[1];
			fRec22[0] = fSlow30 * fRec23[0] + fSlow14 * fRec22[1];
			fRec21[0] = fSlow30 * fRec22[0] + fSlow14 * fRec21[1];
			fRec20[0] = 0.5f * (fSlow29 * fRec21[0] + fSlow28 * fRec21[1]) - fSlow3 * fRec20[1];
			fRec25[0] = fSlow38 + fConst9 * fRec25[1];
			float fTemp7 = fRec25[0] * (fRec20[0] + fRec14[1]);
			fVec2[0] = fTemp7;
			fRec19[0] = fVec2[1] + fSlow42 * (fTemp7 - fRec19[1]);
			fRec18[0] = fRec19[1] + fSlow42 * (fRec19[0] - fRec18[1]);
			fRec17[IOTA0 & 8191] = fRec18[1] + fSlow42 * (fRec18[0] - fRec17[(IOTA0 - 1) & 8191]);
			float fTemp8 = fSlow77 * fRec17[(IOTA0 - iSlow72) & 8191];
			float fTemp9 = fRec20[0] + fRec25[0] * fRec15[1];
			fVec3[0] = fTemp9;
			fRec29[0] = fVec3[1] + fSlow42 * (fTemp9 - fRec29[1]);
			fRec28[0] = fRec29[1] + fSlow42 * (fRec29[0] - fRec28[1]);
			fRec27[IOTA0 & 8191] = fRec28[1] + fSlow42 * (fRec28[0] - fRec27[(IOTA0 - 1) & 8191]);
			float fTemp10 = fSlow91 * fRec27[(IOTA0 - iSlow87) & 8191];
			float fTemp11 = fSlow93 * fRec27[(IOTA0 - iSlow92) & 8191];
			float fTemp12 = fSlow95 * fRec17[(IOTA0 - iSlow94) & 8191];
			float fTemp13 = fTemp12 + fTemp11 + fTemp10 + fTemp8;
			fVec4[0] = fTemp13;
			fRec26[0] = fSlow96 * (2.0f * (fSlow58 * fTemp13 + fSlow55 * fVec4[1]) - fSlow61 * fRec26[1]);
			fRec14[0] = fTemp12 + fRec26[0] + fTemp8;
			fRec15[0] = fTemp10 + fRec26[0] + fTemp11;
			float fRec16 = fTemp13;
			fRec30[0] = fSlow100 * fRec16 - fSlow99 * (fSlow99 * fRec30[2] - fSlow97 * fRec30[1]);
			float fTemp14 = fSlow101 * (fRec30[0] - fRec30[2]) + fRec16 + fRec0[0];
			fVec5[IOTA0 & 8191] = fTemp14;
			fRec31[0] = fSlow102 + fConst9 * fRec31[1];
			float fTemp15 = 1.0f - fRec31[0];
			fRec43[0] = -(fConst19 * (fConst18 * fRec43[1] - (fRec36[1] + fRec36[2])));
			fRec42[0] = fSlow114 * (fRec36[1] + fSlow113 * fRec43[0]) + fSlow112 * fRec42[1];
			fVec6[IOTA0 & 16383] = 0.35355338f * fRec42[0] + 1e-20f;
			fVec7[IOTA0 & 4095] = fSlow104 * fRec31[0] * fTemp14;
			float fTemp16 = 0.3f * fVec7[(IOTA0 - iConst23) & 4095];
			float fTemp17 = fTemp16 + fVec6[(IOTA0 - iConst22) & 16383] - 0.6f * fRec40[1];
			fVec8[IOTA0 & 4095] = fTemp17;
			fRec40[0] = fVec8[(IOTA0 - iConst24) & 4095];
			float fRec41 = 0.6f * fTemp17;
			fRec47[0] = -(fConst19 * (fConst18 * fRec47[1] - (fRec32[1] + fRec32[2])));
			fRec46[0] = fSlow123 * (fRec32[1] + fSlow122 * fRec47[0]) + fSlow121 * fRec46[1];
			fVec9[IOTA0 & 16383] = 0.35355338f * fRec46[0] + 1e-20f;
			float fTemp18 = fVec9[(IOTA0 - iConst30) & 16383] + fTemp16 - 0.6f * fRec44[1];
			fVec10[IOTA0 & 2047] = fTemp18;
			fRec44[0] = fVec10[(IOTA0 - iConst31) & 2047];
			float fRec45 = 0.6f * fTemp18;
			float fTemp19 = fRec45 + fRec41;
			fRec51[0] = -(fConst19 * (fConst18 * fRec51[1] - (fRec34[1] + fRec34[2])));
			fRec50[0] = fSlow132 * (fRec34[1] + fSlow131 * fRec51[0]) + fSlow130 * fRec50[1];
			fVec11[IOTA0 & 16383] = 0.35355338f * fRec50[0] + 1e-20f;
			float fTemp20 = fVec11[(IOTA0 - iConst37) & 16383] - (fTemp16 + 0.6f * fRec48[1]);
			fVec12[IOTA0 & 4095] = fTemp20;
			fRec48[0] = fVec12[(IOTA0 - iConst38) & 4095];
			float fRec49 = 0.6f * fTemp20;
			fRec55[0] = -(fConst19 * (fConst18 * fRec55[1] - (fRec38[1] + fRec38[2])));
			fRec54[0] = fSlow141 * (fRec38[1] + fSlow140 * fRec55[0]) + fSlow139 * fRec54[1];
			fVec13[IOTA0 & 16383] = 0.35355338f * fRec54[0] + 1e-20f;
			float fTemp21 = fVec13[(IOTA0 - iConst44) & 16383] - (fTemp16 + 0.6f * fRec52[1]);
			fVec14[IOTA0 & 2047] = fTemp21;
			fRec52[0] = fVec14[(IOTA0 - iConst45) & 2047];
			float fRec53 = 0.6f * fTemp21;
			float fTemp22 = fRec53 + fRec49 + fTemp19;
			fRec59[0] = -(fConst19 * (fConst18 * fRec59[1] - (fRec33[1] + fRec33[2])));
			fRec58[0] = fSlow150 * (fRec33[1] + fSlow149 * fRec59[0]) + fSlow148 * fRec58[1];
			fVec15[IOTA0 & 32767] = 0.35355338f * fRec58[0] + 1e-20f;
			float fTemp23 = fVec5[(IOTA0 - iSlow151) & 8191];
			fVec16[IOTA0 & 4095] = fSlow152 * fRec31[0] * fTemp23;
			float fTemp24 = 0.3f * fVec16[(IOTA0 - iConst23) & 4095];
			float fTemp25 = fTemp24 + 0.6f * fRec56[1] + fVec15[(IOTA0 - iConst51) & 32767];
			fVec17[IOTA0 & 4095] = fTemp25;
			fRec56[0] = fVec17[(IOTA0 - iConst53) & 4095];
			float fRec57 = -(0.6f * fTemp25);
			fRec63[0] = -(fConst19 * (fConst18 * fRec63[1] - (fRec37[1] + fRec37[2])));
			fRec62[0] = fSlow161 * (fRec37[1] + fSlow160 * fRec63[0]) + fSlow159 * fRec62[1];
			fVec18[IOTA0 & 16383] = 0.35355338f * fRec62[0] + 1e-20f;
			float fTemp26 = fVec18[(IOTA0 - iConst59) & 16383] + fTemp24 + 0.6f * fRec60[1];
			fVec19[IOTA0 & 4095] = fTemp26;
			fRec60[0] = fVec19[(IOTA0 - iConst60) & 4095];
			float fRec61 = -(0.6f * fTemp26);
			fRec67[0] = -(fConst19 * (fConst18 * fRec67[1] - (fRec35[1] + fRec35[2])));
			fRec66[0] = fSlow170 * (fRec35[1] + fSlow169 * fRec67[0]) + fSlow168 * fRec66[1];
			fVec20[IOTA0 & 32767] = 0.35355338f * fRec66[0] + 1e-20f;
			float fTemp27 = 0.6f * fRec64[1] + fVec20[(IOTA0 - iConst66) & 32767];
			fVec21[IOTA0 & 4095] = fTemp27 - fTemp24;
			fRec64[0] = fVec21[(IOTA0 - iConst67) & 4095];
			float fRec65 = 0.6f * (fTemp24 - fTemp27);
			fRec71[0] = -(fConst19 * (fConst18 * fRec71[1] - (fRec39[1] + fRec39[2])));
			fRec70[0] = fSlow179 * (fRec39[1] + fSlow178 * fRec71[0]) + fSlow177 * fRec70[1];
			fVec22[IOTA0 & 32767] = 0.35355338f * fRec70[0] + 1e-20f;
			float fTemp28 = 0.6f * fRec68[1] + fVec22[(IOTA0 - iConst73) & 32767];
			fVec23[IOTA0 & 2047] = fTemp28 - fTemp24;
			fRec68[0] = fVec23[(IOTA0 - iConst74) & 2047];
			float fRec69 = 0.6f * (fTemp24 - fTemp28);
			fRec32[0] = fRec68[1] + fRec64[1] + fRec60[1] + fRec56[1] + fRec52[1] + fRec48[1] + fRec40[1] + fRec44[1] + fRec69 + fRec65 + fRec61 + fRec57 + fTemp22;
			fRec33[0] = fRec52[1] + fRec48[1] + fRec40[1] + fRec44[1] + fTemp22 - (fRec68[1] + fRec64[1] + fRec60[1] + fRec56[1] + fRec69 + fRec65 + fRec57 + fRec61);
			float fTemp29 = fRec49 + fRec53;
			fRec34[0] = fRec60[1] + fRec56[1] + fRec40[1] + fRec44[1] + fRec61 + fRec57 + fTemp19 - (fRec68[1] + fRec64[1] + fRec52[1] + fRec48[1] + fRec69 + fRec65 + fTemp29);
			fRec35[0] = fRec68[1] + fRec64[1] + fRec40[1] + fRec44[1] + fRec69 + fRec65 + fTemp19 - (fRec60[1] + fRec56[1] + fRec52[1] + fRec48[1] + fRec61 + fRec57 + fTemp29);
			float fTemp30 = fRec41 + fRec53;
			float fTemp31 = fRec45 + fRec49;
			fRec36[0] = fRec64[1] + fRec56[1] + fRec48[1] + fRec44[1] + fRec65 + fRec57 + fTemp31 - (fRec68[1] + fRec60[1] + fRec52[1] + fRec40[1] + fRec69 + fRec61 + fTemp30);
			fRec37[0] = fRec68[1] + fRec60[1] + fRec48[1] + fRec44[1] + fRec69 + fRec61 + fTemp31 - (fRec64[1] + fRec56[1] + fRec52[1] + fRec40[1] + fRec65 + fRec57 + fTemp30);
			float fTemp32 = fRec41 + fRec49;
			float fTemp33 = fRec45 + fRec53;
			fRec38[0] = fRec68[1] + fRec56[1] + fRec52[1] + fRec44[1] + fRec69 + fRec57 + fTemp33 - (fRec64[1] + fRec60[1] + fRec48[1] + fRec40[1] + fRec65 + fRec61 + fTemp32);
			fRec39[0] = fRec64[1] + fRec60[1] + fRec52[1] + fRec44[1] + fRec65 + fRec61 + fTemp33 - (fRec68[1] + fRec56[1] + fRec48[1] + fRec40[1] + fRec69 + fRec57 + fTemp32);
			output0[i0] = static_cast<FAUSTFLOAT>(0.37f * (fRec33[0] + fRec34[0]) + fSlow104 * fTemp15 * fTemp14);
			output1[i0] = static_cast<FAUSTFLOAT>(0.37f * (fRec33[0] - fRec34[0]) + fSlow152 * fTemp15 * fTemp23);
			fRec11[1] = fRec11[0];
			fRec10[1] = fRec10[0];
			fRec12[1] = fRec12[0];
			iRec13[1] = iRec13[0];
			fVec0[1] = fVec0[0];
			fVec1[1] = fVec1[0];
			fRec9[1] = fRec9[0];
			fRec8[1] = fRec8[0];
			fRec7[1] = fRec7[0];
			fRec6[1] = fRec6[0];
			fRec5[1] = fRec5[0];
			fRec4[2] = fRec4[1];
			fRec4[1] = fRec4[0];
			fRec3[2] = fRec3[1];
			fRec3[1] = fRec3[0];
			fRec2[2] = fRec2[1];
			fRec2[1] = fRec2[0];
			fRec1[2] = fRec1[1];
			fRec1[1] = fRec1[0];
			fRec0[1] = fRec0[0];
			IOTA0 = IOTA0 + 1;
			fRec24[1] = fRec24[0];
			fRec23[1] = fRec23[0];
			fRec22[1] = fRec22[0];
			fRec21[1] = fRec21[0];
			fRec20[1] = fRec20[0];
			fRec25[1] = fRec25[0];
			fVec2[1] = fVec2[0];
			fRec19[1] = fRec19[0];
			fRec18[1] = fRec18[0];
			fVec3[1] = fVec3[0];
			fRec29[1] = fRec29[0];
			fRec28[1] = fRec28[0];
			fVec4[1] = fVec4[0];
			fRec26[1] = fRec26[0];
			fRec14[1] = fRec14[0];
			fRec15[1] = fRec15[0];
			fRec30[2] = fRec30[1];
			fRec30[1] = fRec30[0];
			fRec31[1] = fRec31[0];
			fRec43[1] = fRec43[0];
			fRec42[1] = fRec42[0];
			fRec40[1] = fRec40[0];
			fRec47[1] = fRec47[0];
			fRec46[1] = fRec46[0];
			fRec44[1] = fRec44[0];
			fRec51[1] = fRec51[0];
			fRec50[1] = fRec50[0];
			fRec48[1] = fRec48[0];
			fRec55[1] = fRec55[0];
			fRec54[1] = fRec54[0];
			fRec52[1] = fRec52[0];
			fRec59[1] = fRec59[0];
			fRec58[1] = fRec58[0];
			fRec56[1] = fRec56[0];
			fRec63[1] = fRec63[0];
			fRec62[1] = fRec62[0];
			fRec60[1] = fRec60[0];
			fRec67[1] = fRec67[0];
			fRec66[1] = fRec66[0];
			fRec64[1] = fRec64[0];
			fRec71[1] = fRec71[0];
			fRec70[1] = fRec70[0];
			fRec68[1] = fRec68[0];
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
			fRec37[2] = fRec37[1];
			fRec37[1] = fRec37[0];
			fRec38[2] = fRec38[1];
			fRec38[1] = fRec38[0];
			fRec39[2] = fRec39[1];
			fRec39[1] = fRec39[0];
		}
	}

};

#endif

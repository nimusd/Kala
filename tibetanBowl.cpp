/* ------------------------------------------------------------
author: "Romain Michon"
copyright: "Romain Michon (rmichon@ccrma.stanford.edu)"
name: "tibetanBowl", "tibetanBowl"
version: "1.0"
Code generated with Faust 2.81.10 (https://faust.grame.fr)
Compilation options: -lang cpp -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __tibetanbowldsp_H__
#define  __tibetanbowldsp_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS tibetanbowldsp
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

class tibetanbowldspSIG0 {
	
  private:
	
	int iVec14[2];
	int iRec43[2];
	
  public:
	
	int getNumInputstibetanbowldspSIG0() {
		return 0;
	}
	int getNumOutputstibetanbowldspSIG0() {
		return 1;
	}
	
	void instanceInittibetanbowldspSIG0(int sample_rate) {
		for (int l57 = 0; l57 < 2; l57 = l57 + 1) {
			iVec14[l57] = 0;
		}
		for (int l58 = 0; l58 < 2; l58 = l58 + 1) {
			iRec43[l58] = 0;
		}
	}
	
	void filltibetanbowldspSIG0(int count, float* table) {
		for (int i1 = 0; i1 < count; i1 = i1 + 1) {
			iVec14[0] = 1;
			iRec43[0] = (iVec14[1] + iRec43[1]) % 65536;
			table[i1] = std::sin(9.58738e-05f * static_cast<float>(iRec43[0]));
			iVec14[1] = iVec14[0];
			iRec43[1] = iRec43[0];
		}
	}

};

static tibetanbowldspSIG0* newtibetanbowldspSIG0() { return (tibetanbowldspSIG0*)new tibetanbowldspSIG0(); }
static void deletetibetanbowldspSIG0(tibetanbowldspSIG0* dsp) { delete dsp; }

static float tibetanbowldsp_faustpower2_f(float value) {
	return value * value;
}
static float ftbl0tibetanbowldspSIG0[65536];

class tibetanbowldsp : public dsp {
	
 private:
	
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	int iVec0[2];
	FAUSTFLOAT fEntry0;
	float fConst3;
	float fConst4;
	FAUSTFLOAT fHslider0;
	FAUSTFLOAT fHslider1;
	FAUSTFLOAT fButton0;
	float fVec1[2];
	int iRec14[2];
	float fConst5;
	float fRec15[2];
	float fConst6;
	float fConst7;
	FAUSTFLOAT fEntry1;
	FAUSTFLOAT fHslider2;
	FAUSTFLOAT fEntry2;
	int IOTA0;
	float fVec2[8192];
	float fConst8;
	float fRec13[3];
	float fConst9;
	float fRec12[2];
	float fRec0[2];
	float fConst10;
	float fVec3[8192];
	float fConst11;
	float fRec17[3];
	float fRec16[2];
	float fRec1[2];
	float fConst12;
	float fVec4[4096];
	float fConst13;
	float fRec19[3];
	float fRec18[2];
	float fRec2[2];
	float fConst14;
	float fVec5[4096];
	float fConst15;
	float fRec21[3];
	float fRec20[2];
	float fRec3[2];
	float fConst16;
	float fVec6[2048];
	float fConst17;
	float fRec23[3];
	float fRec22[2];
	float fRec4[2];
	float fRec5[2];
	float fConst18;
	float fVec7[2048];
	float fConst19;
	float fRec25[3];
	float fRec24[2];
	float fRec6[2];
	float fConst20;
	float fVec8[2048];
	float fConst21;
	float fRec27[3];
	float fRec26[2];
	float fRec7[2];
	float fConst22;
	float fVec9[1024];
	float fConst23;
	float fRec29[3];
	float fRec28[2];
	float fRec8[2];
	float fConst24;
	float fVec10[1024];
	float fConst25;
	float fRec31[3];
	float fRec30[2];
	float fRec9[2];
	float fConst26;
	float fVec11[1024];
	float fConst27;
	float fRec33[3];
	float fRec32[2];
	float fRec10[2];
	float fConst28;
	float fVec12[512];
	float fConst29;
	float fRec35[3];
	float fRec34[2];
	float fRec11[2];
	float fVec13[2];
	float fConst30;
	float fConst31;
	FAUSTFLOAT fHslider3;
	float fRec36[2];
	FAUSTFLOAT fEntry3;
	float fRec42[2];
	float fRec41[2];
	float fRec40[2];
	float fRec39[2];
	float fRec38[2];
	float fRec37[2];
	FAUSTFLOAT fHslider4;
	float fRec45[2];
	float fConst32;
	float fRec44[2];
	float fRec51[2];
	float fRec50[2];
	float fRec49[2];
	float fRec48[2];
	float fRec47[2];
	float fRec46[2];
	float fVec15[8192];
	FAUSTFLOAT fHslider5;
	float fRec52[2];
	FAUSTFLOAT fHslider6;
	FAUSTFLOAT fHslider7;
	float fConst33;
	float fConst34;
	float fConst35;
	float fConst36;
	float fConst37;
	float fConst38;
	float fConst39;
	float fRec64[2];
	float fConst40;
	float fRec63[2];
	float fVec16[16384];
	float fConst41;
	int iConst42;
	float fVec17[4096];
	int iConst43;
	float fVec18[4096];
	int iConst44;
	float fRec61[2];
	float fConst45;
	float fConst46;
	float fConst47;
	float fRec68[2];
	float fConst48;
	float fRec67[2];
	float fVec19[16384];
	float fConst49;
	int iConst50;
	float fVec20[2048];
	int iConst51;
	float fRec65[2];
	float fConst52;
	float fConst53;
	float fConst54;
	float fRec72[2];
	float fConst55;
	float fRec71[2];
	float fVec21[16384];
	float fConst56;
	int iConst57;
	float fVec22[4096];
	int iConst58;
	float fRec69[2];
	float fConst59;
	float fConst60;
	float fConst61;
	float fRec76[2];
	float fConst62;
	float fRec75[2];
	float fVec23[16384];
	float fConst63;
	int iConst64;
	float fVec24[2048];
	int iConst65;
	float fRec73[2];
	float fConst66;
	float fConst67;
	float fConst68;
	float fRec80[2];
	float fConst69;
	float fRec79[2];
	float fVec25[32768];
	float fConst70;
	int iConst71;
	FAUSTFLOAT fHslider8;
	float fConst72;
	float fVec26[4096];
	float fVec27[4096];
	int iConst73;
	float fRec77[2];
	float fConst74;
	float fConst75;
	float fConst76;
	float fRec84[2];
	float fConst77;
	float fRec83[2];
	float fVec28[16384];
	float fConst78;
	int iConst79;
	float fVec29[4096];
	int iConst80;
	float fRec81[2];
	float fConst81;
	float fConst82;
	float fConst83;
	float fRec88[2];
	float fConst84;
	float fRec87[2];
	float fVec30[32768];
	float fConst85;
	int iConst86;
	float fVec31[4096];
	int iConst87;
	float fRec85[2];
	float fConst88;
	float fConst89;
	float fConst90;
	float fRec92[2];
	float fConst91;
	float fRec91[2];
	float fVec32[32768];
	float fConst92;
	int iConst93;
	float fVec33[2048];
	int iConst94;
	float fRec89[2];
	float fRec53[3];
	float fRec54[3];
	float fRec55[3];
	float fRec56[3];
	float fRec57[3];
	float fRec58[3];
	float fRec59[3];
	float fRec60[3];
	
 public:
	tibetanbowldsp() {
	}
	
	void metadata(Meta* m) { 
		m->declare("author", "Romain Michon");
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "1.22.0");
		m->declare("compile_options", "-lang cpp -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("copyright", "Romain Michon (rmichon@ccrma.stanford.edu)");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.2.0");
		m->declare("description", "Banded Waveguide Modeld Tibetan Bowl");
		m->declare("envelopes.lib/adsr:author", "Yann Orlarey and Andrey Bundin");
		m->declare("envelopes.lib/author", "GRAME");
		m->declare("envelopes.lib/copyright", "GRAME");
		m->declare("envelopes.lib/license", "LGPL with exception");
		m->declare("envelopes.lib/name", "Faust Envelope Library");
		m->declare("envelopes.lib/version", "1.3.0");
		m->declare("filename", "piano.dsp");
		m->declare("filters.lib/allpass_comb:author", "Julius O. Smith III");
		m->declare("filters.lib/allpass_comb:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/allpass_comb:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/allpassnn:author", "Julius O. Smith III");
		m->declare("filters.lib/allpassnn:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/allpassnn:license", "MIT-style STK-4.3 license");
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
		tibetanbowldspSIG0* sig0 = newtibetanbowldspSIG0();
		sig0->instanceInittibetanbowldspSIG0(sample_rate);
		sig0->filltibetanbowldspSIG0(65536, ftbl0tibetanbowldspSIG0);
		deletetibetanbowldspSIG0(sig0);
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, static_cast<float>(fSampleRate)));
		fConst1 = 100.53097f / fConst0;
		fConst2 = tibetanbowldsp_faustpower2_f(1.0f - fConst1);
		fConst3 = 6.2587333f / fConst0;
		fConst4 = 2.0f * (fConst1 - 1.0f);
		fConst5 = 1.0f / std::max<float>(1.0f, 0.01f * fConst0);
		fConst6 = 0.02f * fConst0;
		fConst7 = 1.0f / std::max<float>(1.0f, fConst6);
		fConst8 = 1.0039068f * fConst0;
		fConst9 = 0.5f * (1.0f - fConst2);
		fConst10 = 6.307637f / fConst0;
		fConst11 = 0.99612343f * fConst0;
		fConst12 = 18.718727f / fConst0;
		fConst13 = 0.33566305f * fConst0;
		fConst14 = 18.807444f / fConst0;
		fConst15 = 0.3340797f * fConst0;
		fConst16 = 35.84213f / fConst0;
		fConst17 = 0.17530167f * fConst0;
		fConst18 = 56.537357f / fConst0;
		fConst19 = 0.11113334f * fConst0;
		fConst20 = 56.646038f / fConst0;
		fConst21 = 0.11092012f * fConst0;
		fConst22 = 80.63231f / fConst0;
		fConst23 = 0.07792392f * fConst0;
		fConst24 = 80.47115f / fConst0;
		fConst25 = 0.07807997f * fConst0;
		fConst26 = 108.578606f / fConst0;
		fConst27 = 0.057867616f * fConst0;
		fConst28 = 138.07945f / fConst0;
		fConst29 = 0.04550413f * fConst0;
		fConst30 = 44.1f / fConst0;
		fConst31 = 1.0f - fConst30;
		fConst32 = 1.0f / fConst0;
		fConst33 = std::floor(0.174713f * fConst0 + 0.5f);
		fConst34 = fConst33 / fConst0;
		fConst35 = 3.4538777f * fConst34;
		fConst36 = std::cos(37699.113f / fConst0);
		fConst37 = 1.0f / std::tan(628.31854f / fConst0);
		fConst38 = 1.0f - fConst37;
		fConst39 = 1.0f / (fConst37 + 1.0f);
		fConst40 = 2.3025851f * fConst34;
		fConst41 = std::floor(0.022904f * fConst0 + 0.5f);
		iConst42 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst33 - fConst41)));
		iConst43 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst6)));
		iConst44 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst41 + -1.0f)));
		fConst45 = std::floor(0.153129f * fConst0 + 0.5f);
		fConst46 = fConst45 / fConst0;
		fConst47 = 3.4538777f * fConst46;
		fConst48 = 2.3025851f * fConst46;
		fConst49 = std::floor(0.020346f * fConst0 + 0.5f);
		iConst50 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst45 - fConst49)));
		iConst51 = static_cast<int>(std::min<float>(1024.0f, std::max<float>(0.0f, fConst49 + -1.0f)));
		fConst52 = std::floor(0.127837f * fConst0 + 0.5f);
		fConst53 = fConst52 / fConst0;
		fConst54 = 3.4538777f * fConst53;
		fConst55 = 2.3025851f * fConst53;
		fConst56 = std::floor(0.031604f * fConst0 + 0.5f);
		iConst57 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst52 - fConst56)));
		iConst58 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst56 + -1.0f)));
		fConst59 = std::floor(0.125f * fConst0 + 0.5f);
		fConst60 = fConst59 / fConst0;
		fConst61 = 3.4538777f * fConst60;
		fConst62 = 2.3025851f * fConst60;
		fConst63 = std::floor(0.013458f * fConst0 + 0.5f);
		iConst64 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst59 - fConst63)));
		iConst65 = static_cast<int>(std::min<float>(1024.0f, std::max<float>(0.0f, fConst63 + -1.0f)));
		fConst66 = std::floor(0.210389f * fConst0 + 0.5f);
		fConst67 = fConst66 / fConst0;
		fConst68 = 3.4538777f * fConst67;
		fConst69 = 2.3025851f * fConst67;
		fConst70 = std::floor(0.024421f * fConst0 + 0.5f);
		iConst71 = static_cast<int>(std::min<float>(16384.0f, std::max<float>(0.0f, fConst66 - fConst70)));
		fConst72 = 0.5f * fConst0;
		iConst73 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst70 + -1.0f)));
		fConst74 = std::floor(0.192303f * fConst0 + 0.5f);
		fConst75 = fConst74 / fConst0;
		fConst76 = 3.4538777f * fConst75;
		fConst77 = 2.3025851f * fConst75;
		fConst78 = std::floor(0.029291f * fConst0 + 0.5f);
		iConst79 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst74 - fConst78)));
		iConst80 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst78 + -1.0f)));
		fConst81 = std::floor(0.256891f * fConst0 + 0.5f);
		fConst82 = fConst81 / fConst0;
		fConst83 = 3.4538777f * fConst82;
		fConst84 = 2.3025851f * fConst82;
		fConst85 = std::floor(0.027333f * fConst0 + 0.5f);
		iConst86 = static_cast<int>(std::min<float>(16384.0f, std::max<float>(0.0f, fConst81 - fConst85)));
		iConst87 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst85 + -1.0f)));
		fConst88 = std::floor(0.219991f * fConst0 + 0.5f);
		fConst89 = fConst88 / fConst0;
		fConst90 = 3.4538777f * fConst89;
		fConst91 = 2.3025851f * fConst89;
		fConst92 = std::floor(0.019123f * fConst0 + 0.5f);
		iConst93 = static_cast<int>(std::min<float>(16384.0f, std::max<float>(0.0f, fConst88 - fConst92)));
		iConst94 = static_cast<int>(std::min<float>(1024.0f, std::max<float>(0.0f, fConst92 + -1.0f)));
	}
	
	virtual void instanceResetUserInterface() {
		fEntry0 = static_cast<FAUSTFLOAT>(4.4e+02f);
		fHslider0 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider1 = static_cast<FAUSTFLOAT>(1.0f);
		fButton0 = static_cast<FAUSTFLOAT>(0.0f);
		fEntry1 = static_cast<FAUSTFLOAT>(0.8f);
		fHslider2 = static_cast<FAUSTFLOAT>(0.2f);
		fEntry2 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider3 = static_cast<FAUSTFLOAT>(0.0f);
		fEntry3 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider4 = static_cast<FAUSTFLOAT>(2.2e+02f);
		fHslider5 = static_cast<FAUSTFLOAT>(0.137f);
		fHslider6 = static_cast<FAUSTFLOAT>(0.6f);
		fHslider7 = static_cast<FAUSTFLOAT>(0.72f);
		fHslider8 = static_cast<FAUSTFLOAT>(0.5f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iVec0[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fVec1[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			iRec14[l2] = 0;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec15[l3] = 0.0f;
		}
		IOTA0 = 0;
		for (int l4 = 0; l4 < 8192; l4 = l4 + 1) {
			fVec2[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 3; l5 = l5 + 1) {
			fRec13[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fRec12[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			fRec0[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 8192; l8 = l8 + 1) {
			fVec3[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 3; l9 = l9 + 1) {
			fRec17[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			fRec16[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fRec1[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 4096; l12 = l12 + 1) {
			fVec4[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 3; l13 = l13 + 1) {
			fRec19[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
			fRec18[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fRec2[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 4096; l16 = l16 + 1) {
			fVec5[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 3; l17 = l17 + 1) {
			fRec21[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec20[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 2; l19 = l19 + 1) {
			fRec3[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 2048; l20 = l20 + 1) {
			fVec6[l20] = 0.0f;
		}
		for (int l21 = 0; l21 < 3; l21 = l21 + 1) {
			fRec23[l21] = 0.0f;
		}
		for (int l22 = 0; l22 < 2; l22 = l22 + 1) {
			fRec22[l22] = 0.0f;
		}
		for (int l23 = 0; l23 < 2; l23 = l23 + 1) {
			fRec4[l23] = 0.0f;
		}
		for (int l24 = 0; l24 < 2; l24 = l24 + 1) {
			fRec5[l24] = 0.0f;
		}
		for (int l25 = 0; l25 < 2048; l25 = l25 + 1) {
			fVec7[l25] = 0.0f;
		}
		for (int l26 = 0; l26 < 3; l26 = l26 + 1) {
			fRec25[l26] = 0.0f;
		}
		for (int l27 = 0; l27 < 2; l27 = l27 + 1) {
			fRec24[l27] = 0.0f;
		}
		for (int l28 = 0; l28 < 2; l28 = l28 + 1) {
			fRec6[l28] = 0.0f;
		}
		for (int l29 = 0; l29 < 2048; l29 = l29 + 1) {
			fVec8[l29] = 0.0f;
		}
		for (int l30 = 0; l30 < 3; l30 = l30 + 1) {
			fRec27[l30] = 0.0f;
		}
		for (int l31 = 0; l31 < 2; l31 = l31 + 1) {
			fRec26[l31] = 0.0f;
		}
		for (int l32 = 0; l32 < 2; l32 = l32 + 1) {
			fRec7[l32] = 0.0f;
		}
		for (int l33 = 0; l33 < 1024; l33 = l33 + 1) {
			fVec9[l33] = 0.0f;
		}
		for (int l34 = 0; l34 < 3; l34 = l34 + 1) {
			fRec29[l34] = 0.0f;
		}
		for (int l35 = 0; l35 < 2; l35 = l35 + 1) {
			fRec28[l35] = 0.0f;
		}
		for (int l36 = 0; l36 < 2; l36 = l36 + 1) {
			fRec8[l36] = 0.0f;
		}
		for (int l37 = 0; l37 < 1024; l37 = l37 + 1) {
			fVec10[l37] = 0.0f;
		}
		for (int l38 = 0; l38 < 3; l38 = l38 + 1) {
			fRec31[l38] = 0.0f;
		}
		for (int l39 = 0; l39 < 2; l39 = l39 + 1) {
			fRec30[l39] = 0.0f;
		}
		for (int l40 = 0; l40 < 2; l40 = l40 + 1) {
			fRec9[l40] = 0.0f;
		}
		for (int l41 = 0; l41 < 1024; l41 = l41 + 1) {
			fVec11[l41] = 0.0f;
		}
		for (int l42 = 0; l42 < 3; l42 = l42 + 1) {
			fRec33[l42] = 0.0f;
		}
		for (int l43 = 0; l43 < 2; l43 = l43 + 1) {
			fRec32[l43] = 0.0f;
		}
		for (int l44 = 0; l44 < 2; l44 = l44 + 1) {
			fRec10[l44] = 0.0f;
		}
		for (int l45 = 0; l45 < 512; l45 = l45 + 1) {
			fVec12[l45] = 0.0f;
		}
		for (int l46 = 0; l46 < 3; l46 = l46 + 1) {
			fRec35[l46] = 0.0f;
		}
		for (int l47 = 0; l47 < 2; l47 = l47 + 1) {
			fRec34[l47] = 0.0f;
		}
		for (int l48 = 0; l48 < 2; l48 = l48 + 1) {
			fRec11[l48] = 0.0f;
		}
		for (int l49 = 0; l49 < 2; l49 = l49 + 1) {
			fVec13[l49] = 0.0f;
		}
		for (int l50 = 0; l50 < 2; l50 = l50 + 1) {
			fRec36[l50] = 0.0f;
		}
		for (int l51 = 0; l51 < 2; l51 = l51 + 1) {
			fRec42[l51] = 0.0f;
		}
		for (int l52 = 0; l52 < 2; l52 = l52 + 1) {
			fRec41[l52] = 0.0f;
		}
		for (int l53 = 0; l53 < 2; l53 = l53 + 1) {
			fRec40[l53] = 0.0f;
		}
		for (int l54 = 0; l54 < 2; l54 = l54 + 1) {
			fRec39[l54] = 0.0f;
		}
		for (int l55 = 0; l55 < 2; l55 = l55 + 1) {
			fRec38[l55] = 0.0f;
		}
		for (int l56 = 0; l56 < 2; l56 = l56 + 1) {
			fRec37[l56] = 0.0f;
		}
		for (int l59 = 0; l59 < 2; l59 = l59 + 1) {
			fRec45[l59] = 0.0f;
		}
		for (int l60 = 0; l60 < 2; l60 = l60 + 1) {
			fRec44[l60] = 0.0f;
		}
		for (int l61 = 0; l61 < 2; l61 = l61 + 1) {
			fRec51[l61] = 0.0f;
		}
		for (int l62 = 0; l62 < 2; l62 = l62 + 1) {
			fRec50[l62] = 0.0f;
		}
		for (int l63 = 0; l63 < 2; l63 = l63 + 1) {
			fRec49[l63] = 0.0f;
		}
		for (int l64 = 0; l64 < 2; l64 = l64 + 1) {
			fRec48[l64] = 0.0f;
		}
		for (int l65 = 0; l65 < 2; l65 = l65 + 1) {
			fRec47[l65] = 0.0f;
		}
		for (int l66 = 0; l66 < 2; l66 = l66 + 1) {
			fRec46[l66] = 0.0f;
		}
		for (int l67 = 0; l67 < 8192; l67 = l67 + 1) {
			fVec15[l67] = 0.0f;
		}
		for (int l68 = 0; l68 < 2; l68 = l68 + 1) {
			fRec52[l68] = 0.0f;
		}
		for (int l69 = 0; l69 < 2; l69 = l69 + 1) {
			fRec64[l69] = 0.0f;
		}
		for (int l70 = 0; l70 < 2; l70 = l70 + 1) {
			fRec63[l70] = 0.0f;
		}
		for (int l71 = 0; l71 < 16384; l71 = l71 + 1) {
			fVec16[l71] = 0.0f;
		}
		for (int l72 = 0; l72 < 4096; l72 = l72 + 1) {
			fVec17[l72] = 0.0f;
		}
		for (int l73 = 0; l73 < 4096; l73 = l73 + 1) {
			fVec18[l73] = 0.0f;
		}
		for (int l74 = 0; l74 < 2; l74 = l74 + 1) {
			fRec61[l74] = 0.0f;
		}
		for (int l75 = 0; l75 < 2; l75 = l75 + 1) {
			fRec68[l75] = 0.0f;
		}
		for (int l76 = 0; l76 < 2; l76 = l76 + 1) {
			fRec67[l76] = 0.0f;
		}
		for (int l77 = 0; l77 < 16384; l77 = l77 + 1) {
			fVec19[l77] = 0.0f;
		}
		for (int l78 = 0; l78 < 2048; l78 = l78 + 1) {
			fVec20[l78] = 0.0f;
		}
		for (int l79 = 0; l79 < 2; l79 = l79 + 1) {
			fRec65[l79] = 0.0f;
		}
		for (int l80 = 0; l80 < 2; l80 = l80 + 1) {
			fRec72[l80] = 0.0f;
		}
		for (int l81 = 0; l81 < 2; l81 = l81 + 1) {
			fRec71[l81] = 0.0f;
		}
		for (int l82 = 0; l82 < 16384; l82 = l82 + 1) {
			fVec21[l82] = 0.0f;
		}
		for (int l83 = 0; l83 < 4096; l83 = l83 + 1) {
			fVec22[l83] = 0.0f;
		}
		for (int l84 = 0; l84 < 2; l84 = l84 + 1) {
			fRec69[l84] = 0.0f;
		}
		for (int l85 = 0; l85 < 2; l85 = l85 + 1) {
			fRec76[l85] = 0.0f;
		}
		for (int l86 = 0; l86 < 2; l86 = l86 + 1) {
			fRec75[l86] = 0.0f;
		}
		for (int l87 = 0; l87 < 16384; l87 = l87 + 1) {
			fVec23[l87] = 0.0f;
		}
		for (int l88 = 0; l88 < 2048; l88 = l88 + 1) {
			fVec24[l88] = 0.0f;
		}
		for (int l89 = 0; l89 < 2; l89 = l89 + 1) {
			fRec73[l89] = 0.0f;
		}
		for (int l90 = 0; l90 < 2; l90 = l90 + 1) {
			fRec80[l90] = 0.0f;
		}
		for (int l91 = 0; l91 < 2; l91 = l91 + 1) {
			fRec79[l91] = 0.0f;
		}
		for (int l92 = 0; l92 < 32768; l92 = l92 + 1) {
			fVec25[l92] = 0.0f;
		}
		for (int l93 = 0; l93 < 4096; l93 = l93 + 1) {
			fVec26[l93] = 0.0f;
		}
		for (int l94 = 0; l94 < 4096; l94 = l94 + 1) {
			fVec27[l94] = 0.0f;
		}
		for (int l95 = 0; l95 < 2; l95 = l95 + 1) {
			fRec77[l95] = 0.0f;
		}
		for (int l96 = 0; l96 < 2; l96 = l96 + 1) {
			fRec84[l96] = 0.0f;
		}
		for (int l97 = 0; l97 < 2; l97 = l97 + 1) {
			fRec83[l97] = 0.0f;
		}
		for (int l98 = 0; l98 < 16384; l98 = l98 + 1) {
			fVec28[l98] = 0.0f;
		}
		for (int l99 = 0; l99 < 4096; l99 = l99 + 1) {
			fVec29[l99] = 0.0f;
		}
		for (int l100 = 0; l100 < 2; l100 = l100 + 1) {
			fRec81[l100] = 0.0f;
		}
		for (int l101 = 0; l101 < 2; l101 = l101 + 1) {
			fRec88[l101] = 0.0f;
		}
		for (int l102 = 0; l102 < 2; l102 = l102 + 1) {
			fRec87[l102] = 0.0f;
		}
		for (int l103 = 0; l103 < 32768; l103 = l103 + 1) {
			fVec30[l103] = 0.0f;
		}
		for (int l104 = 0; l104 < 4096; l104 = l104 + 1) {
			fVec31[l104] = 0.0f;
		}
		for (int l105 = 0; l105 < 2; l105 = l105 + 1) {
			fRec85[l105] = 0.0f;
		}
		for (int l106 = 0; l106 < 2; l106 = l106 + 1) {
			fRec92[l106] = 0.0f;
		}
		for (int l107 = 0; l107 < 2; l107 = l107 + 1) {
			fRec91[l107] = 0.0f;
		}
		for (int l108 = 0; l108 < 32768; l108 = l108 + 1) {
			fVec32[l108] = 0.0f;
		}
		for (int l109 = 0; l109 < 2048; l109 = l109 + 1) {
			fVec33[l109] = 0.0f;
		}
		for (int l110 = 0; l110 < 2; l110 = l110 + 1) {
			fRec89[l110] = 0.0f;
		}
		for (int l111 = 0; l111 < 3; l111 = l111 + 1) {
			fRec53[l111] = 0.0f;
		}
		for (int l112 = 0; l112 < 3; l112 = l112 + 1) {
			fRec54[l112] = 0.0f;
		}
		for (int l113 = 0; l113 < 3; l113 = l113 + 1) {
			fRec55[l113] = 0.0f;
		}
		for (int l114 = 0; l114 < 3; l114 = l114 + 1) {
			fRec56[l114] = 0.0f;
		}
		for (int l115 = 0; l115 < 3; l115 = l115 + 1) {
			fRec57[l115] = 0.0f;
		}
		for (int l116 = 0; l116 < 3; l116 = l116 + 1) {
			fRec58[l116] = 0.0f;
		}
		for (int l117 = 0; l117 < 3; l117 = l117 + 1) {
			fRec59[l117] = 0.0f;
		}
		for (int l118 = 0; l118 < 3; l118 = l118 + 1) {
			fRec60[l118] = 0.0f;
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
	
	virtual tibetanbowldsp* clone() {
		return new tibetanbowldsp();
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
		ui_interface->addNumEntry("gain", &fEntry1, FAUSTFLOAT(0.8f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fButton0, "1", "");
		ui_interface->declare(&fButton0, "tooltip", "noteOn = 1, noteOff = 0");
		ui_interface->addButton("gate", &fButton0);
		ui_interface->closeBox();
		ui_interface->openHorizontalBox("Physical_and_Nonlinearity");
		ui_interface->openVerticalBox("Nonlinear_Filter_Parameters");
		ui_interface->declare(&fHslider4, "3", "");
		ui_interface->declare(&fHslider4, "tooltip", "Frequency of the sine wave for the modulation of theta (works if Modulation Type=3)");
		ui_interface->declare(&fHslider4, "unit", "Hz");
		ui_interface->addHorizontalSlider("Modulation_Frequency", &fHslider4, FAUSTFLOAT(2.2e+02f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(1e+03f), FAUSTFLOAT(0.1f));
		ui_interface->declare(&fEntry3, "3", "");
		ui_interface->declare(&fEntry3, "tooltip", "0=theta is modulated by the incoming signal; 1=theta is modulated by the averaged incoming signal;  2=theta is modulated by the squared incoming signal; 3=theta is modulated by a sine wave of frequency freqMod;  4=theta is modulated by a sine wave of frequency freq;");
		ui_interface->addNumEntry("Modulation_Type", &fEntry3, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(4.0f), FAUSTFLOAT(1.0f));
		ui_interface->declare(&fHslider3, "3", "");
		ui_interface->declare(&fHslider3, "tooltip", "Nonlinearity factor (value between 0 and 1)");
		ui_interface->addHorizontalSlider("Nonlinearity", &fHslider3, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Physical_Parameters");
		ui_interface->declare(&fHslider1, "2", "");
		ui_interface->declare(&fHslider1, "tooltip", "A value between 0 and 1");
		ui_interface->addHorizontalSlider("Base_Gain", &fHslider1, FAUSTFLOAT(1.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider2, "2", "");
		ui_interface->declare(&fHslider2, "tooltip", "Bow pressure on the instrument (Value between 0 and 1)");
		ui_interface->addHorizontalSlider("Bow_Pressure", &fHslider2, FAUSTFLOAT(0.2f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fEntry2, "2", "");
		ui_interface->declare(&fEntry2, "tooltip", "0=Bow; 1=Strike");
		ui_interface->addNumEntry("Excitation_Selector", &fEntry2, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(1.0f));
		ui_interface->declare(&fHslider0, "2", "");
		ui_interface->declare(&fHslider0, "tooltip", "A value between 0 and 1");
		ui_interface->addHorizontalSlider("Integration_Constant", &fHslider0, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Reverb");
		ui_interface->addHorizontalSlider("reverbGain", &fHslider5, FAUSTFLOAT(0.137f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("roomSize", &fHslider7, FAUSTFLOAT(0.72f), FAUSTFLOAT(0.01f), FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Spat");
		ui_interface->addHorizontalSlider("pan angle", &fHslider6, FAUSTFLOAT(0.6f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("spatial width", &fHslider8, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = static_cast<float>(fEntry0);
		float fSlow1 = fConst4 * std::cos(fConst3 * fSlow0);
		float fSlow2 = static_cast<float>(fHslider0);
		float fSlow3 = 0.1f * static_cast<float>(fHslider1) + 0.9f;
		float fSlow4 = static_cast<float>(fButton0);
		int iSlow5 = fSlow4 == 0.0f;
		float fSlow6 = 0.1f * static_cast<float>(fEntry1) + 0.03f;
		float fSlow7 = 1e+01f - 9.0f * static_cast<float>(fHslider2);
		float fSlow8 = static_cast<float>(fEntry2);
		float fSlow9 = 0.083333336f * (fSlow8 + -1.0f);
		float fSlow10 = 1.1900357f * fSlow8;
		int iSlow11 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst8 / fSlow0)));
		float fSlow12 = fConst4 * std::cos(fConst10 * fSlow0);
		int iSlow13 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst11 / fSlow0)));
		float fSlow14 = fConst4 * std::cos(fConst12 * fSlow0);
		float fSlow15 = 1.0914886f * fSlow8;
		int iSlow16 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst13 / fSlow0)));
		float fSlow17 = fConst4 * std::cos(fConst14 * fSlow0);
		int iSlow18 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst15 / fSlow0)));
		float fSlow19 = fConst4 * std::cos(fConst16 * fSlow0);
		float fSlow20 = 4.2995043f * fSlow8;
		int iSlow21 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst17 / fSlow0)));
		float fSlow22 = fConst4 * std::cos(fConst18 * fSlow0);
		float fSlow23 = 4.0063033f * fSlow8;
		int iSlow24 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst19 / fSlow0)));
		float fSlow25 = fConst4 * std::cos(fConst20 * fSlow0);
		int iSlow26 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst21 / fSlow0)));
		float fSlow27 = fConst4 * std::cos(fConst22 * fSlow0);
		float fSlow28 = 0.7063034f * fSlow8;
		int iSlow29 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst23 / fSlow0)));
		float fSlow30 = fConst4 * std::cos(fConst24 * fSlow0);
		int iSlow31 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst25 / fSlow0)));
		float fSlow32 = fConst4 * std::cos(fConst26 * fSlow0);
		float fSlow33 = 5.7063036f * fSlow8;
		int iSlow34 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst27 / fSlow0)));
		float fSlow35 = fConst4 * std::cos(fConst28 * fSlow0);
		int iSlow36 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst29 / fSlow0)));
		float fSlow37 = fConst30 * static_cast<float>(fHslider3);
		float fSlow38 = static_cast<float>(fEntry3);
		float fSlow39 = 3.1415927f * static_cast<float>(fSlow38 == 2.0f);
		float fSlow40 = 1.5707964f * static_cast<float>(fSlow38 == 1.0f);
		float fSlow41 = 3.1415927f * static_cast<float>(fSlow38 == 0.0f);
		float fSlow42 = static_cast<float>(fSlow38 < 3.0f);
		float fSlow43 = fConst30 * static_cast<float>(fHslider4);
		float fSlow44 = static_cast<float>(fSlow38 != 4.0f);
		float fSlow45 = fSlow0 * static_cast<float>(fSlow38 == 4.0f);
		float fSlow46 = static_cast<float>(fSlow38 >= 3.0f);
		float fSlow47 = fConst30 * static_cast<float>(fHslider5);
		float fSlow48 = static_cast<float>(fHslider6);
		float fSlow49 = 1.0f - fSlow48;
		float fSlow50 = static_cast<float>(fHslider7);
		float fSlow51 = std::exp(-(fConst35 / fSlow50));
		float fSlow52 = tibetanbowldsp_faustpower2_f(fSlow51);
		float fSlow53 = 1.0f - fSlow52;
		float fSlow54 = 1.0f - fConst36 * fSlow52;
		float fSlow55 = std::sqrt(std::max<float>(0.0f, tibetanbowldsp_faustpower2_f(fSlow54) / tibetanbowldsp_faustpower2_f(fSlow53) + -1.0f));
		float fSlow56 = fSlow54 / fSlow53;
		float fSlow57 = fSlow56 - fSlow55;
		float fSlow58 = std::exp(-(fConst40 / fSlow50)) / fSlow51 + -1.0f;
		float fSlow59 = fSlow51 * (fSlow55 + (1.0f - fSlow56));
		float fSlow60 = std::exp(-(fConst47 / fSlow50));
		float fSlow61 = tibetanbowldsp_faustpower2_f(fSlow60);
		float fSlow62 = 1.0f - fSlow61;
		float fSlow63 = 1.0f - fConst36 * fSlow61;
		float fSlow64 = std::sqrt(std::max<float>(0.0f, tibetanbowldsp_faustpower2_f(fSlow63) / tibetanbowldsp_faustpower2_f(fSlow62) + -1.0f));
		float fSlow65 = fSlow63 / fSlow62;
		float fSlow66 = fSlow65 - fSlow64;
		float fSlow67 = std::exp(-(fConst48 / fSlow50)) / fSlow60 + -1.0f;
		float fSlow68 = fSlow60 * (fSlow64 + (1.0f - fSlow65));
		float fSlow69 = std::exp(-(fConst54 / fSlow50));
		float fSlow70 = tibetanbowldsp_faustpower2_f(fSlow69);
		float fSlow71 = 1.0f - fSlow70;
		float fSlow72 = 1.0f - fConst36 * fSlow70;
		float fSlow73 = std::sqrt(std::max<float>(0.0f, tibetanbowldsp_faustpower2_f(fSlow72) / tibetanbowldsp_faustpower2_f(fSlow71) + -1.0f));
		float fSlow74 = fSlow72 / fSlow71;
		float fSlow75 = fSlow74 - fSlow73;
		float fSlow76 = std::exp(-(fConst55 / fSlow50)) / fSlow69 + -1.0f;
		float fSlow77 = fSlow69 * (fSlow73 + (1.0f - fSlow74));
		float fSlow78 = std::exp(-(fConst61 / fSlow50));
		float fSlow79 = tibetanbowldsp_faustpower2_f(fSlow78);
		float fSlow80 = 1.0f - fSlow79;
		float fSlow81 = 1.0f - fConst36 * fSlow79;
		float fSlow82 = std::sqrt(std::max<float>(0.0f, tibetanbowldsp_faustpower2_f(fSlow81) / tibetanbowldsp_faustpower2_f(fSlow80) + -1.0f));
		float fSlow83 = fSlow81 / fSlow80;
		float fSlow84 = fSlow83 - fSlow82;
		float fSlow85 = std::exp(-(fConst62 / fSlow50)) / fSlow78 + -1.0f;
		float fSlow86 = fSlow78 * (fSlow82 + (1.0f - fSlow83));
		float fSlow87 = std::exp(-(fConst68 / fSlow50));
		float fSlow88 = tibetanbowldsp_faustpower2_f(fSlow87);
		float fSlow89 = 1.0f - fSlow88;
		float fSlow90 = 1.0f - fConst36 * fSlow88;
		float fSlow91 = std::sqrt(std::max<float>(0.0f, tibetanbowldsp_faustpower2_f(fSlow90) / tibetanbowldsp_faustpower2_f(fSlow89) + -1.0f));
		float fSlow92 = fSlow90 / fSlow89;
		float fSlow93 = fSlow92 - fSlow91;
		float fSlow94 = std::exp(-(fConst69 / fSlow50)) / fSlow87 + -1.0f;
		float fSlow95 = fSlow87 * (fSlow91 + (1.0f - fSlow92));
		int iSlow96 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst72 * (static_cast<float>(fHslider8) / fSlow0))));
		float fSlow97 = std::exp(-(fConst76 / fSlow50));
		float fSlow98 = tibetanbowldsp_faustpower2_f(fSlow97);
		float fSlow99 = 1.0f - fSlow98;
		float fSlow100 = 1.0f - fConst36 * fSlow98;
		float fSlow101 = std::sqrt(std::max<float>(0.0f, tibetanbowldsp_faustpower2_f(fSlow100) / tibetanbowldsp_faustpower2_f(fSlow99) + -1.0f));
		float fSlow102 = fSlow100 / fSlow99;
		float fSlow103 = fSlow102 - fSlow101;
		float fSlow104 = std::exp(-(fConst77 / fSlow50)) / fSlow97 + -1.0f;
		float fSlow105 = fSlow97 * (fSlow101 + (1.0f - fSlow102));
		float fSlow106 = std::exp(-(fConst83 / fSlow50));
		float fSlow107 = tibetanbowldsp_faustpower2_f(fSlow106);
		float fSlow108 = 1.0f - fSlow107;
		float fSlow109 = 1.0f - fConst36 * fSlow107;
		float fSlow110 = std::sqrt(std::max<float>(0.0f, tibetanbowldsp_faustpower2_f(fSlow109) / tibetanbowldsp_faustpower2_f(fSlow108) + -1.0f));
		float fSlow111 = fSlow109 / fSlow108;
		float fSlow112 = fSlow111 - fSlow110;
		float fSlow113 = std::exp(-(fConst84 / fSlow50)) / fSlow106 + -1.0f;
		float fSlow114 = fSlow106 * (fSlow110 + (1.0f - fSlow111));
		float fSlow115 = std::exp(-(fConst90 / fSlow50));
		float fSlow116 = tibetanbowldsp_faustpower2_f(fSlow115);
		float fSlow117 = 1.0f - fSlow116;
		float fSlow118 = 1.0f - fConst36 * fSlow116;
		float fSlow119 = std::sqrt(std::max<float>(0.0f, tibetanbowldsp_faustpower2_f(fSlow118) / tibetanbowldsp_faustpower2_f(fSlow117) + -1.0f));
		float fSlow120 = fSlow118 / fSlow117;
		float fSlow121 = fSlow120 - fSlow119;
		float fSlow122 = std::exp(-(fConst91 / fSlow50)) / fSlow115 + -1.0f;
		float fSlow123 = fSlow115 * (fSlow119 + (1.0f - fSlow120));
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iVec0[0] = 1;
			fVec1[0] = fSlow4;
			iRec14[0] = iSlow5 * (iRec14[1] + 1);
			fRec15[0] = fSlow4 + fRec15[1] * static_cast<float>(fVec1[1] >= fSlow4);
			float fTemp0 = fSlow6 * std::max<float>(0.0f, std::min<float>(fConst7 * fRec15[0], 1.0f) * (1.0f - fConst5 * static_cast<float>(iRec14[0]))) - fSlow3 * (fRec0[1] + fRec2[1] + fRec4[1] + fRec6[1] + fRec8[1] + fRec10[1] + fRec1[1] + fRec3[1] + fRec5[1] + fRec7[1] + fRec9[1] + fRec11[1]) - fSlow2;
			float fTemp1 = std::pow(std::fabs(fSlow7 * fTemp0) + 0.75f, -4.0f);
			float fTemp2 = fSlow9 * fTemp0 * (static_cast<float>(fTemp1 > 1.0f) + fTemp1 * static_cast<float>(fTemp1 <= 1.0f));
			fVec2[IOTA0 & 8191] = fSlow10 + (fRec12[1] - fTemp2);
			fRec13[0] = 0.999926f * fVec2[(IOTA0 - iSlow11) & 8191] - (fSlow1 * fRec13[1] + fConst2 * fRec13[2]);
			fRec12[0] = fConst9 * (fRec13[0] - fRec13[2]);
			fRec0[0] = fRec12[0];
			fVec3[IOTA0 & 8191] = fSlow10 + (fRec16[1] - fTemp2);
			fRec17[0] = 0.999926f * fVec3[(IOTA0 - iSlow13) & 8191] - (fSlow12 * fRec17[1] + fConst2 * fRec17[2]);
			fRec16[0] = fConst9 * (fRec17[0] - fRec17[2]);
			fRec1[0] = fRec16[0];
			fVec4[IOTA0 & 4095] = fSlow15 + (fRec18[1] - fTemp2);
			fRec19[0] = 0.9999828f * fVec4[(IOTA0 - iSlow16) & 4095] - (fSlow14 * fRec19[1] + fConst2 * fRec19[2]);
			fRec18[0] = fConst9 * (fRec19[0] - fRec19[2]);
			fRec2[0] = fRec18[0];
			fVec5[IOTA0 & 4095] = fSlow15 + (fRec20[1] - fTemp2);
			fRec21[0] = 0.9999828f * fVec5[(IOTA0 - iSlow18) & 4095] - (fSlow17 * fRec21[1] + fConst2 * fRec21[2]);
			fRec20[0] = fConst9 * (fRec21[0] - fRec21[2]);
			fRec3[0] = fRec20[0];
			fVec6[IOTA0 & 2047] = fSlow20 + (fRec22[1] - fTemp2);
			fRec23[0] = fVec6[(IOTA0 - iSlow21) & 2047] - (fSlow19 * fRec23[1] + fConst2 * fRec23[2]);
			fRec22[0] = fConst9 * (fRec23[0] - fRec23[2]);
			fRec4[0] = fRec22[0];
			fRec5[0] = fRec22[0];
			fVec7[IOTA0 & 2047] = fSlow23 + (fRec24[1] - fTemp2);
			fRec25[0] = fVec7[(IOTA0 - iSlow24) & 2047] - (fSlow22 * fRec25[1] + fConst2 * fRec25[2]);
			fRec24[0] = fConst9 * (fRec25[0] - fRec25[2]);
			fRec6[0] = fRec24[0];
			fVec8[IOTA0 & 2047] = fSlow23 + (fRec26[1] - fTemp2);
			fRec27[0] = fVec8[(IOTA0 - iSlow26) & 2047] - (fSlow25 * fRec27[1] + fConst2 * fRec27[2]);
			fRec26[0] = fConst9 * (fRec27[0] - fRec27[2]);
			fRec7[0] = fRec26[0];
			fVec9[IOTA0 & 1023] = fSlow28 + (fRec28[1] - fTemp2);
			fRec29[0] = 0.9999655f * fVec9[(IOTA0 - iSlow29) & 1023] - (fSlow27 * fRec29[1] + fConst2 * fRec29[2]);
			fRec28[0] = fConst9 * (fRec29[0] - fRec29[2]);
			fRec8[0] = fRec28[0];
			fVec10[IOTA0 & 1023] = fSlow28 + (fRec30[1] - fTemp2);
			fRec31[0] = 0.9999655f * fVec10[(IOTA0 - iSlow31) & 1023] - (fSlow30 * fRec31[1] + fConst2 * fRec31[2]);
			fRec30[0] = fConst9 * (fRec31[0] - fRec31[2]);
			fRec9[0] = fRec30[0];
			fVec11[IOTA0 & 1023] = fSlow33 + (fRec32[1] - fTemp2);
			fRec33[0] = fVec11[(IOTA0 - iSlow34) & 1023] - (fSlow32 * fRec33[1] + fConst2 * fRec33[2]);
			fRec32[0] = fConst9 * (fRec33[0] - fRec33[2]);
			fRec10[0] = fRec32[0];
			fVec12[IOTA0 & 511] = fSlow33 + (fRec34[1] - fTemp2);
			fRec35[0] = fVec12[(IOTA0 - iSlow36) & 511] - (fSlow35 * fRec35[1] + fConst2 * fRec35[2]);
			fRec34[0] = fConst9 * (fRec35[0] - fRec35[2]);
			fRec11[0] = fRec34[0];
			float fTemp3 = fRec11[0] + fRec9[0] + fRec7[0] + fRec5[0] + fRec3[0] + fRec0[0] + fRec2[0] + fRec4[0] + fRec6[0] + fRec8[0] + fRec10[0] + fRec1[0];
			fVec13[0] = fTemp3;
			fRec36[0] = fSlow37 + fConst31 * fRec36[1];
			float fTemp4 = fRec36[0] * (fSlow41 * fTemp3 + fSlow40 * (fTemp3 + fVec13[1]) + fSlow39 * tibetanbowldsp_faustpower2_f(fTemp3));
			float fTemp5 = std::cos(fTemp4);
			float fTemp6 = std::sin(fTemp4);
			float fTemp7 = fTemp3 * fTemp5 - fTemp6 * fRec37[1];
			float fTemp8 = fTemp5 * fTemp7 - fTemp6 * fRec38[1];
			float fTemp9 = fTemp5 * fTemp8 - fTemp6 * fRec39[1];
			float fTemp10 = fTemp5 * fTemp9 - fTemp6 * fRec40[1];
			float fTemp11 = fTemp5 * fTemp10 - fTemp6 * fRec41[1];
			fRec42[0] = fTemp5 * fTemp11 - fTemp6 * fRec42[1];
			fRec41[0] = fTemp6 * fTemp11 + fTemp5 * fRec42[1];
			fRec40[0] = fTemp6 * fTemp10 + fTemp5 * fRec41[1];
			fRec39[0] = fTemp6 * fTemp9 + fTemp5 * fRec40[1];
			fRec38[0] = fTemp6 * fTemp8 + fTemp5 * fRec39[1];
			fRec37[0] = fTemp6 * fTemp7 + fTemp5 * fRec38[1];
			fRec45[0] = fSlow43 + fConst31 * fRec45[1];
			float fTemp12 = ((1 - iVec0[1]) ? 0.0f : fRec44[1] + fConst32 * (fSlow45 + fSlow44 * fRec45[0]));
			fRec44[0] = fTemp12 - std::floor(fTemp12);
			float fTemp13 = 3.1415927f * fRec36[0] * ftbl0tibetanbowldspSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec44[0]), 65535))];
			float fTemp14 = std::cos(fTemp13);
			float fTemp15 = std::sin(fTemp13);
			float fTemp16 = fTemp3 * fTemp14 - fTemp15 * fRec46[1];
			float fTemp17 = fTemp14 * fTemp16 - fTemp15 * fRec47[1];
			float fTemp18 = fTemp14 * fTemp17 - fTemp15 * fRec48[1];
			float fTemp19 = fTemp14 * fTemp18 - fTemp15 * fRec49[1];
			float fTemp20 = fTemp14 * fTemp19 - fTemp15 * fRec50[1];
			fRec51[0] = fTemp14 * fTemp20 - fTemp15 * fRec51[1];
			fRec50[0] = fTemp15 * fTemp20 + fTemp14 * fRec51[1];
			fRec49[0] = fTemp15 * fTemp19 + fTemp14 * fRec50[1];
			fRec48[0] = fTemp15 * fTemp18 + fTemp14 * fRec49[1];
			fRec47[0] = fTemp15 * fTemp17 + fTemp14 * fRec48[1];
			fRec46[0] = fTemp15 * fTemp16 + fTemp14 * fRec47[1];
			float fTemp21 = fSlow46 * (fTemp3 * fTemp15 + fRec46[1] * fTemp14) + fSlow42 * (fRec36[0] * (fTemp3 * fTemp6 + fRec37[1] * fTemp5) + (1.0f - fRec36[0]) * fTemp3);
			fVec15[IOTA0 & 8191] = fTemp21;
			fRec52[0] = fSlow47 + fConst31 * fRec52[1];
			float fTemp22 = 1.0f - fRec52[0];
			fRec64[0] = -(fConst39 * (fConst38 * fRec64[1] - (fRec57[1] + fRec57[2])));
			fRec63[0] = fSlow59 * (fRec57[1] + fSlow58 * fRec64[0]) + fSlow57 * fRec63[1];
			fVec16[IOTA0 & 16383] = 0.35355338f * fRec63[0] + 1e-20f;
			fVec17[IOTA0 & 4095] = fSlow49 * fRec52[0] * fTemp21;
			float fTemp23 = 0.3f * fVec17[(IOTA0 - iConst43) & 4095];
			float fTemp24 = fTemp23 + fVec16[(IOTA0 - iConst42) & 16383] - 0.6f * fRec61[1];
			fVec18[IOTA0 & 4095] = fTemp24;
			fRec61[0] = fVec18[(IOTA0 - iConst44) & 4095];
			float fRec62 = 0.6f * fTemp24;
			fRec68[0] = -(fConst39 * (fConst38 * fRec68[1] - (fRec53[1] + fRec53[2])));
			fRec67[0] = fSlow68 * (fRec53[1] + fSlow67 * fRec68[0]) + fSlow66 * fRec67[1];
			fVec19[IOTA0 & 16383] = 0.35355338f * fRec67[0] + 1e-20f;
			float fTemp25 = fVec19[(IOTA0 - iConst50) & 16383] + fTemp23 - 0.6f * fRec65[1];
			fVec20[IOTA0 & 2047] = fTemp25;
			fRec65[0] = fVec20[(IOTA0 - iConst51) & 2047];
			float fRec66 = 0.6f * fTemp25;
			float fTemp26 = fRec66 + fRec62;
			fRec72[0] = -(fConst39 * (fConst38 * fRec72[1] - (fRec55[1] + fRec55[2])));
			fRec71[0] = fSlow77 * (fRec55[1] + fSlow76 * fRec72[0]) + fSlow75 * fRec71[1];
			fVec21[IOTA0 & 16383] = 0.35355338f * fRec71[0] + 1e-20f;
			float fTemp27 = fVec21[(IOTA0 - iConst57) & 16383] - (fTemp23 + 0.6f * fRec69[1]);
			fVec22[IOTA0 & 4095] = fTemp27;
			fRec69[0] = fVec22[(IOTA0 - iConst58) & 4095];
			float fRec70 = 0.6f * fTemp27;
			fRec76[0] = -(fConst39 * (fConst38 * fRec76[1] - (fRec59[1] + fRec59[2])));
			fRec75[0] = fSlow86 * (fRec59[1] + fSlow85 * fRec76[0]) + fSlow84 * fRec75[1];
			fVec23[IOTA0 & 16383] = 0.35355338f * fRec75[0] + 1e-20f;
			float fTemp28 = fVec23[(IOTA0 - iConst64) & 16383] - (fTemp23 + 0.6f * fRec73[1]);
			fVec24[IOTA0 & 2047] = fTemp28;
			fRec73[0] = fVec24[(IOTA0 - iConst65) & 2047];
			float fRec74 = 0.6f * fTemp28;
			float fTemp29 = fRec74 + fRec70 + fTemp26;
			fRec80[0] = -(fConst39 * (fConst38 * fRec80[1] - (fRec54[1] + fRec54[2])));
			fRec79[0] = fSlow95 * (fRec54[1] + fSlow94 * fRec80[0]) + fSlow93 * fRec79[1];
			fVec25[IOTA0 & 32767] = 0.35355338f * fRec79[0] + 1e-20f;
			float fTemp30 = fVec15[(IOTA0 - iSlow96) & 8191];
			fVec26[IOTA0 & 4095] = fSlow48 * fRec52[0] * fTemp30;
			float fTemp31 = 0.3f * fVec26[(IOTA0 - iConst43) & 4095];
			float fTemp32 = fTemp31 + 0.6f * fRec77[1] + fVec25[(IOTA0 - iConst71) & 32767];
			fVec27[IOTA0 & 4095] = fTemp32;
			fRec77[0] = fVec27[(IOTA0 - iConst73) & 4095];
			float fRec78 = -(0.6f * fTemp32);
			fRec84[0] = -(fConst39 * (fConst38 * fRec84[1] - (fRec58[1] + fRec58[2])));
			fRec83[0] = fSlow105 * (fRec58[1] + fSlow104 * fRec84[0]) + fSlow103 * fRec83[1];
			fVec28[IOTA0 & 16383] = 0.35355338f * fRec83[0] + 1e-20f;
			float fTemp33 = fVec28[(IOTA0 - iConst79) & 16383] + fTemp31 + 0.6f * fRec81[1];
			fVec29[IOTA0 & 4095] = fTemp33;
			fRec81[0] = fVec29[(IOTA0 - iConst80) & 4095];
			float fRec82 = -(0.6f * fTemp33);
			fRec88[0] = -(fConst39 * (fConst38 * fRec88[1] - (fRec56[1] + fRec56[2])));
			fRec87[0] = fSlow114 * (fRec56[1] + fSlow113 * fRec88[0]) + fSlow112 * fRec87[1];
			fVec30[IOTA0 & 32767] = 0.35355338f * fRec87[0] + 1e-20f;
			float fTemp34 = 0.6f * fRec85[1] + fVec30[(IOTA0 - iConst86) & 32767];
			fVec31[IOTA0 & 4095] = fTemp34 - fTemp31;
			fRec85[0] = fVec31[(IOTA0 - iConst87) & 4095];
			float fRec86 = 0.6f * (fTemp31 - fTemp34);
			fRec92[0] = -(fConst39 * (fConst38 * fRec92[1] - (fRec60[1] + fRec60[2])));
			fRec91[0] = fSlow123 * (fRec60[1] + fSlow122 * fRec92[0]) + fSlow121 * fRec91[1];
			fVec32[IOTA0 & 32767] = 0.35355338f * fRec91[0] + 1e-20f;
			float fTemp35 = 0.6f * fRec89[1] + fVec32[(IOTA0 - iConst93) & 32767];
			fVec33[IOTA0 & 2047] = fTemp35 - fTemp31;
			fRec89[0] = fVec33[(IOTA0 - iConst94) & 2047];
			float fRec90 = 0.6f * (fTemp31 - fTemp35);
			fRec53[0] = fRec89[1] + fRec85[1] + fRec81[1] + fRec77[1] + fRec73[1] + fRec69[1] + fRec61[1] + fRec65[1] + fRec90 + fRec86 + fRec82 + fRec78 + fTemp29;
			fRec54[0] = fRec73[1] + fRec69[1] + fRec61[1] + fRec65[1] + fTemp29 - (fRec89[1] + fRec85[1] + fRec81[1] + fRec77[1] + fRec90 + fRec86 + fRec78 + fRec82);
			float fTemp36 = fRec70 + fRec74;
			fRec55[0] = fRec81[1] + fRec77[1] + fRec61[1] + fRec65[1] + fRec82 + fRec78 + fTemp26 - (fRec89[1] + fRec85[1] + fRec73[1] + fRec69[1] + fRec90 + fRec86 + fTemp36);
			fRec56[0] = fRec89[1] + fRec85[1] + fRec61[1] + fRec65[1] + fRec90 + fRec86 + fTemp26 - (fRec81[1] + fRec77[1] + fRec73[1] + fRec69[1] + fRec82 + fRec78 + fTemp36);
			float fTemp37 = fRec62 + fRec74;
			float fTemp38 = fRec66 + fRec70;
			fRec57[0] = fRec85[1] + fRec77[1] + fRec69[1] + fRec65[1] + fRec86 + fRec78 + fTemp38 - (fRec89[1] + fRec81[1] + fRec73[1] + fRec61[1] + fRec90 + fRec82 + fTemp37);
			fRec58[0] = fRec89[1] + fRec81[1] + fRec69[1] + fRec65[1] + fRec90 + fRec82 + fTemp38 - (fRec85[1] + fRec77[1] + fRec73[1] + fRec61[1] + fRec86 + fRec78 + fTemp37);
			float fTemp39 = fRec62 + fRec70;
			float fTemp40 = fRec66 + fRec74;
			fRec59[0] = fRec89[1] + fRec77[1] + fRec73[1] + fRec65[1] + fRec90 + fRec78 + fTemp40 - (fRec85[1] + fRec81[1] + fRec69[1] + fRec61[1] + fRec86 + fRec82 + fTemp39);
			fRec60[0] = fRec85[1] + fRec81[1] + fRec73[1] + fRec65[1] + fRec86 + fRec82 + fTemp40 - (fRec89[1] + fRec77[1] + fRec69[1] + fRec61[1] + fRec90 + fRec78 + fTemp39);
			output0[i0] = static_cast<FAUSTFLOAT>(0.37f * (fRec54[0] + fRec55[0]) + fSlow49 * fTemp22 * fTemp21);
			output1[i0] = static_cast<FAUSTFLOAT>(0.37f * (fRec54[0] - fRec55[0]) + fSlow48 * fTemp22 * fTemp30);
			iVec0[1] = iVec0[0];
			fVec1[1] = fVec1[0];
			iRec14[1] = iRec14[0];
			fRec15[1] = fRec15[0];
			IOTA0 = IOTA0 + 1;
			fRec13[2] = fRec13[1];
			fRec13[1] = fRec13[0];
			fRec12[1] = fRec12[0];
			fRec0[1] = fRec0[0];
			fRec17[2] = fRec17[1];
			fRec17[1] = fRec17[0];
			fRec16[1] = fRec16[0];
			fRec1[1] = fRec1[0];
			fRec19[2] = fRec19[1];
			fRec19[1] = fRec19[0];
			fRec18[1] = fRec18[0];
			fRec2[1] = fRec2[0];
			fRec21[2] = fRec21[1];
			fRec21[1] = fRec21[0];
			fRec20[1] = fRec20[0];
			fRec3[1] = fRec3[0];
			fRec23[2] = fRec23[1];
			fRec23[1] = fRec23[0];
			fRec22[1] = fRec22[0];
			fRec4[1] = fRec4[0];
			fRec5[1] = fRec5[0];
			fRec25[2] = fRec25[1];
			fRec25[1] = fRec25[0];
			fRec24[1] = fRec24[0];
			fRec6[1] = fRec6[0];
			fRec27[2] = fRec27[1];
			fRec27[1] = fRec27[0];
			fRec26[1] = fRec26[0];
			fRec7[1] = fRec7[0];
			fRec29[2] = fRec29[1];
			fRec29[1] = fRec29[0];
			fRec28[1] = fRec28[0];
			fRec8[1] = fRec8[0];
			fRec31[2] = fRec31[1];
			fRec31[1] = fRec31[0];
			fRec30[1] = fRec30[0];
			fRec9[1] = fRec9[0];
			fRec33[2] = fRec33[1];
			fRec33[1] = fRec33[0];
			fRec32[1] = fRec32[0];
			fRec10[1] = fRec10[0];
			fRec35[2] = fRec35[1];
			fRec35[1] = fRec35[0];
			fRec34[1] = fRec34[0];
			fRec11[1] = fRec11[0];
			fVec13[1] = fVec13[0];
			fRec36[1] = fRec36[0];
			fRec42[1] = fRec42[0];
			fRec41[1] = fRec41[0];
			fRec40[1] = fRec40[0];
			fRec39[1] = fRec39[0];
			fRec38[1] = fRec38[0];
			fRec37[1] = fRec37[0];
			fRec45[1] = fRec45[0];
			fRec44[1] = fRec44[0];
			fRec51[1] = fRec51[0];
			fRec50[1] = fRec50[0];
			fRec49[1] = fRec49[0];
			fRec48[1] = fRec48[0];
			fRec47[1] = fRec47[0];
			fRec46[1] = fRec46[0];
			fRec52[1] = fRec52[0];
			fRec64[1] = fRec64[0];
			fRec63[1] = fRec63[0];
			fRec61[1] = fRec61[0];
			fRec68[1] = fRec68[0];
			fRec67[1] = fRec67[0];
			fRec65[1] = fRec65[0];
			fRec72[1] = fRec72[0];
			fRec71[1] = fRec71[0];
			fRec69[1] = fRec69[0];
			fRec76[1] = fRec76[0];
			fRec75[1] = fRec75[0];
			fRec73[1] = fRec73[0];
			fRec80[1] = fRec80[0];
			fRec79[1] = fRec79[0];
			fRec77[1] = fRec77[0];
			fRec84[1] = fRec84[0];
			fRec83[1] = fRec83[0];
			fRec81[1] = fRec81[0];
			fRec88[1] = fRec88[0];
			fRec87[1] = fRec87[0];
			fRec85[1] = fRec85[0];
			fRec92[1] = fRec92[0];
			fRec91[1] = fRec91[0];
			fRec89[1] = fRec89[0];
			fRec53[2] = fRec53[1];
			fRec53[1] = fRec53[0];
			fRec54[2] = fRec54[1];
			fRec54[1] = fRec54[0];
			fRec55[2] = fRec55[1];
			fRec55[1] = fRec55[0];
			fRec56[2] = fRec56[1];
			fRec56[1] = fRec56[0];
			fRec57[2] = fRec57[1];
			fRec57[1] = fRec57[0];
			fRec58[2] = fRec58[1];
			fRec58[1] = fRec58[0];
			fRec59[2] = fRec59[1];
			fRec59[1] = fRec59[0];
			fRec60[2] = fRec60[1];
			fRec60[1] = fRec60[0];
		}
	}

};

#endif

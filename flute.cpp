/* ------------------------------------------------------------
author: "Romain Michon (rmichon@ccrma.stanford.edu)"
copyright: "Romain Michon"
name: "flute"
version: "1.0"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
Compilation options: -lang cpp -fpga-mem-th 4 -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __mydsp_H__
#define  __mydsp_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS mydsp
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

class mydspSIG0 {
	
  private:
	
	int iVec3[2];
	int iRec12[2];
	int fSampleRate;
	
  public:
	
	int getNumInputsmydspSIG0() {
		return 0;
	}
	int getNumOutputsmydspSIG0() {
		return 1;
	}
	
	void instanceInitmydspSIG0(int sample_rate) {
		fSampleRate = sample_rate;
		for (int l13 = 0; l13 < 2; l13 = l13 + 1) {
			iVec3[l13] = 0;
		}
		for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
			iRec12[l14] = 0;
		}
	}
	
	void fillmydspSIG0(int count, float* table) {
		for (int i1 = 0; i1 < count; i1 = i1 + 1) {
			iVec3[0] = 1;
			iRec12[0] = (iVec3[1] + iRec12[1]) % 65536;
			table[i1] = std::sin(9.58738e-05f * static_cast<float>(iRec12[0]));
			iVec3[1] = iVec3[0];
			iRec12[1] = iRec12[0];
		}
	}

};

static mydspSIG0* newmydspSIG0() { return (mydspSIG0*)new mydspSIG0(); }
static void deletemydspSIG0(mydspSIG0* dsp) { delete dsp; }

static float mydsp_faustpower2_f(float value) {
	return value * value;
}
static float ftbl0mydspSIG0[65536];
static float mydsp_faustpower3_f(float value) {
	return value * value * value;
}

class mydsp : public dsp {
	
 private:
	
	int iVec0[2];
	FAUSTFLOAT fButton0;
	int iVec1[2];
	int iRec0[2];
	FAUSTFLOAT fHslider0;
	int fSampleRate;
	float fConst0;
	int iRec1[2];
	FAUSTFLOAT fHslider1;
	float fConst1;
	float fConst2;
	FAUSTFLOAT fHslider2;
	float fRec2[2];
	int IOTA0;
	FAUSTFLOAT fEntry0;
	float fVec2[2];
	FAUSTFLOAT fHslider3;
	float fRec5[2];
	FAUSTFLOAT fEntry1;
	float fConst3;
	FAUSTFLOAT fHslider4;
	float fRec11[2];
	float fRec10[2];
	float fRec9[2];
	float fRec8[2];
	float fRec7[2];
	float fRec6[2];
	FAUSTFLOAT fHslider5;
	float fRec14[2];
	float fConst4;
	float fRec13[2];
	float fRec20[2];
	float fRec19[2];
	float fRec18[2];
	float fRec17[2];
	float fRec16[2];
	float fRec15[2];
	FAUSTFLOAT fHslider6;
	float fRec21[2];
	int iRec22[2];
	int iRec23[2];
	FAUSTFLOAT fHslider7;
	FAUSTFLOAT fHslider8;
	FAUSTFLOAT fHslider9;
	float fRec24[2];
	FAUSTFLOAT fHslider10;
	int iRec25[2];
	FAUSTFLOAT fHslider11;
	FAUSTFLOAT fCheckbox0;
	int iVec4[2];
	int iRec26[2];
	FAUSTFLOAT fHslider12;
	int iRec27[2];
	FAUSTFLOAT fHslider13;
	FAUSTFLOAT fHslider14;
	FAUSTFLOAT fHslider15;
	float fRec28[2];
	float fVec5[8192];
	float fConst5;
	float fVec6[2];
	float fConst6;
	float fConst7;
	float fConst8;
	float fRec4[2];
	float fRec3[8192];
	FAUSTFLOAT fHslider16;
	FAUSTFLOAT fEntry2;
	FAUSTFLOAT fHslider17;
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
	float fVec7[16384];
	float fConst17;
	int iConst18;
	float fVec8[4096];
	int iConst19;
	float fVec9[4096];
	int iConst20;
	float fRec37[2];
	float fConst21;
	float fConst22;
	float fConst23;
	float fRec44[2];
	float fConst24;
	float fRec43[2];
	float fVec10[16384];
	float fConst25;
	int iConst26;
	float fVec11[2048];
	int iConst27;
	float fRec41[2];
	float fConst28;
	float fConst29;
	float fConst30;
	float fRec48[2];
	float fConst31;
	float fRec47[2];
	float fVec12[16384];
	float fConst32;
	int iConst33;
	float fVec13[4096];
	int iConst34;
	float fRec45[2];
	float fConst35;
	float fConst36;
	float fConst37;
	float fRec52[2];
	float fConst38;
	float fRec51[2];
	float fVec14[16384];
	float fConst39;
	int iConst40;
	float fVec15[2048];
	int iConst41;
	float fRec49[2];
	float fConst42;
	float fConst43;
	float fConst44;
	float fRec56[2];
	float fConst45;
	float fRec55[2];
	float fVec16[32768];
	float fConst46;
	int iConst47;
	float fVec17[8192];
	FAUSTFLOAT fHslider18;
	float fVec18[4096];
	float fVec19[4096];
	int iConst48;
	float fRec53[2];
	float fConst49;
	float fConst50;
	float fConst51;
	float fRec60[2];
	float fConst52;
	float fRec59[2];
	float fVec20[16384];
	float fConst53;
	int iConst54;
	float fVec21[4096];
	int iConst55;
	float fRec57[2];
	float fConst56;
	float fConst57;
	float fConst58;
	float fRec64[2];
	float fConst59;
	float fRec63[2];
	float fVec22[32768];
	float fConst60;
	int iConst61;
	float fVec23[4096];
	int iConst62;
	float fRec61[2];
	float fConst63;
	float fConst64;
	float fConst65;
	float fRec68[2];
	float fConst66;
	float fRec67[2];
	float fVec24[32768];
	float fConst67;
	int iConst68;
	float fVec25[2048];
	int iConst69;
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
	mydsp() {
	}
	
	mydsp(const mydsp&) = default;
	
	virtual ~mydsp() = default;
	
	mydsp& operator=(const mydsp&) = default;
	
	void metadata(Meta* m) { 
		m->declare("author", "Romain Michon (rmichon@ccrma.stanford.edu)");
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "1.22.0");
		m->declare("compile_options", "-lang cpp -fpga-mem-th 4 -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("copyright", "Romain Michon");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.2.0");
		m->declare("description", "Nonlinear WaveGuide Flute");
		m->declare("envelopes.lib/adsr:author", "Yann Orlarey and Andrey Bundin");
		m->declare("envelopes.lib/asr:author", "Yann Orlarey, Stéphane Letz");
		m->declare("envelopes.lib/author", "GRAME");
		m->declare("envelopes.lib/copyright", "GRAME");
		m->declare("envelopes.lib/license", "LGPL with exception");
		m->declare("envelopes.lib/name", "Faust Envelope Library");
		m->declare("envelopes.lib/version", "1.3.0");
		m->declare("filename", "flute.dsp");
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
		m->declare("name", "flute");
		m->declare("noises.lib/name", "Faust Noise Generator Library");
		m->declare("noises.lib/version", "1.5.0");
		m->declare("oscillators.lib/name", "Faust Oscillator Library");
		m->declare("oscillators.lib/version", "1.7.0");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("reverbs.lib/name", "Faust Reverb Library");
		m->declare("reverbs.lib/version", "1.5.1");
		m->declare("routes.lib/hadamard:author", "Remy Muller, revised by Romain Michon");
		m->declare("routes.lib/name", "Faust Signal Routing Library");
		m->declare("routes.lib/version", "1.3.0");
		m->declare("signals.lib/name", "Faust Routing Library");
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
		mydspSIG0* sig0 = newmydspSIG0();
		sig0->instanceInitmydspSIG0(sample_rate);
		sig0->fillmydspSIG0(65536, ftbl0mydspSIG0);
		deletemydspSIG0(sig0);
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, static_cast<float>(fSampleRate)));
		fConst1 = 44.1f / fConst0;
		fConst2 = 1.0f - fConst1;
		fConst3 = 1.0f / std::max<float>(1.0f, 0.1f * fConst0);
		fConst4 = 1.0f / fConst0;
		fConst5 = 0.5f * fConst0;
		fConst6 = 1.0f / std::tan(6283.1855f / fConst0);
		fConst7 = 1.0f - fConst6;
		fConst8 = 1.0f / (fConst6 + 1.0f);
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
		iConst48 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst46 + -1.0f)));
		fConst49 = std::floor(0.192303f * fConst0 + 0.5f);
		fConst50 = fConst49 / fConst0;
		fConst51 = 3.4538777f * fConst50;
		fConst52 = 2.3025851f * fConst50;
		fConst53 = std::floor(0.029291f * fConst0 + 0.5f);
		iConst54 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst49 - fConst53)));
		iConst55 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst53 + -1.0f)));
		fConst56 = std::floor(0.256891f * fConst0 + 0.5f);
		fConst57 = fConst56 / fConst0;
		fConst58 = 3.4538777f * fConst57;
		fConst59 = 2.3025851f * fConst57;
		fConst60 = std::floor(0.027333f * fConst0 + 0.5f);
		iConst61 = static_cast<int>(std::min<float>(16384.0f, std::max<float>(0.0f, fConst56 - fConst60)));
		iConst62 = static_cast<int>(std::min<float>(2048.0f, std::max<float>(0.0f, fConst60 + -1.0f)));
		fConst63 = std::floor(0.219991f * fConst0 + 0.5f);
		fConst64 = fConst63 / fConst0;
		fConst65 = 3.4538777f * fConst64;
		fConst66 = 2.3025851f * fConst64;
		fConst67 = std::floor(0.019123f * fConst0 + 0.5f);
		iConst68 = static_cast<int>(std::min<float>(16384.0f, std::max<float>(0.0f, fConst63 - fConst67)));
		iConst69 = static_cast<int>(std::min<float>(1024.0f, std::max<float>(0.0f, fConst67 + -1.0f)));
	}
	
	virtual void instanceResetUserInterface() {
		fButton0 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider0 = static_cast<FAUSTFLOAT>(0.1f);
		fHslider1 = static_cast<FAUSTFLOAT>(0.1f);
		fHslider2 = static_cast<FAUSTFLOAT>(0.137f);
		fEntry0 = static_cast<FAUSTFLOAT>(4.4e+02f);
		fHslider3 = static_cast<FAUSTFLOAT>(0.0f);
		fEntry1 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider4 = static_cast<FAUSTFLOAT>(0.1f);
		fHslider5 = static_cast<FAUSTFLOAT>(2.2e+02f);
		fHslider6 = static_cast<FAUSTFLOAT>(5.0f);
		fHslider7 = static_cast<FAUSTFLOAT>(0.2f);
		fHslider8 = static_cast<FAUSTFLOAT>(0.1f);
		fHslider9 = static_cast<FAUSTFLOAT>(0.5f);
		fHslider10 = static_cast<FAUSTFLOAT>(0.1f);
		fHslider11 = static_cast<FAUSTFLOAT>(0.1f);
		fCheckbox0 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider12 = static_cast<FAUSTFLOAT>(1.0f);
		fHslider13 = static_cast<FAUSTFLOAT>(0.05f);
		fHslider14 = static_cast<FAUSTFLOAT>(0.2f);
		fHslider15 = static_cast<FAUSTFLOAT>(0.9f);
		fHslider16 = static_cast<FAUSTFLOAT>(0.6f);
		fEntry2 = static_cast<FAUSTFLOAT>(1.0f);
		fHslider17 = static_cast<FAUSTFLOAT>(0.72f);
		fHslider18 = static_cast<FAUSTFLOAT>(0.5f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iVec0[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			iVec1[l1] = 0;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			iRec0[l2] = 0;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			iRec1[l3] = 0;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fRec2[l4] = 0.0f;
		}
		IOTA0 = 0;
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fVec2[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fRec5[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			fRec11[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
			fRec10[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
			fRec9[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			fRec8[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fRec7[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			fRec6[l12] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fRec14[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 2; l16 = l16 + 1) {
			fRec13[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2; l17 = l17 + 1) {
			fRec20[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec19[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 2; l19 = l19 + 1) {
			fRec18[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 2; l20 = l20 + 1) {
			fRec17[l20] = 0.0f;
		}
		for (int l21 = 0; l21 < 2; l21 = l21 + 1) {
			fRec16[l21] = 0.0f;
		}
		for (int l22 = 0; l22 < 2; l22 = l22 + 1) {
			fRec15[l22] = 0.0f;
		}
		for (int l23 = 0; l23 < 2; l23 = l23 + 1) {
			fRec21[l23] = 0.0f;
		}
		for (int l24 = 0; l24 < 2; l24 = l24 + 1) {
			iRec22[l24] = 0;
		}
		for (int l25 = 0; l25 < 2; l25 = l25 + 1) {
			iRec23[l25] = 0;
		}
		for (int l26 = 0; l26 < 2; l26 = l26 + 1) {
			fRec24[l26] = 0.0f;
		}
		for (int l27 = 0; l27 < 2; l27 = l27 + 1) {
			iRec25[l27] = 0;
		}
		for (int l28 = 0; l28 < 2; l28 = l28 + 1) {
			iVec4[l28] = 0;
		}
		for (int l29 = 0; l29 < 2; l29 = l29 + 1) {
			iRec26[l29] = 0;
		}
		for (int l30 = 0; l30 < 2; l30 = l30 + 1) {
			iRec27[l30] = 0;
		}
		for (int l31 = 0; l31 < 2; l31 = l31 + 1) {
			fRec28[l31] = 0.0f;
		}
		for (int l32 = 0; l32 < 8192; l32 = l32 + 1) {
			fVec5[l32] = 0.0f;
		}
		for (int l33 = 0; l33 < 2; l33 = l33 + 1) {
			fVec6[l33] = 0.0f;
		}
		for (int l34 = 0; l34 < 2; l34 = l34 + 1) {
			fRec4[l34] = 0.0f;
		}
		for (int l35 = 0; l35 < 8192; l35 = l35 + 1) {
			fRec3[l35] = 0.0f;
		}
		for (int l36 = 0; l36 < 2; l36 = l36 + 1) {
			fRec40[l36] = 0.0f;
		}
		for (int l37 = 0; l37 < 2; l37 = l37 + 1) {
			fRec39[l37] = 0.0f;
		}
		for (int l38 = 0; l38 < 16384; l38 = l38 + 1) {
			fVec7[l38] = 0.0f;
		}
		for (int l39 = 0; l39 < 4096; l39 = l39 + 1) {
			fVec8[l39] = 0.0f;
		}
		for (int l40 = 0; l40 < 4096; l40 = l40 + 1) {
			fVec9[l40] = 0.0f;
		}
		for (int l41 = 0; l41 < 2; l41 = l41 + 1) {
			fRec37[l41] = 0.0f;
		}
		for (int l42 = 0; l42 < 2; l42 = l42 + 1) {
			fRec44[l42] = 0.0f;
		}
		for (int l43 = 0; l43 < 2; l43 = l43 + 1) {
			fRec43[l43] = 0.0f;
		}
		for (int l44 = 0; l44 < 16384; l44 = l44 + 1) {
			fVec10[l44] = 0.0f;
		}
		for (int l45 = 0; l45 < 2048; l45 = l45 + 1) {
			fVec11[l45] = 0.0f;
		}
		for (int l46 = 0; l46 < 2; l46 = l46 + 1) {
			fRec41[l46] = 0.0f;
		}
		for (int l47 = 0; l47 < 2; l47 = l47 + 1) {
			fRec48[l47] = 0.0f;
		}
		for (int l48 = 0; l48 < 2; l48 = l48 + 1) {
			fRec47[l48] = 0.0f;
		}
		for (int l49 = 0; l49 < 16384; l49 = l49 + 1) {
			fVec12[l49] = 0.0f;
		}
		for (int l50 = 0; l50 < 4096; l50 = l50 + 1) {
			fVec13[l50] = 0.0f;
		}
		for (int l51 = 0; l51 < 2; l51 = l51 + 1) {
			fRec45[l51] = 0.0f;
		}
		for (int l52 = 0; l52 < 2; l52 = l52 + 1) {
			fRec52[l52] = 0.0f;
		}
		for (int l53 = 0; l53 < 2; l53 = l53 + 1) {
			fRec51[l53] = 0.0f;
		}
		for (int l54 = 0; l54 < 16384; l54 = l54 + 1) {
			fVec14[l54] = 0.0f;
		}
		for (int l55 = 0; l55 < 2048; l55 = l55 + 1) {
			fVec15[l55] = 0.0f;
		}
		for (int l56 = 0; l56 < 2; l56 = l56 + 1) {
			fRec49[l56] = 0.0f;
		}
		for (int l57 = 0; l57 < 2; l57 = l57 + 1) {
			fRec56[l57] = 0.0f;
		}
		for (int l58 = 0; l58 < 2; l58 = l58 + 1) {
			fRec55[l58] = 0.0f;
		}
		for (int l59 = 0; l59 < 32768; l59 = l59 + 1) {
			fVec16[l59] = 0.0f;
		}
		for (int l60 = 0; l60 < 8192; l60 = l60 + 1) {
			fVec17[l60] = 0.0f;
		}
		for (int l61 = 0; l61 < 4096; l61 = l61 + 1) {
			fVec18[l61] = 0.0f;
		}
		for (int l62 = 0; l62 < 4096; l62 = l62 + 1) {
			fVec19[l62] = 0.0f;
		}
		for (int l63 = 0; l63 < 2; l63 = l63 + 1) {
			fRec53[l63] = 0.0f;
		}
		for (int l64 = 0; l64 < 2; l64 = l64 + 1) {
			fRec60[l64] = 0.0f;
		}
		for (int l65 = 0; l65 < 2; l65 = l65 + 1) {
			fRec59[l65] = 0.0f;
		}
		for (int l66 = 0; l66 < 16384; l66 = l66 + 1) {
			fVec20[l66] = 0.0f;
		}
		for (int l67 = 0; l67 < 4096; l67 = l67 + 1) {
			fVec21[l67] = 0.0f;
		}
		for (int l68 = 0; l68 < 2; l68 = l68 + 1) {
			fRec57[l68] = 0.0f;
		}
		for (int l69 = 0; l69 < 2; l69 = l69 + 1) {
			fRec64[l69] = 0.0f;
		}
		for (int l70 = 0; l70 < 2; l70 = l70 + 1) {
			fRec63[l70] = 0.0f;
		}
		for (int l71 = 0; l71 < 32768; l71 = l71 + 1) {
			fVec22[l71] = 0.0f;
		}
		for (int l72 = 0; l72 < 4096; l72 = l72 + 1) {
			fVec23[l72] = 0.0f;
		}
		for (int l73 = 0; l73 < 2; l73 = l73 + 1) {
			fRec61[l73] = 0.0f;
		}
		for (int l74 = 0; l74 < 2; l74 = l74 + 1) {
			fRec68[l74] = 0.0f;
		}
		for (int l75 = 0; l75 < 2; l75 = l75 + 1) {
			fRec67[l75] = 0.0f;
		}
		for (int l76 = 0; l76 < 32768; l76 = l76 + 1) {
			fVec24[l76] = 0.0f;
		}
		for (int l77 = 0; l77 < 2048; l77 = l77 + 1) {
			fVec25[l77] = 0.0f;
		}
		for (int l78 = 0; l78 < 2; l78 = l78 + 1) {
			fRec65[l78] = 0.0f;
		}
		for (int l79 = 0; l79 < 3; l79 = l79 + 1) {
			fRec29[l79] = 0.0f;
		}
		for (int l80 = 0; l80 < 3; l80 = l80 + 1) {
			fRec30[l80] = 0.0f;
		}
		for (int l81 = 0; l81 < 3; l81 = l81 + 1) {
			fRec31[l81] = 0.0f;
		}
		for (int l82 = 0; l82 < 3; l82 = l82 + 1) {
			fRec32[l82] = 0.0f;
		}
		for (int l83 = 0; l83 < 3; l83 = l83 + 1) {
			fRec33[l83] = 0.0f;
		}
		for (int l84 = 0; l84 < 3; l84 = l84 + 1) {
			fRec34[l84] = 0.0f;
		}
		for (int l85 = 0; l85 < 3; l85 = l85 + 1) {
			fRec35[l85] = 0.0f;
		}
		for (int l86 = 0; l86 < 3; l86 = l86 + 1) {
			fRec36[l86] = 0.0f;
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
	
	virtual mydsp* clone() {
		return new mydsp(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("flute");
		ui_interface->openHorizontalBox("Basic_Parameters");
		ui_interface->declare(&fEntry0, "1", "");
		ui_interface->declare(&fEntry0, "tooltip", "Tone frequency");
		ui_interface->declare(&fEntry0, "unit", "Hz");
		ui_interface->addNumEntry("freq", &fEntry0, FAUSTFLOAT(4.4e+02f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(2e+04f), FAUSTFLOAT(1.0f));
		ui_interface->declare(&fEntry2, "1", "");
		ui_interface->declare(&fEntry2, "tooltip", "Gain (value between 0 and 1)");
		ui_interface->addNumEntry("gain", &fEntry2, FAUSTFLOAT(1.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fButton0, "1", "");
		ui_interface->declare(&fButton0, "tooltip", "noteOn = 1, noteOff = 0");
		ui_interface->addButton("gate", &fButton0);
		ui_interface->closeBox();
		ui_interface->openHorizontalBox("Envelopes_and_Vibrato");
		ui_interface->openVerticalBox("Global_Envelope_Parameters");
		ui_interface->declare(&fHslider1, "6", "");
		ui_interface->declare(&fHslider1, "tooltip", "Global envelope attack duration");
		ui_interface->declare(&fHslider1, "unit", "s");
		ui_interface->addHorizontalSlider("Glob_Env_Attack", &fHslider1, FAUSTFLOAT(0.1f), FAUSTFLOAT(0.0f), FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider0, "6", "");
		ui_interface->declare(&fHslider0, "tooltip", "Global envelope release duration");
		ui_interface->declare(&fHslider0, "unit", "s");
		ui_interface->addHorizontalSlider("Glob_Env_Release", &fHslider0, FAUSTFLOAT(0.1f), FAUSTFLOAT(0.0f), FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Pressure_Envelope_Parameters");
		ui_interface->declare(&fHslider13, "5", "");
		ui_interface->declare(&fHslider13, "tooltip", "Pressure envelope attack duration");
		ui_interface->declare(&fHslider13, "unit", "s");
		ui_interface->addHorizontalSlider("Press_Env_Attack", &fHslider13, FAUSTFLOAT(0.05f), FAUSTFLOAT(0.0f), FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider14, "5", "");
		ui_interface->declare(&fHslider14, "tooltip", "Pressure envelope decay duration");
		ui_interface->declare(&fHslider14, "unit", "s");
		ui_interface->addHorizontalSlider("Press_Env_Decay", &fHslider14, FAUSTFLOAT(0.2f), FAUSTFLOAT(0.0f), FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider12, "5", "");
		ui_interface->declare(&fHslider12, "tooltip", "Pressure envelope release duration");
		ui_interface->declare(&fHslider12, "unit", "s");
		ui_interface->addHorizontalSlider("Press_Env_Release", &fHslider12, FAUSTFLOAT(1.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fCheckbox0, "5", "");
		ui_interface->declare(&fCheckbox0, "tooltip", "Activate Pressure envelope");
		ui_interface->declare(&fCheckbox0, "unit", "s");
		ui_interface->addCheckButton("Pressure_Env", &fCheckbox0);
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Vibrato_Parameters");
		ui_interface->declare(&fHslider9, "4", "");
		ui_interface->declare(&fHslider9, "tooltip", "Vibrato attack duration");
		ui_interface->declare(&fHslider9, "unit", "s");
		ui_interface->addHorizontalSlider("Vibrato_Attack", &fHslider9, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider8, "4", "");
		ui_interface->declare(&fHslider8, "tooltip", "Vibrato silence duration before attack");
		ui_interface->declare(&fHslider8, "unit", "s");
		ui_interface->addHorizontalSlider("Vibrato_Begin", &fHslider8, FAUSTFLOAT(0.1f), FAUSTFLOAT(0.0f), FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider6, "4", "");
		ui_interface->declare(&fHslider6, "unit", "Hz");
		ui_interface->addHorizontalSlider("Vibrato_Freq", &fHslider6, FAUSTFLOAT(5.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(15.0f), FAUSTFLOAT(0.1f));
		ui_interface->declare(&fHslider10, "4", "");
		ui_interface->declare(&fHslider10, "tooltip", "A value between 0 and 1");
		ui_interface->addHorizontalSlider("Vibrato_Gain", &fHslider10, FAUSTFLOAT(0.1f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider7, "4", "");
		ui_interface->declare(&fHslider7, "tooltip", "Vibrato release duration");
		ui_interface->declare(&fHslider7, "unit", "s");
		ui_interface->addHorizontalSlider("Vibrato_Release", &fHslider7, FAUSTFLOAT(0.2f), FAUSTFLOAT(0.0f), FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->closeBox();
		ui_interface->openHorizontalBox("Physical_and_Nonlinearity");
		ui_interface->openVerticalBox("Nonlinear_Filter_Parameters");
		ui_interface->declare(&fHslider5, "3", "");
		ui_interface->declare(&fHslider5, "tooltip", "Frequency of the sine wave for the modulation of theta (works if Modulation Type=3)");
		ui_interface->declare(&fHslider5, "unit", "Hz");
		ui_interface->addHorizontalSlider("Modulation_Frequency", &fHslider5, FAUSTFLOAT(2.2e+02f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(1e+03f), FAUSTFLOAT(0.1f));
		ui_interface->declare(&fEntry1, "3", "");
		ui_interface->declare(&fEntry1, "tooltip", "0=theta is modulated by the incoming signal; 1=theta is modulated by the averaged incoming signal; 2=theta is modulated by the squared incoming signal; 3=theta is modulated by a sine wave of frequency freqMod; 4=theta is modulated by a sine wave of frequency freq;");
		ui_interface->addNumEntry("Modulation_Type", &fEntry1, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(4.0f), FAUSTFLOAT(1.0f));
		ui_interface->declare(&fHslider3, "3", "");
		ui_interface->declare(&fHslider3, "tooltip", "Nonlinearity factor (value between 0 and 1)");
		ui_interface->addHorizontalSlider("Nonlinearity", &fHslider3, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider4, "3", "");
		ui_interface->declare(&fHslider4, "Attack duration of the nonlinearity", "");
		ui_interface->declare(&fHslider4, "unit", "s");
		ui_interface->addHorizontalSlider("Nonlinearity Attack", &fHslider4, FAUSTFLOAT(0.1f), FAUSTFLOAT(0.0f), FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Physical_Parameters");
		ui_interface->declare(&fHslider11, "2", "");
		ui_interface->declare(&fHslider11, "tooltip", "Breath noise gain (value between 0 and 1)");
		ui_interface->addHorizontalSlider("Noise Gain", &fHslider11, FAUSTFLOAT(0.1f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->declare(&fHslider15, "2", "");
		ui_interface->declare(&fHslider15, "tooltip", "Breath pressure (value bewteen 0 and 1)");
		ui_interface->addHorizontalSlider("Pressure", &fHslider15, FAUSTFLOAT(0.9f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.5f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Reverb");
		ui_interface->addHorizontalSlider("reverbGain", &fHslider2, FAUSTFLOAT(0.137f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("roomSize", &fHslider17, FAUSTFLOAT(0.72f), FAUSTFLOAT(0.01f), FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->openVerticalBox("Spat");
		ui_interface->addHorizontalSlider("pan angle", &fHslider16, FAUSTFLOAT(0.6f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("spatial width", &fHslider18, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		int iSlow0 = static_cast<int>(static_cast<float>(fButton0));
		int iSlow1 = iSlow0 == 0;
		float fSlow2 = 1.0f / std::max<float>(1.0f, fConst0 * static_cast<float>(fHslider0));
		float fSlow3 = 1.0f / std::max<float>(1.0f, fConst0 * static_cast<float>(fHslider1));
		float fSlow4 = fConst1 * static_cast<float>(fHslider2);
		float fSlow5 = static_cast<float>(fEntry0);
		float fSlow6 = fConst0 / fSlow5;
		float fSlow7 = fSlow6 + -2.0f;
		int iSlow8 = static_cast<int>(fSlow7);
		int iSlow9 = std::min<int>(4097, std::max<int>(0, iSlow8 + 1)) + 1;
		float fSlow10 = std::floor(fSlow7);
		float fSlow11 = fSlow6 + (-2.0f - fSlow10);
		int iSlow12 = std::min<int>(4097, std::max<int>(0, iSlow8)) + 1;
		float fSlow13 = fSlow10 + (3.0f - fSlow6);
		float fSlow14 = fConst1 * static_cast<float>(fHslider3);
		float fSlow15 = static_cast<float>(fEntry1);
		float fSlow16 = 3.1415927f * static_cast<float>(fSlow15 == 2.0f);
		float fSlow17 = 1.5707964f * static_cast<float>(fSlow15 == 1.0f);
		float fSlow18 = 3.1415927f * static_cast<float>(fSlow15 == 0.0f);
		float fSlow19 = 1.0f / std::max<float>(1.0f, fConst0 * static_cast<float>(fHslider4));
		float fSlow20 = static_cast<float>(fSlow15 < 3.0f);
		float fSlow21 = fConst1 * static_cast<float>(fHslider5);
		float fSlow22 = static_cast<float>(fSlow15 != 4.0f);
		float fSlow23 = fSlow5 * static_cast<float>(fSlow15 == 4.0f);
		float fSlow24 = static_cast<float>(fSlow15 >= 3.0f);
		float fSlow25 = fConst4 * static_cast<float>(fHslider6);
		int iSlow26 = iSlow0 > 0;
		int iSlow27 = iSlow0 <= 0;
		float fSlow28 = static_cast<float>(fHslider7);
		float fSlow29 = 1.0f - 1.0f / std::pow(1e+05f, 1.0f / (static_cast<float>(fSlow28 == 0.0f) + fConst0 * fSlow28));
		float fSlow30 = static_cast<float>(fHslider8);
		float fSlow31 = fConst0 * fSlow30;
		float fSlow32 = static_cast<float>(fSlow30 == 0.0f) + fSlow31;
		float fSlow33 = static_cast<float>(fHslider9);
		float fSlow34 = 1.0f / (static_cast<float>(fSlow33 == 0.0f) + fConst0 * fSlow33);
		float fSlow35 = static_cast<float>(fHslider10);
		float fSlow36 = 5.122274e-11f * static_cast<float>(fHslider11);
		int iSlow37 = iSlow0 | static_cast<int>(static_cast<float>(fCheckbox0));
		int iSlow38 = iSlow37 == 0;
		float fSlow39 = 1.0f / std::max<float>(1.0f, fConst0 * static_cast<float>(fHslider12));
		float fSlow40 = std::max<float>(1.0f, fConst0 * static_cast<float>(fHslider13));
		float fSlow41 = 1.0f / fSlow40;
		float fSlow42 = 0.1f / std::max<float>(1.0f, fConst0 * static_cast<float>(fHslider14));
		float fSlow43 = fConst1 * static_cast<float>(fHslider15);
		float fSlow44 = fConst5 / fSlow5;
		float fSlow45 = fSlow44 + -2.0f;
		int iSlow46 = static_cast<int>(fSlow45);
		int iSlow47 = std::min<int>(4097, std::max<int>(0, iSlow46 + 1));
		float fSlow48 = std::floor(fSlow45);
		float fSlow49 = fSlow44 + (-2.0f - fSlow48);
		int iSlow50 = std::min<int>(4097, std::max<int>(0, iSlow46));
		float fSlow51 = fSlow48 + (3.0f - fSlow44);
		float fSlow52 = static_cast<float>(fHslider16);
		float fSlow53 = static_cast<float>(fEntry2);
		float fSlow54 = 0.5f * fSlow53 * (1.0f - fSlow52);
		float fSlow55 = static_cast<float>(fHslider17);
		float fSlow56 = std::exp(-(fConst11 / fSlow55));
		float fSlow57 = mydsp_faustpower2_f(fSlow56);
		float fSlow58 = 1.0f - fSlow57;
		float fSlow59 = 1.0f - fConst12 * fSlow57;
		float fSlow60 = std::sqrt(std::max<float>(0.0f, mydsp_faustpower2_f(fSlow59) / mydsp_faustpower2_f(fSlow58) + -1.0f));
		float fSlow61 = fSlow59 / fSlow58;
		float fSlow62 = fSlow61 - fSlow60;
		float fSlow63 = std::exp(-(fConst16 / fSlow55)) / fSlow56 + -1.0f;
		float fSlow64 = fSlow56 * (fSlow60 + (1.0f - fSlow61));
		float fSlow65 = std::exp(-(fConst23 / fSlow55));
		float fSlow66 = mydsp_faustpower2_f(fSlow65);
		float fSlow67 = 1.0f - fSlow66;
		float fSlow68 = 1.0f - fConst12 * fSlow66;
		float fSlow69 = std::sqrt(std::max<float>(0.0f, mydsp_faustpower2_f(fSlow68) / mydsp_faustpower2_f(fSlow67) + -1.0f));
		float fSlow70 = fSlow68 / fSlow67;
		float fSlow71 = fSlow70 - fSlow69;
		float fSlow72 = std::exp(-(fConst24 / fSlow55)) / fSlow65 + -1.0f;
		float fSlow73 = fSlow65 * (fSlow69 + (1.0f - fSlow70));
		float fSlow74 = std::exp(-(fConst30 / fSlow55));
		float fSlow75 = mydsp_faustpower2_f(fSlow74);
		float fSlow76 = 1.0f - fSlow75;
		float fSlow77 = 1.0f - fConst12 * fSlow75;
		float fSlow78 = std::sqrt(std::max<float>(0.0f, mydsp_faustpower2_f(fSlow77) / mydsp_faustpower2_f(fSlow76) + -1.0f));
		float fSlow79 = fSlow77 / fSlow76;
		float fSlow80 = fSlow79 - fSlow78;
		float fSlow81 = std::exp(-(fConst31 / fSlow55)) / fSlow74 + -1.0f;
		float fSlow82 = fSlow74 * (fSlow78 + (1.0f - fSlow79));
		float fSlow83 = std::exp(-(fConst37 / fSlow55));
		float fSlow84 = mydsp_faustpower2_f(fSlow83);
		float fSlow85 = 1.0f - fSlow84;
		float fSlow86 = 1.0f - fConst12 * fSlow84;
		float fSlow87 = std::sqrt(std::max<float>(0.0f, mydsp_faustpower2_f(fSlow86) / mydsp_faustpower2_f(fSlow85) + -1.0f));
		float fSlow88 = fSlow86 / fSlow85;
		float fSlow89 = fSlow88 - fSlow87;
		float fSlow90 = std::exp(-(fConst38 / fSlow55)) / fSlow83 + -1.0f;
		float fSlow91 = fSlow83 * (fSlow87 + (1.0f - fSlow88));
		float fSlow92 = std::exp(-(fConst44 / fSlow55));
		float fSlow93 = mydsp_faustpower2_f(fSlow92);
		float fSlow94 = 1.0f - fSlow93;
		float fSlow95 = 1.0f - fConst12 * fSlow93;
		float fSlow96 = std::sqrt(std::max<float>(0.0f, mydsp_faustpower2_f(fSlow95) / mydsp_faustpower2_f(fSlow94) + -1.0f));
		float fSlow97 = fSlow95 / fSlow94;
		float fSlow98 = fSlow97 - fSlow96;
		float fSlow99 = std::exp(-(fConst45 / fSlow55)) / fSlow92 + -1.0f;
		float fSlow100 = fSlow92 * (fSlow96 + (1.0f - fSlow97));
		float fSlow101 = 0.5f * fSlow53;
		int iSlow102 = static_cast<int>(std::min<float>(4096.0f, std::max<float>(0.0f, fConst5 * (static_cast<float>(fHslider18) / fSlow5))));
		float fSlow103 = std::exp(-(fConst51 / fSlow55));
		float fSlow104 = mydsp_faustpower2_f(fSlow103);
		float fSlow105 = 1.0f - fSlow104;
		float fSlow106 = 1.0f - fConst12 * fSlow104;
		float fSlow107 = std::sqrt(std::max<float>(0.0f, mydsp_faustpower2_f(fSlow106) / mydsp_faustpower2_f(fSlow105) + -1.0f));
		float fSlow108 = fSlow106 / fSlow105;
		float fSlow109 = fSlow108 - fSlow107;
		float fSlow110 = std::exp(-(fConst52 / fSlow55)) / fSlow103 + -1.0f;
		float fSlow111 = fSlow103 * (fSlow107 + (1.0f - fSlow108));
		float fSlow112 = std::exp(-(fConst58 / fSlow55));
		float fSlow113 = mydsp_faustpower2_f(fSlow112);
		float fSlow114 = 1.0f - fSlow113;
		float fSlow115 = 1.0f - fConst12 * fSlow113;
		float fSlow116 = std::sqrt(std::max<float>(0.0f, mydsp_faustpower2_f(fSlow115) / mydsp_faustpower2_f(fSlow114) + -1.0f));
		float fSlow117 = fSlow115 / fSlow114;
		float fSlow118 = fSlow117 - fSlow116;
		float fSlow119 = std::exp(-(fConst59 / fSlow55)) / fSlow112 + -1.0f;
		float fSlow120 = fSlow112 * (fSlow116 + (1.0f - fSlow117));
		float fSlow121 = std::exp(-(fConst65 / fSlow55));
		float fSlow122 = mydsp_faustpower2_f(fSlow121);
		float fSlow123 = 1.0f - fSlow122;
		float fSlow124 = 1.0f - fConst12 * fSlow122;
		float fSlow125 = std::sqrt(std::max<float>(0.0f, mydsp_faustpower2_f(fSlow124) / mydsp_faustpower2_f(fSlow123) + -1.0f));
		float fSlow126 = fSlow124 / fSlow123;
		float fSlow127 = fSlow126 - fSlow125;
		float fSlow128 = std::exp(-(fConst66 / fSlow55)) / fSlow121 + -1.0f;
		float fSlow129 = fSlow121 * (fSlow125 + (1.0f - fSlow126));
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iVec0[0] = 1;
			iVec1[0] = iSlow0;
			iRec0[0] = iSlow1 * (iRec0[1] + 1);
			float fTemp0 = static_cast<float>(iRec0[0]);
			iRec1[0] = iSlow0 + (iVec1[1] >= iSlow0) * iRec1[1];
			float fTemp1 = static_cast<float>(iRec1[0]);
			float fTemp2 = std::max<float>(0.0f, std::min<float>(fSlow3 * fTemp1, 1.0f) - fSlow2 * fTemp0);
			fRec2[0] = fSlow4 + fConst2 * fRec2[1];
			float fTemp3 = 1.0f - fRec2[0];
			float fTemp4 = fSlow13 * fRec3[(IOTA0 - iSlow12) & 8191] + fSlow11 * fRec3[(IOTA0 - iSlow9) & 8191];
			fVec2[0] = fTemp4;
			fRec5[0] = fSlow14 + fConst2 * fRec5[1];
			float fTemp5 = std::max<float>(0.0f, std::min<float>(fSlow19 * fTemp1, 1.0f) - fConst3 * fTemp0);
			float fTemp6 = fRec5[0] * fTemp5 * (fSlow18 * fTemp4 + fSlow17 * (fTemp4 + fVec2[1]) + fSlow16 * mydsp_faustpower2_f(fTemp4));
			float fTemp7 = std::cos(fTemp6);
			float fTemp8 = std::sin(fTemp6);
			float fTemp9 = fTemp4 * fTemp7 - fTemp8 * fRec6[1];
			float fTemp10 = fTemp7 * fTemp9 - fTemp8 * fRec7[1];
			float fTemp11 = fTemp7 * fTemp10 - fTemp8 * fRec8[1];
			float fTemp12 = fTemp7 * fTemp11 - fTemp8 * fRec9[1];
			float fTemp13 = fTemp7 * fTemp12 - fTemp8 * fRec10[1];
			fRec11[0] = fTemp7 * fTemp13 - fTemp8 * fRec11[1];
			fRec10[0] = fTemp8 * fTemp13 + fTemp7 * fRec11[1];
			fRec9[0] = fTemp8 * fTemp12 + fTemp7 * fRec10[1];
			fRec8[0] = fTemp8 * fTemp11 + fTemp7 * fRec9[1];
			fRec7[0] = fTemp8 * fTemp10 + fTemp7 * fRec8[1];
			fRec6[0] = fTemp8 * fTemp9 + fTemp7 * fRec7[1];
			int iTemp14 = 1 - iVec0[1];
			fRec14[0] = fSlow21 + fConst2 * fRec14[1];
			float fTemp15 = ((iTemp14) ? 0.0f : fRec13[1] + fConst4 * (fSlow23 + fSlow22 * fRec14[0]));
			fRec13[0] = fTemp15 - std::floor(fTemp15);
			float fTemp16 = 3.1415927f * fRec5[0] * ftbl0mydspSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec13[0]), 65535))] * fTemp5;
			float fTemp17 = std::cos(fTemp16);
			float fTemp18 = std::sin(fTemp16);
			float fTemp19 = fTemp4 * fTemp17 - fTemp18 * fRec15[1];
			float fTemp20 = fTemp17 * fTemp19 - fTemp18 * fRec16[1];
			float fTemp21 = fTemp17 * fTemp20 - fTemp18 * fRec17[1];
			float fTemp22 = fTemp17 * fTemp21 - fTemp18 * fRec18[1];
			float fTemp23 = fTemp17 * fTemp22 - fTemp18 * fRec19[1];
			fRec20[0] = fTemp17 * fTemp23 - fTemp18 * fRec20[1];
			fRec19[0] = fTemp18 * fTemp23 + fTemp17 * fRec20[1];
			fRec18[0] = fTemp18 * fTemp22 + fTemp17 * fRec19[1];
			fRec17[0] = fTemp18 * fTemp21 + fTemp17 * fRec18[1];
			fRec16[0] = fTemp18 * fTemp20 + fTemp17 * fRec17[1];
			fRec15[0] = fTemp18 * fTemp19 + fTemp17 * fRec16[1];
			float fTemp24 = 0.4f * (fSlow24 * (fTemp4 * fTemp18 + fRec15[1] * fTemp17) + fSlow20 * (fRec5[0] * (fTemp4 * fTemp8 + fRec6[1] * fTemp7) + (1.0f - fRec5[0]) * fTemp4));
			float fTemp25 = ((iTemp14) ? 0.0f : fSlow25 + fRec21[1]);
			fRec21[0] = fTemp25 - std::floor(fTemp25);
			iRec22[0] = iSlow26 & (iRec22[1] | (fRec24[1] >= 1.0f));
			iRec23[0] = iSlow26 * (iRec23[1] + 1);
			int iTemp26 = iSlow27 & (fRec24[1] > 0.0f);
			float fTemp27 = static_cast<float>(iRec23[1]);
			fRec24[0] = (fSlow34 * static_cast<float>(((((iRec22[1] == 0) & iSlow26) & (fRec24[1] < 1.0f)) & (fTemp27 > fSlow31)) * (1 - (fTemp27 < fSlow32))) + fRec24[1] * (1.0f - fSlow29 * static_cast<float>(iTemp26))) * static_cast<float>((iTemp26 == 0) | (fRec24[1] >= 1e-06f));
			iRec25[0] = 1103515245 * iRec25[1] + 12345;
			iVec4[0] = iSlow37;
			iRec26[0] = iSlow38 * (iRec26[1] + 1);
			iRec27[0] = iSlow37 + iRec27[1] * (iVec4[1] >= iSlow37);
			float fTemp28 = static_cast<float>(iRec27[0]);
			fRec28[0] = fSlow43 + fConst2 * fRec28[1];
			float fTemp29 = fRec28[0] * std::max<float>(0.0f, std::min<float>(fSlow41 * fTemp28, std::max<float>(fSlow42 * (fSlow40 - fTemp28) + 1.0f, 0.9f)) * (1.0f - fSlow39 * static_cast<float>(iRec26[0]))) * (fSlow36 * static_cast<float>(iRec25[0]) + 1.1f) + fSlow35 * fRec24[0] * ftbl0mydspSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec21[0]), 65535))] + fTemp24;
			fVec5[IOTA0 & 8191] = fTemp29;
			float fTemp30 = fSlow49 * fVec5[(IOTA0 - iSlow47) & 8191];
			float fTemp31 = fSlow51 * fVec5[(IOTA0 - iSlow50) & 8191];
			float fTemp32 = fTemp30 + fTemp24 + fTemp31 - mydsp_faustpower3_f(fTemp31 + fTemp30);
			fVec6[0] = fTemp32;
			fRec4[0] = -(fConst8 * (fConst7 * fRec4[1] - (fTemp32 + fVec6[1])));
			fRec3[IOTA0 & 8191] = fRec4[0];
			float fTemp33 = fRec3[IOTA0 & 8191];
			fRec40[0] = -(fConst15 * (fConst14 * fRec40[1] - (fRec33[1] + fRec33[2])));
			fRec39[0] = fSlow64 * (fRec33[1] + fSlow63 * fRec40[0]) + fSlow62 * fRec39[1];
			fVec7[IOTA0 & 16383] = 0.35355338f * fRec39[0] + 1e-20f;
			fVec8[IOTA0 & 4095] = fSlow54 * fTemp33 * fRec2[0] * fTemp2;
			float fTemp34 = 0.3f * fVec8[(IOTA0 - iConst19) & 4095];
			float fTemp35 = fTemp34 + fVec7[(IOTA0 - iConst18) & 16383] - 0.6f * fRec37[1];
			fVec9[IOTA0 & 4095] = fTemp35;
			fRec37[0] = fVec9[(IOTA0 - iConst20) & 4095];
			float fRec38 = 0.6f * fTemp35;
			fRec44[0] = -(fConst15 * (fConst14 * fRec44[1] - (fRec29[1] + fRec29[2])));
			fRec43[0] = fSlow73 * (fRec29[1] + fSlow72 * fRec44[0]) + fSlow71 * fRec43[1];
			fVec10[IOTA0 & 16383] = 0.35355338f * fRec43[0] + 1e-20f;
			float fTemp36 = fVec10[(IOTA0 - iConst26) & 16383] + fTemp34 - 0.6f * fRec41[1];
			fVec11[IOTA0 & 2047] = fTemp36;
			fRec41[0] = fVec11[(IOTA0 - iConst27) & 2047];
			float fRec42 = 0.6f * fTemp36;
			float fTemp37 = fRec42 + fRec38;
			fRec48[0] = -(fConst15 * (fConst14 * fRec48[1] - (fRec31[1] + fRec31[2])));
			fRec47[0] = fSlow82 * (fRec31[1] + fSlow81 * fRec48[0]) + fSlow80 * fRec47[1];
			fVec12[IOTA0 & 16383] = 0.35355338f * fRec47[0] + 1e-20f;
			float fTemp38 = fVec12[(IOTA0 - iConst33) & 16383] - (fTemp34 + 0.6f * fRec45[1]);
			fVec13[IOTA0 & 4095] = fTemp38;
			fRec45[0] = fVec13[(IOTA0 - iConst34) & 4095];
			float fRec46 = 0.6f * fTemp38;
			fRec52[0] = -(fConst15 * (fConst14 * fRec52[1] - (fRec35[1] + fRec35[2])));
			fRec51[0] = fSlow91 * (fRec35[1] + fSlow90 * fRec52[0]) + fSlow89 * fRec51[1];
			fVec14[IOTA0 & 16383] = 0.35355338f * fRec51[0] + 1e-20f;
			float fTemp39 = fVec14[(IOTA0 - iConst40) & 16383] - (fTemp34 + 0.6f * fRec49[1]);
			fVec15[IOTA0 & 2047] = fTemp39;
			fRec49[0] = fVec15[(IOTA0 - iConst41) & 2047];
			float fRec50 = 0.6f * fTemp39;
			float fTemp40 = fRec50 + fRec46 + fTemp37;
			fRec56[0] = -(fConst15 * (fConst14 * fRec56[1] - (fRec30[1] + fRec30[2])));
			fRec55[0] = fSlow100 * (fRec30[1] + fSlow99 * fRec56[0]) + fSlow98 * fRec55[1];
			fVec16[IOTA0 & 32767] = 0.35355338f * fRec55[0] + 1e-20f;
			fVec17[IOTA0 & 8191] = fSlow101 * fTemp33 * fTemp2;
			float fTemp41 = fVec17[(IOTA0 - iSlow102) & 8191];
			fVec18[IOTA0 & 4095] = fSlow52 * fRec2[0] * fTemp41;
			float fTemp42 = 0.3f * fVec18[(IOTA0 - iConst19) & 4095];
			float fTemp43 = fTemp42 + 0.6f * fRec53[1] + fVec16[(IOTA0 - iConst47) & 32767];
			fVec19[IOTA0 & 4095] = fTemp43;
			fRec53[0] = fVec19[(IOTA0 - iConst48) & 4095];
			float fRec54 = -(0.6f * fTemp43);
			fRec60[0] = -(fConst15 * (fConst14 * fRec60[1] - (fRec34[1] + fRec34[2])));
			fRec59[0] = fSlow111 * (fRec34[1] + fSlow110 * fRec60[0]) + fSlow109 * fRec59[1];
			fVec20[IOTA0 & 16383] = 0.35355338f * fRec59[0] + 1e-20f;
			float fTemp44 = fVec20[(IOTA0 - iConst54) & 16383] + fTemp42 + 0.6f * fRec57[1];
			fVec21[IOTA0 & 4095] = fTemp44;
			fRec57[0] = fVec21[(IOTA0 - iConst55) & 4095];
			float fRec58 = -(0.6f * fTemp44);
			fRec64[0] = -(fConst15 * (fConst14 * fRec64[1] - (fRec32[1] + fRec32[2])));
			fRec63[0] = fSlow120 * (fRec32[1] + fSlow119 * fRec64[0]) + fSlow118 * fRec63[1];
			fVec22[IOTA0 & 32767] = 0.35355338f * fRec63[0] + 1e-20f;
			float fTemp45 = 0.6f * fRec61[1] + fVec22[(IOTA0 - iConst61) & 32767];
			fVec23[IOTA0 & 4095] = fTemp45 - fTemp42;
			fRec61[0] = fVec23[(IOTA0 - iConst62) & 4095];
			float fRec62 = 0.6f * (fTemp42 - fTemp45);
			fRec68[0] = -(fConst15 * (fConst14 * fRec68[1] - (fRec36[1] + fRec36[2])));
			fRec67[0] = fSlow129 * (fRec36[1] + fSlow128 * fRec68[0]) + fSlow127 * fRec67[1];
			fVec24[IOTA0 & 32767] = 0.35355338f * fRec67[0] + 1e-20f;
			float fTemp46 = 0.6f * fRec65[1] + fVec24[(IOTA0 - iConst68) & 32767];
			fVec25[IOTA0 & 2047] = fTemp46 - fTemp42;
			fRec65[0] = fVec25[(IOTA0 - iConst69) & 2047];
			float fRec66 = 0.6f * (fTemp42 - fTemp46);
			fRec29[0] = fRec65[1] + fRec61[1] + fRec57[1] + fRec53[1] + fRec49[1] + fRec45[1] + fRec37[1] + fRec41[1] + fRec66 + fRec62 + fRec58 + fRec54 + fTemp40;
			fRec30[0] = fRec49[1] + fRec45[1] + fRec37[1] + fRec41[1] + fTemp40 - (fRec65[1] + fRec61[1] + fRec57[1] + fRec53[1] + fRec66 + fRec62 + fRec54 + fRec58);
			float fTemp47 = fRec46 + fRec50;
			fRec31[0] = fRec57[1] + fRec53[1] + fRec37[1] + fRec41[1] + fRec58 + fRec54 + fTemp37 - (fRec65[1] + fRec61[1] + fRec49[1] + fRec45[1] + fRec66 + fRec62 + fTemp47);
			fRec32[0] = fRec65[1] + fRec61[1] + fRec37[1] + fRec41[1] + fRec66 + fRec62 + fTemp37 - (fRec57[1] + fRec53[1] + fRec49[1] + fRec45[1] + fRec58 + fRec54 + fTemp47);
			float fTemp48 = fRec38 + fRec50;
			float fTemp49 = fRec42 + fRec46;
			fRec33[0] = fRec61[1] + fRec53[1] + fRec45[1] + fRec41[1] + fRec62 + fRec54 + fTemp49 - (fRec65[1] + fRec57[1] + fRec49[1] + fRec37[1] + fRec66 + fRec58 + fTemp48);
			fRec34[0] = fRec65[1] + fRec57[1] + fRec45[1] + fRec41[1] + fRec66 + fRec58 + fTemp49 - (fRec61[1] + fRec53[1] + fRec49[1] + fRec37[1] + fRec62 + fRec54 + fTemp48);
			float fTemp50 = fRec38 + fRec46;
			float fTemp51 = fRec42 + fRec50;
			fRec35[0] = fRec65[1] + fRec53[1] + fRec49[1] + fRec41[1] + fRec66 + fRec54 + fTemp51 - (fRec61[1] + fRec57[1] + fRec45[1] + fRec37[1] + fRec62 + fRec58 + fTemp50);
			fRec36[0] = fRec61[1] + fRec57[1] + fRec49[1] + fRec41[1] + fRec62 + fRec58 + fTemp51 - (fRec65[1] + fRec53[1] + fRec45[1] + fRec37[1] + fRec66 + fRec54 + fTemp50);
			output0[i0] = static_cast<FAUSTFLOAT>(0.37f * (fRec30[0] + fRec31[0]) + fSlow54 * fTemp33 * fTemp3 * fTemp2);
			output1[i0] = static_cast<FAUSTFLOAT>(0.37f * (fRec30[0] - fRec31[0]) + fSlow52 * fTemp3 * fTemp41);
			iVec0[1] = iVec0[0];
			iVec1[1] = iVec1[0];
			iRec0[1] = iRec0[0];
			iRec1[1] = iRec1[0];
			fRec2[1] = fRec2[0];
			IOTA0 = IOTA0 + 1;
			fVec2[1] = fVec2[0];
			fRec5[1] = fRec5[0];
			fRec11[1] = fRec11[0];
			fRec10[1] = fRec10[0];
			fRec9[1] = fRec9[0];
			fRec8[1] = fRec8[0];
			fRec7[1] = fRec7[0];
			fRec6[1] = fRec6[0];
			fRec14[1] = fRec14[0];
			fRec13[1] = fRec13[0];
			fRec20[1] = fRec20[0];
			fRec19[1] = fRec19[0];
			fRec18[1] = fRec18[0];
			fRec17[1] = fRec17[0];
			fRec16[1] = fRec16[0];
			fRec15[1] = fRec15[0];
			fRec21[1] = fRec21[0];
			iRec22[1] = iRec22[0];
			iRec23[1] = iRec23[0];
			fRec24[1] = fRec24[0];
			iRec25[1] = iRec25[0];
			iVec4[1] = iVec4[0];
			iRec26[1] = iRec26[0];
			iRec27[1] = iRec27[0];
			fRec28[1] = fRec28[0];
			fVec6[1] = fVec6[0];
			fRec4[1] = fRec4[0];
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

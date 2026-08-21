/* ------------------------------------------------------------
name: "guitar_oud"
Code generated with Faust 2.85.9 (https://faust.grame.fr)
Compilation options: -lang cpp -fpga-mem-th 4 -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __guitardsp_H__
#define  __guitardsp_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS guitardsp
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

static float guitardsp_faustpower2_f(float value) {
	return value * value;
}

class guitardsp : public dsp {
	
 private:
	
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	float fConst3;
	float fConst4;
	float fConst5;
	float fConst6;
	float fVec0[2];
	FAUSTFLOAT fHslider0;
	FAUSTFLOAT fHslider1;
	float fConst7;
	int iRec12[2];
	int IOTA0;
	float fConst8;
	FAUSTFLOAT fHslider2;
	float fConst9;
	float fRec24[2];
	float fRec27[2];
	float fRec29[4];
	float fRec30[2048];
	float fVec1[2];
	FAUSTFLOAT fButton0;
	float fVec2[2];
	int iRec32[2];
	FAUSTFLOAT fHslider3;
	FAUSTFLOAT fHslider4;
	float fConst10;
	float fConst11;
	int iRec34[2];
	float fRec33[3];
	FAUSTFLOAT fHslider5;
	float fVec3[2];
	float fRec31[2];
	float fVec4[2];
	float fRec28[2048];
	float fRec21[2];
	float fRec18[2048];
	float fRec20[2];
	float fRec17[4];
	int iRec8[2];
	float fRec4[2048];
	float fRec2[2];
	float fRec1[3];
	int iRec46[2];
	FAUSTFLOAT fHslider6;
	float fRec58[2];
	float fRec61[2];
	float fRec63[4];
	float fRec64[2048];
	float fVec5[2];
	FAUSTFLOAT fButton1;
	float fVec6[2];
	int iRec66[2];
	float fVec7[2];
	float fRec65[2];
	float fVec8[2];
	float fRec62[2048];
	float fRec55[2];
	float fRec52[2048];
	float fRec54[2];
	float fRec51[4];
	int iRec42[2];
	float fRec38[2048];
	float fRec36[2];
	float fRec35[3];
	int iRec78[2];
	FAUSTFLOAT fHslider7;
	float fRec90[2];
	float fRec93[2];
	float fRec95[4];
	float fRec96[2048];
	float fVec9[2];
	FAUSTFLOAT fButton2;
	float fVec10[2];
	int iRec98[2];
	float fVec11[2];
	float fRec97[2];
	float fVec12[2];
	float fRec94[2048];
	float fRec87[2];
	float fRec84[2048];
	float fRec86[2];
	float fRec83[4];
	int iRec74[2];
	float fRec70[2048];
	float fRec68[2];
	float fRec67[3];
	int iRec109[2];
	FAUSTFLOAT fHslider8;
	float fRec121[2];
	float fRec124[2];
	float fRec126[4];
	float fRec127[2048];
	float fVec13[2];
	FAUSTFLOAT fButton3;
	float fVec14[2];
	int iRec129[2];
	float fVec15[2];
	float fRec128[2];
	float fVec16[2];
	float fRec125[2048];
	float fRec118[2];
	float fRec115[2048];
	float fRec117[2];
	float fRec114[4];
	int iRec105[2];
	float fRec101[2048];
	float fRec99[2];
	int iRec140[2];
	FAUSTFLOAT fHslider9;
	float fRec152[2];
	float fRec155[2];
	float fRec157[4];
	float fRec158[2048];
	float fVec17[2];
	FAUSTFLOAT fButton4;
	float fVec18[2];
	int iRec160[2];
	float fVec19[2];
	float fRec159[2];
	float fVec20[2];
	float fRec156[2048];
	float fRec149[2];
	float fRec146[2048];
	float fRec148[2];
	float fRec145[4];
	int iRec136[2];
	float fRec132[2048];
	float fRec130[2];
	int iRec171[2];
	FAUSTFLOAT fHslider10;
	float fRec183[2];
	float fRec186[2];
	float fRec188[4];
	float fRec189[2048];
	float fVec21[2];
	FAUSTFLOAT fButton5;
	float fVec22[2];
	int iRec191[2];
	float fVec23[2];
	float fRec190[2];
	float fVec24[2];
	float fRec187[2048];
	float fRec180[2];
	float fRec177[2048];
	float fRec179[2];
	float fRec176[4];
	int iRec167[2];
	float fRec163[2048];
	float fRec161[2];
	FAUSTFLOAT fHslider11;
	float fRec0[3];
	float fConst12;
	float fConst13;
	float fConst14;
	float fConst15;
	float fConst16;
	float fConst17;
	float fConst18;
	float fRec192[3];
	float fConst19;
	float fConst20;
	float fConst21;
	float fConst22;
	float fConst23;
	float fConst24;
	float fConst25;
	float fRec193[3];
	float fConst26;
	float fConst27;
	float fConst28;
	float fConst29;
	float fConst30;
	float fConst31;
	float fConst32;
	float fRec194[3];
	float fConst33;
	float fConst34;
	float fConst35;
	float fConst36;
	float fConst37;
	float fConst38;
	float fConst39;
	float fRec195[3];
	float fConst40;
	float fConst41;
	float fConst42;
	float fConst43;
	float fConst44;
	float fConst45;
	float fConst46;
	float fRec196[3];
	float fConst47;
	float fConst48;
	float fConst49;
	float fConst50;
	float fConst51;
	float fConst52;
	float fConst53;
	float fRec197[3];
	float fConst54;
	float fConst55;
	float fConst56;
	float fConst57;
	float fConst58;
	float fConst59;
	float fConst60;
	float fRec198[3];
	float fConst61;
	float fConst62;
	float fConst63;
	float fConst64;
	float fConst65;
	float fConst66;
	float fConst67;
	float fRec199[3];
	float fConst68;
	float fConst69;
	float fConst70;
	float fConst71;
	float fConst72;
	float fConst73;
	float fConst74;
	float fRec200[3];
	float fConst75;
	float fConst76;
	float fConst77;
	float fConst78;
	float fConst79;
	float fConst80;
	float fConst81;
	float fRec201[3];
	float fConst82;
	float fConst83;
	float fConst84;
	float fConst85;
	float fConst86;
	float fConst87;
	float fConst88;
	float fRec202[3];
	float fConst89;
	float fConst90;
	float fConst91;
	float fConst92;
	float fConst93;
	float fConst94;
	float fConst95;
	float fRec203[3];
	float fConst96;
	FAUSTFLOAT fHslider12;
	float fConst97;
	float fRec204[3];
	float fRec205[3];
	FAUSTFLOAT fHslider13;
	float fRec206[3];
	float fConst98;
	float fConst99;
	float fConst100;
	float fConst101;
	float fConst102;
	float fConst103;
	float fRec207[3];
	float fConst104;
	float fConst105;
	float fRec208[3];
	float fConst106;
	float fConst107;
	float fConst108;
	float fConst109;
	float fConst110;
	float fConst111;
	float fRec209[3];
	float fConst112;
	float fConst113;
	float fConst114;
	float fConst115;
	float fConst116;
	float fConst117;
	float fConst118;
	float fRec210[3];
	float fConst119;
	FAUSTFLOAT fHslider14;
	
 public:
	guitardsp() {
	}
	
	guitardsp(const guitardsp&) = default;
	
	virtual ~guitardsp() = default;
	
	guitardsp& operator=(const guitardsp&) = default;
	
	void metadata(Meta* m) { 
		m->declare("compile_options", "-lang cpp -fpga-mem-th 4 -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("delays.lib/fdelay4:author", "Julius O. Smith III");
		m->declare("delays.lib/fdelayltv:author", "Julius O. Smith III");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.2.0");
		m->declare("envelopes.lib/ar:author", "Yann Orlarey, Stéphane Letz");
		m->declare("envelopes.lib/author", "GRAME");
		m->declare("envelopes.lib/copyright", "GRAME");
		m->declare("envelopes.lib/license", "LGPL with exception");
		m->declare("envelopes.lib/name", "Faust Envelope Library");
		m->declare("envelopes.lib/version", "1.3.0");
		m->declare("filename", "guitar_oud.dsp");
		m->declare("filters.lib/fir:author", "Julius O. Smith III");
		m->declare("filters.lib/fir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/fir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/iir:author", "Julius O. Smith III");
		m->declare("filters.lib/iir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/iir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/resonbp:author", "Julius O. Smith III");
		m->declare("filters.lib/resonbp:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/resonbp:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf1:author", "Julius O. Smith III");
		m->declare("filters.lib/tf1:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf1:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf1s:author", "Julius O. Smith III");
		m->declare("filters.lib/tf1s:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf1s:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf2:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf2s:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2s:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2s:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/version", "1.7.1");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.9.0");
		m->declare("name", "guitar_oud");
		m->declare("noises.lib/name", "Faust Noise Generator Library");
		m->declare("noises.lib/version", "1.5.0");
		m->declare("physmodels.lib/name", "Faust Physical Models Library");
		m->declare("physmodels.lib/version", "1.2.0");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("routes.lib/name", "Faust Signal Routing Library");
		m->declare("routes.lib/version", "1.3.0");
		m->declare("signals.lib/name", "Faust Routing Library");
		m->declare("signals.lib/version", "1.6.0");
	}

	virtual int getNumInputs() {
		return 1;
	}
	virtual int getNumOutputs() {
		return 1;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, static_cast<float>(fSampleRate)));
		fConst1 = std::tan(9424.778f / fConst0);
		fConst2 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst1));
		fConst3 = 1.0f / fConst1;
		fConst4 = (fConst3 + -0.6666667f) / fConst1 + 1.0f;
		fConst5 = (fConst3 + 0.6666667f) / fConst1 + 1.0f;
		fConst6 = 1.0f / fConst5;
		fConst7 = 3.1415927f / fConst0;
		fConst8 = 0.00882353f * fConst0;
		fConst9 = 0.0014705883f * fConst0;
		fConst10 = 0.002f * fConst0;
		fConst11 = 15.707963f / fConst0;
		fConst12 = 0.02f / (fConst1 * fConst5);
		fConst13 = std::tan(8545.132f / fConst0);
		fConst14 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst13));
		fConst15 = 1.0f / fConst13;
		fConst16 = (fConst15 + -0.5555556f) / fConst13 + 1.0f;
		fConst17 = (fConst15 + 0.5555556f) / fConst13 + 1.0f;
		fConst18 = 1.0f / fConst17;
		fConst19 = 0.022222223f / (fConst13 * fConst17);
		fConst20 = std::tan(7728.318f / fConst0);
		fConst21 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst20));
		fConst22 = 1.0f / fConst20;
		fConst23 = (fConst22 + -0.5f) / fConst20 + 1.0f;
		fConst24 = (fConst22 + 0.5f) / fConst20 + 1.0f;
		fConst25 = 1.0f / fConst24;
		fConst26 = 0.025f / (fConst20 * fConst24);
		fConst27 = std::tan(6974.3354f / fConst0);
		fConst28 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst27));
		fConst29 = 1.0f / fConst27;
		fConst30 = (fConst29 + -0.4347826f) / fConst27 + 1.0f;
		fConst31 = (fConst29 + 0.4347826f) / fConst27 + 1.0f;
		fConst32 = 1.0f / fConst31;
		fConst33 = 0.026086956f / (fConst27 * fConst31);
		fConst34 = std::tan(6283.1855f / fConst0);
		fConst35 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst34));
		fConst36 = 1.0f / fConst34;
		fConst37 = (fConst36 + -0.4f) / fConst34 + 1.0f;
		fConst38 = (fConst36 + 0.4f) / fConst34 + 1.0f;
		fConst39 = 1.0f / fConst38;
		fConst40 = 0.028f / (fConst34 * fConst38);
		fConst41 = std::tan(5057.9644f / fConst0);
		fConst42 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst41));
		fConst43 = 1.0f / fConst41;
		fConst44 = (fConst43 + -0.33333334f) / fConst41 + 1.0f;
		fConst45 = (fConst43 + 0.33333334f) / fConst41 + 1.0f;
		fConst46 = 1.0f / fConst45;
		fConst47 = 1.0f / (fConst41 * fConst45);
		fConst48 = std::tan(4021.2385f / fConst0);
		fConst49 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst48));
		fConst50 = 1.0f / fConst48;
		fConst51 = (fConst50 + -0.25f) / fConst48 + 1.0f;
		fConst52 = (fConst50 + 0.25f) / fConst48 + 1.0f;
		fConst53 = 1.0f / fConst52;
		fConst54 = 1.0f / (fConst48 * fConst52);
		fConst55 = std::tan(3549.9998f / fConst0);
		fConst56 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst55));
		fConst57 = 1.0f / fConst55;
		fConst58 = (fConst57 + -0.22222222f) / fConst55 + 1.0f;
		fConst59 = (fConst57 + 0.22222222f) / fConst55 + 1.0f;
		fConst60 = 1.0f / fConst59;
		fConst61 = 0.031111112f / (fConst55 * fConst59);
		fConst62 = std::tan(3110.1768f / fConst0);
		fConst63 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst62));
		fConst64 = 1.0f / fConst62;
		fConst65 = (fConst64 + -0.2f) / fConst62 + 1.0f;
		fConst66 = (fConst64 + 0.2f) / fConst62 + 1.0f;
		fConst67 = 1.0f / fConst66;
		fConst68 = 0.032f / (fConst62 * fConst66);
		fConst69 = std::tan(2701.7698f / fConst0);
		fConst70 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst69));
		fConst71 = 1.0f / fConst69;
		fConst72 = (fConst71 + -0.18181819f) / fConst69 + 1.0f;
		fConst73 = (fConst71 + 0.18181819f) / fConst69 + 1.0f;
		fConst74 = 1.0f / fConst73;
		fConst75 = 0.03272727f / (fConst69 * fConst73);
		fConst76 = std::tan(2324.7786f / fConst0);
		fConst77 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst76));
		fConst78 = 1.0f / fConst76;
		fConst79 = (fConst78 + -0.16666667f) / fConst76 + 1.0f;
		fConst80 = (fConst78 + 0.16666667f) / fConst76 + 1.0f;
		fConst81 = 1.0f / fConst80;
		fConst82 = 0.033333335f / (fConst76 * fConst80);
		fConst83 = std::tan(1947.7875f / fConst0);
		fConst84 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst83));
		fConst85 = 1.0f / fConst83;
		fConst86 = (fConst85 + -0.15384616f) / fConst83 + 1.0f;
		fConst87 = (fConst85 + 0.15384616f) / fConst83 + 1.0f;
		fConst88 = 1.0f / fConst87;
		fConst89 = 0.033846155f / (fConst83 * fConst87);
		fConst90 = std::tan(1602.2123f / fConst0);
		fConst91 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst90));
		fConst92 = 1.0f / fConst90;
		fConst93 = (fConst92 + -0.14285715f) / fConst90 + 1.0f;
		fConst94 = (fConst92 + 0.14285715f) / fConst90 + 1.0f;
		fConst95 = 1.0f / fConst94;
		fConst96 = 0.035714287f / (fConst90 * fConst94);
		fConst97 = 5.969026f / fConst0;
		fConst98 = std::tan(1319.4689f / fConst0);
		fConst99 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst98));
		fConst100 = 1.0f / fConst98;
		fConst101 = (fConst100 + -0.125f) / fConst98 + 1.0f;
		fConst102 = (fConst100 + 0.125f) / fConst98 + 1.0f;
		fConst103 = 1.0f / fConst102;
		fConst104 = 1.0f / (fConst98 * fConst102);
		fConst105 = 4.24115f / fConst0;
		fConst106 = std::tan(5654.8667f / fConst0);
		fConst107 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst106));
		fConst108 = 1.0f / fConst106;
		fConst109 = (fConst108 + -0.35714287f) / fConst106 + 1.0f;
		fConst110 = (fConst108 + 0.35714287f) / fConst106 + 1.0f;
		fConst111 = 1.0f / fConst110;
		fConst112 = 1.0f / (fConst106 * fConst110);
		fConst113 = std::tan(4523.8936f / fConst0);
		fConst114 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fConst113));
		fConst115 = 1.0f / fConst113;
		fConst116 = (fConst115 + -0.2857143f) / fConst113 + 1.0f;
		fConst117 = (fConst115 + 0.2857143f) / fConst113 + 1.0f;
		fConst118 = 1.0f / fConst117;
		fConst119 = 1.0f / (fConst113 * fConst117);
	}
	
	virtual void instanceResetUserInterface() {
		fHslider0 = static_cast<FAUSTFLOAT>(0.3f);
		fHslider1 = static_cast<FAUSTFLOAT>(4.4e+02f);
		fHslider2 = static_cast<FAUSTFLOAT>(0.8f);
		fButton0 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider3 = static_cast<FAUSTFLOAT>(0.6f);
		fHslider4 = static_cast<FAUSTFLOAT>(0.8f);
		fHslider5 = static_cast<FAUSTFLOAT>(0.8f);
		fHslider6 = static_cast<FAUSTFLOAT>(0.8f);
		fButton1 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider7 = static_cast<FAUSTFLOAT>(0.8f);
		fButton2 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider8 = static_cast<FAUSTFLOAT>(0.8f);
		fButton3 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider9 = static_cast<FAUSTFLOAT>(0.8f);
		fButton4 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider10 = static_cast<FAUSTFLOAT>(0.8f);
		fButton5 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider11 = static_cast<FAUSTFLOAT>(0.01f);
		fHslider12 = static_cast<FAUSTFLOAT>(2e+02f);
		fHslider13 = static_cast<FAUSTFLOAT>(1e+02f);
		fHslider14 = static_cast<FAUSTFLOAT>(0.5f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fVec0[l0] = 0.0f;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			iRec12[l1] = 0;
		}
		IOTA0 = 0;
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec24[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec27[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 4; l4 = l4 + 1) {
			fRec29[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2048; l5 = l5 + 1) {
			fRec30[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fVec1[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			fVec2[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
			iRec32[l8] = 0;
		}
		for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
			iRec34[l9] = 0;
		}
		for (int l10 = 0; l10 < 3; l10 = l10 + 1) {
			fRec33[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fVec3[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			fRec31[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 2; l13 = l13 + 1) {
			fVec4[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 2048; l14 = l14 + 1) {
			fRec28[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fRec21[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 2048; l16 = l16 + 1) {
			fRec18[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2; l17 = l17 + 1) {
			fRec20[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 4; l18 = l18 + 1) {
			fRec17[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 2; l19 = l19 + 1) {
			iRec8[l19] = 0;
		}
		for (int l20 = 0; l20 < 2048; l20 = l20 + 1) {
			fRec4[l20] = 0.0f;
		}
		for (int l21 = 0; l21 < 2; l21 = l21 + 1) {
			fRec2[l21] = 0.0f;
		}
		for (int l22 = 0; l22 < 3; l22 = l22 + 1) {
			fRec1[l22] = 0.0f;
		}
		for (int l23 = 0; l23 < 2; l23 = l23 + 1) {
			iRec46[l23] = 0;
		}
		for (int l24 = 0; l24 < 2; l24 = l24 + 1) {
			fRec58[l24] = 0.0f;
		}
		for (int l25 = 0; l25 < 2; l25 = l25 + 1) {
			fRec61[l25] = 0.0f;
		}
		for (int l26 = 0; l26 < 4; l26 = l26 + 1) {
			fRec63[l26] = 0.0f;
		}
		for (int l27 = 0; l27 < 2048; l27 = l27 + 1) {
			fRec64[l27] = 0.0f;
		}
		for (int l28 = 0; l28 < 2; l28 = l28 + 1) {
			fVec5[l28] = 0.0f;
		}
		for (int l29 = 0; l29 < 2; l29 = l29 + 1) {
			fVec6[l29] = 0.0f;
		}
		for (int l30 = 0; l30 < 2; l30 = l30 + 1) {
			iRec66[l30] = 0;
		}
		for (int l31 = 0; l31 < 2; l31 = l31 + 1) {
			fVec7[l31] = 0.0f;
		}
		for (int l32 = 0; l32 < 2; l32 = l32 + 1) {
			fRec65[l32] = 0.0f;
		}
		for (int l33 = 0; l33 < 2; l33 = l33 + 1) {
			fVec8[l33] = 0.0f;
		}
		for (int l34 = 0; l34 < 2048; l34 = l34 + 1) {
			fRec62[l34] = 0.0f;
		}
		for (int l35 = 0; l35 < 2; l35 = l35 + 1) {
			fRec55[l35] = 0.0f;
		}
		for (int l36 = 0; l36 < 2048; l36 = l36 + 1) {
			fRec52[l36] = 0.0f;
		}
		for (int l37 = 0; l37 < 2; l37 = l37 + 1) {
			fRec54[l37] = 0.0f;
		}
		for (int l38 = 0; l38 < 4; l38 = l38 + 1) {
			fRec51[l38] = 0.0f;
		}
		for (int l39 = 0; l39 < 2; l39 = l39 + 1) {
			iRec42[l39] = 0;
		}
		for (int l40 = 0; l40 < 2048; l40 = l40 + 1) {
			fRec38[l40] = 0.0f;
		}
		for (int l41 = 0; l41 < 2; l41 = l41 + 1) {
			fRec36[l41] = 0.0f;
		}
		for (int l42 = 0; l42 < 3; l42 = l42 + 1) {
			fRec35[l42] = 0.0f;
		}
		for (int l43 = 0; l43 < 2; l43 = l43 + 1) {
			iRec78[l43] = 0;
		}
		for (int l44 = 0; l44 < 2; l44 = l44 + 1) {
			fRec90[l44] = 0.0f;
		}
		for (int l45 = 0; l45 < 2; l45 = l45 + 1) {
			fRec93[l45] = 0.0f;
		}
		for (int l46 = 0; l46 < 4; l46 = l46 + 1) {
			fRec95[l46] = 0.0f;
		}
		for (int l47 = 0; l47 < 2048; l47 = l47 + 1) {
			fRec96[l47] = 0.0f;
		}
		for (int l48 = 0; l48 < 2; l48 = l48 + 1) {
			fVec9[l48] = 0.0f;
		}
		for (int l49 = 0; l49 < 2; l49 = l49 + 1) {
			fVec10[l49] = 0.0f;
		}
		for (int l50 = 0; l50 < 2; l50 = l50 + 1) {
			iRec98[l50] = 0;
		}
		for (int l51 = 0; l51 < 2; l51 = l51 + 1) {
			fVec11[l51] = 0.0f;
		}
		for (int l52 = 0; l52 < 2; l52 = l52 + 1) {
			fRec97[l52] = 0.0f;
		}
		for (int l53 = 0; l53 < 2; l53 = l53 + 1) {
			fVec12[l53] = 0.0f;
		}
		for (int l54 = 0; l54 < 2048; l54 = l54 + 1) {
			fRec94[l54] = 0.0f;
		}
		for (int l55 = 0; l55 < 2; l55 = l55 + 1) {
			fRec87[l55] = 0.0f;
		}
		for (int l56 = 0; l56 < 2048; l56 = l56 + 1) {
			fRec84[l56] = 0.0f;
		}
		for (int l57 = 0; l57 < 2; l57 = l57 + 1) {
			fRec86[l57] = 0.0f;
		}
		for (int l58 = 0; l58 < 4; l58 = l58 + 1) {
			fRec83[l58] = 0.0f;
		}
		for (int l59 = 0; l59 < 2; l59 = l59 + 1) {
			iRec74[l59] = 0;
		}
		for (int l60 = 0; l60 < 2048; l60 = l60 + 1) {
			fRec70[l60] = 0.0f;
		}
		for (int l61 = 0; l61 < 2; l61 = l61 + 1) {
			fRec68[l61] = 0.0f;
		}
		for (int l62 = 0; l62 < 3; l62 = l62 + 1) {
			fRec67[l62] = 0.0f;
		}
		for (int l63 = 0; l63 < 2; l63 = l63 + 1) {
			iRec109[l63] = 0;
		}
		for (int l64 = 0; l64 < 2; l64 = l64 + 1) {
			fRec121[l64] = 0.0f;
		}
		for (int l65 = 0; l65 < 2; l65 = l65 + 1) {
			fRec124[l65] = 0.0f;
		}
		for (int l66 = 0; l66 < 4; l66 = l66 + 1) {
			fRec126[l66] = 0.0f;
		}
		for (int l67 = 0; l67 < 2048; l67 = l67 + 1) {
			fRec127[l67] = 0.0f;
		}
		for (int l68 = 0; l68 < 2; l68 = l68 + 1) {
			fVec13[l68] = 0.0f;
		}
		for (int l69 = 0; l69 < 2; l69 = l69 + 1) {
			fVec14[l69] = 0.0f;
		}
		for (int l70 = 0; l70 < 2; l70 = l70 + 1) {
			iRec129[l70] = 0;
		}
		for (int l71 = 0; l71 < 2; l71 = l71 + 1) {
			fVec15[l71] = 0.0f;
		}
		for (int l72 = 0; l72 < 2; l72 = l72 + 1) {
			fRec128[l72] = 0.0f;
		}
		for (int l73 = 0; l73 < 2; l73 = l73 + 1) {
			fVec16[l73] = 0.0f;
		}
		for (int l74 = 0; l74 < 2048; l74 = l74 + 1) {
			fRec125[l74] = 0.0f;
		}
		for (int l75 = 0; l75 < 2; l75 = l75 + 1) {
			fRec118[l75] = 0.0f;
		}
		for (int l76 = 0; l76 < 2048; l76 = l76 + 1) {
			fRec115[l76] = 0.0f;
		}
		for (int l77 = 0; l77 < 2; l77 = l77 + 1) {
			fRec117[l77] = 0.0f;
		}
		for (int l78 = 0; l78 < 4; l78 = l78 + 1) {
			fRec114[l78] = 0.0f;
		}
		for (int l79 = 0; l79 < 2; l79 = l79 + 1) {
			iRec105[l79] = 0;
		}
		for (int l80 = 0; l80 < 2048; l80 = l80 + 1) {
			fRec101[l80] = 0.0f;
		}
		for (int l81 = 0; l81 < 2; l81 = l81 + 1) {
			fRec99[l81] = 0.0f;
		}
		for (int l82 = 0; l82 < 2; l82 = l82 + 1) {
			iRec140[l82] = 0;
		}
		for (int l83 = 0; l83 < 2; l83 = l83 + 1) {
			fRec152[l83] = 0.0f;
		}
		for (int l84 = 0; l84 < 2; l84 = l84 + 1) {
			fRec155[l84] = 0.0f;
		}
		for (int l85 = 0; l85 < 4; l85 = l85 + 1) {
			fRec157[l85] = 0.0f;
		}
		for (int l86 = 0; l86 < 2048; l86 = l86 + 1) {
			fRec158[l86] = 0.0f;
		}
		for (int l87 = 0; l87 < 2; l87 = l87 + 1) {
			fVec17[l87] = 0.0f;
		}
		for (int l88 = 0; l88 < 2; l88 = l88 + 1) {
			fVec18[l88] = 0.0f;
		}
		for (int l89 = 0; l89 < 2; l89 = l89 + 1) {
			iRec160[l89] = 0;
		}
		for (int l90 = 0; l90 < 2; l90 = l90 + 1) {
			fVec19[l90] = 0.0f;
		}
		for (int l91 = 0; l91 < 2; l91 = l91 + 1) {
			fRec159[l91] = 0.0f;
		}
		for (int l92 = 0; l92 < 2; l92 = l92 + 1) {
			fVec20[l92] = 0.0f;
		}
		for (int l93 = 0; l93 < 2048; l93 = l93 + 1) {
			fRec156[l93] = 0.0f;
		}
		for (int l94 = 0; l94 < 2; l94 = l94 + 1) {
			fRec149[l94] = 0.0f;
		}
		for (int l95 = 0; l95 < 2048; l95 = l95 + 1) {
			fRec146[l95] = 0.0f;
		}
		for (int l96 = 0; l96 < 2; l96 = l96 + 1) {
			fRec148[l96] = 0.0f;
		}
		for (int l97 = 0; l97 < 4; l97 = l97 + 1) {
			fRec145[l97] = 0.0f;
		}
		for (int l98 = 0; l98 < 2; l98 = l98 + 1) {
			iRec136[l98] = 0;
		}
		for (int l99 = 0; l99 < 2048; l99 = l99 + 1) {
			fRec132[l99] = 0.0f;
		}
		for (int l100 = 0; l100 < 2; l100 = l100 + 1) {
			fRec130[l100] = 0.0f;
		}
		for (int l101 = 0; l101 < 2; l101 = l101 + 1) {
			iRec171[l101] = 0;
		}
		for (int l102 = 0; l102 < 2; l102 = l102 + 1) {
			fRec183[l102] = 0.0f;
		}
		for (int l103 = 0; l103 < 2; l103 = l103 + 1) {
			fRec186[l103] = 0.0f;
		}
		for (int l104 = 0; l104 < 4; l104 = l104 + 1) {
			fRec188[l104] = 0.0f;
		}
		for (int l105 = 0; l105 < 2048; l105 = l105 + 1) {
			fRec189[l105] = 0.0f;
		}
		for (int l106 = 0; l106 < 2; l106 = l106 + 1) {
			fVec21[l106] = 0.0f;
		}
		for (int l107 = 0; l107 < 2; l107 = l107 + 1) {
			fVec22[l107] = 0.0f;
		}
		for (int l108 = 0; l108 < 2; l108 = l108 + 1) {
			iRec191[l108] = 0;
		}
		for (int l109 = 0; l109 < 2; l109 = l109 + 1) {
			fVec23[l109] = 0.0f;
		}
		for (int l110 = 0; l110 < 2; l110 = l110 + 1) {
			fRec190[l110] = 0.0f;
		}
		for (int l111 = 0; l111 < 2; l111 = l111 + 1) {
			fVec24[l111] = 0.0f;
		}
		for (int l112 = 0; l112 < 2048; l112 = l112 + 1) {
			fRec187[l112] = 0.0f;
		}
		for (int l113 = 0; l113 < 2; l113 = l113 + 1) {
			fRec180[l113] = 0.0f;
		}
		for (int l114 = 0; l114 < 2048; l114 = l114 + 1) {
			fRec177[l114] = 0.0f;
		}
		for (int l115 = 0; l115 < 2; l115 = l115 + 1) {
			fRec179[l115] = 0.0f;
		}
		for (int l116 = 0; l116 < 4; l116 = l116 + 1) {
			fRec176[l116] = 0.0f;
		}
		for (int l117 = 0; l117 < 2; l117 = l117 + 1) {
			iRec167[l117] = 0;
		}
		for (int l118 = 0; l118 < 2048; l118 = l118 + 1) {
			fRec163[l118] = 0.0f;
		}
		for (int l119 = 0; l119 < 2; l119 = l119 + 1) {
			fRec161[l119] = 0.0f;
		}
		for (int l120 = 0; l120 < 3; l120 = l120 + 1) {
			fRec0[l120] = 0.0f;
		}
		for (int l121 = 0; l121 < 3; l121 = l121 + 1) {
			fRec192[l121] = 0.0f;
		}
		for (int l122 = 0; l122 < 3; l122 = l122 + 1) {
			fRec193[l122] = 0.0f;
		}
		for (int l123 = 0; l123 < 3; l123 = l123 + 1) {
			fRec194[l123] = 0.0f;
		}
		for (int l124 = 0; l124 < 3; l124 = l124 + 1) {
			fRec195[l124] = 0.0f;
		}
		for (int l125 = 0; l125 < 3; l125 = l125 + 1) {
			fRec196[l125] = 0.0f;
		}
		for (int l126 = 0; l126 < 3; l126 = l126 + 1) {
			fRec197[l126] = 0.0f;
		}
		for (int l127 = 0; l127 < 3; l127 = l127 + 1) {
			fRec198[l127] = 0.0f;
		}
		for (int l128 = 0; l128 < 3; l128 = l128 + 1) {
			fRec199[l128] = 0.0f;
		}
		for (int l129 = 0; l129 < 3; l129 = l129 + 1) {
			fRec200[l129] = 0.0f;
		}
		for (int l130 = 0; l130 < 3; l130 = l130 + 1) {
			fRec201[l130] = 0.0f;
		}
		for (int l131 = 0; l131 < 3; l131 = l131 + 1) {
			fRec202[l131] = 0.0f;
		}
		for (int l132 = 0; l132 < 3; l132 = l132 + 1) {
			fRec203[l132] = 0.0f;
		}
		for (int l133 = 0; l133 < 3; l133 = l133 + 1) {
			fRec204[l133] = 0.0f;
		}
		for (int l134 = 0; l134 < 3; l134 = l134 + 1) {
			fRec205[l134] = 0.0f;
		}
		for (int l135 = 0; l135 < 3; l135 = l135 + 1) {
			fRec206[l135] = 0.0f;
		}
		for (int l136 = 0; l136 < 3; l136 = l136 + 1) {
			fRec207[l136] = 0.0f;
		}
		for (int l137 = 0; l137 < 3; l137 = l137 + 1) {
			fRec208[l137] = 0.0f;
		}
		for (int l138 = 0; l138 < 3; l138 = l138 + 1) {
			fRec209[l138] = 0.0f;
		}
		for (int l139 = 0; l139 < 3; l139 = l139 + 1) {
			fRec210[l139] = 0.0f;
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
	
	virtual guitardsp* clone() {
		return new guitardsp(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("guitar_oud");
		ui_interface->addHorizontalSlider("AirResonance", &fHslider13, FAUSTFLOAT(1e+02f), FAUSTFLOAT(8e+01f), FAUSTFLOAT(1.5e+02f), FAUSTFLOAT(0.1f));
		ui_interface->addButton("Gate0", &fButton5);
		ui_interface->addButton("Gate1", &fButton4);
		ui_interface->addButton("Gate2", &fButton3);
		ui_interface->addButton("Gate3", &fButton2);
		ui_interface->addButton("Gate4", &fButton1);
		ui_interface->addButton("Gate5", &fButton0);
		ui_interface->addHorizontalSlider("NailFleshRatio", &fHslider3, FAUSTFLOAT(0.6f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("PluckHardness", &fHslider4, FAUSTFLOAT(0.8f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("PluckPos0", &fHslider10, FAUSTFLOAT(0.8f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("PluckPos1", &fHslider9, FAUSTFLOAT(0.8f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("PluckPos2", &fHslider8, FAUSTFLOAT(0.8f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("PluckPos3", &fHslider7, FAUSTFLOAT(0.8f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("PluckPos4", &fHslider6, FAUSTFLOAT(0.8f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("PluckPos5", &fHslider2, FAUSTFLOAT(0.8f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("SympatheticGain", &fHslider11, FAUSTFLOAT(0.01f), FAUSTFLOAT(0.0f), FAUSTFLOAT(0.05f), FAUSTFLOAT(0.0001f));
		ui_interface->addHorizontalSlider("TopResonance", &fHslider12, FAUSTFLOAT(2e+02f), FAUSTFLOAT(1.5e+02f), FAUSTFLOAT(3e+02f), FAUSTFLOAT(0.1f));
		ui_interface->addHorizontalSlider("WoundDamping", &fHslider0, FAUSTFLOAT(0.3f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("freq", &fHslider1, FAUSTFLOAT(4.4e+02f), FAUSTFLOAT(5e+01f), FAUSTFLOAT(1e+03f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("gain", &fHslider5, FAUSTFLOAT(0.8f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("outGain", &fHslider14, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* output0 = outputs[0];
		float fSlow0 = static_cast<float>(fHslider1);
		float fSlow1 = std::tan(fConst7 * fSlow0 * (9.0f - 6.0f * static_cast<float>(fHslider0)));
		float fSlow2 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fSlow1));
		float fSlow3 = 1.0f / fSlow1;
		float fSlow4 = (fSlow3 + -1.4142135f) / fSlow1 + 1.0f;
		float fSlow5 = 1.0f / ((fSlow3 + 1.4142135f) / fSlow1 + 1.0f);
		float fSlow6 = 3.4e+02f / fSlow0 + -0.11f;
		float fSlow7 = static_cast<float>(fHslider2);
		float fSlow8 = fConst9 * (1.0f - fSlow7) * fSlow6;
		float fSlow9 = fSlow8 + -1.499995f;
		int iSlow10 = static_cast<int>(fSlow9);
		int iSlow11 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow10 + 4))));
		int iSlow12 = iSlow11 + 1;
		float fSlow13 = std::floor(fSlow9);
		float fSlow14 = fSlow8 + (-3.0f - fSlow13);
		float fSlow15 = fSlow8 + (-2.0f - fSlow13);
		float fSlow16 = fSlow8 + (-1.0f - fSlow13);
		float fSlow17 = fSlow8 - fSlow13;
		float fSlow18 = fSlow17 * fSlow16;
		float fSlow19 = fSlow18 * fSlow15;
		float fSlow20 = 0.041666668f * fSlow19 * fSlow14;
		int iSlow21 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow10 + 3))));
		int iSlow22 = iSlow21 + 1;
		float fSlow23 = 0.16666667f * fSlow19;
		int iSlow24 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow10 + 2))));
		int iSlow25 = iSlow24 + 1;
		float fSlow26 = 0.25f * fSlow18;
		int iSlow27 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow10 + 1))));
		int iSlow28 = iSlow27 + 1;
		float fSlow29 = 0.16666667f * fSlow17;
		int iSlow30 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow10))));
		int iSlow31 = iSlow30 + 1;
		float fSlow32 = 0.041666668f * fSlow16;
		float fSlow33 = fSlow8 + (-4.0f - fSlow13);
		float fSlow34 = fConst9 * fSlow7 * fSlow6;
		float fSlow35 = fSlow34 + -1.499995f;
		int iSlow36 = static_cast<int>(fSlow35);
		int iSlow37 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow36 + 4))));
		int iSlow38 = iSlow37 + 2;
		float fSlow39 = std::floor(fSlow35);
		float fSlow40 = fSlow34 + (-3.0f - fSlow39);
		float fSlow41 = fSlow34 + (-2.0f - fSlow39);
		float fSlow42 = fSlow34 + (-1.0f - fSlow39);
		float fSlow43 = fSlow34 - fSlow39;
		float fSlow44 = fSlow43 * fSlow42;
		float fSlow45 = fSlow44 * fSlow41;
		float fSlow46 = 0.041666668f * fSlow45 * fSlow40;
		int iSlow47 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow36 + 3))));
		int iSlow48 = iSlow47 + 2;
		float fSlow49 = 0.16666667f * fSlow45;
		int iSlow50 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow36 + 2))));
		int iSlow51 = iSlow50 + 2;
		float fSlow52 = 0.25f * fSlow44;
		int iSlow53 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow36 + 1))));
		int iSlow54 = iSlow53 + 2;
		float fSlow55 = 0.16666667f * fSlow43;
		int iSlow56 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow36))));
		int iSlow57 = iSlow56 + 2;
		float fSlow58 = 0.041666668f * fSlow42;
		float fSlow59 = fSlow34 + (-4.0f - fSlow39);
		float fSlow60 = static_cast<float>(fButton0);
		float fSlow61 = static_cast<float>(fHslider3);
		float fSlow62 = static_cast<float>(fHslider4);
		float fSlow63 = 1.0f / std::max<float>(1.0f, fConst10 * (0.2f * fSlow62 + 0.65f * fSlow61 + 0.15f) * guitardsp_faustpower2_f(1.0f - 0.0005f * (fSlow0 / (0.3f * fSlow62 + 0.7f * fSlow61 + 0.5f))));
		float fSlow64 = std::tan(fConst11 * fSlow0 * (0.25f * (fSlow62 + 1.0f) + 0.5f * fSlow61));
		float fSlow65 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fSlow64));
		float fSlow66 = 1.0f / fSlow64;
		float fSlow67 = (fSlow66 + -1.4142135f) / fSlow64 + 1.0f;
		float fSlow68 = (fSlow66 + 1.4142135f) / fSlow64 + 1.0f;
		float fSlow69 = 1.0f / fSlow68;
		float fSlow70 = static_cast<float>(fHslider5) * (0.8f * (1.0f - fSlow61) + 1.5f) / fSlow68;
		float fSlow71 = 1.0f / std::tan(fConst7 * (8e+02f * fSlow61 + 2e+02f));
		float fSlow72 = 1.0f - fSlow71;
		float fSlow73 = 1.0f / (fSlow71 + 1.0f);
		int iSlow74 = iSlow37 + 1;
		int iSlow75 = iSlow47 + 1;
		int iSlow76 = iSlow50 + 1;
		int iSlow77 = iSlow53 + 1;
		int iSlow78 = iSlow56 + 1;
		float fSlow79 = static_cast<float>(fHslider6);
		float fSlow80 = fConst9 * (1.0f - fSlow79) * fSlow6;
		float fSlow81 = fSlow80 + -1.499995f;
		int iSlow82 = static_cast<int>(fSlow81);
		int iSlow83 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow82 + 4))));
		int iSlow84 = iSlow83 + 1;
		float fSlow85 = std::floor(fSlow81);
		float fSlow86 = fSlow80 + (-3.0f - fSlow85);
		float fSlow87 = fSlow80 + (-2.0f - fSlow85);
		float fSlow88 = fSlow80 + (-1.0f - fSlow85);
		float fSlow89 = fSlow80 - fSlow85;
		float fSlow90 = fSlow89 * fSlow88;
		float fSlow91 = fSlow90 * fSlow87;
		float fSlow92 = 0.041666668f * fSlow91 * fSlow86;
		int iSlow93 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow82 + 3))));
		int iSlow94 = iSlow93 + 1;
		float fSlow95 = 0.16666667f * fSlow91;
		int iSlow96 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow82 + 2))));
		int iSlow97 = iSlow96 + 1;
		float fSlow98 = 0.25f * fSlow90;
		int iSlow99 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow82 + 1))));
		int iSlow100 = iSlow99 + 1;
		float fSlow101 = 0.16666667f * fSlow89;
		int iSlow102 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow82))));
		int iSlow103 = iSlow102 + 1;
		float fSlow104 = 0.041666668f * fSlow88;
		float fSlow105 = fSlow80 + (-4.0f - fSlow85);
		float fSlow106 = fConst9 * fSlow79 * fSlow6;
		float fSlow107 = fSlow106 + -1.499995f;
		int iSlow108 = static_cast<int>(fSlow107);
		int iSlow109 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow108 + 4))));
		int iSlow110 = iSlow109 + 2;
		float fSlow111 = std::floor(fSlow107);
		float fSlow112 = fSlow106 + (-3.0f - fSlow111);
		float fSlow113 = fSlow106 + (-2.0f - fSlow111);
		float fSlow114 = fSlow106 + (-1.0f - fSlow111);
		float fSlow115 = fSlow106 - fSlow111;
		float fSlow116 = fSlow115 * fSlow114;
		float fSlow117 = fSlow116 * fSlow113;
		float fSlow118 = 0.041666668f * fSlow117 * fSlow112;
		int iSlow119 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow108 + 3))));
		int iSlow120 = iSlow119 + 2;
		float fSlow121 = 0.16666667f * fSlow117;
		int iSlow122 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow108 + 2))));
		int iSlow123 = iSlow122 + 2;
		float fSlow124 = 0.25f * fSlow116;
		int iSlow125 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow108 + 1))));
		int iSlow126 = iSlow125 + 2;
		float fSlow127 = 0.16666667f * fSlow115;
		int iSlow128 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow108))));
		int iSlow129 = iSlow128 + 2;
		float fSlow130 = 0.041666668f * fSlow114;
		float fSlow131 = fSlow106 + (-4.0f - fSlow111);
		float fSlow132 = static_cast<float>(fButton1);
		int iSlow133 = iSlow109 + 1;
		int iSlow134 = iSlow119 + 1;
		int iSlow135 = iSlow122 + 1;
		int iSlow136 = iSlow125 + 1;
		int iSlow137 = iSlow128 + 1;
		float fSlow138 = static_cast<float>(fHslider7);
		float fSlow139 = fConst9 * (1.0f - fSlow138) * fSlow6;
		float fSlow140 = fSlow139 + -1.499995f;
		int iSlow141 = static_cast<int>(fSlow140);
		int iSlow142 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow141 + 4))));
		int iSlow143 = iSlow142 + 1;
		float fSlow144 = std::floor(fSlow140);
		float fSlow145 = fSlow139 + (-3.0f - fSlow144);
		float fSlow146 = fSlow139 + (-2.0f - fSlow144);
		float fSlow147 = fSlow139 + (-1.0f - fSlow144);
		float fSlow148 = fSlow139 - fSlow144;
		float fSlow149 = fSlow148 * fSlow147;
		float fSlow150 = fSlow149 * fSlow146;
		float fSlow151 = 0.041666668f * fSlow150 * fSlow145;
		int iSlow152 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow141 + 3))));
		int iSlow153 = iSlow152 + 1;
		float fSlow154 = 0.16666667f * fSlow150;
		int iSlow155 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow141 + 2))));
		int iSlow156 = iSlow155 + 1;
		float fSlow157 = 0.25f * fSlow149;
		int iSlow158 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow141 + 1))));
		int iSlow159 = iSlow158 + 1;
		float fSlow160 = 0.16666667f * fSlow148;
		int iSlow161 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow141))));
		int iSlow162 = iSlow161 + 1;
		float fSlow163 = 0.041666668f * fSlow147;
		float fSlow164 = fSlow139 + (-4.0f - fSlow144);
		float fSlow165 = fConst9 * fSlow138 * fSlow6;
		float fSlow166 = fSlow165 + -1.499995f;
		int iSlow167 = static_cast<int>(fSlow166);
		int iSlow168 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow167 + 4))));
		int iSlow169 = iSlow168 + 2;
		float fSlow170 = std::floor(fSlow166);
		float fSlow171 = fSlow165 + (-3.0f - fSlow170);
		float fSlow172 = fSlow165 + (-2.0f - fSlow170);
		float fSlow173 = fSlow165 + (-1.0f - fSlow170);
		float fSlow174 = fSlow165 - fSlow170;
		float fSlow175 = fSlow174 * fSlow173;
		float fSlow176 = fSlow175 * fSlow172;
		float fSlow177 = 0.041666668f * fSlow176 * fSlow171;
		int iSlow178 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow167 + 3))));
		int iSlow179 = iSlow178 + 2;
		float fSlow180 = 0.16666667f * fSlow176;
		int iSlow181 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow167 + 2))));
		int iSlow182 = iSlow181 + 2;
		float fSlow183 = 0.25f * fSlow175;
		int iSlow184 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow167 + 1))));
		int iSlow185 = iSlow184 + 2;
		float fSlow186 = 0.16666667f * fSlow174;
		int iSlow187 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow167))));
		int iSlow188 = iSlow187 + 2;
		float fSlow189 = 0.041666668f * fSlow173;
		float fSlow190 = fSlow165 + (-4.0f - fSlow170);
		float fSlow191 = static_cast<float>(fButton2);
		int iSlow192 = iSlow168 + 1;
		int iSlow193 = iSlow178 + 1;
		int iSlow194 = iSlow181 + 1;
		int iSlow195 = iSlow184 + 1;
		int iSlow196 = iSlow187 + 1;
		float fSlow197 = static_cast<float>(fHslider8);
		float fSlow198 = fConst9 * (1.0f - fSlow197) * fSlow6;
		float fSlow199 = fSlow198 + -1.499995f;
		int iSlow200 = static_cast<int>(fSlow199);
		int iSlow201 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow200 + 4))));
		int iSlow202 = iSlow201 + 1;
		float fSlow203 = std::floor(fSlow199);
		float fSlow204 = fSlow198 + (-3.0f - fSlow203);
		float fSlow205 = fSlow198 + (-2.0f - fSlow203);
		float fSlow206 = fSlow198 + (-1.0f - fSlow203);
		float fSlow207 = fSlow198 - fSlow203;
		float fSlow208 = fSlow207 * fSlow206;
		float fSlow209 = fSlow208 * fSlow205;
		float fSlow210 = 0.041666668f * fSlow209 * fSlow204;
		int iSlow211 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow200 + 3))));
		int iSlow212 = iSlow211 + 1;
		float fSlow213 = 0.16666667f * fSlow209;
		int iSlow214 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow200 + 2))));
		int iSlow215 = iSlow214 + 1;
		float fSlow216 = 0.25f * fSlow208;
		int iSlow217 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow200 + 1))));
		int iSlow218 = iSlow217 + 1;
		float fSlow219 = 0.16666667f * fSlow207;
		int iSlow220 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow200))));
		int iSlow221 = iSlow220 + 1;
		float fSlow222 = 0.041666668f * fSlow206;
		float fSlow223 = fSlow198 + (-4.0f - fSlow203);
		float fSlow224 = fConst9 * fSlow197 * fSlow6;
		float fSlow225 = fSlow224 + -1.499995f;
		int iSlow226 = static_cast<int>(fSlow225);
		int iSlow227 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow226 + 4))));
		int iSlow228 = iSlow227 + 2;
		float fSlow229 = std::floor(fSlow225);
		float fSlow230 = fSlow224 + (-3.0f - fSlow229);
		float fSlow231 = fSlow224 + (-2.0f - fSlow229);
		float fSlow232 = fSlow224 + (-1.0f - fSlow229);
		float fSlow233 = fSlow224 - fSlow229;
		float fSlow234 = fSlow233 * fSlow232;
		float fSlow235 = fSlow234 * fSlow231;
		float fSlow236 = 0.041666668f * fSlow235 * fSlow230;
		int iSlow237 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow226 + 3))));
		int iSlow238 = iSlow237 + 2;
		float fSlow239 = 0.16666667f * fSlow235;
		int iSlow240 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow226 + 2))));
		int iSlow241 = iSlow240 + 2;
		float fSlow242 = 0.25f * fSlow234;
		int iSlow243 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow226 + 1))));
		int iSlow244 = iSlow243 + 2;
		float fSlow245 = 0.16666667f * fSlow233;
		int iSlow246 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow226))));
		int iSlow247 = iSlow246 + 2;
		float fSlow248 = 0.041666668f * fSlow232;
		float fSlow249 = fSlow224 + (-4.0f - fSlow229);
		float fSlow250 = static_cast<float>(fButton3);
		int iSlow251 = iSlow227 + 1;
		int iSlow252 = iSlow237 + 1;
		int iSlow253 = iSlow240 + 1;
		int iSlow254 = iSlow243 + 1;
		int iSlow255 = iSlow246 + 1;
		float fSlow256 = static_cast<float>(fHslider9);
		float fSlow257 = fConst9 * (1.0f - fSlow256) * fSlow6;
		float fSlow258 = fSlow257 + -1.499995f;
		int iSlow259 = static_cast<int>(fSlow258);
		int iSlow260 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow259 + 4))));
		int iSlow261 = iSlow260 + 1;
		float fSlow262 = std::floor(fSlow258);
		float fSlow263 = fSlow257 + (-3.0f - fSlow262);
		float fSlow264 = fSlow257 + (-2.0f - fSlow262);
		float fSlow265 = fSlow257 + (-1.0f - fSlow262);
		float fSlow266 = fSlow257 - fSlow262;
		float fSlow267 = fSlow266 * fSlow265;
		float fSlow268 = fSlow267 * fSlow264;
		float fSlow269 = 0.041666668f * fSlow268 * fSlow263;
		int iSlow270 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow259 + 3))));
		int iSlow271 = iSlow270 + 1;
		float fSlow272 = 0.16666667f * fSlow268;
		int iSlow273 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow259 + 2))));
		int iSlow274 = iSlow273 + 1;
		float fSlow275 = 0.25f * fSlow267;
		int iSlow276 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow259 + 1))));
		int iSlow277 = iSlow276 + 1;
		float fSlow278 = 0.16666667f * fSlow266;
		int iSlow279 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow259))));
		int iSlow280 = iSlow279 + 1;
		float fSlow281 = 0.041666668f * fSlow265;
		float fSlow282 = fSlow257 + (-4.0f - fSlow262);
		float fSlow283 = fConst9 * fSlow256 * fSlow6;
		float fSlow284 = fSlow283 + -1.499995f;
		int iSlow285 = static_cast<int>(fSlow284);
		int iSlow286 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow285 + 4))));
		int iSlow287 = iSlow286 + 2;
		float fSlow288 = std::floor(fSlow284);
		float fSlow289 = fSlow283 + (-3.0f - fSlow288);
		float fSlow290 = fSlow283 + (-2.0f - fSlow288);
		float fSlow291 = fSlow283 + (-1.0f - fSlow288);
		float fSlow292 = fSlow283 - fSlow288;
		float fSlow293 = fSlow292 * fSlow291;
		float fSlow294 = fSlow293 * fSlow290;
		float fSlow295 = 0.041666668f * fSlow294 * fSlow289;
		int iSlow296 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow285 + 3))));
		int iSlow297 = iSlow296 + 2;
		float fSlow298 = 0.16666667f * fSlow294;
		int iSlow299 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow285 + 2))));
		int iSlow300 = iSlow299 + 2;
		float fSlow301 = 0.25f * fSlow293;
		int iSlow302 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow285 + 1))));
		int iSlow303 = iSlow302 + 2;
		float fSlow304 = 0.16666667f * fSlow292;
		int iSlow305 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow285))));
		int iSlow306 = iSlow305 + 2;
		float fSlow307 = 0.041666668f * fSlow291;
		float fSlow308 = fSlow283 + (-4.0f - fSlow288);
		float fSlow309 = static_cast<float>(fButton4);
		int iSlow310 = iSlow286 + 1;
		int iSlow311 = iSlow296 + 1;
		int iSlow312 = iSlow299 + 1;
		int iSlow313 = iSlow302 + 1;
		int iSlow314 = iSlow305 + 1;
		float fSlow315 = static_cast<float>(fHslider10);
		float fSlow316 = fConst9 * (1.0f - fSlow315) * fSlow6;
		float fSlow317 = fSlow316 + -1.499995f;
		int iSlow318 = static_cast<int>(fSlow317);
		int iSlow319 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow318 + 4))));
		int iSlow320 = iSlow319 + 1;
		float fSlow321 = std::floor(fSlow317);
		float fSlow322 = fSlow316 + (-3.0f - fSlow321);
		float fSlow323 = fSlow316 + (-2.0f - fSlow321);
		float fSlow324 = fSlow316 + (-1.0f - fSlow321);
		float fSlow325 = fSlow316 - fSlow321;
		float fSlow326 = fSlow325 * fSlow324;
		float fSlow327 = fSlow326 * fSlow323;
		float fSlow328 = 0.041666668f * fSlow327 * fSlow322;
		int iSlow329 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow318 + 3))));
		int iSlow330 = iSlow329 + 1;
		float fSlow331 = 0.16666667f * fSlow327;
		int iSlow332 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow318 + 2))));
		int iSlow333 = iSlow332 + 1;
		float fSlow334 = 0.25f * fSlow326;
		int iSlow335 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow318 + 1))));
		int iSlow336 = iSlow335 + 1;
		float fSlow337 = 0.16666667f * fSlow325;
		int iSlow338 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow318))));
		int iSlow339 = iSlow338 + 1;
		float fSlow340 = 0.041666668f * fSlow324;
		float fSlow341 = fSlow316 + (-4.0f - fSlow321);
		float fSlow342 = fConst9 * fSlow315 * fSlow6;
		float fSlow343 = fSlow342 + -1.499995f;
		int iSlow344 = static_cast<int>(fSlow343);
		int iSlow345 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow344 + 4))));
		int iSlow346 = iSlow345 + 2;
		float fSlow347 = std::floor(fSlow343);
		float fSlow348 = fSlow342 + (-3.0f - fSlow347);
		float fSlow349 = fSlow342 + (-2.0f - fSlow347);
		float fSlow350 = fSlow342 + (-1.0f - fSlow347);
		float fSlow351 = fSlow342 - fSlow347;
		float fSlow352 = fSlow351 * fSlow350;
		float fSlow353 = fSlow352 * fSlow349;
		float fSlow354 = 0.041666668f * fSlow353 * fSlow348;
		int iSlow355 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow344 + 3))));
		int iSlow356 = iSlow355 + 2;
		float fSlow357 = 0.16666667f * fSlow353;
		int iSlow358 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow344 + 2))));
		int iSlow359 = iSlow358 + 2;
		float fSlow360 = 0.25f * fSlow352;
		int iSlow361 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow344 + 1))));
		int iSlow362 = iSlow361 + 2;
		float fSlow363 = 0.16666667f * fSlow351;
		int iSlow364 = static_cast<int>(std::min<float>(fConst8, static_cast<float>(std::max<int>(0, iSlow344))));
		int iSlow365 = iSlow364 + 2;
		float fSlow366 = 0.041666668f * fSlow350;
		float fSlow367 = fSlow342 + (-4.0f - fSlow347);
		float fSlow368 = static_cast<float>(fButton5);
		int iSlow369 = iSlow345 + 1;
		int iSlow370 = iSlow355 + 1;
		int iSlow371 = iSlow358 + 1;
		int iSlow372 = iSlow361 + 1;
		int iSlow373 = iSlow364 + 1;
		float fSlow374 = static_cast<float>(fHslider11);
		float fSlow375 = static_cast<float>(fHslider12);
		float fSlow376 = std::tan(fConst97 * fSlow375);
		float fSlow377 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fSlow376));
		float fSlow378 = 1.0f / fSlow376;
		float fSlow379 = (fSlow378 + -0.1f) / fSlow376 + 1.0f;
		float fSlow380 = (fSlow378 + 0.1f) / fSlow376 + 1.0f;
		float fSlow381 = 1.0f / fSlow380;
		float fSlow382 = 0.035f / (fSlow376 * fSlow380);
		float fSlow383 = std::tan(fConst7 * fSlow375);
		float fSlow384 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fSlow383));
		float fSlow385 = 1.0f / fSlow383;
		float fSlow386 = (fSlow385 + -0.06666667f) / fSlow383 + 1.0f;
		float fSlow387 = (fSlow385 + 0.06666667f) / fSlow383 + 1.0f;
		float fSlow388 = 1.0f / fSlow387;
		float fSlow389 = 1.0f / (fSlow383 * fSlow387);
		float fSlow390 = std::tan(fConst7 * static_cast<float>(fHslider13));
		float fSlow391 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fSlow390));
		float fSlow392 = 1.0f / fSlow390;
		float fSlow393 = (fSlow392 + -0.05f) / fSlow390 + 1.0f;
		float fSlow394 = (fSlow392 + 0.05f) / fSlow390 + 1.0f;
		float fSlow395 = 1.0f / fSlow394;
		float fSlow396 = 1.0f / (fSlow390 * fSlow394);
		float fSlow397 = std::tan(fConst105 * fSlow375);
		float fSlow398 = 2.0f * (1.0f - 1.0f / guitardsp_faustpower2_f(fSlow397));
		float fSlow399 = 1.0f / fSlow397;
		float fSlow400 = (fSlow399 + -0.083333336f) / fSlow397 + 1.0f;
		float fSlow401 = (fSlow399 + 0.083333336f) / fSlow397 + 1.0f;
		float fSlow402 = 1.0f / fSlow401;
		float fSlow403 = 1.0f / (fSlow397 * fSlow401);
		float fSlow404 = static_cast<float>(fHslider14);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			fVec0[0] = static_cast<float>(input0[i0]);
			iRec12[0] = 0;
			int iRec13 = iRec12[1];
			float fRec16 = static_cast<float>(iRec8[1]) - 0.9978437f * (0.7f * fRec17[2] + 0.15f * (fRec17[1] + fRec17[3]));
			fRec24[0] = fSlow33 * (fSlow14 * (fSlow15 * (fSlow32 * fRec4[(IOTA0 - iSlow31) & 2047] - fSlow29 * fRec4[(IOTA0 - iSlow28) & 2047]) + fSlow26 * fRec4[(IOTA0 - iSlow25) & 2047]) - fSlow23 * fRec4[(IOTA0 - iSlow22) & 2047]) + fSlow20 * fRec4[(IOTA0 - iSlow12) & 2047];
			fRec27[0] = 0.6f * fRec24[1] + 0.4f * fRec27[1];
			float fRec25 = fRec27[0];
			fRec29[0] = fRec2[1];
			fRec30[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec29[2] + 0.15f * (fRec29[1] + fRec29[3])));
			fVec1[0] = fSlow59 * (fSlow40 * (fSlow41 * (fSlow58 * fRec30[(IOTA0 - iSlow57) & 2047] - fSlow55 * fRec30[(IOTA0 - iSlow54) & 2047]) + fSlow52 * fRec30[(IOTA0 - iSlow51) & 2047]) - fSlow49 * fRec30[(IOTA0 - iSlow48) & 2047]) + fSlow46 * fRec30[(IOTA0 - iSlow38) & 2047];
			fVec2[0] = fSlow60;
			iRec32[0] = (fSlow60 > fVec2[1]) + (fSlow60 <= fVec2[1]) * (iRec32[1] + (iRec32[1] > 0));
			float fTemp0 = fSlow63 * static_cast<float>(iRec32[0]);
			iRec34[0] = 1103515245 * iRec34[1] + 12345;
			fRec33[0] = 7.0e-8f * static_cast<float>(iRec34[0]) - fSlow69 * (fSlow67 * fRec33[2] + fSlow65 * fRec33[1]);
			float fTemp1 = fRec33[2] + fRec33[0] + 2.0f * fRec33[1];
			float fTemp2 = fSlow70 * fTemp1 * std::max<float>(0.0f, std::min<float>(fTemp0, 2.0f - fTemp0));
			fVec3[0] = fTemp2;
			fRec31[0] = -(fSlow73 * (fSlow72 * fRec31[1] - (fTemp2 + fVec3[1])));
			fVec4[0] = fRec31[0] + fVec1[1];
			fRec28[IOTA0 & 2047] = 0.6f * fVec4[1] + 0.4f * fRec28[(IOTA0 - 1) & 2047];
			float fRec26 = fSlow33 * (fSlow14 * (fSlow15 * (fSlow32 * fRec28[(IOTA0 - iSlow30) & 2047] - fSlow29 * fRec28[(IOTA0 - iSlow27) & 2047]) + fSlow26 * fRec28[(IOTA0 - iSlow24) & 2047]) - fSlow23 * fRec28[(IOTA0 - iSlow21) & 2047]) + fSlow20 * fRec28[(IOTA0 - iSlow11) & 2047];
			fRec21[0] = fRec25;
			float fRec22 = fRec31[0] + fRec21[1];
			float fRec23 = fRec26;
			fRec18[IOTA0 & 2047] = fRec22;
			float fRec19 = fSlow59 * (fSlow40 * (fSlow41 * (fSlow58 * fRec18[(IOTA0 - iSlow78) & 2047] - fSlow55 * fRec18[(IOTA0 - iSlow77) & 2047]) + fSlow52 * fRec18[(IOTA0 - iSlow76) & 2047]) - fSlow49 * fRec18[(IOTA0 - iSlow75) & 2047]) + fSlow46 * fRec18[(IOTA0 - iSlow74) & 2047];
			fRec20[0] = fRec23;
			fRec17[0] = fRec20[1];
			float fRec14 = fRec17[1];
			float fRec15 = fRec17[1];
			iRec8[0] = iRec13;
			float fRec9 = fRec16;
			float fRec10 = fRec14;
			float fRec11 = fRec15;
			fRec4[IOTA0 & 2047] = fRec9;
			float fRec5 = fRec19;
			float fRec6 = fRec10;
			float fRec7 = fRec11;
			fRec2[0] = fRec5;
			float fRec3 = fRec7;
			fRec1[0] = fRec3 - fSlow5 * (fSlow4 * fRec1[2] + fSlow2 * fRec1[1]);
			iRec46[0] = 0;
			int iRec47 = iRec46[1];
			float fRec50 = static_cast<float>(iRec42[1]) - 0.9978437f * (0.7f * fRec51[2] + 0.15f * (fRec51[1] + fRec51[3]));
			fRec58[0] = fSlow105 * (fSlow86 * (fSlow87 * (fSlow104 * fRec38[(IOTA0 - iSlow103) & 2047] - fSlow101 * fRec38[(IOTA0 - iSlow100) & 2047]) + fSlow98 * fRec38[(IOTA0 - iSlow97) & 2047]) - fSlow95 * fRec38[(IOTA0 - iSlow94) & 2047]) + fSlow92 * fRec38[(IOTA0 - iSlow84) & 2047];
			fRec61[0] = 0.6f * fRec58[1] + 0.4f * fRec61[1];
			float fRec59 = fRec61[0];
			fRec63[0] = fRec36[1];
			fRec64[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec63[2] + 0.15f * (fRec63[1] + fRec63[3])));
			fVec5[0] = fSlow131 * (fSlow112 * (fSlow113 * (fSlow130 * fRec64[(IOTA0 - iSlow129) & 2047] - fSlow127 * fRec64[(IOTA0 - iSlow126) & 2047]) + fSlow124 * fRec64[(IOTA0 - iSlow123) & 2047]) - fSlow121 * fRec64[(IOTA0 - iSlow120) & 2047]) + fSlow118 * fRec64[(IOTA0 - iSlow110) & 2047];
			fVec6[0] = fSlow132;
			iRec66[0] = (fSlow132 > fVec6[1]) + (fSlow132 <= fVec6[1]) * (iRec66[1] + (iRec66[1] > 0));
			float fTemp3 = fSlow63 * static_cast<float>(iRec66[0]);
			float fTemp4 = fSlow70 * fTemp1 * std::max<float>(0.0f, std::min<float>(fTemp3, 2.0f - fTemp3));
			fVec7[0] = fTemp4;
			fRec65[0] = -(fSlow73 * (fSlow72 * fRec65[1] - (fTemp4 + fVec7[1])));
			fVec8[0] = fRec65[0] + fVec5[1];
			fRec62[IOTA0 & 2047] = 0.6f * fVec8[1] + 0.4f * fRec62[(IOTA0 - 1) & 2047];
			float fRec60 = fSlow105 * (fSlow86 * (fSlow87 * (fSlow104 * fRec62[(IOTA0 - iSlow102) & 2047] - fSlow101 * fRec62[(IOTA0 - iSlow99) & 2047]) + fSlow98 * fRec62[(IOTA0 - iSlow96) & 2047]) - fSlow95 * fRec62[(IOTA0 - iSlow93) & 2047]) + fSlow92 * fRec62[(IOTA0 - iSlow83) & 2047];
			fRec55[0] = fRec59;
			float fRec56 = fRec65[0] + fRec55[1];
			float fRec57 = fRec60;
			fRec52[IOTA0 & 2047] = fRec56;
			float fRec53 = fSlow131 * (fSlow112 * (fSlow113 * (fSlow130 * fRec52[(IOTA0 - iSlow137) & 2047] - fSlow127 * fRec52[(IOTA0 - iSlow136) & 2047]) + fSlow124 * fRec52[(IOTA0 - iSlow135) & 2047]) - fSlow121 * fRec52[(IOTA0 - iSlow134) & 2047]) + fSlow118 * fRec52[(IOTA0 - iSlow133) & 2047];
			fRec54[0] = fRec57;
			fRec51[0] = fRec54[1];
			float fRec48 = fRec51[1];
			float fRec49 = fRec51[1];
			iRec42[0] = iRec47;
			float fRec43 = fRec50;
			float fRec44 = fRec48;
			float fRec45 = fRec49;
			fRec38[IOTA0 & 2047] = fRec43;
			float fRec39 = fRec53;
			float fRec40 = fRec44;
			float fRec41 = fRec45;
			fRec36[0] = fRec39;
			float fRec37 = fRec41;
			fRec35[0] = fRec37 - fSlow5 * (fSlow4 * fRec35[2] + fSlow2 * fRec35[1]);
			iRec78[0] = 0;
			int iRec79 = iRec78[1];
			float fRec82 = static_cast<float>(iRec74[1]) - 0.9978437f * (0.7f * fRec83[2] + 0.15f * (fRec83[1] + fRec83[3]));
			fRec90[0] = fSlow164 * (fSlow145 * (fSlow146 * (fSlow163 * fRec70[(IOTA0 - iSlow162) & 2047] - fSlow160 * fRec70[(IOTA0 - iSlow159) & 2047]) + fSlow157 * fRec70[(IOTA0 - iSlow156) & 2047]) - fSlow154 * fRec70[(IOTA0 - iSlow153) & 2047]) + fSlow151 * fRec70[(IOTA0 - iSlow143) & 2047];
			fRec93[0] = 0.6f * fRec90[1] + 0.4f * fRec93[1];
			float fRec91 = fRec93[0];
			fRec95[0] = fRec68[1];
			fRec96[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec95[2] + 0.15f * (fRec95[1] + fRec95[3])));
			fVec9[0] = fSlow190 * (fSlow171 * (fSlow172 * (fSlow189 * fRec96[(IOTA0 - iSlow188) & 2047] - fSlow186 * fRec96[(IOTA0 - iSlow185) & 2047]) + fSlow183 * fRec96[(IOTA0 - iSlow182) & 2047]) - fSlow180 * fRec96[(IOTA0 - iSlow179) & 2047]) + fSlow177 * fRec96[(IOTA0 - iSlow169) & 2047];
			fVec10[0] = fSlow191;
			iRec98[0] = (fSlow191 > fVec10[1]) + (fSlow191 <= fVec10[1]) * (iRec98[1] + (iRec98[1] > 0));
			float fTemp5 = fSlow63 * static_cast<float>(iRec98[0]);
			float fTemp6 = fSlow70 * fTemp1 * std::max<float>(0.0f, std::min<float>(fTemp5, 2.0f - fTemp5));
			fVec11[0] = fTemp6;
			fRec97[0] = -(fSlow73 * (fSlow72 * fRec97[1] - (fTemp6 + fVec11[1])));
			fVec12[0] = fRec97[0] + fVec9[1];
			fRec94[IOTA0 & 2047] = 0.6f * fVec12[1] + 0.4f * fRec94[(IOTA0 - 1) & 2047];
			float fRec92 = fSlow164 * (fSlow145 * (fSlow146 * (fSlow163 * fRec94[(IOTA0 - iSlow161) & 2047] - fSlow160 * fRec94[(IOTA0 - iSlow158) & 2047]) + fSlow157 * fRec94[(IOTA0 - iSlow155) & 2047]) - fSlow154 * fRec94[(IOTA0 - iSlow152) & 2047]) + fSlow151 * fRec94[(IOTA0 - iSlow142) & 2047];
			fRec87[0] = fRec91;
			float fRec88 = fRec97[0] + fRec87[1];
			float fRec89 = fRec92;
			fRec84[IOTA0 & 2047] = fRec88;
			float fRec85 = fSlow190 * (fSlow171 * (fSlow172 * (fSlow189 * fRec84[(IOTA0 - iSlow196) & 2047] - fSlow186 * fRec84[(IOTA0 - iSlow195) & 2047]) + fSlow183 * fRec84[(IOTA0 - iSlow194) & 2047]) - fSlow180 * fRec84[(IOTA0 - iSlow193) & 2047]) + fSlow177 * fRec84[(IOTA0 - iSlow192) & 2047];
			fRec86[0] = fRec89;
			fRec83[0] = fRec86[1];
			float fRec80 = fRec83[1];
			float fRec81 = fRec83[1];
			iRec74[0] = iRec79;
			float fRec75 = fRec82;
			float fRec76 = fRec80;
			float fRec77 = fRec81;
			fRec70[IOTA0 & 2047] = fRec75;
			float fRec71 = fRec85;
			float fRec72 = fRec76;
			float fRec73 = fRec77;
			fRec68[0] = fRec71;
			float fRec69 = fRec73;
			fRec67[0] = fRec69 - fSlow5 * (fSlow4 * fRec67[2] + fSlow2 * fRec67[1]);
			iRec109[0] = 0;
			int iRec110 = iRec109[1];
			float fRec113 = static_cast<float>(iRec105[1]) - 0.9978437f * (0.7f * fRec114[2] + 0.15f * (fRec114[1] + fRec114[3]));
			fRec121[0] = fSlow223 * (fSlow204 * (fSlow205 * (fSlow222 * fRec101[(IOTA0 - iSlow221) & 2047] - fSlow219 * fRec101[(IOTA0 - iSlow218) & 2047]) + fSlow216 * fRec101[(IOTA0 - iSlow215) & 2047]) - fSlow213 * fRec101[(IOTA0 - iSlow212) & 2047]) + fSlow210 * fRec101[(IOTA0 - iSlow202) & 2047];
			fRec124[0] = 0.6f * fRec121[1] + 0.4f * fRec124[1];
			float fRec122 = fRec124[0];
			fRec126[0] = fRec99[1];
			fRec127[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec126[2] + 0.15f * (fRec126[1] + fRec126[3])));
			fVec13[0] = fSlow249 * (fSlow230 * (fSlow231 * (fSlow248 * fRec127[(IOTA0 - iSlow247) & 2047] - fSlow245 * fRec127[(IOTA0 - iSlow244) & 2047]) + fSlow242 * fRec127[(IOTA0 - iSlow241) & 2047]) - fSlow239 * fRec127[(IOTA0 - iSlow238) & 2047]) + fSlow236 * fRec127[(IOTA0 - iSlow228) & 2047];
			fVec14[0] = fSlow250;
			iRec129[0] = (fSlow250 > fVec14[1]) + (fSlow250 <= fVec14[1]) * (iRec129[1] + (iRec129[1] > 0));
			float fTemp7 = fSlow63 * static_cast<float>(iRec129[0]);
			float fTemp8 = fSlow70 * fTemp1 * std::max<float>(0.0f, std::min<float>(fTemp7, 2.0f - fTemp7));
			fVec15[0] = fTemp8;
			fRec128[0] = -(fSlow73 * (fSlow72 * fRec128[1] - (fTemp8 + fVec15[1])));
			fVec16[0] = fRec128[0] + fVec13[1];
			fRec125[IOTA0 & 2047] = 0.6f * fVec16[1] + 0.4f * fRec125[(IOTA0 - 1) & 2047];
			float fRec123 = fSlow223 * (fSlow204 * (fSlow205 * (fSlow222 * fRec125[(IOTA0 - iSlow220) & 2047] - fSlow219 * fRec125[(IOTA0 - iSlow217) & 2047]) + fSlow216 * fRec125[(IOTA0 - iSlow214) & 2047]) - fSlow213 * fRec125[(IOTA0 - iSlow211) & 2047]) + fSlow210 * fRec125[(IOTA0 - iSlow201) & 2047];
			fRec118[0] = fRec122;
			float fRec119 = fRec128[0] + fRec118[1];
			float fRec120 = fRec123;
			fRec115[IOTA0 & 2047] = fRec119;
			float fRec116 = fSlow249 * (fSlow230 * (fSlow231 * (fSlow248 * fRec115[(IOTA0 - iSlow255) & 2047] - fSlow245 * fRec115[(IOTA0 - iSlow254) & 2047]) + fSlow242 * fRec115[(IOTA0 - iSlow253) & 2047]) - fSlow239 * fRec115[(IOTA0 - iSlow252) & 2047]) + fSlow236 * fRec115[(IOTA0 - iSlow251) & 2047];
			fRec117[0] = fRec120;
			fRec114[0] = fRec117[1];
			float fRec111 = fRec114[1];
			float fRec112 = fRec114[1];
			iRec105[0] = iRec110;
			float fRec106 = fRec113;
			float fRec107 = fRec111;
			float fRec108 = fRec112;
			fRec101[IOTA0 & 2047] = fRec106;
			float fRec102 = fRec116;
			float fRec103 = fRec107;
			float fRec104 = fRec108;
			fRec99[0] = fRec102;
			float fRec100 = fRec104;
			iRec140[0] = 0;
			int iRec141 = iRec140[1];
			float fRec144 = static_cast<float>(iRec136[1]) - 0.9978437f * (0.7f * fRec145[2] + 0.15f * (fRec145[1] + fRec145[3]));
			fRec152[0] = fSlow282 * (fSlow263 * (fSlow264 * (fSlow281 * fRec132[(IOTA0 - iSlow280) & 2047] - fSlow278 * fRec132[(IOTA0 - iSlow277) & 2047]) + fSlow275 * fRec132[(IOTA0 - iSlow274) & 2047]) - fSlow272 * fRec132[(IOTA0 - iSlow271) & 2047]) + fSlow269 * fRec132[(IOTA0 - iSlow261) & 2047];
			fRec155[0] = 0.6f * fRec152[1] + 0.4f * fRec155[1];
			float fRec153 = fRec155[0];
			fRec157[0] = fRec130[1];
			fRec158[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec157[2] + 0.15f * (fRec157[1] + fRec157[3])));
			fVec17[0] = fSlow308 * (fSlow289 * (fSlow290 * (fSlow307 * fRec158[(IOTA0 - iSlow306) & 2047] - fSlow304 * fRec158[(IOTA0 - iSlow303) & 2047]) + fSlow301 * fRec158[(IOTA0 - iSlow300) & 2047]) - fSlow298 * fRec158[(IOTA0 - iSlow297) & 2047]) + fSlow295 * fRec158[(IOTA0 - iSlow287) & 2047];
			fVec18[0] = fSlow309;
			iRec160[0] = (fSlow309 > fVec18[1]) + (fSlow309 <= fVec18[1]) * (iRec160[1] + (iRec160[1] > 0));
			float fTemp9 = fSlow63 * static_cast<float>(iRec160[0]);
			float fTemp10 = fSlow70 * fTemp1 * std::max<float>(0.0f, std::min<float>(fTemp9, 2.0f - fTemp9));
			fVec19[0] = fTemp10;
			fRec159[0] = -(fSlow73 * (fSlow72 * fRec159[1] - (fTemp10 + fVec19[1])));
			fVec20[0] = fRec159[0] + fVec17[1];
			fRec156[IOTA0 & 2047] = 0.6f * fVec20[1] + 0.4f * fRec156[(IOTA0 - 1) & 2047];
			float fRec154 = fSlow282 * (fSlow263 * (fSlow264 * (fSlow281 * fRec156[(IOTA0 - iSlow279) & 2047] - fSlow278 * fRec156[(IOTA0 - iSlow276) & 2047]) + fSlow275 * fRec156[(IOTA0 - iSlow273) & 2047]) - fSlow272 * fRec156[(IOTA0 - iSlow270) & 2047]) + fSlow269 * fRec156[(IOTA0 - iSlow260) & 2047];
			fRec149[0] = fRec153;
			float fRec150 = fRec159[0] + fRec149[1];
			float fRec151 = fRec154;
			fRec146[IOTA0 & 2047] = fRec150;
			float fRec147 = fSlow308 * (fSlow289 * (fSlow290 * (fSlow307 * fRec146[(IOTA0 - iSlow314) & 2047] - fSlow304 * fRec146[(IOTA0 - iSlow313) & 2047]) + fSlow301 * fRec146[(IOTA0 - iSlow312) & 2047]) - fSlow298 * fRec146[(IOTA0 - iSlow311) & 2047]) + fSlow295 * fRec146[(IOTA0 - iSlow310) & 2047];
			fRec148[0] = fRec151;
			fRec145[0] = fRec148[1];
			float fRec142 = fRec145[1];
			float fRec143 = fRec145[1];
			iRec136[0] = iRec141;
			float fRec137 = fRec144;
			float fRec138 = fRec142;
			float fRec139 = fRec143;
			fRec132[IOTA0 & 2047] = fRec137;
			float fRec133 = fRec147;
			float fRec134 = fRec138;
			float fRec135 = fRec139;
			fRec130[0] = fRec133;
			float fRec131 = fRec135;
			iRec171[0] = 0;
			int iRec172 = iRec171[1];
			float fRec175 = static_cast<float>(iRec167[1]) - 0.9978437f * (0.7f * fRec176[2] + 0.15f * (fRec176[1] + fRec176[3]));
			fRec183[0] = fSlow341 * (fSlow322 * (fSlow323 * (fSlow340 * fRec163[(IOTA0 - iSlow339) & 2047] - fSlow337 * fRec163[(IOTA0 - iSlow336) & 2047]) + fSlow334 * fRec163[(IOTA0 - iSlow333) & 2047]) - fSlow331 * fRec163[(IOTA0 - iSlow330) & 2047]) + fSlow328 * fRec163[(IOTA0 - iSlow320) & 2047];
			fRec186[0] = 0.6f * fRec183[1] + 0.4f * fRec186[1];
			float fRec184 = fRec186[0];
			fRec188[0] = fRec161[1];
			fRec189[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec188[2] + 0.15f * (fRec188[1] + fRec188[3])));
			fVec21[0] = fSlow367 * (fSlow348 * (fSlow349 * (fSlow366 * fRec189[(IOTA0 - iSlow365) & 2047] - fSlow363 * fRec189[(IOTA0 - iSlow362) & 2047]) + fSlow360 * fRec189[(IOTA0 - iSlow359) & 2047]) - fSlow357 * fRec189[(IOTA0 - iSlow356) & 2047]) + fSlow354 * fRec189[(IOTA0 - iSlow346) & 2047];
			fVec22[0] = fSlow368;
			iRec191[0] = (fSlow368 > fVec22[1]) + (fSlow368 <= fVec22[1]) * (iRec191[1] + (iRec191[1] > 0));
			float fTemp11 = fSlow63 * static_cast<float>(iRec191[0]);
			float fTemp12 = fSlow70 * fTemp1 * std::max<float>(0.0f, std::min<float>(fTemp11, 2.0f - fTemp11));
			fVec23[0] = fTemp12;
			fRec190[0] = -(fSlow73 * (fSlow72 * fRec190[1] - (fTemp12 + fVec23[1])));
			fVec24[0] = fRec190[0] + fVec21[1];
			fRec187[IOTA0 & 2047] = 0.6f * fVec24[1] + 0.4f * fRec187[(IOTA0 - 1) & 2047];
			float fRec185 = fSlow341 * (fSlow322 * (fSlow323 * (fSlow340 * fRec187[(IOTA0 - iSlow338) & 2047] - fSlow337 * fRec187[(IOTA0 - iSlow335) & 2047]) + fSlow334 * fRec187[(IOTA0 - iSlow332) & 2047]) - fSlow331 * fRec187[(IOTA0 - iSlow329) & 2047]) + fSlow328 * fRec187[(IOTA0 - iSlow319) & 2047];
			fRec180[0] = fRec184;
			float fRec181 = fRec190[0] + fRec180[1];
			float fRec182 = fRec185;
			fRec177[IOTA0 & 2047] = fRec181;
			float fRec178 = fSlow367 * (fSlow348 * (fSlow349 * (fSlow366 * fRec177[(IOTA0 - iSlow373) & 2047] - fSlow363 * fRec177[(IOTA0 - iSlow372) & 2047]) + fSlow360 * fRec177[(IOTA0 - iSlow371) & 2047]) - fSlow357 * fRec177[(IOTA0 - iSlow370) & 2047]) + fSlow354 * fRec177[(IOTA0 - iSlow369) & 2047];
			fRec179[0] = fRec182;
			fRec176[0] = fRec179[1];
			float fRec173 = fRec176[1];
			float fRec174 = fRec176[1];
			iRec167[0] = iRec172;
			float fRec168 = fRec175;
			float fRec169 = fRec173;
			float fRec170 = fRec174;
			fRec163[IOTA0 & 2047] = fRec168;
			float fRec164 = fRec178;
			float fRec165 = fRec169;
			float fRec166 = fRec170;
			fRec161[0] = fRec164;
			float fRec162 = fRec166;
			float fTemp13 = fRec162 + fRec131 + fRec100 + fSlow5 * (fRec67[2] + fRec67[0] + 2.0f * fRec67[1] + fRec35[2] + fRec35[0] + 2.0f * fRec35[1] + fRec1[2] + fRec1[0] + 2.0f * fRec1[1]);
			float fTemp14 = fTemp13 + fSlow374 * fVec0[static_cast<int>(std::min<float>(1.0f, std::max<float>(0.0f, fTemp13)))];
			fRec0[0] = fTemp14 - fConst6 * (fConst4 * fRec0[2] + fConst2 * fRec0[1]);
			fRec192[0] = fTemp14 - fConst18 * (fConst16 * fRec192[2] + fConst14 * fRec192[1]);
			fRec193[0] = fTemp14 - fConst25 * (fConst23 * fRec193[2] + fConst21 * fRec193[1]);
			fRec194[0] = fTemp14 - fConst32 * (fConst30 * fRec194[2] + fConst28 * fRec194[1]);
			fRec195[0] = fTemp14 - fConst39 * (fConst37 * fRec195[2] + fConst35 * fRec195[1]);
			fRec196[0] = fTemp14 - fConst46 * (fConst44 * fRec196[2] + fConst42 * fRec196[1]);
			fRec197[0] = fTemp14 - fConst53 * (fConst51 * fRec197[2] + fConst49 * fRec197[1]);
			fRec198[0] = fTemp14 - fConst60 * (fConst58 * fRec198[2] + fConst56 * fRec198[1]);
			fRec199[0] = fTemp14 - fConst67 * (fConst65 * fRec199[2] + fConst63 * fRec199[1]);
			fRec200[0] = fTemp14 - fConst74 * (fConst72 * fRec200[2] + fConst70 * fRec200[1]);
			fRec201[0] = fTemp14 - fConst81 * (fConst79 * fRec201[2] + fConst77 * fRec201[1]);
			fRec202[0] = fTemp14 - fConst88 * (fConst86 * fRec202[2] + fConst84 * fRec202[1]);
			fRec203[0] = fTemp14 - fConst95 * (fConst93 * fRec203[2] + fConst91 * fRec203[1]);
			fRec204[0] = fTemp14 - fSlow381 * (fSlow379 * fRec204[2] + fSlow377 * fRec204[1]);
			fRec205[0] = fTemp14 - fSlow388 * (fSlow386 * fRec205[2] + fSlow384 * fRec205[1]);
			fRec206[0] = fTemp14 - fSlow395 * (fSlow393 * fRec206[2] + fSlow391 * fRec206[1]);
			fRec207[0] = fTemp14 - fConst103 * (fConst101 * fRec207[2] + fConst99 * fRec207[1]);
			fRec208[0] = fTemp14 - fSlow402 * (fSlow400 * fRec208[2] + fSlow398 * fRec208[1]);
			fRec209[0] = fTemp14 - fConst111 * (fConst109 * fRec209[2] + fConst107 * fRec209[1]);
			fRec210[0] = fTemp14 - fConst118 * (fConst116 * fRec210[2] + fConst114 * fRec210[1]);
			output0[i0] = static_cast<FAUSTFLOAT>(fSlow404 * (0.028571429f * (fConst119 * (fRec210[0] - fRec210[2]) + fConst112 * (fRec209[0] - fRec209[2])) + 0.0375f * (fSlow403 * (fRec208[0] - fRec208[2]) + fConst104 * (fRec207[0] - fRec207[2])) + 0.04f * (fSlow396 * (fRec206[0] - fRec206[2]) + fSlow389 * (fRec205[0] - fRec205[2])) + fSlow382 * (fRec204[0] - fRec204[2]) + fConst96 * (fRec203[0] - fRec203[2]) + fConst89 * (fRec202[0] - fRec202[2]) + fConst82 * (fRec201[0] - fRec201[2]) + fConst75 * (fRec200[0] - fRec200[2]) + fConst68 * (fRec199[0] - fRec199[2]) + fConst61 * (fRec198[0] - fRec198[2]) + 0.03f * (fConst54 * (fRec197[0] - fRec197[2]) + fConst47 * (fRec196[0] - fRec196[2])) + fConst40 * (fRec195[0] - fRec195[2]) + fConst33 * (fRec194[0] - fRec194[2]) + fConst26 * (fRec193[0] - fRec193[2]) + fConst19 * (fRec192[0] - fRec192[2]) + fConst12 * (fRec0[0] - fRec0[2])));
			fVec0[1] = fVec0[0];
			iRec12[1] = iRec12[0];
			IOTA0 = IOTA0 + 1;
			fRec24[1] = fRec24[0];
			fRec27[1] = fRec27[0];
			for (int j0 = 3; j0 > 0; j0 = j0 - 1) {
				fRec29[j0] = fRec29[j0 - 1];
			}
			fVec1[1] = fVec1[0];
			fVec2[1] = fVec2[0];
			iRec32[1] = iRec32[0];
			iRec34[1] = iRec34[0];
			fRec33[2] = fRec33[1];
			fRec33[1] = fRec33[0];
			fVec3[1] = fVec3[0];
			fRec31[1] = fRec31[0];
			fVec4[1] = fVec4[0];
			fRec21[1] = fRec21[0];
			fRec20[1] = fRec20[0];
			for (int j1 = 3; j1 > 0; j1 = j1 - 1) {
				fRec17[j1] = fRec17[j1 - 1];
			}
			iRec8[1] = iRec8[0];
			fRec2[1] = fRec2[0];
			fRec1[2] = fRec1[1];
			fRec1[1] = fRec1[0];
			iRec46[1] = iRec46[0];
			fRec58[1] = fRec58[0];
			fRec61[1] = fRec61[0];
			for (int j2 = 3; j2 > 0; j2 = j2 - 1) {
				fRec63[j2] = fRec63[j2 - 1];
			}
			fVec5[1] = fVec5[0];
			fVec6[1] = fVec6[0];
			iRec66[1] = iRec66[0];
			fVec7[1] = fVec7[0];
			fRec65[1] = fRec65[0];
			fVec8[1] = fVec8[0];
			fRec55[1] = fRec55[0];
			fRec54[1] = fRec54[0];
			for (int j3 = 3; j3 > 0; j3 = j3 - 1) {
				fRec51[j3] = fRec51[j3 - 1];
			}
			iRec42[1] = iRec42[0];
			fRec36[1] = fRec36[0];
			fRec35[2] = fRec35[1];
			fRec35[1] = fRec35[0];
			iRec78[1] = iRec78[0];
			fRec90[1] = fRec90[0];
			fRec93[1] = fRec93[0];
			for (int j4 = 3; j4 > 0; j4 = j4 - 1) {
				fRec95[j4] = fRec95[j4 - 1];
			}
			fVec9[1] = fVec9[0];
			fVec10[1] = fVec10[0];
			iRec98[1] = iRec98[0];
			fVec11[1] = fVec11[0];
			fRec97[1] = fRec97[0];
			fVec12[1] = fVec12[0];
			fRec87[1] = fRec87[0];
			fRec86[1] = fRec86[0];
			for (int j5 = 3; j5 > 0; j5 = j5 - 1) {
				fRec83[j5] = fRec83[j5 - 1];
			}
			iRec74[1] = iRec74[0];
			fRec68[1] = fRec68[0];
			fRec67[2] = fRec67[1];
			fRec67[1] = fRec67[0];
			iRec109[1] = iRec109[0];
			fRec121[1] = fRec121[0];
			fRec124[1] = fRec124[0];
			for (int j6 = 3; j6 > 0; j6 = j6 - 1) {
				fRec126[j6] = fRec126[j6 - 1];
			}
			fVec13[1] = fVec13[0];
			fVec14[1] = fVec14[0];
			iRec129[1] = iRec129[0];
			fVec15[1] = fVec15[0];
			fRec128[1] = fRec128[0];
			fVec16[1] = fVec16[0];
			fRec118[1] = fRec118[0];
			fRec117[1] = fRec117[0];
			for (int j7 = 3; j7 > 0; j7 = j7 - 1) {
				fRec114[j7] = fRec114[j7 - 1];
			}
			iRec105[1] = iRec105[0];
			fRec99[1] = fRec99[0];
			iRec140[1] = iRec140[0];
			fRec152[1] = fRec152[0];
			fRec155[1] = fRec155[0];
			for (int j8 = 3; j8 > 0; j8 = j8 - 1) {
				fRec157[j8] = fRec157[j8 - 1];
			}
			fVec17[1] = fVec17[0];
			fVec18[1] = fVec18[0];
			iRec160[1] = iRec160[0];
			fVec19[1] = fVec19[0];
			fRec159[1] = fRec159[0];
			fVec20[1] = fVec20[0];
			fRec149[1] = fRec149[0];
			fRec148[1] = fRec148[0];
			for (int j9 = 3; j9 > 0; j9 = j9 - 1) {
				fRec145[j9] = fRec145[j9 - 1];
			}
			iRec136[1] = iRec136[0];
			fRec130[1] = fRec130[0];
			iRec171[1] = iRec171[0];
			fRec183[1] = fRec183[0];
			fRec186[1] = fRec186[0];
			for (int j10 = 3; j10 > 0; j10 = j10 - 1) {
				fRec188[j10] = fRec188[j10 - 1];
			}
			fVec21[1] = fVec21[0];
			fVec22[1] = fVec22[0];
			iRec191[1] = iRec191[0];
			fVec23[1] = fVec23[0];
			fRec190[1] = fRec190[0];
			fVec24[1] = fVec24[0];
			fRec180[1] = fRec180[0];
			fRec179[1] = fRec179[0];
			for (int j11 = 3; j11 > 0; j11 = j11 - 1) {
				fRec176[j11] = fRec176[j11 - 1];
			}
			iRec167[1] = iRec167[0];
			fRec161[1] = fRec161[0];
			fRec0[2] = fRec0[1];
			fRec0[1] = fRec0[0];
			fRec192[2] = fRec192[1];
			fRec192[1] = fRec192[0];
			fRec193[2] = fRec193[1];
			fRec193[1] = fRec193[0];
			fRec194[2] = fRec194[1];
			fRec194[1] = fRec194[0];
			fRec195[2] = fRec195[1];
			fRec195[1] = fRec195[0];
			fRec196[2] = fRec196[1];
			fRec196[1] = fRec196[0];
			fRec197[2] = fRec197[1];
			fRec197[1] = fRec197[0];
			fRec198[2] = fRec198[1];
			fRec198[1] = fRec198[0];
			fRec199[2] = fRec199[1];
			fRec199[1] = fRec199[0];
			fRec200[2] = fRec200[1];
			fRec200[1] = fRec200[0];
			fRec201[2] = fRec201[1];
			fRec201[1] = fRec201[0];
			fRec202[2] = fRec202[1];
			fRec202[1] = fRec202[0];
			fRec203[2] = fRec203[1];
			fRec203[1] = fRec203[0];
			fRec204[2] = fRec204[1];
			fRec204[1] = fRec204[0];
			fRec205[2] = fRec205[1];
			fRec205[1] = fRec205[0];
			fRec206[2] = fRec206[1];
			fRec206[1] = fRec206[0];
			fRec207[2] = fRec207[1];
			fRec207[1] = fRec207[0];
			fRec208[2] = fRec208[1];
			fRec208[1] = fRec208[0];
			fRec209[2] = fRec209[1];
			fRec209[1] = fRec209[0];
			fRec210[2] = fRec210[1];
			fRec210[1] = fRec210[0];
		}
	}

};

#endif

/* ------------------------------------------------------------
name: "oud"
Code generated with Faust 2.85.9 (https://faust.grame.fr)
Compilation options: -lang cpp -fpga-mem-th 4 -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 45 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __ouddsp_H__
#define  __ouddsp_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS ouddsp
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

static float ouddsp_faustpower2_f(float value) {
	return value * value;
}

class ouddsp : public dsp {
	
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
	int iRec11[2];
	int IOTA0;
	float fConst7;
	FAUSTFLOAT fHslider0;
	FAUSTFLOAT fHslider1;
	float fConst8;
	float fRec23[2];
	float fRec26[2];
	FAUSTFLOAT fButton0;
	float fVec1[2];
	int iRec28[2];
	FAUSTFLOAT fHslider2;
	FAUSTFLOAT fHslider3;
	float fConst9;
	float fConst10;
	int iRec30[2];
	float fRec29[3];
	FAUSTFLOAT fHslider4;
	float fRec31[4];
	float fRec32[2048];
	float fVec2[2];
	float fVec3[2];
	float fRec27[2048];
	float fRec20[2];
	float fRec17[2048];
	float fRec19[2];
	float fRec16[4];
	int iRec7[2];
	float fRec3[2048];
	float fRec1[2];
	int iRec43[2];
	FAUSTFLOAT fHslider5;
	float fRec55[2];
	float fRec58[2];
	FAUSTFLOAT fButton1;
	float fVec4[2];
	int iRec60[2];
	float fRec61[4];
	float fRec62[2048];
	float fVec5[2];
	float fVec6[2];
	float fRec59[2048];
	float fRec52[2];
	float fRec49[2048];
	float fRec51[2];
	float fRec48[4];
	int iRec39[2];
	float fRec35[2048];
	float fRec33[2];
	int iRec73[2];
	FAUSTFLOAT fHslider6;
	float fRec85[2];
	float fRec88[2];
	float fRec90[4];
	float fRec91[2048];
	float fVec7[2];
	float fVec8[2];
	float fRec89[2048];
	float fRec82[2];
	float fRec79[2048];
	float fRec81[2];
	float fRec78[4];
	int iRec69[2];
	float fRec65[2048];
	float fRec63[2];
	int iRec102[2];
	FAUSTFLOAT fHslider7;
	float fRec114[2];
	float fRec117[2];
	FAUSTFLOAT fButton2;
	float fVec9[2];
	int iRec119[2];
	float fRec120[4];
	float fRec121[2048];
	float fVec10[2];
	float fVec11[2];
	float fRec118[2048];
	float fRec111[2];
	float fRec108[2048];
	float fRec110[2];
	float fRec107[4];
	int iRec98[2];
	float fRec94[2048];
	float fRec92[2];
	int iRec132[2];
	float fRec144[2];
	float fRec147[2];
	float fRec149[4];
	float fRec150[2048];
	float fVec12[2];
	float fVec13[2];
	float fRec148[2048];
	float fRec141[2];
	float fRec138[2048];
	float fRec140[2];
	float fRec137[4];
	int iRec128[2];
	float fRec124[2048];
	float fRec122[2];
	int iRec161[2];
	FAUSTFLOAT fHslider8;
	float fRec173[2];
	float fRec176[2];
	FAUSTFLOAT fButton3;
	float fVec14[2];
	int iRec178[2];
	float fRec179[4];
	float fRec180[2048];
	float fVec15[2];
	float fVec16[2];
	float fRec177[2048];
	float fRec170[2];
	float fRec167[2048];
	float fRec169[2];
	float fRec166[4];
	int iRec157[2];
	float fRec153[2048];
	float fRec151[2];
	int iRec191[2];
	float fRec203[2];
	float fRec206[2];
	float fRec208[4];
	float fRec209[2048];
	float fVec17[2];
	float fVec18[2];
	float fRec207[2048];
	float fRec200[2];
	float fRec197[2048];
	float fRec199[2];
	float fRec196[4];
	int iRec187[2];
	float fRec183[2048];
	float fRec181[2];
	int iRec220[2];
	FAUSTFLOAT fHslider9;
	float fRec232[2];
	float fRec235[2];
	FAUSTFLOAT fButton4;
	float fVec19[2];
	int iRec237[2];
	float fRec238[4];
	float fRec239[2048];
	float fVec20[2];
	float fVec21[2];
	float fRec236[2048];
	float fRec229[2];
	float fRec226[2048];
	float fRec228[2];
	float fRec225[4];
	int iRec216[2];
	float fRec212[2048];
	float fRec210[2];
	int iRec250[2];
	float fRec262[2];
	float fRec265[2];
	float fRec267[4];
	float fRec268[2048];
	float fVec22[2];
	float fVec23[2];
	float fRec266[2048];
	float fRec259[2];
	float fRec256[2048];
	float fRec258[2];
	float fRec255[4];
	int iRec246[2];
	float fRec242[2048];
	float fRec240[2];
	int iRec279[2];
	FAUSTFLOAT fHslider10;
	float fRec291[2];
	float fRec294[2];
	FAUSTFLOAT fButton5;
	float fVec24[2];
	int iRec296[2];
	float fRec297[4];
	float fRec298[2048];
	float fVec25[2];
	float fVec26[2];
	float fRec295[2048];
	float fRec288[2];
	float fRec285[2048];
	float fRec287[2];
	float fRec284[4];
	int iRec275[2];
	float fRec271[2048];
	float fRec269[2];
	int iRec309[2];
	float fRec321[2];
	float fRec324[2];
	float fRec326[4];
	float fRec327[2048];
	float fVec27[2];
	float fVec28[2];
	float fRec325[2048];
	float fRec318[2];
	float fRec315[2048];
	float fRec317[2];
	float fRec314[4];
	int iRec305[2];
	float fRec301[2048];
	float fRec299[2];
	FAUSTFLOAT fHslider11;
	float fRec0[3];
	float fConst11;
	float fConst12;
	float fConst13;
	float fConst14;
	float fConst15;
	float fConst16;
	float fConst17;
	float fRec328[3];
	float fConst18;
	float fConst19;
	float fConst20;
	float fConst21;
	float fConst22;
	float fConst23;
	float fConst24;
	float fRec329[3];
	float fConst25;
	float fConst26;
	float fConst27;
	float fConst28;
	float fConst29;
	float fConst30;
	float fConst31;
	float fRec330[3];
	float fConst32;
	float fConst33;
	float fConst34;
	float fConst35;
	float fConst36;
	float fConst37;
	float fConst38;
	float fRec331[3];
	float fConst39;
	float fConst40;
	float fConst41;
	float fConst42;
	float fConst43;
	float fConst44;
	float fConst45;
	float fRec332[3];
	float fConst46;
	float fConst47;
	float fConst48;
	float fConst49;
	float fConst50;
	float fConst51;
	float fConst52;
	float fRec333[3];
	float fConst53;
	float fConst54;
	float fConst55;
	float fConst56;
	float fConst57;
	float fConst58;
	float fConst59;
	float fRec334[3];
	float fConst60;
	float fConst61;
	float fConst62;
	float fConst63;
	float fConst64;
	float fConst65;
	float fConst66;
	float fRec335[3];
	float fConst67;
	float fConst68;
	float fConst69;
	float fConst70;
	float fConst71;
	float fConst72;
	float fConst73;
	float fRec336[3];
	float fConst74;
	float fConst75;
	float fConst76;
	float fConst77;
	float fConst78;
	float fConst79;
	float fConst80;
	float fRec337[3];
	float fConst81;
	float fConst82;
	float fConst83;
	float fConst84;
	float fConst85;
	float fConst86;
	float fConst87;
	float fRec338[3];
	float fConst88;
	float fConst89;
	float fConst90;
	float fConst91;
	float fConst92;
	float fConst93;
	float fConst94;
	float fRec339[3];
	float fConst95;
	float fConst96;
	float fConst97;
	float fConst98;
	float fConst99;
	float fConst100;
	float fConst101;
	float fRec340[3];
	float fConst102;
	float fConst103;
	float fConst104;
	float fConst105;
	float fConst106;
	float fConst107;
	float fConst108;
	float fRec341[3];
	float fConst109;
	float fConst110;
	float fConst111;
	float fConst112;
	float fConst113;
	float fConst114;
	float fConst115;
	float fRec342[3];
	float fConst116;
	FAUSTFLOAT fHslider12;
	float fConst117;
	float fRec343[3];
	float fConst118;
	float fRec344[3];
	float fConst119;
	float fRec345[3];
	FAUSTFLOAT fHslider13;
	float fRec346[3];
	FAUSTFLOAT fHslider14;
	
 public:
	ouddsp() {
	}
	
	ouddsp(const ouddsp&) = default;
	
	virtual ~ouddsp() = default;
	
	ouddsp& operator=(const ouddsp&) = default;
	
	void metadata(Meta* m) { 
		m->declare("compile_options", "-lang cpp -fpga-mem-th 4 -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 45 -single -ftz 0");
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
		m->declare("filename", "oud.dsp");
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
		m->declare("name", "oud");
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
		fConst1 = std::tan(10461.504f / fConst0);
		fConst2 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst1));
		fConst3 = 1.0f / fConst1;
		fConst4 = (fConst3 + -0.5555556f) / fConst1 + 1.0f;
		fConst5 = (fConst3 + 0.5555556f) / fConst1 + 1.0f;
		fConst6 = 1.0f / fConst5;
		fConst7 = 0.00882353f * fConst0;
		fConst8 = 0.0014705883f * fConst0;
		fConst9 = 0.002f * fConst0;
		fConst10 = 15.707963f / fConst0;
		fConst11 = 0.016666668f / (fConst1 * fConst5);
		fConst12 = std::tan(9519.025f / fConst0);
		fConst13 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst12));
		fConst14 = 1.0f / fConst12;
		fConst15 = (fConst14 + -0.5f) / fConst12 + 1.0f;
		fConst16 = (fConst14 + 0.5f) / fConst12 + 1.0f;
		fConst17 = 1.0f / fConst16;
		fConst18 = 0.02f / (fConst12 * fConst16);
		fConst19 = std::tan(8639.38f / fConst0);
		fConst20 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst19));
		fConst21 = 1.0f / fConst19;
		fConst22 = (fConst21 + -0.4347826f) / fConst19 + 1.0f;
		fConst23 = (fConst21 + 0.4347826f) / fConst19 + 1.0f;
		fConst24 = 1.0f / fConst23;
		fConst25 = 0.02173913f / (fConst19 * fConst23);
		fConst26 = std::tan(7822.566f / fConst0);
		fConst27 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst26));
		fConst28 = 1.0f / fConst26;
		fConst29 = (fConst28 + -0.3846154f) / fConst26 + 1.0f;
		fConst30 = (fConst28 + 0.3846154f) / fConst26 + 1.0f;
		fConst31 = 1.0f / fConst30;
		fConst32 = 0.023076924f / (fConst26 * fConst30);
		fConst33 = std::tan(7068.5835f / fConst0);
		fConst34 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst33));
		fConst35 = 1.0f / fConst33;
		fConst36 = (fConst35 + -0.3448276f) / fConst33 + 1.0f;
		fConst37 = (fConst35 + 0.3448276f) / fConst33 + 1.0f;
		fConst38 = 1.0f / fConst37;
		fConst39 = 0.024137931f / (fConst33 * fConst37);
		fConst40 = std::tan(6377.433f / fConst0);
		fConst41 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst40));
		fConst42 = 1.0f / fConst40;
		fConst43 = (fConst42 + -0.3125f) / fConst40 + 1.0f;
		fConst44 = (fConst42 + 0.3125f) / fConst40 + 1.0f;
		fConst45 = 1.0f / fConst44;
		fConst46 = 0.025f / (fConst40 * fConst44);
		fConst47 = std::tan(5717.6987f / fConst0);
		fConst48 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst47));
		fConst49 = 1.0f / fConst47;
		fConst50 = (fConst49 + -0.2857143f) / fConst47 + 1.0f;
		fConst51 = (fConst49 + 0.2857143f) / fConst47 + 1.0f;
		fConst52 = 1.0f / fConst51;
		fConst53 = 0.025714286f / (fConst47 * fConst51);
		fConst54 = std::tan(5120.796f / fConst0);
		fConst55 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst54));
		fConst56 = 1.0f / fConst54;
		fConst57 = (fConst56 + -0.25f) / fConst54 + 1.0f;
		fConst58 = (fConst56 + 0.25f) / fConst54 + 1.0f;
		fConst59 = 1.0f / fConst58;
		fConst60 = 0.025f / (fConst54 * fConst58);
		fConst61 = std::tan(4555.3096f / fConst0);
		fConst62 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst61));
		fConst63 = 1.0f / fConst61;
		fConst64 = (fConst63 + -0.22222222f) / fConst61 + 1.0f;
		fConst65 = (fConst63 + 0.22222222f) / fConst61 + 1.0f;
		fConst66 = 1.0f / fConst65;
		fConst67 = 0.026666667f / (fConst61 * fConst65);
		fConst68 = std::tan(4021.2385f / fConst0);
		fConst69 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst68));
		fConst70 = 1.0f / fConst68;
		fConst71 = (fConst70 + -0.2f) / fConst68 + 1.0f;
		fConst72 = (fConst70 + 0.2f) / fConst68 + 1.0f;
		fConst73 = 1.0f / fConst72;
		fConst74 = 0.028f / (fConst68 * fConst72);
		fConst75 = std::tan(3518.5837f / fConst0);
		fConst76 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst75));
		fConst77 = 1.0f / fConst75;
		fConst78 = (fConst77 + -0.18181819f) / fConst75 + 1.0f;
		fConst79 = (fConst77 + 0.18181819f) / fConst75 + 1.0f;
		fConst80 = 1.0f / fConst79;
		fConst81 = 0.02909091f / (fConst75 * fConst79);
		fConst82 = std::tan(3047.345f / fConst0);
		fConst83 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst82));
		fConst84 = 1.0f / fConst82;
		fConst85 = (fConst84 + -0.16666667f) / fConst82 + 1.0f;
		fConst86 = (fConst84 + 0.16666667f) / fConst82 + 1.0f;
		fConst87 = 1.0f / fConst86;
		fConst88 = 0.03f / (fConst82 * fConst86);
		fConst89 = std::tan(2607.522f / fConst0);
		fConst90 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst89));
		fConst91 = 1.0f / fConst89;
		fConst92 = (fConst91 + -0.15384616f) / fConst89 + 1.0f;
		fConst93 = (fConst91 + 0.15384616f) / fConst89 + 1.0f;
		fConst94 = 1.0f / fConst93;
		fConst95 = 0.03076923f / (fConst89 * fConst93);
		fConst96 = std::tan(2199.1147f / fConst0);
		fConst97 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst96));
		fConst98 = 1.0f / fConst96;
		fConst99 = (fConst98 + -0.14285715f) / fConst96 + 1.0f;
		fConst100 = (fConst98 + 0.14285715f) / fConst96 + 1.0f;
		fConst101 = 1.0f / fConst100;
		fConst102 = 0.03142857f / (fConst96 * fConst100);
		fConst103 = std::tan(1822.1238f / fConst0);
		fConst104 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst103));
		fConst105 = 1.0f / fConst103;
		fConst106 = (fConst105 + -0.13333334f) / fConst103 + 1.0f;
		fConst107 = (fConst105 + 0.13333334f) / fConst103 + 1.0f;
		fConst108 = 1.0f / fConst107;
		fConst109 = 0.033333335f / (fConst103 * fConst107);
		fConst110 = std::tan(1507.9645f / fConst0);
		fConst111 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fConst110));
		fConst112 = 1.0f / fConst110;
		fConst113 = (fConst112 + -0.11764706f) / fConst110 + 1.0f;
		fConst114 = (fConst112 + 0.11764706f) / fConst110 + 1.0f;
		fConst115 = 1.0f / fConst114;
		fConst116 = 0.03529412f / (fConst110 * fConst114);
		fConst117 = 5.8119464f / fConst0;
		fConst118 = 4.0840707f / fConst0;
		fConst119 = 3.1415927f / fConst0;
	}
	
	virtual void instanceResetUserInterface() {
		fHslider0 = static_cast<FAUSTFLOAT>(2.2e+02f);
		fHslider1 = static_cast<FAUSTFLOAT>(0.5f);
		fButton0 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider2 = static_cast<FAUSTFLOAT>(0.6f);
		fHslider3 = static_cast<FAUSTFLOAT>(0.7f);
		fHslider4 = static_cast<FAUSTFLOAT>(0.8f);
		fHslider5 = static_cast<FAUSTFLOAT>(0.5f);
		fButton1 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider6 = static_cast<FAUSTFLOAT>(0.003f);
		fHslider7 = static_cast<FAUSTFLOAT>(0.5f);
		fButton2 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider8 = static_cast<FAUSTFLOAT>(0.5f);
		fButton3 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider9 = static_cast<FAUSTFLOAT>(0.5f);
		fButton4 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider10 = static_cast<FAUSTFLOAT>(0.5f);
		fButton5 = static_cast<FAUSTFLOAT>(0.0f);
		fHslider11 = static_cast<FAUSTFLOAT>(0.015f);
		fHslider12 = static_cast<FAUSTFLOAT>(3e+02f);
		fHslider13 = static_cast<FAUSTFLOAT>(1.4e+02f);
		fHslider14 = static_cast<FAUSTFLOAT>(0.5f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fVec0[l0] = 0.0f;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			iRec11[l1] = 0;
		}
		IOTA0 = 0;
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec23[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec26[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fVec1[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			iRec28[l5] = 0;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			iRec30[l6] = 0;
		}
		for (int l7 = 0; l7 < 3; l7 = l7 + 1) {
			fRec29[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 4; l8 = l8 + 1) {
			fRec31[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 2048; l9 = l9 + 1) {
			fRec32[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			fVec2[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fVec3[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 2048; l12 = l12 + 1) {
			fRec27[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 2; l13 = l13 + 1) {
			fRec20[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 2048; l14 = l14 + 1) {
			fRec17[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fRec19[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 4; l16 = l16 + 1) {
			fRec16[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2; l17 = l17 + 1) {
			iRec7[l17] = 0;
		}
		for (int l18 = 0; l18 < 2048; l18 = l18 + 1) {
			fRec3[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 2; l19 = l19 + 1) {
			fRec1[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 2; l20 = l20 + 1) {
			iRec43[l20] = 0;
		}
		for (int l21 = 0; l21 < 2; l21 = l21 + 1) {
			fRec55[l21] = 0.0f;
		}
		for (int l22 = 0; l22 < 2; l22 = l22 + 1) {
			fRec58[l22] = 0.0f;
		}
		for (int l23 = 0; l23 < 2; l23 = l23 + 1) {
			fVec4[l23] = 0.0f;
		}
		for (int l24 = 0; l24 < 2; l24 = l24 + 1) {
			iRec60[l24] = 0;
		}
		for (int l25 = 0; l25 < 4; l25 = l25 + 1) {
			fRec61[l25] = 0.0f;
		}
		for (int l26 = 0; l26 < 2048; l26 = l26 + 1) {
			fRec62[l26] = 0.0f;
		}
		for (int l27 = 0; l27 < 2; l27 = l27 + 1) {
			fVec5[l27] = 0.0f;
		}
		for (int l28 = 0; l28 < 2; l28 = l28 + 1) {
			fVec6[l28] = 0.0f;
		}
		for (int l29 = 0; l29 < 2048; l29 = l29 + 1) {
			fRec59[l29] = 0.0f;
		}
		for (int l30 = 0; l30 < 2; l30 = l30 + 1) {
			fRec52[l30] = 0.0f;
		}
		for (int l31 = 0; l31 < 2048; l31 = l31 + 1) {
			fRec49[l31] = 0.0f;
		}
		for (int l32 = 0; l32 < 2; l32 = l32 + 1) {
			fRec51[l32] = 0.0f;
		}
		for (int l33 = 0; l33 < 4; l33 = l33 + 1) {
			fRec48[l33] = 0.0f;
		}
		for (int l34 = 0; l34 < 2; l34 = l34 + 1) {
			iRec39[l34] = 0;
		}
		for (int l35 = 0; l35 < 2048; l35 = l35 + 1) {
			fRec35[l35] = 0.0f;
		}
		for (int l36 = 0; l36 < 2; l36 = l36 + 1) {
			fRec33[l36] = 0.0f;
		}
		for (int l37 = 0; l37 < 2; l37 = l37 + 1) {
			iRec73[l37] = 0;
		}
		for (int l38 = 0; l38 < 2; l38 = l38 + 1) {
			fRec85[l38] = 0.0f;
		}
		for (int l39 = 0; l39 < 2; l39 = l39 + 1) {
			fRec88[l39] = 0.0f;
		}
		for (int l40 = 0; l40 < 4; l40 = l40 + 1) {
			fRec90[l40] = 0.0f;
		}
		for (int l41 = 0; l41 < 2048; l41 = l41 + 1) {
			fRec91[l41] = 0.0f;
		}
		for (int l42 = 0; l42 < 2; l42 = l42 + 1) {
			fVec7[l42] = 0.0f;
		}
		for (int l43 = 0; l43 < 2; l43 = l43 + 1) {
			fVec8[l43] = 0.0f;
		}
		for (int l44 = 0; l44 < 2048; l44 = l44 + 1) {
			fRec89[l44] = 0.0f;
		}
		for (int l45 = 0; l45 < 2; l45 = l45 + 1) {
			fRec82[l45] = 0.0f;
		}
		for (int l46 = 0; l46 < 2048; l46 = l46 + 1) {
			fRec79[l46] = 0.0f;
		}
		for (int l47 = 0; l47 < 2; l47 = l47 + 1) {
			fRec81[l47] = 0.0f;
		}
		for (int l48 = 0; l48 < 4; l48 = l48 + 1) {
			fRec78[l48] = 0.0f;
		}
		for (int l49 = 0; l49 < 2; l49 = l49 + 1) {
			iRec69[l49] = 0;
		}
		for (int l50 = 0; l50 < 2048; l50 = l50 + 1) {
			fRec65[l50] = 0.0f;
		}
		for (int l51 = 0; l51 < 2; l51 = l51 + 1) {
			fRec63[l51] = 0.0f;
		}
		for (int l52 = 0; l52 < 2; l52 = l52 + 1) {
			iRec102[l52] = 0;
		}
		for (int l53 = 0; l53 < 2; l53 = l53 + 1) {
			fRec114[l53] = 0.0f;
		}
		for (int l54 = 0; l54 < 2; l54 = l54 + 1) {
			fRec117[l54] = 0.0f;
		}
		for (int l55 = 0; l55 < 2; l55 = l55 + 1) {
			fVec9[l55] = 0.0f;
		}
		for (int l56 = 0; l56 < 2; l56 = l56 + 1) {
			iRec119[l56] = 0;
		}
		for (int l57 = 0; l57 < 4; l57 = l57 + 1) {
			fRec120[l57] = 0.0f;
		}
		for (int l58 = 0; l58 < 2048; l58 = l58 + 1) {
			fRec121[l58] = 0.0f;
		}
		for (int l59 = 0; l59 < 2; l59 = l59 + 1) {
			fVec10[l59] = 0.0f;
		}
		for (int l60 = 0; l60 < 2; l60 = l60 + 1) {
			fVec11[l60] = 0.0f;
		}
		for (int l61 = 0; l61 < 2048; l61 = l61 + 1) {
			fRec118[l61] = 0.0f;
		}
		for (int l62 = 0; l62 < 2; l62 = l62 + 1) {
			fRec111[l62] = 0.0f;
		}
		for (int l63 = 0; l63 < 2048; l63 = l63 + 1) {
			fRec108[l63] = 0.0f;
		}
		for (int l64 = 0; l64 < 2; l64 = l64 + 1) {
			fRec110[l64] = 0.0f;
		}
		for (int l65 = 0; l65 < 4; l65 = l65 + 1) {
			fRec107[l65] = 0.0f;
		}
		for (int l66 = 0; l66 < 2; l66 = l66 + 1) {
			iRec98[l66] = 0;
		}
		for (int l67 = 0; l67 < 2048; l67 = l67 + 1) {
			fRec94[l67] = 0.0f;
		}
		for (int l68 = 0; l68 < 2; l68 = l68 + 1) {
			fRec92[l68] = 0.0f;
		}
		for (int l69 = 0; l69 < 2; l69 = l69 + 1) {
			iRec132[l69] = 0;
		}
		for (int l70 = 0; l70 < 2; l70 = l70 + 1) {
			fRec144[l70] = 0.0f;
		}
		for (int l71 = 0; l71 < 2; l71 = l71 + 1) {
			fRec147[l71] = 0.0f;
		}
		for (int l72 = 0; l72 < 4; l72 = l72 + 1) {
			fRec149[l72] = 0.0f;
		}
		for (int l73 = 0; l73 < 2048; l73 = l73 + 1) {
			fRec150[l73] = 0.0f;
		}
		for (int l74 = 0; l74 < 2; l74 = l74 + 1) {
			fVec12[l74] = 0.0f;
		}
		for (int l75 = 0; l75 < 2; l75 = l75 + 1) {
			fVec13[l75] = 0.0f;
		}
		for (int l76 = 0; l76 < 2048; l76 = l76 + 1) {
			fRec148[l76] = 0.0f;
		}
		for (int l77 = 0; l77 < 2; l77 = l77 + 1) {
			fRec141[l77] = 0.0f;
		}
		for (int l78 = 0; l78 < 2048; l78 = l78 + 1) {
			fRec138[l78] = 0.0f;
		}
		for (int l79 = 0; l79 < 2; l79 = l79 + 1) {
			fRec140[l79] = 0.0f;
		}
		for (int l80 = 0; l80 < 4; l80 = l80 + 1) {
			fRec137[l80] = 0.0f;
		}
		for (int l81 = 0; l81 < 2; l81 = l81 + 1) {
			iRec128[l81] = 0;
		}
		for (int l82 = 0; l82 < 2048; l82 = l82 + 1) {
			fRec124[l82] = 0.0f;
		}
		for (int l83 = 0; l83 < 2; l83 = l83 + 1) {
			fRec122[l83] = 0.0f;
		}
		for (int l84 = 0; l84 < 2; l84 = l84 + 1) {
			iRec161[l84] = 0;
		}
		for (int l85 = 0; l85 < 2; l85 = l85 + 1) {
			fRec173[l85] = 0.0f;
		}
		for (int l86 = 0; l86 < 2; l86 = l86 + 1) {
			fRec176[l86] = 0.0f;
		}
		for (int l87 = 0; l87 < 2; l87 = l87 + 1) {
			fVec14[l87] = 0.0f;
		}
		for (int l88 = 0; l88 < 2; l88 = l88 + 1) {
			iRec178[l88] = 0;
		}
		for (int l89 = 0; l89 < 4; l89 = l89 + 1) {
			fRec179[l89] = 0.0f;
		}
		for (int l90 = 0; l90 < 2048; l90 = l90 + 1) {
			fRec180[l90] = 0.0f;
		}
		for (int l91 = 0; l91 < 2; l91 = l91 + 1) {
			fVec15[l91] = 0.0f;
		}
		for (int l92 = 0; l92 < 2; l92 = l92 + 1) {
			fVec16[l92] = 0.0f;
		}
		for (int l93 = 0; l93 < 2048; l93 = l93 + 1) {
			fRec177[l93] = 0.0f;
		}
		for (int l94 = 0; l94 < 2; l94 = l94 + 1) {
			fRec170[l94] = 0.0f;
		}
		for (int l95 = 0; l95 < 2048; l95 = l95 + 1) {
			fRec167[l95] = 0.0f;
		}
		for (int l96 = 0; l96 < 2; l96 = l96 + 1) {
			fRec169[l96] = 0.0f;
		}
		for (int l97 = 0; l97 < 4; l97 = l97 + 1) {
			fRec166[l97] = 0.0f;
		}
		for (int l98 = 0; l98 < 2; l98 = l98 + 1) {
			iRec157[l98] = 0;
		}
		for (int l99 = 0; l99 < 2048; l99 = l99 + 1) {
			fRec153[l99] = 0.0f;
		}
		for (int l100 = 0; l100 < 2; l100 = l100 + 1) {
			fRec151[l100] = 0.0f;
		}
		for (int l101 = 0; l101 < 2; l101 = l101 + 1) {
			iRec191[l101] = 0;
		}
		for (int l102 = 0; l102 < 2; l102 = l102 + 1) {
			fRec203[l102] = 0.0f;
		}
		for (int l103 = 0; l103 < 2; l103 = l103 + 1) {
			fRec206[l103] = 0.0f;
		}
		for (int l104 = 0; l104 < 4; l104 = l104 + 1) {
			fRec208[l104] = 0.0f;
		}
		for (int l105 = 0; l105 < 2048; l105 = l105 + 1) {
			fRec209[l105] = 0.0f;
		}
		for (int l106 = 0; l106 < 2; l106 = l106 + 1) {
			fVec17[l106] = 0.0f;
		}
		for (int l107 = 0; l107 < 2; l107 = l107 + 1) {
			fVec18[l107] = 0.0f;
		}
		for (int l108 = 0; l108 < 2048; l108 = l108 + 1) {
			fRec207[l108] = 0.0f;
		}
		for (int l109 = 0; l109 < 2; l109 = l109 + 1) {
			fRec200[l109] = 0.0f;
		}
		for (int l110 = 0; l110 < 2048; l110 = l110 + 1) {
			fRec197[l110] = 0.0f;
		}
		for (int l111 = 0; l111 < 2; l111 = l111 + 1) {
			fRec199[l111] = 0.0f;
		}
		for (int l112 = 0; l112 < 4; l112 = l112 + 1) {
			fRec196[l112] = 0.0f;
		}
		for (int l113 = 0; l113 < 2; l113 = l113 + 1) {
			iRec187[l113] = 0;
		}
		for (int l114 = 0; l114 < 2048; l114 = l114 + 1) {
			fRec183[l114] = 0.0f;
		}
		for (int l115 = 0; l115 < 2; l115 = l115 + 1) {
			fRec181[l115] = 0.0f;
		}
		for (int l116 = 0; l116 < 2; l116 = l116 + 1) {
			iRec220[l116] = 0;
		}
		for (int l117 = 0; l117 < 2; l117 = l117 + 1) {
			fRec232[l117] = 0.0f;
		}
		for (int l118 = 0; l118 < 2; l118 = l118 + 1) {
			fRec235[l118] = 0.0f;
		}
		for (int l119 = 0; l119 < 2; l119 = l119 + 1) {
			fVec19[l119] = 0.0f;
		}
		for (int l120 = 0; l120 < 2; l120 = l120 + 1) {
			iRec237[l120] = 0;
		}
		for (int l121 = 0; l121 < 4; l121 = l121 + 1) {
			fRec238[l121] = 0.0f;
		}
		for (int l122 = 0; l122 < 2048; l122 = l122 + 1) {
			fRec239[l122] = 0.0f;
		}
		for (int l123 = 0; l123 < 2; l123 = l123 + 1) {
			fVec20[l123] = 0.0f;
		}
		for (int l124 = 0; l124 < 2; l124 = l124 + 1) {
			fVec21[l124] = 0.0f;
		}
		for (int l125 = 0; l125 < 2048; l125 = l125 + 1) {
			fRec236[l125] = 0.0f;
		}
		for (int l126 = 0; l126 < 2; l126 = l126 + 1) {
			fRec229[l126] = 0.0f;
		}
		for (int l127 = 0; l127 < 2048; l127 = l127 + 1) {
			fRec226[l127] = 0.0f;
		}
		for (int l128 = 0; l128 < 2; l128 = l128 + 1) {
			fRec228[l128] = 0.0f;
		}
		for (int l129 = 0; l129 < 4; l129 = l129 + 1) {
			fRec225[l129] = 0.0f;
		}
		for (int l130 = 0; l130 < 2; l130 = l130 + 1) {
			iRec216[l130] = 0;
		}
		for (int l131 = 0; l131 < 2048; l131 = l131 + 1) {
			fRec212[l131] = 0.0f;
		}
		for (int l132 = 0; l132 < 2; l132 = l132 + 1) {
			fRec210[l132] = 0.0f;
		}
		for (int l133 = 0; l133 < 2; l133 = l133 + 1) {
			iRec250[l133] = 0;
		}
		for (int l134 = 0; l134 < 2; l134 = l134 + 1) {
			fRec262[l134] = 0.0f;
		}
		for (int l135 = 0; l135 < 2; l135 = l135 + 1) {
			fRec265[l135] = 0.0f;
		}
		for (int l136 = 0; l136 < 4; l136 = l136 + 1) {
			fRec267[l136] = 0.0f;
		}
		for (int l137 = 0; l137 < 2048; l137 = l137 + 1) {
			fRec268[l137] = 0.0f;
		}
		for (int l138 = 0; l138 < 2; l138 = l138 + 1) {
			fVec22[l138] = 0.0f;
		}
		for (int l139 = 0; l139 < 2; l139 = l139 + 1) {
			fVec23[l139] = 0.0f;
		}
		for (int l140 = 0; l140 < 2048; l140 = l140 + 1) {
			fRec266[l140] = 0.0f;
		}
		for (int l141 = 0; l141 < 2; l141 = l141 + 1) {
			fRec259[l141] = 0.0f;
		}
		for (int l142 = 0; l142 < 2048; l142 = l142 + 1) {
			fRec256[l142] = 0.0f;
		}
		for (int l143 = 0; l143 < 2; l143 = l143 + 1) {
			fRec258[l143] = 0.0f;
		}
		for (int l144 = 0; l144 < 4; l144 = l144 + 1) {
			fRec255[l144] = 0.0f;
		}
		for (int l145 = 0; l145 < 2; l145 = l145 + 1) {
			iRec246[l145] = 0;
		}
		for (int l146 = 0; l146 < 2048; l146 = l146 + 1) {
			fRec242[l146] = 0.0f;
		}
		for (int l147 = 0; l147 < 2; l147 = l147 + 1) {
			fRec240[l147] = 0.0f;
		}
		for (int l148 = 0; l148 < 2; l148 = l148 + 1) {
			iRec279[l148] = 0;
		}
		for (int l149 = 0; l149 < 2; l149 = l149 + 1) {
			fRec291[l149] = 0.0f;
		}
		for (int l150 = 0; l150 < 2; l150 = l150 + 1) {
			fRec294[l150] = 0.0f;
		}
		for (int l151 = 0; l151 < 2; l151 = l151 + 1) {
			fVec24[l151] = 0.0f;
		}
		for (int l152 = 0; l152 < 2; l152 = l152 + 1) {
			iRec296[l152] = 0;
		}
		for (int l153 = 0; l153 < 4; l153 = l153 + 1) {
			fRec297[l153] = 0.0f;
		}
		for (int l154 = 0; l154 < 2048; l154 = l154 + 1) {
			fRec298[l154] = 0.0f;
		}
		for (int l155 = 0; l155 < 2; l155 = l155 + 1) {
			fVec25[l155] = 0.0f;
		}
		for (int l156 = 0; l156 < 2; l156 = l156 + 1) {
			fVec26[l156] = 0.0f;
		}
		for (int l157 = 0; l157 < 2048; l157 = l157 + 1) {
			fRec295[l157] = 0.0f;
		}
		for (int l158 = 0; l158 < 2; l158 = l158 + 1) {
			fRec288[l158] = 0.0f;
		}
		for (int l159 = 0; l159 < 2048; l159 = l159 + 1) {
			fRec285[l159] = 0.0f;
		}
		for (int l160 = 0; l160 < 2; l160 = l160 + 1) {
			fRec287[l160] = 0.0f;
		}
		for (int l161 = 0; l161 < 4; l161 = l161 + 1) {
			fRec284[l161] = 0.0f;
		}
		for (int l162 = 0; l162 < 2; l162 = l162 + 1) {
			iRec275[l162] = 0;
		}
		for (int l163 = 0; l163 < 2048; l163 = l163 + 1) {
			fRec271[l163] = 0.0f;
		}
		for (int l164 = 0; l164 < 2; l164 = l164 + 1) {
			fRec269[l164] = 0.0f;
		}
		for (int l165 = 0; l165 < 2; l165 = l165 + 1) {
			iRec309[l165] = 0;
		}
		for (int l166 = 0; l166 < 2; l166 = l166 + 1) {
			fRec321[l166] = 0.0f;
		}
		for (int l167 = 0; l167 < 2; l167 = l167 + 1) {
			fRec324[l167] = 0.0f;
		}
		for (int l168 = 0; l168 < 4; l168 = l168 + 1) {
			fRec326[l168] = 0.0f;
		}
		for (int l169 = 0; l169 < 2048; l169 = l169 + 1) {
			fRec327[l169] = 0.0f;
		}
		for (int l170 = 0; l170 < 2; l170 = l170 + 1) {
			fVec27[l170] = 0.0f;
		}
		for (int l171 = 0; l171 < 2; l171 = l171 + 1) {
			fVec28[l171] = 0.0f;
		}
		for (int l172 = 0; l172 < 2048; l172 = l172 + 1) {
			fRec325[l172] = 0.0f;
		}
		for (int l173 = 0; l173 < 2; l173 = l173 + 1) {
			fRec318[l173] = 0.0f;
		}
		for (int l174 = 0; l174 < 2048; l174 = l174 + 1) {
			fRec315[l174] = 0.0f;
		}
		for (int l175 = 0; l175 < 2; l175 = l175 + 1) {
			fRec317[l175] = 0.0f;
		}
		for (int l176 = 0; l176 < 4; l176 = l176 + 1) {
			fRec314[l176] = 0.0f;
		}
		for (int l177 = 0; l177 < 2; l177 = l177 + 1) {
			iRec305[l177] = 0;
		}
		for (int l178 = 0; l178 < 2048; l178 = l178 + 1) {
			fRec301[l178] = 0.0f;
		}
		for (int l179 = 0; l179 < 2; l179 = l179 + 1) {
			fRec299[l179] = 0.0f;
		}
		for (int l180 = 0; l180 < 3; l180 = l180 + 1) {
			fRec0[l180] = 0.0f;
		}
		for (int l181 = 0; l181 < 3; l181 = l181 + 1) {
			fRec328[l181] = 0.0f;
		}
		for (int l182 = 0; l182 < 3; l182 = l182 + 1) {
			fRec329[l182] = 0.0f;
		}
		for (int l183 = 0; l183 < 3; l183 = l183 + 1) {
			fRec330[l183] = 0.0f;
		}
		for (int l184 = 0; l184 < 3; l184 = l184 + 1) {
			fRec331[l184] = 0.0f;
		}
		for (int l185 = 0; l185 < 3; l185 = l185 + 1) {
			fRec332[l185] = 0.0f;
		}
		for (int l186 = 0; l186 < 3; l186 = l186 + 1) {
			fRec333[l186] = 0.0f;
		}
		for (int l187 = 0; l187 < 3; l187 = l187 + 1) {
			fRec334[l187] = 0.0f;
		}
		for (int l188 = 0; l188 < 3; l188 = l188 + 1) {
			fRec335[l188] = 0.0f;
		}
		for (int l189 = 0; l189 < 3; l189 = l189 + 1) {
			fRec336[l189] = 0.0f;
		}
		for (int l190 = 0; l190 < 3; l190 = l190 + 1) {
			fRec337[l190] = 0.0f;
		}
		for (int l191 = 0; l191 < 3; l191 = l191 + 1) {
			fRec338[l191] = 0.0f;
		}
		for (int l192 = 0; l192 < 3; l192 = l192 + 1) {
			fRec339[l192] = 0.0f;
		}
		for (int l193 = 0; l193 < 3; l193 = l193 + 1) {
			fRec340[l193] = 0.0f;
		}
		for (int l194 = 0; l194 < 3; l194 = l194 + 1) {
			fRec341[l194] = 0.0f;
		}
		for (int l195 = 0; l195 < 3; l195 = l195 + 1) {
			fRec342[l195] = 0.0f;
		}
		for (int l196 = 0; l196 < 3; l196 = l196 + 1) {
			fRec343[l196] = 0.0f;
		}
		for (int l197 = 0; l197 < 3; l197 = l197 + 1) {
			fRec344[l197] = 0.0f;
		}
		for (int l198 = 0; l198 < 3; l198 = l198 + 1) {
			fRec345[l198] = 0.0f;
		}
		for (int l199 = 0; l199 < 3; l199 = l199 + 1) {
			fRec346[l199] = 0.0f;
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
	
	virtual ouddsp* clone() {
		return new ouddsp(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("oud");
		ui_interface->addHorizontalSlider("AirResonance", &fHslider13, FAUSTFLOAT(1.4e+02f), FAUSTFLOAT(1.2e+02f), FAUSTFLOAT(1.8e+02f), FAUSTFLOAT(0.1f));
		ui_interface->addHorizontalSlider("CourseDetune", &fHslider6, FAUSTFLOAT(0.003f), FAUSTFLOAT(0.0f), FAUSTFLOAT(0.01f), FAUSTFLOAT(0.0001f));
		ui_interface->addButton("Gate0", &fButton1);
		ui_interface->addButton("Gate1", &fButton0);
		ui_interface->addButton("Gate2", &fButton2);
		ui_interface->addButton("Gate3", &fButton3);
		ui_interface->addButton("Gate4", &fButton4);
		ui_interface->addButton("Gate5", &fButton5);
		ui_interface->addHorizontalSlider("PlectrumBrightness", &fHslider2, FAUSTFLOAT(0.6f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("PlectrumHardness", &fHslider3, FAUSTFLOAT(0.7f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("PluckPos0", &fHslider5, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("PluckPos1", &fHslider1, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("PluckPos2", &fHslider7, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("PluckPos3", &fHslider8, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("PluckPos4", &fHslider9, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("PluckPos5", &fHslider10, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("SympatheticGain", &fHslider11, FAUSTFLOAT(0.015f), FAUSTFLOAT(0.0f), FAUSTFLOAT(0.05f), FAUSTFLOAT(0.0001f));
		ui_interface->addHorizontalSlider("TopResonance", &fHslider12, FAUSTFLOAT(3e+02f), FAUSTFLOAT(2.5e+02f), FAUSTFLOAT(4e+02f), FAUSTFLOAT(0.1f));
		ui_interface->addHorizontalSlider("freq", &fHslider0, FAUSTFLOAT(2.2e+02f), FAUSTFLOAT(5e+01f), FAUSTFLOAT(6e+02f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("gain", &fHslider4, FAUSTFLOAT(0.8f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addHorizontalSlider("outGain", &fHslider14, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* output0 = outputs[0];
		float fSlow0 = static_cast<float>(fHslider0);
		float fSlow1 = 3.4e+02f / fSlow0 + -0.11f;
		float fSlow2 = static_cast<float>(fHslider1);
		float fSlow3 = 1.0f - fSlow2;
		float fSlow4 = fConst8 * fSlow3 * fSlow1;
		float fSlow5 = fSlow4 + -1.499995f;
		int iSlow6 = static_cast<int>(fSlow5);
		int iSlow7 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow6 + 4))));
		int iSlow8 = iSlow7 + 1;
		float fSlow9 = std::floor(fSlow5);
		float fSlow10 = fSlow4 + (-3.0f - fSlow9);
		float fSlow11 = fSlow4 + (-2.0f - fSlow9);
		float fSlow12 = fSlow4 + (-1.0f - fSlow9);
		float fSlow13 = fSlow4 - fSlow9;
		float fSlow14 = fSlow13 * fSlow12;
		float fSlow15 = fSlow14 * fSlow11;
		float fSlow16 = 0.041666668f * fSlow15 * fSlow10;
		int iSlow17 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow6 + 3))));
		int iSlow18 = iSlow17 + 1;
		float fSlow19 = 0.16666667f * fSlow15;
		int iSlow20 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow6 + 2))));
		int iSlow21 = iSlow20 + 1;
		float fSlow22 = 0.25f * fSlow14;
		int iSlow23 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow6 + 1))));
		int iSlow24 = iSlow23 + 1;
		float fSlow25 = 0.16666667f * fSlow13;
		int iSlow26 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow6))));
		int iSlow27 = iSlow26 + 1;
		float fSlow28 = 0.041666668f * fSlow12;
		float fSlow29 = fSlow4 + (-4.0f - fSlow9);
		float fSlow30 = static_cast<float>(fButton0);
		float fSlow31 = static_cast<float>(fHslider2);
		float fSlow32 = static_cast<float>(fHslider3);
		float fSlow33 = 1.0f / std::max<float>(1.0f, fConst9 * (0.25f * (fSlow32 + 1.0f) + 0.5f * fSlow31) * ouddsp_faustpower2_f(1.0f - 0.0005f * (fSlow0 / (0.25f * fSlow32 + 0.2f * fSlow31 + 0.55f))));
		float fSlow34 = 0.3f * fSlow32;
		float fSlow35 = std::tan(fConst10 * fSlow0 * (fSlow34 + 0.35f * (fSlow31 + 1.0f)));
		float fSlow36 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fSlow35));
		float fSlow37 = 1.0f / fSlow35;
		float fSlow38 = (fSlow37 + -1.4142135f) / fSlow35 + 1.0f;
		float fSlow39 = (fSlow37 + 1.4142135f) / fSlow35 + 1.0f;
		float fSlow40 = 1.0f / fSlow39;
		float fSlow41 = static_cast<float>(fHslider4) * (fSlow34 + 1.0f) / fSlow39;
		float fSlow42 = fConst8 * fSlow2 * fSlow1;
		float fSlow43 = fSlow42 + -1.499995f;
		int iSlow44 = static_cast<int>(fSlow43);
		int iSlow45 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow44 + 4))));
		int iSlow46 = iSlow45 + 2;
		float fSlow47 = std::floor(fSlow43);
		float fSlow48 = fSlow42 + (-3.0f - fSlow47);
		float fSlow49 = fSlow42 + (-2.0f - fSlow47);
		float fSlow50 = fSlow42 + (-1.0f - fSlow47);
		float fSlow51 = fSlow42 - fSlow47;
		float fSlow52 = fSlow51 * fSlow50;
		float fSlow53 = fSlow52 * fSlow49;
		float fSlow54 = 0.041666668f * fSlow53 * fSlow48;
		int iSlow55 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow44 + 3))));
		int iSlow56 = iSlow55 + 2;
		float fSlow57 = 0.16666667f * fSlow53;
		int iSlow58 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow44 + 2))));
		int iSlow59 = iSlow58 + 2;
		float fSlow60 = 0.25f * fSlow52;
		int iSlow61 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow44 + 1))));
		int iSlow62 = iSlow61 + 2;
		float fSlow63 = 0.16666667f * fSlow51;
		int iSlow64 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow44))));
		int iSlow65 = iSlow64 + 2;
		float fSlow66 = 0.041666668f * fSlow50;
		float fSlow67 = fSlow42 + (-4.0f - fSlow47);
		int iSlow68 = iSlow45 + 1;
		int iSlow69 = iSlow55 + 1;
		int iSlow70 = iSlow58 + 1;
		int iSlow71 = iSlow61 + 1;
		int iSlow72 = iSlow64 + 1;
		float fSlow73 = static_cast<float>(fHslider5);
		float fSlow74 = fConst8 * (1.0f - fSlow73) * fSlow1;
		float fSlow75 = fSlow74 + -1.499995f;
		int iSlow76 = static_cast<int>(fSlow75);
		int iSlow77 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow76 + 4))));
		int iSlow78 = iSlow77 + 1;
		float fSlow79 = std::floor(fSlow75);
		float fSlow80 = fSlow74 + (-3.0f - fSlow79);
		float fSlow81 = fSlow74 + (-2.0f - fSlow79);
		float fSlow82 = fSlow74 + (-1.0f - fSlow79);
		float fSlow83 = fSlow74 - fSlow79;
		float fSlow84 = fSlow83 * fSlow82;
		float fSlow85 = fSlow84 * fSlow81;
		float fSlow86 = 0.041666668f * fSlow85 * fSlow80;
		int iSlow87 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow76 + 3))));
		int iSlow88 = iSlow87 + 1;
		float fSlow89 = 0.16666667f * fSlow85;
		int iSlow90 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow76 + 2))));
		int iSlow91 = iSlow90 + 1;
		float fSlow92 = 0.25f * fSlow84;
		int iSlow93 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow76 + 1))));
		int iSlow94 = iSlow93 + 1;
		float fSlow95 = 0.16666667f * fSlow83;
		int iSlow96 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow76))));
		int iSlow97 = iSlow96 + 1;
		float fSlow98 = 0.041666668f * fSlow82;
		float fSlow99 = fSlow74 + (-4.0f - fSlow79);
		float fSlow100 = static_cast<float>(fButton1);
		float fSlow101 = fConst8 * fSlow73 * fSlow1;
		float fSlow102 = fSlow101 + -1.499995f;
		int iSlow103 = static_cast<int>(fSlow102);
		int iSlow104 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow103 + 4))));
		int iSlow105 = iSlow104 + 2;
		float fSlow106 = std::floor(fSlow102);
		float fSlow107 = fSlow101 + (-3.0f - fSlow106);
		float fSlow108 = fSlow101 + (-2.0f - fSlow106);
		float fSlow109 = fSlow101 + (-1.0f - fSlow106);
		float fSlow110 = fSlow101 - fSlow106;
		float fSlow111 = fSlow110 * fSlow109;
		float fSlow112 = fSlow111 * fSlow108;
		float fSlow113 = 0.041666668f * fSlow112 * fSlow107;
		int iSlow114 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow103 + 3))));
		int iSlow115 = iSlow114 + 2;
		float fSlow116 = 0.16666667f * fSlow112;
		int iSlow117 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow103 + 2))));
		int iSlow118 = iSlow117 + 2;
		float fSlow119 = 0.25f * fSlow111;
		int iSlow120 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow103 + 1))));
		int iSlow121 = iSlow120 + 2;
		float fSlow122 = 0.16666667f * fSlow110;
		int iSlow123 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow103))));
		int iSlow124 = iSlow123 + 2;
		float fSlow125 = 0.041666668f * fSlow109;
		float fSlow126 = fSlow101 + (-4.0f - fSlow106);
		int iSlow127 = iSlow104 + 1;
		int iSlow128 = iSlow114 + 1;
		int iSlow129 = iSlow117 + 1;
		int iSlow130 = iSlow120 + 1;
		int iSlow131 = iSlow123 + 1;
		float fSlow132 = 3.4e+02f / (fSlow0 * (static_cast<float>(fHslider6) + 1.0f)) + -0.11f;
		float fSlow133 = fConst8 * fSlow2 * fSlow132;
		float fSlow134 = fSlow133 + -1.499995f;
		int iSlow135 = static_cast<int>(fSlow134);
		int iSlow136 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow135 + 4))));
		int iSlow137 = iSlow136 + 1;
		float fSlow138 = std::floor(fSlow134);
		float fSlow139 = fSlow133 + (-3.0f - fSlow138);
		float fSlow140 = fSlow133 + (-2.0f - fSlow138);
		float fSlow141 = fSlow133 + (-1.0f - fSlow138);
		float fSlow142 = fSlow133 - fSlow138;
		float fSlow143 = fSlow142 * fSlow141;
		float fSlow144 = fSlow143 * fSlow140;
		float fSlow145 = 0.041666668f * fSlow144 * fSlow139;
		int iSlow146 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow135 + 3))));
		int iSlow147 = iSlow146 + 1;
		float fSlow148 = 0.16666667f * fSlow144;
		int iSlow149 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow135 + 2))));
		int iSlow150 = iSlow149 + 1;
		float fSlow151 = 0.25f * fSlow143;
		int iSlow152 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow135 + 1))));
		int iSlow153 = iSlow152 + 1;
		float fSlow154 = 0.16666667f * fSlow142;
		int iSlow155 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow135))));
		int iSlow156 = iSlow155 + 1;
		float fSlow157 = 0.041666668f * fSlow141;
		float fSlow158 = fSlow133 + (-4.0f - fSlow138);
		float fSlow159 = fConst8 * fSlow3 * fSlow132;
		float fSlow160 = fSlow159 + -1.499995f;
		int iSlow161 = static_cast<int>(fSlow160);
		int iSlow162 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow161 + 4))));
		int iSlow163 = iSlow162 + 2;
		float fSlow164 = std::floor(fSlow160);
		float fSlow165 = fSlow159 + (-3.0f - fSlow164);
		float fSlow166 = fSlow159 + (-2.0f - fSlow164);
		float fSlow167 = fSlow159 + (-1.0f - fSlow164);
		float fSlow168 = fSlow159 - fSlow164;
		float fSlow169 = fSlow168 * fSlow167;
		float fSlow170 = fSlow169 * fSlow166;
		float fSlow171 = 0.041666668f * fSlow170 * fSlow165;
		int iSlow172 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow161 + 3))));
		int iSlow173 = iSlow172 + 2;
		float fSlow174 = 0.16666667f * fSlow170;
		int iSlow175 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow161 + 2))));
		int iSlow176 = iSlow175 + 2;
		float fSlow177 = 0.25f * fSlow169;
		int iSlow178 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow161 + 1))));
		int iSlow179 = iSlow178 + 2;
		float fSlow180 = 0.16666667f * fSlow168;
		int iSlow181 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow161))));
		int iSlow182 = iSlow181 + 2;
		float fSlow183 = 0.041666668f * fSlow167;
		float fSlow184 = fSlow159 + (-4.0f - fSlow164);
		int iSlow185 = iSlow162 + 1;
		int iSlow186 = iSlow172 + 1;
		int iSlow187 = iSlow175 + 1;
		int iSlow188 = iSlow178 + 1;
		int iSlow189 = iSlow181 + 1;
		float fSlow190 = static_cast<float>(fHslider7);
		float fSlow191 = 1.0f - fSlow190;
		float fSlow192 = fConst8 * fSlow191 * fSlow1;
		float fSlow193 = fSlow192 + -1.499995f;
		int iSlow194 = static_cast<int>(fSlow193);
		int iSlow195 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow194 + 4))));
		int iSlow196 = iSlow195 + 1;
		float fSlow197 = std::floor(fSlow193);
		float fSlow198 = fSlow192 + (-3.0f - fSlow197);
		float fSlow199 = fSlow192 + (-2.0f - fSlow197);
		float fSlow200 = fSlow192 + (-1.0f - fSlow197);
		float fSlow201 = fSlow192 - fSlow197;
		float fSlow202 = fSlow201 * fSlow200;
		float fSlow203 = fSlow202 * fSlow199;
		float fSlow204 = 0.041666668f * fSlow203 * fSlow198;
		int iSlow205 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow194 + 3))));
		int iSlow206 = iSlow205 + 1;
		float fSlow207 = 0.16666667f * fSlow203;
		int iSlow208 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow194 + 2))));
		int iSlow209 = iSlow208 + 1;
		float fSlow210 = 0.25f * fSlow202;
		int iSlow211 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow194 + 1))));
		int iSlow212 = iSlow211 + 1;
		float fSlow213 = 0.16666667f * fSlow201;
		int iSlow214 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow194))));
		int iSlow215 = iSlow214 + 1;
		float fSlow216 = 0.041666668f * fSlow200;
		float fSlow217 = fSlow192 + (-4.0f - fSlow197);
		float fSlow218 = static_cast<float>(fButton2);
		float fSlow219 = fConst8 * fSlow190 * fSlow1;
		float fSlow220 = fSlow219 + -1.499995f;
		int iSlow221 = static_cast<int>(fSlow220);
		int iSlow222 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow221 + 4))));
		int iSlow223 = iSlow222 + 2;
		float fSlow224 = std::floor(fSlow220);
		float fSlow225 = fSlow219 + (-3.0f - fSlow224);
		float fSlow226 = fSlow219 + (-2.0f - fSlow224);
		float fSlow227 = fSlow219 + (-1.0f - fSlow224);
		float fSlow228 = fSlow219 - fSlow224;
		float fSlow229 = fSlow228 * fSlow227;
		float fSlow230 = fSlow229 * fSlow226;
		float fSlow231 = 0.041666668f * fSlow230 * fSlow225;
		int iSlow232 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow221 + 3))));
		int iSlow233 = iSlow232 + 2;
		float fSlow234 = 0.16666667f * fSlow230;
		int iSlow235 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow221 + 2))));
		int iSlow236 = iSlow235 + 2;
		float fSlow237 = 0.25f * fSlow229;
		int iSlow238 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow221 + 1))));
		int iSlow239 = iSlow238 + 2;
		float fSlow240 = 0.16666667f * fSlow228;
		int iSlow241 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow221))));
		int iSlow242 = iSlow241 + 2;
		float fSlow243 = 0.041666668f * fSlow227;
		float fSlow244 = fSlow219 + (-4.0f - fSlow224);
		int iSlow245 = iSlow222 + 1;
		int iSlow246 = iSlow232 + 1;
		int iSlow247 = iSlow235 + 1;
		int iSlow248 = iSlow238 + 1;
		int iSlow249 = iSlow241 + 1;
		float fSlow250 = fConst8 * fSlow190 * fSlow132;
		float fSlow251 = fSlow250 + -1.499995f;
		int iSlow252 = static_cast<int>(fSlow251);
		int iSlow253 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow252 + 4))));
		int iSlow254 = iSlow253 + 1;
		float fSlow255 = std::floor(fSlow251);
		float fSlow256 = fSlow250 + (-3.0f - fSlow255);
		float fSlow257 = fSlow250 + (-2.0f - fSlow255);
		float fSlow258 = fSlow250 + (-1.0f - fSlow255);
		float fSlow259 = fSlow250 - fSlow255;
		float fSlow260 = fSlow259 * fSlow258;
		float fSlow261 = fSlow260 * fSlow257;
		float fSlow262 = 0.041666668f * fSlow261 * fSlow256;
		int iSlow263 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow252 + 3))));
		int iSlow264 = iSlow263 + 1;
		float fSlow265 = 0.16666667f * fSlow261;
		int iSlow266 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow252 + 2))));
		int iSlow267 = iSlow266 + 1;
		float fSlow268 = 0.25f * fSlow260;
		int iSlow269 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow252 + 1))));
		int iSlow270 = iSlow269 + 1;
		float fSlow271 = 0.16666667f * fSlow259;
		int iSlow272 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow252))));
		int iSlow273 = iSlow272 + 1;
		float fSlow274 = 0.041666668f * fSlow258;
		float fSlow275 = fSlow250 + (-4.0f - fSlow255);
		float fSlow276 = fConst8 * fSlow191 * fSlow132;
		float fSlow277 = fSlow276 + -1.499995f;
		int iSlow278 = static_cast<int>(fSlow277);
		int iSlow279 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow278 + 4))));
		int iSlow280 = iSlow279 + 2;
		float fSlow281 = std::floor(fSlow277);
		float fSlow282 = fSlow276 + (-3.0f - fSlow281);
		float fSlow283 = fSlow276 + (-2.0f - fSlow281);
		float fSlow284 = fSlow276 + (-1.0f - fSlow281);
		float fSlow285 = fSlow276 - fSlow281;
		float fSlow286 = fSlow285 * fSlow284;
		float fSlow287 = fSlow286 * fSlow283;
		float fSlow288 = 0.041666668f * fSlow287 * fSlow282;
		int iSlow289 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow278 + 3))));
		int iSlow290 = iSlow289 + 2;
		float fSlow291 = 0.16666667f * fSlow287;
		int iSlow292 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow278 + 2))));
		int iSlow293 = iSlow292 + 2;
		float fSlow294 = 0.25f * fSlow286;
		int iSlow295 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow278 + 1))));
		int iSlow296 = iSlow295 + 2;
		float fSlow297 = 0.16666667f * fSlow285;
		int iSlow298 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow278))));
		int iSlow299 = iSlow298 + 2;
		float fSlow300 = 0.041666668f * fSlow284;
		float fSlow301 = fSlow276 + (-4.0f - fSlow281);
		int iSlow302 = iSlow279 + 1;
		int iSlow303 = iSlow289 + 1;
		int iSlow304 = iSlow292 + 1;
		int iSlow305 = iSlow295 + 1;
		int iSlow306 = iSlow298 + 1;
		float fSlow307 = static_cast<float>(fHslider8);
		float fSlow308 = 1.0f - fSlow307;
		float fSlow309 = fConst8 * fSlow308 * fSlow1;
		float fSlow310 = fSlow309 + -1.499995f;
		int iSlow311 = static_cast<int>(fSlow310);
		int iSlow312 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow311 + 4))));
		int iSlow313 = iSlow312 + 1;
		float fSlow314 = std::floor(fSlow310);
		float fSlow315 = fSlow309 + (-3.0f - fSlow314);
		float fSlow316 = fSlow309 + (-2.0f - fSlow314);
		float fSlow317 = fSlow309 + (-1.0f - fSlow314);
		float fSlow318 = fSlow309 - fSlow314;
		float fSlow319 = fSlow318 * fSlow317;
		float fSlow320 = fSlow319 * fSlow316;
		float fSlow321 = 0.041666668f * fSlow320 * fSlow315;
		int iSlow322 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow311 + 3))));
		int iSlow323 = iSlow322 + 1;
		float fSlow324 = 0.16666667f * fSlow320;
		int iSlow325 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow311 + 2))));
		int iSlow326 = iSlow325 + 1;
		float fSlow327 = 0.25f * fSlow319;
		int iSlow328 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow311 + 1))));
		int iSlow329 = iSlow328 + 1;
		float fSlow330 = 0.16666667f * fSlow318;
		int iSlow331 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow311))));
		int iSlow332 = iSlow331 + 1;
		float fSlow333 = 0.041666668f * fSlow317;
		float fSlow334 = fSlow309 + (-4.0f - fSlow314);
		float fSlow335 = static_cast<float>(fButton3);
		float fSlow336 = fConst8 * fSlow307 * fSlow1;
		float fSlow337 = fSlow336 + -1.499995f;
		int iSlow338 = static_cast<int>(fSlow337);
		int iSlow339 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow338 + 4))));
		int iSlow340 = iSlow339 + 2;
		float fSlow341 = std::floor(fSlow337);
		float fSlow342 = fSlow336 + (-3.0f - fSlow341);
		float fSlow343 = fSlow336 + (-2.0f - fSlow341);
		float fSlow344 = fSlow336 + (-1.0f - fSlow341);
		float fSlow345 = fSlow336 - fSlow341;
		float fSlow346 = fSlow345 * fSlow344;
		float fSlow347 = fSlow346 * fSlow343;
		float fSlow348 = 0.041666668f * fSlow347 * fSlow342;
		int iSlow349 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow338 + 3))));
		int iSlow350 = iSlow349 + 2;
		float fSlow351 = 0.16666667f * fSlow347;
		int iSlow352 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow338 + 2))));
		int iSlow353 = iSlow352 + 2;
		float fSlow354 = 0.25f * fSlow346;
		int iSlow355 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow338 + 1))));
		int iSlow356 = iSlow355 + 2;
		float fSlow357 = 0.16666667f * fSlow345;
		int iSlow358 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow338))));
		int iSlow359 = iSlow358 + 2;
		float fSlow360 = 0.041666668f * fSlow344;
		float fSlow361 = fSlow336 + (-4.0f - fSlow341);
		int iSlow362 = iSlow339 + 1;
		int iSlow363 = iSlow349 + 1;
		int iSlow364 = iSlow352 + 1;
		int iSlow365 = iSlow355 + 1;
		int iSlow366 = iSlow358 + 1;
		float fSlow367 = fConst8 * fSlow307 * fSlow132;
		float fSlow368 = fSlow367 + -1.499995f;
		int iSlow369 = static_cast<int>(fSlow368);
		int iSlow370 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow369 + 4))));
		int iSlow371 = iSlow370 + 1;
		float fSlow372 = std::floor(fSlow368);
		float fSlow373 = fSlow367 + (-3.0f - fSlow372);
		float fSlow374 = fSlow367 + (-2.0f - fSlow372);
		float fSlow375 = fSlow367 + (-1.0f - fSlow372);
		float fSlow376 = fSlow367 - fSlow372;
		float fSlow377 = fSlow376 * fSlow375;
		float fSlow378 = fSlow377 * fSlow374;
		float fSlow379 = 0.041666668f * fSlow378 * fSlow373;
		int iSlow380 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow369 + 3))));
		int iSlow381 = iSlow380 + 1;
		float fSlow382 = 0.16666667f * fSlow378;
		int iSlow383 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow369 + 2))));
		int iSlow384 = iSlow383 + 1;
		float fSlow385 = 0.25f * fSlow377;
		int iSlow386 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow369 + 1))));
		int iSlow387 = iSlow386 + 1;
		float fSlow388 = 0.16666667f * fSlow376;
		int iSlow389 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow369))));
		int iSlow390 = iSlow389 + 1;
		float fSlow391 = 0.041666668f * fSlow375;
		float fSlow392 = fSlow367 + (-4.0f - fSlow372);
		float fSlow393 = fConst8 * fSlow308 * fSlow132;
		float fSlow394 = fSlow393 + -1.499995f;
		int iSlow395 = static_cast<int>(fSlow394);
		int iSlow396 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow395 + 4))));
		int iSlow397 = iSlow396 + 2;
		float fSlow398 = std::floor(fSlow394);
		float fSlow399 = fSlow393 + (-3.0f - fSlow398);
		float fSlow400 = fSlow393 + (-2.0f - fSlow398);
		float fSlow401 = fSlow393 + (-1.0f - fSlow398);
		float fSlow402 = fSlow393 - fSlow398;
		float fSlow403 = fSlow402 * fSlow401;
		float fSlow404 = fSlow403 * fSlow400;
		float fSlow405 = 0.041666668f * fSlow404 * fSlow399;
		int iSlow406 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow395 + 3))));
		int iSlow407 = iSlow406 + 2;
		float fSlow408 = 0.16666667f * fSlow404;
		int iSlow409 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow395 + 2))));
		int iSlow410 = iSlow409 + 2;
		float fSlow411 = 0.25f * fSlow403;
		int iSlow412 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow395 + 1))));
		int iSlow413 = iSlow412 + 2;
		float fSlow414 = 0.16666667f * fSlow402;
		int iSlow415 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow395))));
		int iSlow416 = iSlow415 + 2;
		float fSlow417 = 0.041666668f * fSlow401;
		float fSlow418 = fSlow393 + (-4.0f - fSlow398);
		int iSlow419 = iSlow396 + 1;
		int iSlow420 = iSlow406 + 1;
		int iSlow421 = iSlow409 + 1;
		int iSlow422 = iSlow412 + 1;
		int iSlow423 = iSlow415 + 1;
		float fSlow424 = static_cast<float>(fHslider9);
		float fSlow425 = 1.0f - fSlow424;
		float fSlow426 = fConst8 * fSlow425 * fSlow1;
		float fSlow427 = fSlow426 + -1.499995f;
		int iSlow428 = static_cast<int>(fSlow427);
		int iSlow429 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow428 + 4))));
		int iSlow430 = iSlow429 + 1;
		float fSlow431 = std::floor(fSlow427);
		float fSlow432 = fSlow426 + (-3.0f - fSlow431);
		float fSlow433 = fSlow426 + (-2.0f - fSlow431);
		float fSlow434 = fSlow426 + (-1.0f - fSlow431);
		float fSlow435 = fSlow426 - fSlow431;
		float fSlow436 = fSlow435 * fSlow434;
		float fSlow437 = fSlow436 * fSlow433;
		float fSlow438 = 0.041666668f * fSlow437 * fSlow432;
		int iSlow439 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow428 + 3))));
		int iSlow440 = iSlow439 + 1;
		float fSlow441 = 0.16666667f * fSlow437;
		int iSlow442 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow428 + 2))));
		int iSlow443 = iSlow442 + 1;
		float fSlow444 = 0.25f * fSlow436;
		int iSlow445 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow428 + 1))));
		int iSlow446 = iSlow445 + 1;
		float fSlow447 = 0.16666667f * fSlow435;
		int iSlow448 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow428))));
		int iSlow449 = iSlow448 + 1;
		float fSlow450 = 0.041666668f * fSlow434;
		float fSlow451 = fSlow426 + (-4.0f - fSlow431);
		float fSlow452 = static_cast<float>(fButton4);
		float fSlow453 = fConst8 * fSlow424 * fSlow1;
		float fSlow454 = fSlow453 + -1.499995f;
		int iSlow455 = static_cast<int>(fSlow454);
		int iSlow456 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow455 + 4))));
		int iSlow457 = iSlow456 + 2;
		float fSlow458 = std::floor(fSlow454);
		float fSlow459 = fSlow453 + (-3.0f - fSlow458);
		float fSlow460 = fSlow453 + (-2.0f - fSlow458);
		float fSlow461 = fSlow453 + (-1.0f - fSlow458);
		float fSlow462 = fSlow453 - fSlow458;
		float fSlow463 = fSlow462 * fSlow461;
		float fSlow464 = fSlow463 * fSlow460;
		float fSlow465 = 0.041666668f * fSlow464 * fSlow459;
		int iSlow466 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow455 + 3))));
		int iSlow467 = iSlow466 + 2;
		float fSlow468 = 0.16666667f * fSlow464;
		int iSlow469 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow455 + 2))));
		int iSlow470 = iSlow469 + 2;
		float fSlow471 = 0.25f * fSlow463;
		int iSlow472 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow455 + 1))));
		int iSlow473 = iSlow472 + 2;
		float fSlow474 = 0.16666667f * fSlow462;
		int iSlow475 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow455))));
		int iSlow476 = iSlow475 + 2;
		float fSlow477 = 0.041666668f * fSlow461;
		float fSlow478 = fSlow453 + (-4.0f - fSlow458);
		int iSlow479 = iSlow456 + 1;
		int iSlow480 = iSlow466 + 1;
		int iSlow481 = iSlow469 + 1;
		int iSlow482 = iSlow472 + 1;
		int iSlow483 = iSlow475 + 1;
		float fSlow484 = fConst8 * fSlow424 * fSlow132;
		float fSlow485 = fSlow484 + -1.499995f;
		int iSlow486 = static_cast<int>(fSlow485);
		int iSlow487 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow486 + 4))));
		int iSlow488 = iSlow487 + 1;
		float fSlow489 = std::floor(fSlow485);
		float fSlow490 = fSlow484 + (-3.0f - fSlow489);
		float fSlow491 = fSlow484 + (-2.0f - fSlow489);
		float fSlow492 = fSlow484 + (-1.0f - fSlow489);
		float fSlow493 = fSlow484 - fSlow489;
		float fSlow494 = fSlow493 * fSlow492;
		float fSlow495 = fSlow494 * fSlow491;
		float fSlow496 = 0.041666668f * fSlow495 * fSlow490;
		int iSlow497 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow486 + 3))));
		int iSlow498 = iSlow497 + 1;
		float fSlow499 = 0.16666667f * fSlow495;
		int iSlow500 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow486 + 2))));
		int iSlow501 = iSlow500 + 1;
		float fSlow502 = 0.25f * fSlow494;
		int iSlow503 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow486 + 1))));
		int iSlow504 = iSlow503 + 1;
		float fSlow505 = 0.16666667f * fSlow493;
		int iSlow506 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow486))));
		int iSlow507 = iSlow506 + 1;
		float fSlow508 = 0.041666668f * fSlow492;
		float fSlow509 = fSlow484 + (-4.0f - fSlow489);
		float fSlow510 = fConst8 * fSlow425 * fSlow132;
		float fSlow511 = fSlow510 + -1.499995f;
		int iSlow512 = static_cast<int>(fSlow511);
		int iSlow513 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow512 + 4))));
		int iSlow514 = iSlow513 + 2;
		float fSlow515 = std::floor(fSlow511);
		float fSlow516 = fSlow510 + (-3.0f - fSlow515);
		float fSlow517 = fSlow510 + (-2.0f - fSlow515);
		float fSlow518 = fSlow510 + (-1.0f - fSlow515);
		float fSlow519 = fSlow510 - fSlow515;
		float fSlow520 = fSlow519 * fSlow518;
		float fSlow521 = fSlow520 * fSlow517;
		float fSlow522 = 0.041666668f * fSlow521 * fSlow516;
		int iSlow523 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow512 + 3))));
		int iSlow524 = iSlow523 + 2;
		float fSlow525 = 0.16666667f * fSlow521;
		int iSlow526 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow512 + 2))));
		int iSlow527 = iSlow526 + 2;
		float fSlow528 = 0.25f * fSlow520;
		int iSlow529 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow512 + 1))));
		int iSlow530 = iSlow529 + 2;
		float fSlow531 = 0.16666667f * fSlow519;
		int iSlow532 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow512))));
		int iSlow533 = iSlow532 + 2;
		float fSlow534 = 0.041666668f * fSlow518;
		float fSlow535 = fSlow510 + (-4.0f - fSlow515);
		int iSlow536 = iSlow513 + 1;
		int iSlow537 = iSlow523 + 1;
		int iSlow538 = iSlow526 + 1;
		int iSlow539 = iSlow529 + 1;
		int iSlow540 = iSlow532 + 1;
		float fSlow541 = static_cast<float>(fHslider10);
		float fSlow542 = 1.0f - fSlow541;
		float fSlow543 = fConst8 * fSlow542 * fSlow1;
		float fSlow544 = fSlow543 + -1.499995f;
		int iSlow545 = static_cast<int>(fSlow544);
		int iSlow546 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow545 + 4))));
		int iSlow547 = iSlow546 + 1;
		float fSlow548 = std::floor(fSlow544);
		float fSlow549 = fSlow543 + (-3.0f - fSlow548);
		float fSlow550 = fSlow543 + (-2.0f - fSlow548);
		float fSlow551 = fSlow543 + (-1.0f - fSlow548);
		float fSlow552 = fSlow543 - fSlow548;
		float fSlow553 = fSlow552 * fSlow551;
		float fSlow554 = fSlow553 * fSlow550;
		float fSlow555 = 0.041666668f * fSlow554 * fSlow549;
		int iSlow556 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow545 + 3))));
		int iSlow557 = iSlow556 + 1;
		float fSlow558 = 0.16666667f * fSlow554;
		int iSlow559 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow545 + 2))));
		int iSlow560 = iSlow559 + 1;
		float fSlow561 = 0.25f * fSlow553;
		int iSlow562 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow545 + 1))));
		int iSlow563 = iSlow562 + 1;
		float fSlow564 = 0.16666667f * fSlow552;
		int iSlow565 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow545))));
		int iSlow566 = iSlow565 + 1;
		float fSlow567 = 0.041666668f * fSlow551;
		float fSlow568 = fSlow543 + (-4.0f - fSlow548);
		float fSlow569 = static_cast<float>(fButton5);
		float fSlow570 = fConst8 * fSlow541 * fSlow1;
		float fSlow571 = fSlow570 + -1.499995f;
		int iSlow572 = static_cast<int>(fSlow571);
		int iSlow573 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow572 + 4))));
		int iSlow574 = iSlow573 + 2;
		float fSlow575 = std::floor(fSlow571);
		float fSlow576 = fSlow570 + (-3.0f - fSlow575);
		float fSlow577 = fSlow570 + (-2.0f - fSlow575);
		float fSlow578 = fSlow570 + (-1.0f - fSlow575);
		float fSlow579 = fSlow570 - fSlow575;
		float fSlow580 = fSlow579 * fSlow578;
		float fSlow581 = fSlow580 * fSlow577;
		float fSlow582 = 0.041666668f * fSlow581 * fSlow576;
		int iSlow583 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow572 + 3))));
		int iSlow584 = iSlow583 + 2;
		float fSlow585 = 0.16666667f * fSlow581;
		int iSlow586 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow572 + 2))));
		int iSlow587 = iSlow586 + 2;
		float fSlow588 = 0.25f * fSlow580;
		int iSlow589 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow572 + 1))));
		int iSlow590 = iSlow589 + 2;
		float fSlow591 = 0.16666667f * fSlow579;
		int iSlow592 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow572))));
		int iSlow593 = iSlow592 + 2;
		float fSlow594 = 0.041666668f * fSlow578;
		float fSlow595 = fSlow570 + (-4.0f - fSlow575);
		int iSlow596 = iSlow573 + 1;
		int iSlow597 = iSlow583 + 1;
		int iSlow598 = iSlow586 + 1;
		int iSlow599 = iSlow589 + 1;
		int iSlow600 = iSlow592 + 1;
		float fSlow601 = fConst8 * fSlow541 * fSlow132;
		float fSlow602 = fSlow601 + -1.499995f;
		int iSlow603 = static_cast<int>(fSlow602);
		int iSlow604 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow603 + 4))));
		int iSlow605 = iSlow604 + 1;
		float fSlow606 = std::floor(fSlow602);
		float fSlow607 = fSlow601 + (-3.0f - fSlow606);
		float fSlow608 = fSlow601 + (-2.0f - fSlow606);
		float fSlow609 = fSlow601 + (-1.0f - fSlow606);
		float fSlow610 = fSlow601 - fSlow606;
		float fSlow611 = fSlow610 * fSlow609;
		float fSlow612 = fSlow611 * fSlow608;
		float fSlow613 = 0.041666668f * fSlow612 * fSlow607;
		int iSlow614 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow603 + 3))));
		int iSlow615 = iSlow614 + 1;
		float fSlow616 = 0.16666667f * fSlow612;
		int iSlow617 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow603 + 2))));
		int iSlow618 = iSlow617 + 1;
		float fSlow619 = 0.25f * fSlow611;
		int iSlow620 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow603 + 1))));
		int iSlow621 = iSlow620 + 1;
		float fSlow622 = 0.16666667f * fSlow610;
		int iSlow623 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow603))));
		int iSlow624 = iSlow623 + 1;
		float fSlow625 = 0.041666668f * fSlow609;
		float fSlow626 = fSlow601 + (-4.0f - fSlow606);
		float fSlow627 = fConst8 * fSlow542 * fSlow132;
		float fSlow628 = fSlow627 + -1.499995f;
		int iSlow629 = static_cast<int>(fSlow628);
		int iSlow630 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow629 + 4))));
		int iSlow631 = iSlow630 + 2;
		float fSlow632 = std::floor(fSlow628);
		float fSlow633 = fSlow627 + (-3.0f - fSlow632);
		float fSlow634 = fSlow627 + (-2.0f - fSlow632);
		float fSlow635 = fSlow627 + (-1.0f - fSlow632);
		float fSlow636 = fSlow627 - fSlow632;
		float fSlow637 = fSlow636 * fSlow635;
		float fSlow638 = fSlow637 * fSlow634;
		float fSlow639 = 0.041666668f * fSlow638 * fSlow633;
		int iSlow640 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow629 + 3))));
		int iSlow641 = iSlow640 + 2;
		float fSlow642 = 0.16666667f * fSlow638;
		int iSlow643 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow629 + 2))));
		int iSlow644 = iSlow643 + 2;
		float fSlow645 = 0.25f * fSlow637;
		int iSlow646 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow629 + 1))));
		int iSlow647 = iSlow646 + 2;
		float fSlow648 = 0.16666667f * fSlow636;
		int iSlow649 = static_cast<int>(std::min<float>(fConst7, static_cast<float>(std::max<int>(0, iSlow629))));
		int iSlow650 = iSlow649 + 2;
		float fSlow651 = 0.041666668f * fSlow635;
		float fSlow652 = fSlow627 + (-4.0f - fSlow632);
		int iSlow653 = iSlow630 + 1;
		int iSlow654 = iSlow640 + 1;
		int iSlow655 = iSlow643 + 1;
		int iSlow656 = iSlow646 + 1;
		int iSlow657 = iSlow649 + 1;
		float fSlow658 = static_cast<float>(fHslider11);
		float fSlow659 = static_cast<float>(fHslider12);
		float fSlow660 = std::tan(fConst117 * fSlow659);
		float fSlow661 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fSlow660));
		float fSlow662 = 1.0f / fSlow660;
		float fSlow663 = (fSlow662 + -0.09090909f) / fSlow660 + 1.0f;
		float fSlow664 = (fSlow662 + 0.09090909f) / fSlow660 + 1.0f;
		float fSlow665 = 1.0f / fSlow664;
		float fSlow666 = 0.03181818f / (fSlow660 * fSlow664);
		float fSlow667 = std::tan(fConst118 * fSlow659);
		float fSlow668 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fSlow667));
		float fSlow669 = 1.0f / fSlow667;
		float fSlow670 = (fSlow669 + -0.07692308f) / fSlow667 + 1.0f;
		float fSlow671 = (fSlow669 + 0.07692308f) / fSlow667 + 1.0f;
		float fSlow672 = 1.0f / fSlow671;
		float fSlow673 = 0.034615386f / (fSlow667 * fSlow671);
		float fSlow674 = std::tan(fConst119 * fSlow659);
		float fSlow675 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fSlow674));
		float fSlow676 = 1.0f / fSlow674;
		float fSlow677 = (fSlow676 + -0.0625f) / fSlow674 + 1.0f;
		float fSlow678 = (fSlow676 + 0.0625f) / fSlow674 + 1.0f;
		float fSlow679 = 1.0f / fSlow678;
		float fSlow680 = 0.0375f / (fSlow674 * fSlow678);
		float fSlow681 = std::tan(fConst119 * static_cast<float>(fHslider13));
		float fSlow682 = 2.0f * (1.0f - 1.0f / ouddsp_faustpower2_f(fSlow681));
		float fSlow683 = 1.0f / fSlow681;
		float fSlow684 = (fSlow683 + -0.045454547f) / fSlow681 + 1.0f;
		float fSlow685 = (fSlow683 + 0.045454547f) / fSlow681 + 1.0f;
		float fSlow686 = 1.0f / fSlow685;
		float fSlow687 = 0.036363635f / (fSlow681 * fSlow685);
		float fSlow688 = static_cast<float>(fHslider14);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			fVec0[0] = static_cast<float>(input0[i0]);
			iRec11[0] = 0;
			int iRec12 = iRec11[1];
			float fRec15 = static_cast<float>(iRec7[1]) - 0.9978437f * (0.7f * fRec16[2] + 0.15f * (fRec16[1] + fRec16[3]));
			fRec23[0] = fSlow29 * (fSlow10 * (fSlow11 * (fSlow28 * fRec3[(IOTA0 - iSlow27) & 2047] - fSlow25 * fRec3[(IOTA0 - iSlow24) & 2047]) + fSlow22 * fRec3[(IOTA0 - iSlow21) & 2047]) - fSlow19 * fRec3[(IOTA0 - iSlow18) & 2047]) + fSlow16 * fRec3[(IOTA0 - iSlow8) & 2047];
			fRec26[0] = 0.6f * fRec23[1] + 0.4f * fRec26[1];
			float fRec24 = fRec26[0];
			fVec1[0] = fSlow30;
			iRec28[0] = (fSlow30 > fVec1[1]) + (fSlow30 <= fVec1[1]) * (iRec28[1] + (iRec28[1] > 0));
			float fTemp0 = fSlow33 * static_cast<float>(iRec28[0]);
			iRec30[0] = 1103515245 * iRec30[1] + 12345;
			fRec29[0] = 7.0e-8f * static_cast<float>(iRec30[0]) - fSlow40 * (fSlow38 * fRec29[2] + fSlow36 * fRec29[1]);
			float fTemp1 = fRec29[2] + fRec29[0] + 2.0f * fRec29[1];
			float fTemp2 = fSlow41 * fTemp1 * std::max<float>(0.0f, std::min<float>(fTemp0, 2.0f - fTemp0));
			fRec31[0] = fRec1[1];
			fRec32[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec31[2] + 0.15f * (fRec31[1] + fRec31[3])));
			fVec2[0] = fSlow67 * (fSlow48 * (fSlow49 * (fSlow66 * fRec32[(IOTA0 - iSlow65) & 2047] - fSlow63 * fRec32[(IOTA0 - iSlow62) & 2047]) + fSlow60 * fRec32[(IOTA0 - iSlow59) & 2047]) - fSlow57 * fRec32[(IOTA0 - iSlow56) & 2047]) + fSlow54 * fRec32[(IOTA0 - iSlow46) & 2047];
			fVec3[0] = fVec2[1] + fTemp2;
			fRec27[IOTA0 & 2047] = 0.6f * fVec3[1] + 0.4f * fRec27[(IOTA0 - 1) & 2047];
			float fRec25 = fSlow29 * (fSlow10 * (fSlow11 * (fSlow28 * fRec27[(IOTA0 - iSlow26) & 2047] - fSlow25 * fRec27[(IOTA0 - iSlow23) & 2047]) + fSlow22 * fRec27[(IOTA0 - iSlow20) & 2047]) - fSlow19 * fRec27[(IOTA0 - iSlow17) & 2047]) + fSlow16 * fRec27[(IOTA0 - iSlow7) & 2047];
			fRec20[0] = fRec24;
			float fRec21 = fTemp2 + fRec20[1];
			float fRec22 = fRec25;
			fRec17[IOTA0 & 2047] = fRec21;
			float fRec18 = fSlow67 * (fSlow48 * (fSlow49 * (fSlow66 * fRec17[(IOTA0 - iSlow72) & 2047] - fSlow63 * fRec17[(IOTA0 - iSlow71) & 2047]) + fSlow60 * fRec17[(IOTA0 - iSlow70) & 2047]) - fSlow57 * fRec17[(IOTA0 - iSlow69) & 2047]) + fSlow54 * fRec17[(IOTA0 - iSlow68) & 2047];
			fRec19[0] = fRec22;
			fRec16[0] = fRec19[1];
			float fRec13 = fRec16[1];
			float fRec14 = fRec16[1];
			iRec7[0] = iRec12;
			float fRec8 = fRec15;
			float fRec9 = fRec13;
			float fRec10 = fRec14;
			fRec3[IOTA0 & 2047] = fRec8;
			float fRec4 = fRec18;
			float fRec5 = fRec9;
			float fRec6 = fRec10;
			fRec1[0] = fRec4;
			float fRec2 = fRec6;
			iRec43[0] = 0;
			int iRec44 = iRec43[1];
			float fRec47 = static_cast<float>(iRec39[1]) - 0.9978437f * (0.7f * fRec48[2] + 0.15f * (fRec48[1] + fRec48[3]));
			fRec55[0] = fSlow99 * (fSlow80 * (fSlow81 * (fSlow98 * fRec35[(IOTA0 - iSlow97) & 2047] - fSlow95 * fRec35[(IOTA0 - iSlow94) & 2047]) + fSlow92 * fRec35[(IOTA0 - iSlow91) & 2047]) - fSlow89 * fRec35[(IOTA0 - iSlow88) & 2047]) + fSlow86 * fRec35[(IOTA0 - iSlow78) & 2047];
			fRec58[0] = 0.6f * fRec55[1] + 0.4f * fRec58[1];
			float fRec56 = fRec58[0];
			fVec4[0] = fSlow100;
			iRec60[0] = (fSlow100 > fVec4[1]) + (fSlow100 <= fVec4[1]) * (iRec60[1] + (iRec60[1] > 0));
			float fTemp3 = fSlow33 * static_cast<float>(iRec60[0]);
			float fTemp4 = fSlow41 * fTemp1 * std::max<float>(0.0f, std::min<float>(fTemp3, 2.0f - fTemp3));
			fRec61[0] = fRec33[1];
			fRec62[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec61[2] + 0.15f * (fRec61[1] + fRec61[3])));
			fVec5[0] = fSlow126 * (fSlow107 * (fSlow108 * (fSlow125 * fRec62[(IOTA0 - iSlow124) & 2047] - fSlow122 * fRec62[(IOTA0 - iSlow121) & 2047]) + fSlow119 * fRec62[(IOTA0 - iSlow118) & 2047]) - fSlow116 * fRec62[(IOTA0 - iSlow115) & 2047]) + fSlow113 * fRec62[(IOTA0 - iSlow105) & 2047];
			fVec6[0] = fVec5[1] + fTemp4;
			fRec59[IOTA0 & 2047] = 0.6f * fVec6[1] + 0.4f * fRec59[(IOTA0 - 1) & 2047];
			float fRec57 = fSlow99 * (fSlow80 * (fSlow81 * (fSlow98 * fRec59[(IOTA0 - iSlow96) & 2047] - fSlow95 * fRec59[(IOTA0 - iSlow93) & 2047]) + fSlow92 * fRec59[(IOTA0 - iSlow90) & 2047]) - fSlow89 * fRec59[(IOTA0 - iSlow87) & 2047]) + fSlow86 * fRec59[(IOTA0 - iSlow77) & 2047];
			fRec52[0] = fRec56;
			float fRec53 = fTemp4 + fRec52[1];
			float fRec54 = fRec57;
			fRec49[IOTA0 & 2047] = fRec53;
			float fRec50 = fSlow126 * (fSlow107 * (fSlow108 * (fSlow125 * fRec49[(IOTA0 - iSlow131) & 2047] - fSlow122 * fRec49[(IOTA0 - iSlow130) & 2047]) + fSlow119 * fRec49[(IOTA0 - iSlow129) & 2047]) - fSlow116 * fRec49[(IOTA0 - iSlow128) & 2047]) + fSlow113 * fRec49[(IOTA0 - iSlow127) & 2047];
			fRec51[0] = fRec54;
			fRec48[0] = fRec51[1];
			float fRec45 = fRec48[1];
			float fRec46 = fRec48[1];
			iRec39[0] = iRec44;
			float fRec40 = fRec47;
			float fRec41 = fRec45;
			float fRec42 = fRec46;
			fRec35[IOTA0 & 2047] = fRec40;
			float fRec36 = fRec50;
			float fRec37 = fRec41;
			float fRec38 = fRec42;
			fRec33[0] = fRec36;
			float fRec34 = fRec38;
			iRec73[0] = 0;
			int iRec74 = iRec73[1];
			float fRec77 = static_cast<float>(iRec69[1]) - 0.9978437f * (0.7f * fRec78[2] + 0.15f * (fRec78[1] + fRec78[3]));
			fRec85[0] = fSlow158 * (fSlow139 * (fSlow140 * (fSlow157 * fRec65[(IOTA0 - iSlow156) & 2047] - fSlow154 * fRec65[(IOTA0 - iSlow153) & 2047]) + fSlow151 * fRec65[(IOTA0 - iSlow150) & 2047]) - fSlow148 * fRec65[(IOTA0 - iSlow147) & 2047]) + fSlow145 * fRec65[(IOTA0 - iSlow137) & 2047];
			fRec88[0] = 0.6f * fRec85[1] + 0.4f * fRec88[1];
			float fRec86 = fRec88[0];
			fRec90[0] = fRec63[1];
			fRec91[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec90[2] + 0.15f * (fRec90[1] + fRec90[3])));
			fVec7[0] = fSlow184 * (fSlow165 * (fSlow166 * (fSlow183 * fRec91[(IOTA0 - iSlow182) & 2047] - fSlow180 * fRec91[(IOTA0 - iSlow179) & 2047]) + fSlow177 * fRec91[(IOTA0 - iSlow176) & 2047]) - fSlow174 * fRec91[(IOTA0 - iSlow173) & 2047]) + fSlow171 * fRec91[(IOTA0 - iSlow163) & 2047];
			fVec8[0] = fTemp2 + fVec7[1];
			fRec89[IOTA0 & 2047] = 0.6f * fVec8[1] + 0.4f * fRec89[(IOTA0 - 1) & 2047];
			float fRec87 = fSlow158 * (fSlow139 * (fSlow140 * (fSlow157 * fRec89[(IOTA0 - iSlow155) & 2047] - fSlow154 * fRec89[(IOTA0 - iSlow152) & 2047]) + fSlow151 * fRec89[(IOTA0 - iSlow149) & 2047]) - fSlow148 * fRec89[(IOTA0 - iSlow146) & 2047]) + fSlow145 * fRec89[(IOTA0 - iSlow136) & 2047];
			fRec82[0] = fRec86;
			float fRec83 = fTemp2 + fRec82[1];
			float fRec84 = fRec87;
			fRec79[IOTA0 & 2047] = fRec83;
			float fRec80 = fSlow184 * (fSlow165 * (fSlow166 * (fSlow183 * fRec79[(IOTA0 - iSlow189) & 2047] - fSlow180 * fRec79[(IOTA0 - iSlow188) & 2047]) + fSlow177 * fRec79[(IOTA0 - iSlow187) & 2047]) - fSlow174 * fRec79[(IOTA0 - iSlow186) & 2047]) + fSlow171 * fRec79[(IOTA0 - iSlow185) & 2047];
			fRec81[0] = fRec84;
			fRec78[0] = fRec81[1];
			float fRec75 = fRec78[1];
			float fRec76 = fRec78[1];
			iRec69[0] = iRec74;
			float fRec70 = fRec77;
			float fRec71 = fRec75;
			float fRec72 = fRec76;
			fRec65[IOTA0 & 2047] = fRec70;
			float fRec66 = fRec80;
			float fRec67 = fRec71;
			float fRec68 = fRec72;
			fRec63[0] = fRec66;
			float fRec64 = fRec68;
			iRec102[0] = 0;
			int iRec103 = iRec102[1];
			float fRec106 = static_cast<float>(iRec98[1]) - 0.9978437f * (0.7f * fRec107[2] + 0.15f * (fRec107[1] + fRec107[3]));
			fRec114[0] = fSlow217 * (fSlow198 * (fSlow199 * (fSlow216 * fRec94[(IOTA0 - iSlow215) & 2047] - fSlow213 * fRec94[(IOTA0 - iSlow212) & 2047]) + fSlow210 * fRec94[(IOTA0 - iSlow209) & 2047]) - fSlow207 * fRec94[(IOTA0 - iSlow206) & 2047]) + fSlow204 * fRec94[(IOTA0 - iSlow196) & 2047];
			fRec117[0] = 0.6f * fRec114[1] + 0.4f * fRec117[1];
			float fRec115 = fRec117[0];
			fVec9[0] = fSlow218;
			iRec119[0] = (fSlow218 > fVec9[1]) + (fSlow218 <= fVec9[1]) * (iRec119[1] + (iRec119[1] > 0));
			float fTemp5 = fSlow33 * static_cast<float>(iRec119[0]);
			float fTemp6 = fSlow41 * fTemp1 * std::max<float>(0.0f, std::min<float>(fTemp5, 2.0f - fTemp5));
			fRec120[0] = fRec92[1];
			fRec121[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec120[2] + 0.15f * (fRec120[1] + fRec120[3])));
			fVec10[0] = fSlow244 * (fSlow225 * (fSlow226 * (fSlow243 * fRec121[(IOTA0 - iSlow242) & 2047] - fSlow240 * fRec121[(IOTA0 - iSlow239) & 2047]) + fSlow237 * fRec121[(IOTA0 - iSlow236) & 2047]) - fSlow234 * fRec121[(IOTA0 - iSlow233) & 2047]) + fSlow231 * fRec121[(IOTA0 - iSlow223) & 2047];
			fVec11[0] = fVec10[1] + fTemp6;
			fRec118[IOTA0 & 2047] = 0.6f * fVec11[1] + 0.4f * fRec118[(IOTA0 - 1) & 2047];
			float fRec116 = fSlow217 * (fSlow198 * (fSlow199 * (fSlow216 * fRec118[(IOTA0 - iSlow214) & 2047] - fSlow213 * fRec118[(IOTA0 - iSlow211) & 2047]) + fSlow210 * fRec118[(IOTA0 - iSlow208) & 2047]) - fSlow207 * fRec118[(IOTA0 - iSlow205) & 2047]) + fSlow204 * fRec118[(IOTA0 - iSlow195) & 2047];
			fRec111[0] = fRec115;
			float fRec112 = fTemp6 + fRec111[1];
			float fRec113 = fRec116;
			fRec108[IOTA0 & 2047] = fRec112;
			float fRec109 = fSlow244 * (fSlow225 * (fSlow226 * (fSlow243 * fRec108[(IOTA0 - iSlow249) & 2047] - fSlow240 * fRec108[(IOTA0 - iSlow248) & 2047]) + fSlow237 * fRec108[(IOTA0 - iSlow247) & 2047]) - fSlow234 * fRec108[(IOTA0 - iSlow246) & 2047]) + fSlow231 * fRec108[(IOTA0 - iSlow245) & 2047];
			fRec110[0] = fRec113;
			fRec107[0] = fRec110[1];
			float fRec104 = fRec107[1];
			float fRec105 = fRec107[1];
			iRec98[0] = iRec103;
			float fRec99 = fRec106;
			float fRec100 = fRec104;
			float fRec101 = fRec105;
			fRec94[IOTA0 & 2047] = fRec99;
			float fRec95 = fRec109;
			float fRec96 = fRec100;
			float fRec97 = fRec101;
			fRec92[0] = fRec95;
			float fRec93 = fRec97;
			iRec132[0] = 0;
			int iRec133 = iRec132[1];
			float fRec136 = static_cast<float>(iRec128[1]) - 0.9978437f * (0.7f * fRec137[2] + 0.15f * (fRec137[1] + fRec137[3]));
			fRec144[0] = fSlow275 * (fSlow256 * (fSlow257 * (fSlow274 * fRec124[(IOTA0 - iSlow273) & 2047] - fSlow271 * fRec124[(IOTA0 - iSlow270) & 2047]) + fSlow268 * fRec124[(IOTA0 - iSlow267) & 2047]) - fSlow265 * fRec124[(IOTA0 - iSlow264) & 2047]) + fSlow262 * fRec124[(IOTA0 - iSlow254) & 2047];
			fRec147[0] = 0.6f * fRec144[1] + 0.4f * fRec147[1];
			float fRec145 = fRec147[0];
			fRec149[0] = fRec122[1];
			fRec150[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec149[2] + 0.15f * (fRec149[1] + fRec149[3])));
			fVec12[0] = fSlow301 * (fSlow282 * (fSlow283 * (fSlow300 * fRec150[(IOTA0 - iSlow299) & 2047] - fSlow297 * fRec150[(IOTA0 - iSlow296) & 2047]) + fSlow294 * fRec150[(IOTA0 - iSlow293) & 2047]) - fSlow291 * fRec150[(IOTA0 - iSlow290) & 2047]) + fSlow288 * fRec150[(IOTA0 - iSlow280) & 2047];
			fVec13[0] = fTemp6 + fVec12[1];
			fRec148[IOTA0 & 2047] = 0.6f * fVec13[1] + 0.4f * fRec148[(IOTA0 - 1) & 2047];
			float fRec146 = fSlow275 * (fSlow256 * (fSlow257 * (fSlow274 * fRec148[(IOTA0 - iSlow272) & 2047] - fSlow271 * fRec148[(IOTA0 - iSlow269) & 2047]) + fSlow268 * fRec148[(IOTA0 - iSlow266) & 2047]) - fSlow265 * fRec148[(IOTA0 - iSlow263) & 2047]) + fSlow262 * fRec148[(IOTA0 - iSlow253) & 2047];
			fRec141[0] = fRec145;
			float fRec142 = fTemp6 + fRec141[1];
			float fRec143 = fRec146;
			fRec138[IOTA0 & 2047] = fRec142;
			float fRec139 = fSlow301 * (fSlow282 * (fSlow283 * (fSlow300 * fRec138[(IOTA0 - iSlow306) & 2047] - fSlow297 * fRec138[(IOTA0 - iSlow305) & 2047]) + fSlow294 * fRec138[(IOTA0 - iSlow304) & 2047]) - fSlow291 * fRec138[(IOTA0 - iSlow303) & 2047]) + fSlow288 * fRec138[(IOTA0 - iSlow302) & 2047];
			fRec140[0] = fRec143;
			fRec137[0] = fRec140[1];
			float fRec134 = fRec137[1];
			float fRec135 = fRec137[1];
			iRec128[0] = iRec133;
			float fRec129 = fRec136;
			float fRec130 = fRec134;
			float fRec131 = fRec135;
			fRec124[IOTA0 & 2047] = fRec129;
			float fRec125 = fRec139;
			float fRec126 = fRec130;
			float fRec127 = fRec131;
			fRec122[0] = fRec125;
			float fRec123 = fRec127;
			iRec161[0] = 0;
			int iRec162 = iRec161[1];
			float fRec165 = static_cast<float>(iRec157[1]) - 0.9978437f * (0.7f * fRec166[2] + 0.15f * (fRec166[1] + fRec166[3]));
			fRec173[0] = fSlow334 * (fSlow315 * (fSlow316 * (fSlow333 * fRec153[(IOTA0 - iSlow332) & 2047] - fSlow330 * fRec153[(IOTA0 - iSlow329) & 2047]) + fSlow327 * fRec153[(IOTA0 - iSlow326) & 2047]) - fSlow324 * fRec153[(IOTA0 - iSlow323) & 2047]) + fSlow321 * fRec153[(IOTA0 - iSlow313) & 2047];
			fRec176[0] = 0.6f * fRec173[1] + 0.4f * fRec176[1];
			float fRec174 = fRec176[0];
			fVec14[0] = fSlow335;
			iRec178[0] = (fSlow335 > fVec14[1]) + (fSlow335 <= fVec14[1]) * (iRec178[1] + (iRec178[1] > 0));
			float fTemp7 = fSlow33 * static_cast<float>(iRec178[0]);
			float fTemp8 = fSlow41 * fTemp1 * std::max<float>(0.0f, std::min<float>(fTemp7, 2.0f - fTemp7));
			fRec179[0] = fRec151[1];
			fRec180[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec179[2] + 0.15f * (fRec179[1] + fRec179[3])));
			fVec15[0] = fSlow361 * (fSlow342 * (fSlow343 * (fSlow360 * fRec180[(IOTA0 - iSlow359) & 2047] - fSlow357 * fRec180[(IOTA0 - iSlow356) & 2047]) + fSlow354 * fRec180[(IOTA0 - iSlow353) & 2047]) - fSlow351 * fRec180[(IOTA0 - iSlow350) & 2047]) + fSlow348 * fRec180[(IOTA0 - iSlow340) & 2047];
			fVec16[0] = fVec15[1] + fTemp8;
			fRec177[IOTA0 & 2047] = 0.6f * fVec16[1] + 0.4f * fRec177[(IOTA0 - 1) & 2047];
			float fRec175 = fSlow334 * (fSlow315 * (fSlow316 * (fSlow333 * fRec177[(IOTA0 - iSlow331) & 2047] - fSlow330 * fRec177[(IOTA0 - iSlow328) & 2047]) + fSlow327 * fRec177[(IOTA0 - iSlow325) & 2047]) - fSlow324 * fRec177[(IOTA0 - iSlow322) & 2047]) + fSlow321 * fRec177[(IOTA0 - iSlow312) & 2047];
			fRec170[0] = fRec174;
			float fRec171 = fTemp8 + fRec170[1];
			float fRec172 = fRec175;
			fRec167[IOTA0 & 2047] = fRec171;
			float fRec168 = fSlow361 * (fSlow342 * (fSlow343 * (fSlow360 * fRec167[(IOTA0 - iSlow366) & 2047] - fSlow357 * fRec167[(IOTA0 - iSlow365) & 2047]) + fSlow354 * fRec167[(IOTA0 - iSlow364) & 2047]) - fSlow351 * fRec167[(IOTA0 - iSlow363) & 2047]) + fSlow348 * fRec167[(IOTA0 - iSlow362) & 2047];
			fRec169[0] = fRec172;
			fRec166[0] = fRec169[1];
			float fRec163 = fRec166[1];
			float fRec164 = fRec166[1];
			iRec157[0] = iRec162;
			float fRec158 = fRec165;
			float fRec159 = fRec163;
			float fRec160 = fRec164;
			fRec153[IOTA0 & 2047] = fRec158;
			float fRec154 = fRec168;
			float fRec155 = fRec159;
			float fRec156 = fRec160;
			fRec151[0] = fRec154;
			float fRec152 = fRec156;
			iRec191[0] = 0;
			int iRec192 = iRec191[1];
			float fRec195 = static_cast<float>(iRec187[1]) - 0.9978437f * (0.7f * fRec196[2] + 0.15f * (fRec196[1] + fRec196[3]));
			fRec203[0] = fSlow392 * (fSlow373 * (fSlow374 * (fSlow391 * fRec183[(IOTA0 - iSlow390) & 2047] - fSlow388 * fRec183[(IOTA0 - iSlow387) & 2047]) + fSlow385 * fRec183[(IOTA0 - iSlow384) & 2047]) - fSlow382 * fRec183[(IOTA0 - iSlow381) & 2047]) + fSlow379 * fRec183[(IOTA0 - iSlow371) & 2047];
			fRec206[0] = 0.6f * fRec203[1] + 0.4f * fRec206[1];
			float fRec204 = fRec206[0];
			fRec208[0] = fRec181[1];
			fRec209[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec208[2] + 0.15f * (fRec208[1] + fRec208[3])));
			fVec17[0] = fSlow418 * (fSlow399 * (fSlow400 * (fSlow417 * fRec209[(IOTA0 - iSlow416) & 2047] - fSlow414 * fRec209[(IOTA0 - iSlow413) & 2047]) + fSlow411 * fRec209[(IOTA0 - iSlow410) & 2047]) - fSlow408 * fRec209[(IOTA0 - iSlow407) & 2047]) + fSlow405 * fRec209[(IOTA0 - iSlow397) & 2047];
			fVec18[0] = fTemp8 + fVec17[1];
			fRec207[IOTA0 & 2047] = 0.6f * fVec18[1] + 0.4f * fRec207[(IOTA0 - 1) & 2047];
			float fRec205 = fSlow392 * (fSlow373 * (fSlow374 * (fSlow391 * fRec207[(IOTA0 - iSlow389) & 2047] - fSlow388 * fRec207[(IOTA0 - iSlow386) & 2047]) + fSlow385 * fRec207[(IOTA0 - iSlow383) & 2047]) - fSlow382 * fRec207[(IOTA0 - iSlow380) & 2047]) + fSlow379 * fRec207[(IOTA0 - iSlow370) & 2047];
			fRec200[0] = fRec204;
			float fRec201 = fTemp8 + fRec200[1];
			float fRec202 = fRec205;
			fRec197[IOTA0 & 2047] = fRec201;
			float fRec198 = fSlow418 * (fSlow399 * (fSlow400 * (fSlow417 * fRec197[(IOTA0 - iSlow423) & 2047] - fSlow414 * fRec197[(IOTA0 - iSlow422) & 2047]) + fSlow411 * fRec197[(IOTA0 - iSlow421) & 2047]) - fSlow408 * fRec197[(IOTA0 - iSlow420) & 2047]) + fSlow405 * fRec197[(IOTA0 - iSlow419) & 2047];
			fRec199[0] = fRec202;
			fRec196[0] = fRec199[1];
			float fRec193 = fRec196[1];
			float fRec194 = fRec196[1];
			iRec187[0] = iRec192;
			float fRec188 = fRec195;
			float fRec189 = fRec193;
			float fRec190 = fRec194;
			fRec183[IOTA0 & 2047] = fRec188;
			float fRec184 = fRec198;
			float fRec185 = fRec189;
			float fRec186 = fRec190;
			fRec181[0] = fRec184;
			float fRec182 = fRec186;
			iRec220[0] = 0;
			int iRec221 = iRec220[1];
			float fRec224 = static_cast<float>(iRec216[1]) - 0.9978437f * (0.7f * fRec225[2] + 0.15f * (fRec225[1] + fRec225[3]));
			fRec232[0] = fSlow451 * (fSlow432 * (fSlow433 * (fSlow450 * fRec212[(IOTA0 - iSlow449) & 2047] - fSlow447 * fRec212[(IOTA0 - iSlow446) & 2047]) + fSlow444 * fRec212[(IOTA0 - iSlow443) & 2047]) - fSlow441 * fRec212[(IOTA0 - iSlow440) & 2047]) + fSlow438 * fRec212[(IOTA0 - iSlow430) & 2047];
			fRec235[0] = 0.6f * fRec232[1] + 0.4f * fRec235[1];
			float fRec233 = fRec235[0];
			fVec19[0] = fSlow452;
			iRec237[0] = (fSlow452 > fVec19[1]) + (fSlow452 <= fVec19[1]) * (iRec237[1] + (iRec237[1] > 0));
			float fTemp9 = fSlow33 * static_cast<float>(iRec237[0]);
			float fTemp10 = fSlow41 * fTemp1 * std::max<float>(0.0f, std::min<float>(fTemp9, 2.0f - fTemp9));
			fRec238[0] = fRec210[1];
			fRec239[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec238[2] + 0.15f * (fRec238[1] + fRec238[3])));
			fVec20[0] = fSlow478 * (fSlow459 * (fSlow460 * (fSlow477 * fRec239[(IOTA0 - iSlow476) & 2047] - fSlow474 * fRec239[(IOTA0 - iSlow473) & 2047]) + fSlow471 * fRec239[(IOTA0 - iSlow470) & 2047]) - fSlow468 * fRec239[(IOTA0 - iSlow467) & 2047]) + fSlow465 * fRec239[(IOTA0 - iSlow457) & 2047];
			fVec21[0] = fVec20[1] + fTemp10;
			fRec236[IOTA0 & 2047] = 0.6f * fVec21[1] + 0.4f * fRec236[(IOTA0 - 1) & 2047];
			float fRec234 = fSlow451 * (fSlow432 * (fSlow433 * (fSlow450 * fRec236[(IOTA0 - iSlow448) & 2047] - fSlow447 * fRec236[(IOTA0 - iSlow445) & 2047]) + fSlow444 * fRec236[(IOTA0 - iSlow442) & 2047]) - fSlow441 * fRec236[(IOTA0 - iSlow439) & 2047]) + fSlow438 * fRec236[(IOTA0 - iSlow429) & 2047];
			fRec229[0] = fRec233;
			float fRec230 = fTemp10 + fRec229[1];
			float fRec231 = fRec234;
			fRec226[IOTA0 & 2047] = fRec230;
			float fRec227 = fSlow478 * (fSlow459 * (fSlow460 * (fSlow477 * fRec226[(IOTA0 - iSlow483) & 2047] - fSlow474 * fRec226[(IOTA0 - iSlow482) & 2047]) + fSlow471 * fRec226[(IOTA0 - iSlow481) & 2047]) - fSlow468 * fRec226[(IOTA0 - iSlow480) & 2047]) + fSlow465 * fRec226[(IOTA0 - iSlow479) & 2047];
			fRec228[0] = fRec231;
			fRec225[0] = fRec228[1];
			float fRec222 = fRec225[1];
			float fRec223 = fRec225[1];
			iRec216[0] = iRec221;
			float fRec217 = fRec224;
			float fRec218 = fRec222;
			float fRec219 = fRec223;
			fRec212[IOTA0 & 2047] = fRec217;
			float fRec213 = fRec227;
			float fRec214 = fRec218;
			float fRec215 = fRec219;
			fRec210[0] = fRec213;
			float fRec211 = fRec215;
			iRec250[0] = 0;
			int iRec251 = iRec250[1];
			float fRec254 = static_cast<float>(iRec246[1]) - 0.9978437f * (0.7f * fRec255[2] + 0.15f * (fRec255[1] + fRec255[3]));
			fRec262[0] = fSlow509 * (fSlow490 * (fSlow491 * (fSlow508 * fRec242[(IOTA0 - iSlow507) & 2047] - fSlow505 * fRec242[(IOTA0 - iSlow504) & 2047]) + fSlow502 * fRec242[(IOTA0 - iSlow501) & 2047]) - fSlow499 * fRec242[(IOTA0 - iSlow498) & 2047]) + fSlow496 * fRec242[(IOTA0 - iSlow488) & 2047];
			fRec265[0] = 0.6f * fRec262[1] + 0.4f * fRec265[1];
			float fRec263 = fRec265[0];
			fRec267[0] = fRec240[1];
			fRec268[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec267[2] + 0.15f * (fRec267[1] + fRec267[3])));
			fVec22[0] = fSlow535 * (fSlow516 * (fSlow517 * (fSlow534 * fRec268[(IOTA0 - iSlow533) & 2047] - fSlow531 * fRec268[(IOTA0 - iSlow530) & 2047]) + fSlow528 * fRec268[(IOTA0 - iSlow527) & 2047]) - fSlow525 * fRec268[(IOTA0 - iSlow524) & 2047]) + fSlow522 * fRec268[(IOTA0 - iSlow514) & 2047];
			fVec23[0] = fTemp10 + fVec22[1];
			fRec266[IOTA0 & 2047] = 0.6f * fVec23[1] + 0.4f * fRec266[(IOTA0 - 1) & 2047];
			float fRec264 = fSlow509 * (fSlow490 * (fSlow491 * (fSlow508 * fRec266[(IOTA0 - iSlow506) & 2047] - fSlow505 * fRec266[(IOTA0 - iSlow503) & 2047]) + fSlow502 * fRec266[(IOTA0 - iSlow500) & 2047]) - fSlow499 * fRec266[(IOTA0 - iSlow497) & 2047]) + fSlow496 * fRec266[(IOTA0 - iSlow487) & 2047];
			fRec259[0] = fRec263;
			float fRec260 = fTemp10 + fRec259[1];
			float fRec261 = fRec264;
			fRec256[IOTA0 & 2047] = fRec260;
			float fRec257 = fSlow535 * (fSlow516 * (fSlow517 * (fSlow534 * fRec256[(IOTA0 - iSlow540) & 2047] - fSlow531 * fRec256[(IOTA0 - iSlow539) & 2047]) + fSlow528 * fRec256[(IOTA0 - iSlow538) & 2047]) - fSlow525 * fRec256[(IOTA0 - iSlow537) & 2047]) + fSlow522 * fRec256[(IOTA0 - iSlow536) & 2047];
			fRec258[0] = fRec261;
			fRec255[0] = fRec258[1];
			float fRec252 = fRec255[1];
			float fRec253 = fRec255[1];
			iRec246[0] = iRec251;
			float fRec247 = fRec254;
			float fRec248 = fRec252;
			float fRec249 = fRec253;
			fRec242[IOTA0 & 2047] = fRec247;
			float fRec243 = fRec257;
			float fRec244 = fRec248;
			float fRec245 = fRec249;
			fRec240[0] = fRec243;
			float fRec241 = fRec245;
			iRec279[0] = 0;
			int iRec280 = iRec279[1];
			float fRec283 = static_cast<float>(iRec275[1]) - 0.9978437f * (0.7f * fRec284[2] + 0.15f * (fRec284[1] + fRec284[3]));
			fRec291[0] = fSlow568 * (fSlow549 * (fSlow550 * (fSlow567 * fRec271[(IOTA0 - iSlow566) & 2047] - fSlow564 * fRec271[(IOTA0 - iSlow563) & 2047]) + fSlow561 * fRec271[(IOTA0 - iSlow560) & 2047]) - fSlow558 * fRec271[(IOTA0 - iSlow557) & 2047]) + fSlow555 * fRec271[(IOTA0 - iSlow547) & 2047];
			fRec294[0] = 0.6f * fRec291[1] + 0.4f * fRec294[1];
			float fRec292 = fRec294[0];
			fVec24[0] = fSlow569;
			iRec296[0] = (fSlow569 > fVec24[1]) + (fSlow569 <= fVec24[1]) * (iRec296[1] + (iRec296[1] > 0));
			float fTemp11 = fSlow33 * static_cast<float>(iRec296[0]);
			float fTemp12 = fSlow41 * fTemp1 * std::max<float>(0.0f, std::min<float>(fTemp11, 2.0f - fTemp11));
			fRec297[0] = fRec269[1];
			fRec298[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec297[2] + 0.15f * (fRec297[1] + fRec297[3])));
			fVec25[0] = fSlow595 * (fSlow576 * (fSlow577 * (fSlow594 * fRec298[(IOTA0 - iSlow593) & 2047] - fSlow591 * fRec298[(IOTA0 - iSlow590) & 2047]) + fSlow588 * fRec298[(IOTA0 - iSlow587) & 2047]) - fSlow585 * fRec298[(IOTA0 - iSlow584) & 2047]) + fSlow582 * fRec298[(IOTA0 - iSlow574) & 2047];
			fVec26[0] = fVec25[1] + fTemp12;
			fRec295[IOTA0 & 2047] = 0.6f * fVec26[1] + 0.4f * fRec295[(IOTA0 - 1) & 2047];
			float fRec293 = fSlow568 * (fSlow549 * (fSlow550 * (fSlow567 * fRec295[(IOTA0 - iSlow565) & 2047] - fSlow564 * fRec295[(IOTA0 - iSlow562) & 2047]) + fSlow561 * fRec295[(IOTA0 - iSlow559) & 2047]) - fSlow558 * fRec295[(IOTA0 - iSlow556) & 2047]) + fSlow555 * fRec295[(IOTA0 - iSlow546) & 2047];
			fRec288[0] = fRec292;
			float fRec289 = fTemp12 + fRec288[1];
			float fRec290 = fRec293;
			fRec285[IOTA0 & 2047] = fRec289;
			float fRec286 = fSlow595 * (fSlow576 * (fSlow577 * (fSlow594 * fRec285[(IOTA0 - iSlow600) & 2047] - fSlow591 * fRec285[(IOTA0 - iSlow599) & 2047]) + fSlow588 * fRec285[(IOTA0 - iSlow598) & 2047]) - fSlow585 * fRec285[(IOTA0 - iSlow597) & 2047]) + fSlow582 * fRec285[(IOTA0 - iSlow596) & 2047];
			fRec287[0] = fRec290;
			fRec284[0] = fRec287[1];
			float fRec281 = fRec284[1];
			float fRec282 = fRec284[1];
			iRec275[0] = iRec280;
			float fRec276 = fRec283;
			float fRec277 = fRec281;
			float fRec278 = fRec282;
			fRec271[IOTA0 & 2047] = fRec276;
			float fRec272 = fRec286;
			float fRec273 = fRec277;
			float fRec274 = fRec278;
			fRec269[0] = fRec272;
			float fRec270 = fRec274;
			iRec309[0] = 0;
			int iRec310 = iRec309[1];
			float fRec313 = static_cast<float>(iRec305[1]) - 0.9978437f * (0.7f * fRec314[2] + 0.15f * (fRec314[1] + fRec314[3]));
			fRec321[0] = fSlow626 * (fSlow607 * (fSlow608 * (fSlow625 * fRec301[(IOTA0 - iSlow624) & 2047] - fSlow622 * fRec301[(IOTA0 - iSlow621) & 2047]) + fSlow619 * fRec301[(IOTA0 - iSlow618) & 2047]) - fSlow616 * fRec301[(IOTA0 - iSlow615) & 2047]) + fSlow613 * fRec301[(IOTA0 - iSlow605) & 2047];
			fRec324[0] = 0.6f * fRec321[1] + 0.4f * fRec324[1];
			float fRec322 = fRec324[0];
			fRec326[0] = fRec299[1];
			fRec327[IOTA0 & 2047] = -(0.9978437f * (0.7f * fRec326[2] + 0.15f * (fRec326[1] + fRec326[3])));
			fVec27[0] = fSlow652 * (fSlow633 * (fSlow634 * (fSlow651 * fRec327[(IOTA0 - iSlow650) & 2047] - fSlow648 * fRec327[(IOTA0 - iSlow647) & 2047]) + fSlow645 * fRec327[(IOTA0 - iSlow644) & 2047]) - fSlow642 * fRec327[(IOTA0 - iSlow641) & 2047]) + fSlow639 * fRec327[(IOTA0 - iSlow631) & 2047];
			fVec28[0] = fTemp12 + fVec27[1];
			fRec325[IOTA0 & 2047] = 0.6f * fVec28[1] + 0.4f * fRec325[(IOTA0 - 1) & 2047];
			float fRec323 = fSlow626 * (fSlow607 * (fSlow608 * (fSlow625 * fRec325[(IOTA0 - iSlow623) & 2047] - fSlow622 * fRec325[(IOTA0 - iSlow620) & 2047]) + fSlow619 * fRec325[(IOTA0 - iSlow617) & 2047]) - fSlow616 * fRec325[(IOTA0 - iSlow614) & 2047]) + fSlow613 * fRec325[(IOTA0 - iSlow604) & 2047];
			fRec318[0] = fRec322;
			float fRec319 = fTemp12 + fRec318[1];
			float fRec320 = fRec323;
			fRec315[IOTA0 & 2047] = fRec319;
			float fRec316 = fSlow652 * (fSlow633 * (fSlow634 * (fSlow651 * fRec315[(IOTA0 - iSlow657) & 2047] - fSlow648 * fRec315[(IOTA0 - iSlow656) & 2047]) + fSlow645 * fRec315[(IOTA0 - iSlow655) & 2047]) - fSlow642 * fRec315[(IOTA0 - iSlow654) & 2047]) + fSlow639 * fRec315[(IOTA0 - iSlow653) & 2047];
			fRec317[0] = fRec320;
			fRec314[0] = fRec317[1];
			float fRec311 = fRec314[1];
			float fRec312 = fRec314[1];
			iRec305[0] = iRec310;
			float fRec306 = fRec313;
			float fRec307 = fRec311;
			float fRec308 = fRec312;
			fRec301[IOTA0 & 2047] = fRec306;
			float fRec302 = fRec316;
			float fRec303 = fRec307;
			float fRec304 = fRec308;
			fRec299[0] = fRec302;
			float fRec300 = fRec304;
			float fTemp13 = fRec300 + fRec270 + fRec241 + fRec211 + fRec182 + fRec152 + fRec123 + fRec93 + fRec64 + fRec34 + fRec2;
			float fTemp14 = fTemp13 + fSlow658 * fVec0[static_cast<int>(std::min<float>(1.0f, std::max<float>(0.0f, fTemp13)))];
			fRec0[0] = fTemp14 - fConst6 * (fConst4 * fRec0[2] + fConst2 * fRec0[1]);
			fRec328[0] = fTemp14 - fConst17 * (fConst15 * fRec328[2] + fConst13 * fRec328[1]);
			fRec329[0] = fTemp14 - fConst24 * (fConst22 * fRec329[2] + fConst20 * fRec329[1]);
			fRec330[0] = fTemp14 - fConst31 * (fConst29 * fRec330[2] + fConst27 * fRec330[1]);
			fRec331[0] = fTemp14 - fConst38 * (fConst36 * fRec331[2] + fConst34 * fRec331[1]);
			fRec332[0] = fTemp14 - fConst45 * (fConst43 * fRec332[2] + fConst41 * fRec332[1]);
			fRec333[0] = fTemp14 - fConst52 * (fConst50 * fRec333[2] + fConst48 * fRec333[1]);
			fRec334[0] = fTemp14 - fConst59 * (fConst57 * fRec334[2] + fConst55 * fRec334[1]);
			fRec335[0] = fTemp14 - fConst66 * (fConst64 * fRec335[2] + fConst62 * fRec335[1]);
			fRec336[0] = fTemp14 - fConst73 * (fConst71 * fRec336[2] + fConst69 * fRec336[1]);
			fRec337[0] = fTemp14 - fConst80 * (fConst78 * fRec337[2] + fConst76 * fRec337[1]);
			fRec338[0] = fTemp14 - fConst87 * (fConst85 * fRec338[2] + fConst83 * fRec338[1]);
			fRec339[0] = fTemp14 - fConst94 * (fConst92 * fRec339[2] + fConst90 * fRec339[1]);
			fRec340[0] = fTemp14 - fConst101 * (fConst99 * fRec340[2] + fConst97 * fRec340[1]);
			fRec341[0] = fTemp14 - fConst108 * (fConst106 * fRec341[2] + fConst104 * fRec341[1]);
			fRec342[0] = fTemp14 - fConst115 * (fConst113 * fRec342[2] + fConst111 * fRec342[1]);
			fRec343[0] = fTemp14 - fSlow665 * (fSlow663 * fRec343[2] + fSlow661 * fRec343[1]);
			fRec344[0] = fTemp14 - fSlow672 * (fSlow670 * fRec344[2] + fSlow668 * fRec344[1]);
			fRec345[0] = fTemp14 - fSlow679 * (fSlow677 * fRec345[2] + fSlow675 * fRec345[1]);
			fRec346[0] = fTemp14 - fSlow686 * (fSlow684 * fRec346[2] + fSlow682 * fRec346[1]);
			output0[i0] = static_cast<FAUSTFLOAT>(fSlow688 * (fSlow687 * (fRec346[0] - fRec346[2]) + fSlow680 * (fRec345[0] - fRec345[2]) + fSlow673 * (fRec344[0] - fRec344[2]) + fSlow666 * (fRec343[0] - fRec343[2]) + fConst116 * (fRec342[0] - fRec342[2]) + fConst109 * (fRec341[0] - fRec341[2]) + fConst102 * (fRec340[0] - fRec340[2]) + fConst95 * (fRec339[0] - fRec339[2]) + fConst88 * (fRec338[0] - fRec338[2]) + fConst81 * (fRec337[0] - fRec337[2]) + fConst74 * (fRec336[0] - fRec336[2]) + fConst67 * (fRec335[0] - fRec335[2]) + fConst60 * (fRec334[0] - fRec334[2]) + fConst53 * (fRec333[0] - fRec333[2]) + fConst46 * (fRec332[0] - fRec332[2]) + fConst39 * (fRec331[0] - fRec331[2]) + fConst32 * (fRec330[0] - fRec330[2]) + fConst25 * (fRec329[0] - fRec329[2]) + fConst18 * (fRec328[0] - fRec328[2]) + fConst11 * (fRec0[0] - fRec0[2])));
			fVec0[1] = fVec0[0];
			iRec11[1] = iRec11[0];
			IOTA0 = IOTA0 + 1;
			fRec23[1] = fRec23[0];
			fRec26[1] = fRec26[0];
			fVec1[1] = fVec1[0];
			iRec28[1] = iRec28[0];
			iRec30[1] = iRec30[0];
			fRec29[2] = fRec29[1];
			fRec29[1] = fRec29[0];
			for (int j0 = 3; j0 > 0; j0 = j0 - 1) {
				fRec31[j0] = fRec31[j0 - 1];
			}
			fVec2[1] = fVec2[0];
			fVec3[1] = fVec3[0];
			fRec20[1] = fRec20[0];
			fRec19[1] = fRec19[0];
			for (int j1 = 3; j1 > 0; j1 = j1 - 1) {
				fRec16[j1] = fRec16[j1 - 1];
			}
			iRec7[1] = iRec7[0];
			fRec1[1] = fRec1[0];
			iRec43[1] = iRec43[0];
			fRec55[1] = fRec55[0];
			fRec58[1] = fRec58[0];
			fVec4[1] = fVec4[0];
			iRec60[1] = iRec60[0];
			for (int j2 = 3; j2 > 0; j2 = j2 - 1) {
				fRec61[j2] = fRec61[j2 - 1];
			}
			fVec5[1] = fVec5[0];
			fVec6[1] = fVec6[0];
			fRec52[1] = fRec52[0];
			fRec51[1] = fRec51[0];
			for (int j3 = 3; j3 > 0; j3 = j3 - 1) {
				fRec48[j3] = fRec48[j3 - 1];
			}
			iRec39[1] = iRec39[0];
			fRec33[1] = fRec33[0];
			iRec73[1] = iRec73[0];
			fRec85[1] = fRec85[0];
			fRec88[1] = fRec88[0];
			for (int j4 = 3; j4 > 0; j4 = j4 - 1) {
				fRec90[j4] = fRec90[j4 - 1];
			}
			fVec7[1] = fVec7[0];
			fVec8[1] = fVec8[0];
			fRec82[1] = fRec82[0];
			fRec81[1] = fRec81[0];
			for (int j5 = 3; j5 > 0; j5 = j5 - 1) {
				fRec78[j5] = fRec78[j5 - 1];
			}
			iRec69[1] = iRec69[0];
			fRec63[1] = fRec63[0];
			iRec102[1] = iRec102[0];
			fRec114[1] = fRec114[0];
			fRec117[1] = fRec117[0];
			fVec9[1] = fVec9[0];
			iRec119[1] = iRec119[0];
			for (int j6 = 3; j6 > 0; j6 = j6 - 1) {
				fRec120[j6] = fRec120[j6 - 1];
			}
			fVec10[1] = fVec10[0];
			fVec11[1] = fVec11[0];
			fRec111[1] = fRec111[0];
			fRec110[1] = fRec110[0];
			for (int j7 = 3; j7 > 0; j7 = j7 - 1) {
				fRec107[j7] = fRec107[j7 - 1];
			}
			iRec98[1] = iRec98[0];
			fRec92[1] = fRec92[0];
			iRec132[1] = iRec132[0];
			fRec144[1] = fRec144[0];
			fRec147[1] = fRec147[0];
			for (int j8 = 3; j8 > 0; j8 = j8 - 1) {
				fRec149[j8] = fRec149[j8 - 1];
			}
			fVec12[1] = fVec12[0];
			fVec13[1] = fVec13[0];
			fRec141[1] = fRec141[0];
			fRec140[1] = fRec140[0];
			for (int j9 = 3; j9 > 0; j9 = j9 - 1) {
				fRec137[j9] = fRec137[j9 - 1];
			}
			iRec128[1] = iRec128[0];
			fRec122[1] = fRec122[0];
			iRec161[1] = iRec161[0];
			fRec173[1] = fRec173[0];
			fRec176[1] = fRec176[0];
			fVec14[1] = fVec14[0];
			iRec178[1] = iRec178[0];
			for (int j10 = 3; j10 > 0; j10 = j10 - 1) {
				fRec179[j10] = fRec179[j10 - 1];
			}
			fVec15[1] = fVec15[0];
			fVec16[1] = fVec16[0];
			fRec170[1] = fRec170[0];
			fRec169[1] = fRec169[0];
			for (int j11 = 3; j11 > 0; j11 = j11 - 1) {
				fRec166[j11] = fRec166[j11 - 1];
			}
			iRec157[1] = iRec157[0];
			fRec151[1] = fRec151[0];
			iRec191[1] = iRec191[0];
			fRec203[1] = fRec203[0];
			fRec206[1] = fRec206[0];
			for (int j12 = 3; j12 > 0; j12 = j12 - 1) {
				fRec208[j12] = fRec208[j12 - 1];
			}
			fVec17[1] = fVec17[0];
			fVec18[1] = fVec18[0];
			fRec200[1] = fRec200[0];
			fRec199[1] = fRec199[0];
			for (int j13 = 3; j13 > 0; j13 = j13 - 1) {
				fRec196[j13] = fRec196[j13 - 1];
			}
			iRec187[1] = iRec187[0];
			fRec181[1] = fRec181[0];
			iRec220[1] = iRec220[0];
			fRec232[1] = fRec232[0];
			fRec235[1] = fRec235[0];
			fVec19[1] = fVec19[0];
			iRec237[1] = iRec237[0];
			for (int j14 = 3; j14 > 0; j14 = j14 - 1) {
				fRec238[j14] = fRec238[j14 - 1];
			}
			fVec20[1] = fVec20[0];
			fVec21[1] = fVec21[0];
			fRec229[1] = fRec229[0];
			fRec228[1] = fRec228[0];
			for (int j15 = 3; j15 > 0; j15 = j15 - 1) {
				fRec225[j15] = fRec225[j15 - 1];
			}
			iRec216[1] = iRec216[0];
			fRec210[1] = fRec210[0];
			iRec250[1] = iRec250[0];
			fRec262[1] = fRec262[0];
			fRec265[1] = fRec265[0];
			for (int j16 = 3; j16 > 0; j16 = j16 - 1) {
				fRec267[j16] = fRec267[j16 - 1];
			}
			fVec22[1] = fVec22[0];
			fVec23[1] = fVec23[0];
			fRec259[1] = fRec259[0];
			fRec258[1] = fRec258[0];
			for (int j17 = 3; j17 > 0; j17 = j17 - 1) {
				fRec255[j17] = fRec255[j17 - 1];
			}
			iRec246[1] = iRec246[0];
			fRec240[1] = fRec240[0];
			iRec279[1] = iRec279[0];
			fRec291[1] = fRec291[0];
			fRec294[1] = fRec294[0];
			fVec24[1] = fVec24[0];
			iRec296[1] = iRec296[0];
			for (int j18 = 3; j18 > 0; j18 = j18 - 1) {
				fRec297[j18] = fRec297[j18 - 1];
			}
			fVec25[1] = fVec25[0];
			fVec26[1] = fVec26[0];
			fRec288[1] = fRec288[0];
			fRec287[1] = fRec287[0];
			for (int j19 = 3; j19 > 0; j19 = j19 - 1) {
				fRec284[j19] = fRec284[j19 - 1];
			}
			iRec275[1] = iRec275[0];
			fRec269[1] = fRec269[0];
			iRec309[1] = iRec309[0];
			fRec321[1] = fRec321[0];
			fRec324[1] = fRec324[0];
			for (int j20 = 3; j20 > 0; j20 = j20 - 1) {
				fRec326[j20] = fRec326[j20 - 1];
			}
			fVec27[1] = fVec27[0];
			fVec28[1] = fVec28[0];
			fRec318[1] = fRec318[0];
			fRec317[1] = fRec317[0];
			for (int j21 = 3; j21 > 0; j21 = j21 - 1) {
				fRec314[j21] = fRec314[j21 - 1];
			}
			iRec305[1] = iRec305[0];
			fRec299[1] = fRec299[0];
			fRec0[2] = fRec0[1];
			fRec0[1] = fRec0[0];
			fRec328[2] = fRec328[1];
			fRec328[1] = fRec328[0];
			fRec329[2] = fRec329[1];
			fRec329[1] = fRec329[0];
			fRec330[2] = fRec330[1];
			fRec330[1] = fRec330[0];
			fRec331[2] = fRec331[1];
			fRec331[1] = fRec331[0];
			fRec332[2] = fRec332[1];
			fRec332[1] = fRec332[0];
			fRec333[2] = fRec333[1];
			fRec333[1] = fRec333[0];
			fRec334[2] = fRec334[1];
			fRec334[1] = fRec334[0];
			fRec335[2] = fRec335[1];
			fRec335[1] = fRec335[0];
			fRec336[2] = fRec336[1];
			fRec336[1] = fRec336[0];
			fRec337[2] = fRec337[1];
			fRec337[1] = fRec337[0];
			fRec338[2] = fRec338[1];
			fRec338[1] = fRec338[0];
			fRec339[2] = fRec339[1];
			fRec339[1] = fRec339[0];
			fRec340[2] = fRec340[1];
			fRec340[1] = fRec340[0];
			fRec341[2] = fRec341[1];
			fRec341[1] = fRec341[0];
			fRec342[2] = fRec342[1];
			fRec342[1] = fRec342[0];
			fRec343[2] = fRec343[1];
			fRec343[1] = fRec343[0];
			fRec344[2] = fRec344[1];
			fRec344[1] = fRec344[0];
			fRec345[2] = fRec345[1];
			fRec345[1] = fRec345[0];
			fRec346[2] = fRec346[1];
			fRec346[1] = fRec346[0];
		}
	}

};

#endif

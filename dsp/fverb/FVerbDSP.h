/* ------------------------------------------------------------
author: "Jean Pierre Cimalando"
license: "BSD-2-Clause"
name: "fverb"
version: "0.5"
Code generated with Faust 2.74.6 (https://faust.grame.fr)
Compilation options: -a minimal-effect.cpp -lang cpp -ct 1 -cn FVerbDSP -es 1
-mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef __FVerbDSP_H__
#define __FVerbDSP_H__

/************************************************************************
 IMPORTANT NOTE : this file contains two clearly delimited sections :
 the ARCHITECTURE section (in two parts) and the USER section. Each section
 is governed by its own copyright and license. Please check individually
 each section for license and copyright information.
 *************************************************************************/

/******************* BEGIN minimal-effect.cpp ****************/
/************************************************************************
 FAUST Architecture File
 Copyright (C) 2003-2019 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This Architecture section is free software; you can redistribute it
 and/or modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3 of
 the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; If not, see <http://www.gnu.org/licenses/>.

 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.

 ************************************************************************
 ************************************************************************/

#include <algorithm>

#include "faust/dsp/dsp.h"
#include "faust/gui/UI.h"
#include "faust/gui/meta.h"

#if defined(SOUNDFILE)
#include "faust/gui/SoundUI.h"
#endif

using std::max;
using std::min;

/******************************************************************************
 *******************************************************************************

 VECTOR INTRINSICS

 *******************************************************************************
 *******************************************************************************/

/********************END ARCHITECTURE SECTION (part 1/2)****************/

/**************************BEGIN USER SECTION **************************/

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

#include <math.h>

#include <algorithm>
#include <cmath>
#include <cstdint>

#ifndef FAUSTCLASS
#define FAUSTCLASS FVerbDSP
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

class FVerbDSPSIG0 {
 private:
  int iVec6[2];
  int iRec34[2];

 public:
  int getNumInputsFVerbDSPSIG0() { return 0; }
  int getNumOutputsFVerbDSPSIG0() { return 1; }

  void instanceInitFVerbDSPSIG0(int sample_rate) {
    for (int l21 = 0; l21 < 2; l21 = l21 + 1) {
      iVec6[l21] = 0;
    }
    for (int l22 = 0; l22 < 2; l22 = l22 + 1) {
      iRec34[l22] = 0;
    }
  }

  void fillFVerbDSPSIG0(int count, float* table) {
    for (int i1 = 0; i1 < count; i1 = i1 + 1) {
      iVec6[0] = 1;
      iRec34[0] = (iVec6[1] + iRec34[1]) % 65536;
      table[i1] = std::sin(9.58738e-05f * float(iRec34[0]));
      iVec6[1] = iVec6[0];
      iRec34[1] = iRec34[0];
    }
  }
};

static FVerbDSPSIG0* newFVerbDSPSIG0() {
  return (FVerbDSPSIG0*)new FVerbDSPSIG0();
}
static void deleteFVerbDSPSIG0(FVerbDSPSIG0* dsp) { delete dsp; }

static float ftbl0FVerbDSPSIG0[65536];

class FVerbDSP : public dsp {
 private:
  int fSampleRate;
  float fConst0;
  float fConst1;
  float fConst2;
  FAUSTFLOAT fHslider0;
  float fConst3;
  float fRec8[2];
  FAUSTFLOAT fHslider1;
  float fConst4;
  float fRec12[2];
  FAUSTFLOAT fHslider2;
  float fRec15[2];
  FAUSTFLOAT fHslider3;
  float fRec20[2];
  FAUSTFLOAT fHslider4;
  float fRec24[2];
  FAUSTFLOAT fHslider5;
  float fRec25[2];
  FAUSTFLOAT fHslider6;
  float fRec27[2];
  int IOTA0;
  float fVec0[131072];
  FAUSTFLOAT fHslider7;
  float fConst5;
  float fRec28[2];
  float fRec26[2];
  float fRec23[2];
  float fVec1[1024];
  int iConst6;
  float fRec21[2];
  float fVec2[1024];
  int iConst7;
  float fRec18[2];
  float fVec3[4096];
  int iConst8;
  float fRec16[2];
  float fVec4[2048];
  int iConst9;
  float fRec13[2];
  int iConst10;
  FAUSTFLOAT fHslider8;
  float fRec29[2];
  float fVec5[131072];
  FAUSTFLOAT fHslider9;
  float fRec36[2];
  float fConst11;
  float fRec35[2];
  FAUSTFLOAT fHslider10;
  float fRec37[2];
  float fConst12;
  float fRec30[2];
  float fRec31[2];
  int iRec32[2];
  int iRec33[2];
  float fRec10[2];
  float fVec7[32768];
  int iConst13;
  float fRec9[2];
  float fVec8[32768];
  int iConst14;
  float fRec6[2];
  float fRec0[32768];
  float fRec1[16384];
  float fRec2[32768];
  float fVec9[131072];
  float fRec52[2];
  float fRec51[2];
  float fVec10[1024];
  int iConst15;
  float fRec49[2];
  float fVec11[1024];
  int iConst16;
  float fRec47[2];
  float fVec12[4096];
  int iConst17;
  float fRec45[2];
  float fVec13[2048];
  int iConst18;
  float fRec43[2];
  int iConst19;
  float fVec14[131072];
  float fRec53[2];
  float fRec54[2];
  int iRec55[2];
  int iRec56[2];
  float fRec41[2];
  float fVec15[32768];
  int iConst20;
  float fRec40[2];
  float fVec16[16384];
  int iConst21;
  float fRec38[2];
  float fRec3[32768];
  float fRec4[8192];
  float fRec5[32768];
  int iConst22;
  int iConst23;
  int iConst24;
  int iConst25;
  int iConst26;
  int iConst27;
  int iConst28;
  int iConst29;
  int iConst30;
  int iConst31;
  int iConst32;
  int iConst33;
  int iConst34;
  int iConst35;

 public:
  FVerbDSP() {}

  void metadata(Meta* m) {
    m->declare("author", "Jean Pierre Cimalando");
    m->declare("basics.lib/name", "Faust Basic Element Library");
    m->declare("basics.lib/tabulateNd",
               "Copyright (C) 2023 Bart Brouns <bart@magnetophon.nl>");
    m->declare("basics.lib/version", "1.18.0");
    m->declare("compile_options",
               "-a minimal-effect.cpp -lang cpp -ct 1 -cn FVerbDSP -es 1 -mcd "
               "16 -mdd 1024 -mdy 33 -single -ftz 0");
    m->declare("delays.lib/name", "Faust Delay Library");
    m->declare("delays.lib/version", "1.1.0");
    m->declare("filename", "fverb.dsp");
    m->declare("filters.lib/allpass_comb:author", "Julius O. Smith III");
    m->declare("filters.lib/allpass_comb:copyright",
               "Copyright (C) 2003-2019 by Julius O. Smith III "
               "<jos@ccrma.stanford.edu>");
    m->declare("filters.lib/allpass_comb:license", "MIT-style STK-4.3 license");
    m->declare("filters.lib/fir:author", "Julius O. Smith III");
    m->declare("filters.lib/fir:copyright",
               "Copyright (C) 2003-2019 by Julius O. Smith III "
               "<jos@ccrma.stanford.edu>");
    m->declare("filters.lib/fir:license", "MIT-style STK-4.3 license");
    m->declare("filters.lib/iir:author", "Julius O. Smith III");
    m->declare("filters.lib/iir:copyright",
               "Copyright (C) 2003-2019 by Julius O. Smith III "
               "<jos@ccrma.stanford.edu>");
    m->declare("filters.lib/iir:license", "MIT-style STK-4.3 license");
    m->declare("filters.lib/lowpass0_highpass1",
               "Copyright (C) 2003-2019 by Julius O. Smith III "
               "<jos@ccrma.stanford.edu>");
    m->declare("filters.lib/name", "Faust Filters Library");
    m->declare("filters.lib/version", "1.3.0");
    m->declare("license", "BSD-2-Clause");
    m->declare("maths.lib/author", "GRAME");
    m->declare("maths.lib/copyright", "GRAME");
    m->declare("maths.lib/license", "LGPL with exception");
    m->declare("maths.lib/name", "Faust Math Library");
    m->declare("maths.lib/version", "2.8.0");
    m->declare("name", "fverb");
    m->declare("oscillators.lib/name", "Faust Oscillator Library");
    m->declare("oscillators.lib/version", "1.5.1");
    m->declare("platform.lib/name", "Generic Platform Library");
    m->declare("platform.lib/version", "1.3.0");
    m->declare("signals.lib/name", "Faust Signal Routing Library");
    m->declare("signals.lib/version", "1.5.0");
    m->declare("version", "0.5");
  }

  virtual int getNumInputs() { return 2; }
  virtual int getNumOutputs() { return 2; }

  static void classInit(int sample_rate) {
    FVerbDSPSIG0* sig0 = newFVerbDSPSIG0();
    sig0->instanceInitFVerbDSPSIG0(sample_rate);
    sig0->fillFVerbDSPSIG0(65536, ftbl0FVerbDSPSIG0);
    deleteFVerbDSPSIG0(sig0);
  }

  virtual void instanceConstants(int sample_rate) {
    fSampleRate = sample_rate;
    fConst0 =
        std::min<float>(1.92e+05f, std::max<float>(1.0f, float(fSampleRate)));
    fConst1 = 44.1f / fConst0;
    fConst2 = 1.0f - fConst1;
    fConst3 = 6.2831855f / fConst0;
    fConst4 = 0.441f / fConst0;
    fConst5 = 0.0441f / fConst0;
    iConst6 = std::min<int>(
        65536, std::max<int>(0, int(0.0046282047f * fConst0) + -1));
    iConst7 = std::min<int>(
        65536, std::max<int>(0, int(0.0037031686f * fConst0) + -1));
    iConst8 = std::min<int>(65536,
                            std::max<int>(0, int(0.013116831f * fConst0) + -1));
    iConst9 = std::min<int>(65536,
                            std::max<int>(0, int(0.009028259f * fConst0) + -1));
    iConst10 =
        std::min<int>(65536, std::max<int>(0, int(0.10628003f * fConst0))) + 1;
    fConst11 = 1.0f / fConst0;
    fConst12 = 1.0f / float(int(0.01f * fConst0));
    iConst13 =
        std::min<int>(65536, std::max<int>(0, int(0.14169551f * fConst0)));
    iConst14 =
        std::min<int>(65536, std::max<int>(0, int(0.08924431f * fConst0) + -1));
    iConst15 = std::min<int>(
        65536, std::max<int>(0, int(0.0049144854f * fConst0) + -1));
    iConst16 =
        std::min<int>(65536, std::max<int>(0, int(0.00348745f * fConst0) + -1));
    iConst17 = std::min<int>(
        65536, std::max<int>(0, int(0.012352743f * fConst0) + -1));
    iConst18 = std::min<int>(
        65536, std::max<int>(0, int(0.009586708f * fConst0) + -1));
    iConst19 =
        std::min<int>(65536, std::max<int>(0, int(0.1249958f * fConst0))) + 1;
    iConst20 =
        std::min<int>(65536, std::max<int>(0, int(0.14962535f * fConst0)));
    iConst21 =
        std::min<int>(65536, std::max<int>(0, int(0.06048184f * fConst0) + -1));
    iConst22 =
        std::min<int>(65536, std::max<int>(0, int(0.03581869f * fConst0)));
    iConst23 =
        std::min<int>(65536, std::max<int>(0, int(0.006283391f * fConst0)));
    iConst24 =
        std::min<int>(65536, std::max<int>(0, int(0.06686603f * fConst0)));
    iConst25 =
        std::min<int>(65536, std::max<int>(0, int(0.06427875f * fConst0)));
    iConst26 =
        std::min<int>(65536, std::max<int>(0, int(0.06706764f * fConst0)));
    iConst27 =
        std::min<int>(65536, std::max<int>(0, int(0.09992944f * fConst0)));
    iConst28 =
        std::min<int>(65536, std::max<int>(0, int(0.008937872f * fConst0)));
    iConst29 =
        std::min<int>(65536, std::max<int>(0, int(0.0040657236f * fConst0)));
    iConst30 =
        std::min<int>(65536, std::max<int>(0, int(0.011256342f * fConst0)));
    iConst31 =
        std::min<int>(65536, std::max<int>(0, int(0.070931755f * fConst0)));
    iConst32 =
        std::min<int>(65536, std::max<int>(0, int(0.041262053f * fConst0)));
    iConst33 =
        std::min<int>(65536, std::max<int>(0, int(0.08981553f * fConst0)));
    iConst34 =
        std::min<int>(65536, std::max<int>(0, int(0.121870905f * fConst0)));
    iConst35 =
        std::min<int>(65536, std::max<int>(0, int(0.01186116f * fConst0)));
  }

  virtual void instanceResetUserInterface() {
    fHslider0 = FAUSTFLOAT(5.5e+03f);
    fHslider1 = FAUSTFLOAT(7e+01f);
    fHslider2 = FAUSTFLOAT(62.5f);
    fHslider3 = FAUSTFLOAT(75.0f);
    fHslider4 = FAUSTFLOAT(1e+02f);
    fHslider5 = FAUSTFLOAT(1e+04f);
    fHslider6 = FAUSTFLOAT(1e+02f);
    fHslider7 = FAUSTFLOAT(0.0f);
    fHslider8 = FAUSTFLOAT(5e+01f);
    fHslider9 = FAUSTFLOAT(1.0f);
    fHslider10 = FAUSTFLOAT(0.5f);
  }

  virtual void instanceClear() {
    for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
      fRec8[l0] = 0.0f;
    }
    for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
      fRec12[l1] = 0.0f;
    }
    for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
      fRec15[l2] = 0.0f;
    }
    for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
      fRec20[l3] = 0.0f;
    }
    for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
      fRec24[l4] = 0.0f;
    }
    for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
      fRec25[l5] = 0.0f;
    }
    for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
      fRec27[l6] = 0.0f;
    }
    IOTA0 = 0;
    for (int l7 = 0; l7 < 131072; l7 = l7 + 1) {
      fVec0[l7] = 0.0f;
    }
    for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
      fRec28[l8] = 0.0f;
    }
    for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
      fRec26[l9] = 0.0f;
    }
    for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
      fRec23[l10] = 0.0f;
    }
    for (int l11 = 0; l11 < 1024; l11 = l11 + 1) {
      fVec1[l11] = 0.0f;
    }
    for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
      fRec21[l12] = 0.0f;
    }
    for (int l13 = 0; l13 < 1024; l13 = l13 + 1) {
      fVec2[l13] = 0.0f;
    }
    for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
      fRec18[l14] = 0.0f;
    }
    for (int l15 = 0; l15 < 4096; l15 = l15 + 1) {
      fVec3[l15] = 0.0f;
    }
    for (int l16 = 0; l16 < 2; l16 = l16 + 1) {
      fRec16[l16] = 0.0f;
    }
    for (int l17 = 0; l17 < 2048; l17 = l17 + 1) {
      fVec4[l17] = 0.0f;
    }
    for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
      fRec13[l18] = 0.0f;
    }
    for (int l19 = 0; l19 < 2; l19 = l19 + 1) {
      fRec29[l19] = 0.0f;
    }
    for (int l20 = 0; l20 < 131072; l20 = l20 + 1) {
      fVec5[l20] = 0.0f;
    }
    for (int l23 = 0; l23 < 2; l23 = l23 + 1) {
      fRec36[l23] = 0.0f;
    }
    for (int l24 = 0; l24 < 2; l24 = l24 + 1) {
      fRec35[l24] = 0.0f;
    }
    for (int l25 = 0; l25 < 2; l25 = l25 + 1) {
      fRec37[l25] = 0.0f;
    }
    for (int l26 = 0; l26 < 2; l26 = l26 + 1) {
      fRec30[l26] = 0.0f;
    }
    for (int l27 = 0; l27 < 2; l27 = l27 + 1) {
      fRec31[l27] = 0.0f;
    }
    for (int l28 = 0; l28 < 2; l28 = l28 + 1) {
      iRec32[l28] = 0;
    }
    for (int l29 = 0; l29 < 2; l29 = l29 + 1) {
      iRec33[l29] = 0;
    }
    for (int l30 = 0; l30 < 2; l30 = l30 + 1) {
      fRec10[l30] = 0.0f;
    }
    for (int l31 = 0; l31 < 32768; l31 = l31 + 1) {
      fVec7[l31] = 0.0f;
    }
    for (int l32 = 0; l32 < 2; l32 = l32 + 1) {
      fRec9[l32] = 0.0f;
    }
    for (int l33 = 0; l33 < 32768; l33 = l33 + 1) {
      fVec8[l33] = 0.0f;
    }
    for (int l34 = 0; l34 < 2; l34 = l34 + 1) {
      fRec6[l34] = 0.0f;
    }
    for (int l35 = 0; l35 < 32768; l35 = l35 + 1) {
      fRec0[l35] = 0.0f;
    }
    for (int l36 = 0; l36 < 16384; l36 = l36 + 1) {
      fRec1[l36] = 0.0f;
    }
    for (int l37 = 0; l37 < 32768; l37 = l37 + 1) {
      fRec2[l37] = 0.0f;
    }
    for (int l38 = 0; l38 < 131072; l38 = l38 + 1) {
      fVec9[l38] = 0.0f;
    }
    for (int l39 = 0; l39 < 2; l39 = l39 + 1) {
      fRec52[l39] = 0.0f;
    }
    for (int l40 = 0; l40 < 2; l40 = l40 + 1) {
      fRec51[l40] = 0.0f;
    }
    for (int l41 = 0; l41 < 1024; l41 = l41 + 1) {
      fVec10[l41] = 0.0f;
    }
    for (int l42 = 0; l42 < 2; l42 = l42 + 1) {
      fRec49[l42] = 0.0f;
    }
    for (int l43 = 0; l43 < 1024; l43 = l43 + 1) {
      fVec11[l43] = 0.0f;
    }
    for (int l44 = 0; l44 < 2; l44 = l44 + 1) {
      fRec47[l44] = 0.0f;
    }
    for (int l45 = 0; l45 < 4096; l45 = l45 + 1) {
      fVec12[l45] = 0.0f;
    }
    for (int l46 = 0; l46 < 2; l46 = l46 + 1) {
      fRec45[l46] = 0.0f;
    }
    for (int l47 = 0; l47 < 2048; l47 = l47 + 1) {
      fVec13[l47] = 0.0f;
    }
    for (int l48 = 0; l48 < 2; l48 = l48 + 1) {
      fRec43[l48] = 0.0f;
    }
    for (int l49 = 0; l49 < 131072; l49 = l49 + 1) {
      fVec14[l49] = 0.0f;
    }
    for (int l50 = 0; l50 < 2; l50 = l50 + 1) {
      fRec53[l50] = 0.0f;
    }
    for (int l51 = 0; l51 < 2; l51 = l51 + 1) {
      fRec54[l51] = 0.0f;
    }
    for (int l52 = 0; l52 < 2; l52 = l52 + 1) {
      iRec55[l52] = 0;
    }
    for (int l53 = 0; l53 < 2; l53 = l53 + 1) {
      iRec56[l53] = 0;
    }
    for (int l54 = 0; l54 < 2; l54 = l54 + 1) {
      fRec41[l54] = 0.0f;
    }
    for (int l55 = 0; l55 < 32768; l55 = l55 + 1) {
      fVec15[l55] = 0.0f;
    }
    for (int l56 = 0; l56 < 2; l56 = l56 + 1) {
      fRec40[l56] = 0.0f;
    }
    for (int l57 = 0; l57 < 16384; l57 = l57 + 1) {
      fVec16[l57] = 0.0f;
    }
    for (int l58 = 0; l58 < 2; l58 = l58 + 1) {
      fRec38[l58] = 0.0f;
    }
    for (int l59 = 0; l59 < 32768; l59 = l59 + 1) {
      fRec3[l59] = 0.0f;
    }
    for (int l60 = 0; l60 < 8192; l60 = l60 + 1) {
      fRec4[l60] = 0.0f;
    }
    for (int l61 = 0; l61 < 32768; l61 = l61 + 1) {
      fRec5[l61] = 0.0f;
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

  virtual FVerbDSP* clone() { return new FVerbDSP(); }

  virtual int getSampleRate() { return fSampleRate; }

  virtual void buildUserInterface(UI* ui_interface) {
    ui_interface->openVerticalBox("fverb");
    ui_interface->declare(&fHslider7, "01", "");
    ui_interface->declare(&fHslider7, "symbol", "predelay");
    ui_interface->declare(&fHslider7, "unit", "ms");
    ui_interface->addHorizontalSlider("Predelay", &fHslider7, FAUSTFLOAT(0.0f),
                                      FAUSTFLOAT(0.0f), FAUSTFLOAT(3e+02f),
                                      FAUSTFLOAT(1.0f));
    ui_interface->declare(&fHslider6, "02", "");
    ui_interface->declare(&fHslider6, "symbol", "input");
    ui_interface->declare(&fHslider6, "unit", "%");
    ui_interface->addHorizontalSlider("Input amount", &fHslider6,
                                      FAUSTFLOAT(1e+02f), FAUSTFLOAT(0.0f),
                                      FAUSTFLOAT(1e+02f), FAUSTFLOAT(0.01f));
    ui_interface->declare(&fHslider5, "03", "");
    ui_interface->declare(&fHslider5, "scale", "log");
    ui_interface->declare(&fHslider5, "symbol", "input_lowpass");
    ui_interface->declare(&fHslider5, "unit", "Hz");
    ui_interface->addHorizontalSlider("Input low-pass cutoff", &fHslider5,
                                      FAUSTFLOAT(1e+04f), FAUSTFLOAT(1.0f),
                                      FAUSTFLOAT(2e+04f), FAUSTFLOAT(1.0f));
    ui_interface->declare(&fHslider4, "04", "");
    ui_interface->declare(&fHslider4, "scale", "log");
    ui_interface->declare(&fHslider4, "symbol", "input_highpass");
    ui_interface->declare(&fHslider4, "unit", "Hz");
    ui_interface->addHorizontalSlider("Input high-pass cutoff", &fHslider4,
                                      FAUSTFLOAT(1e+02f), FAUSTFLOAT(1.0f),
                                      FAUSTFLOAT(1e+03f), FAUSTFLOAT(1.0f));
    ui_interface->declare(&fHslider3, "05", "");
    ui_interface->declare(&fHslider3, "symbol", "input_diffusion_1");
    ui_interface->declare(&fHslider3, "unit", "%");
    ui_interface->addHorizontalSlider("Input diffusion 1", &fHslider3,
                                      FAUSTFLOAT(75.0f), FAUSTFLOAT(0.0f),
                                      FAUSTFLOAT(1e+02f), FAUSTFLOAT(0.01f));
    ui_interface->declare(&fHslider2, "06", "");
    ui_interface->declare(&fHslider2, "symbol", "input_diffusion_2");
    ui_interface->declare(&fHslider2, "unit", "%");
    ui_interface->addHorizontalSlider("Input diffusion 2", &fHslider2,
                                      FAUSTFLOAT(62.5f), FAUSTFLOAT(0.0f),
                                      FAUSTFLOAT(1e+02f), FAUSTFLOAT(0.01f));
    ui_interface->declare(&fHslider1, "07", "");
    ui_interface->declare(&fHslider1, "symbol", "tail_density");
    ui_interface->declare(&fHslider1, "unit", "%");
    ui_interface->addHorizontalSlider("Tail density", &fHslider1,
                                      FAUSTFLOAT(7e+01f), FAUSTFLOAT(0.0f),
                                      FAUSTFLOAT(1e+02f), FAUSTFLOAT(0.01f));
    ui_interface->declare(&fHslider8, "08", "");
    ui_interface->declare(&fHslider8, "symbol", "decay");
    ui_interface->declare(&fHslider8, "unit", "%");
    ui_interface->addHorizontalSlider("Decay", &fHslider8, FAUSTFLOAT(5e+01f),
                                      FAUSTFLOAT(0.0f), FAUSTFLOAT(1e+02f),
                                      FAUSTFLOAT(0.01f));
    ui_interface->declare(&fHslider0, "09", "");
    ui_interface->declare(&fHslider0, "scale", "log");
    ui_interface->declare(&fHslider0, "symbol", "damping");
    ui_interface->declare(&fHslider0, "unit", "Hz");
    ui_interface->addHorizontalSlider("Damping", &fHslider0,
                                      FAUSTFLOAT(5.5e+03f), FAUSTFLOAT(1e+01f),
                                      FAUSTFLOAT(2e+04f), FAUSTFLOAT(1.0f));
    ui_interface->declare(&fHslider9, "10", "");
    ui_interface->declare(&fHslider9, "symbol", "mod_frequency");
    ui_interface->declare(&fHslider9, "unit", "Hz");
    ui_interface->addHorizontalSlider("Modulator frequency", &fHslider9,
                                      FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f),
                                      FAUSTFLOAT(4.0f), FAUSTFLOAT(0.01f));
    ui_interface->declare(&fHslider10, "11", "");
    ui_interface->declare(&fHslider10, "symbol", "mod_depth");
    ui_interface->declare(&fHslider10, "unit", "ms");
    ui_interface->addHorizontalSlider("Modulator depth", &fHslider10,
                                      FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f),
                                      FAUSTFLOAT(1e+01f), FAUSTFLOAT(0.1f));
    ui_interface->closeBox();
  }

  enum Parameter {
    PREDELAY,
    INPUT_AMOUNT,
    INPUT_LOW_PASS_CUTOFF,
    INPUT_HIGH_PASS_CUTOFF,
    INPUT_DIFFUSION_1,
    INPUT_DIFFUSION_2,
    TAIL_DENSITY,
    DECAY,
    DAMPING,
    MODULATOR_FREQUENCY,
    MODULATOR_DEPTH
  };

  void SetParamValue(Parameter param, FAUSTFLOAT value) {
    switch (param) {
      case PREDELAY:
        fHslider7 = value;
        break;
      case INPUT_AMOUNT:
        fHslider6 = value;
        break;
      case INPUT_LOW_PASS_CUTOFF:
        fHslider5 = value;
        break;
      case INPUT_HIGH_PASS_CUTOFF:
        fHslider4 = value;
        break;
      case INPUT_DIFFUSION_1:
        fHslider3 = value;
        break;
      case INPUT_DIFFUSION_2:
        fHslider2 = value;
        break;
      case TAIL_DENSITY:
        fHslider1 = value;
        break;
      case DECAY:
        fHslider8 = value;
        break;
      case DAMPING:
        fHslider0 = value;
        break;
      case MODULATOR_FREQUENCY:
        fHslider9 = value;
        break;
      case MODULATOR_DEPTH:
        fHslider10 = value;
        break;
    }
  }

  virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs,
                       FAUSTFLOAT** RESTRICT outputs) {
    FAUSTFLOAT* input0 = inputs[0];
    FAUSTFLOAT* input1 = inputs[1];
    FAUSTFLOAT* output0 = outputs[0];
    FAUSTFLOAT* output1 = outputs[1];
    float fSlow0 = fConst1 * std::exp(-(fConst3 * float(fHslider0)));
    float fSlow1 = fConst4 * float(fHslider1);
    float fSlow2 = fConst4 * float(fHslider2);
    float fSlow3 = fConst4 * float(fHslider3);
    float fSlow4 = fConst1 * std::exp(-(fConst3 * float(fHslider4)));
    float fSlow5 = fConst1 * std::exp(-(fConst3 * float(fHslider5)));
    float fSlow6 = fConst4 * float(fHslider6);
    float fSlow7 = fConst5 * float(fHslider7);
    float fSlow8 = fConst4 * float(fHslider8);
    float fSlow9 = fConst1 * float(fHslider9);
    float fSlow10 = fConst5 * float(fHslider10);
    for (int i0 = 0; i0 < count; i0 = i0 + 1) {
      fRec8[0] = fSlow0 + fConst2 * fRec8[1];
      float fTemp0 = 1.0f - fRec8[0];
      fRec12[0] = fSlow1 + fConst2 * fRec12[1];
      fRec15[0] = fSlow2 + fConst2 * fRec15[1];
      fRec20[0] = fSlow3 + fConst2 * fRec20[1];
      fRec24[0] = fSlow4 + fConst2 * fRec24[1];
      fRec25[0] = fSlow5 + fConst2 * fRec25[1];
      float fTemp1 = 1.0f - fRec25[0];
      fRec27[0] = fSlow6 + fConst2 * fRec27[1];
      fVec0[IOTA0 & 131071] = float(input1[i0]) * fRec27[0];
      fRec28[0] = fSlow7 + fConst2 * fRec28[1];
      int iTemp2 =
          std::min<int>(65536, std::max<int>(0, int(fConst0 * fRec28[0])));
      fRec26[0] = fVec0[(IOTA0 - iTemp2) & 131071] + fRec25[0] * fRec26[1];
      fRec23[0] = fRec26[0] * fTemp1 + fRec24[0] * fRec23[1];
      float fTemp3 = fRec24[0] + 1.0f;
      float fTemp4 =
          0.5f * fTemp3 * (fRec23[0] - fRec23[1]) - fRec20[0] * fRec21[1];
      fVec1[IOTA0 & 1023] = fTemp4;
      fRec21[0] = fVec1[(IOTA0 - iConst6) & 1023];
      float fRec22 = fRec20[0] * fTemp4;
      float fTemp5 = fRec22 + fRec21[1] - fRec20[0] * fRec18[1];
      fVec2[IOTA0 & 1023] = fTemp5;
      fRec18[0] = fVec2[(IOTA0 - iConst7) & 1023];
      float fRec19 = fRec20[0] * fTemp5;
      float fTemp6 = fRec19 + fRec18[1] - fRec15[0] * fRec16[1];
      fVec3[IOTA0 & 4095] = fTemp6;
      fRec16[0] = fVec3[(IOTA0 - iConst8) & 4095];
      float fRec17 = fRec15[0] * fTemp6;
      float fTemp7 = fRec17 + fRec16[1] - fRec15[0] * fRec13[1];
      fVec4[IOTA0 & 2047] = fTemp7;
      fRec13[0] = fVec4[(IOTA0 - iConst9) & 2047];
      float fRec14 = fRec15[0] * fTemp7;
      fRec29[0] = fSlow8 + fConst2 * fRec29[1];
      float fTemp8 = fRec13[1] + fRec29[0] * fRec3[(IOTA0 - iConst10) & 32767] +
                     fRec14 + fRec12[0] * fRec10[1];
      fVec5[IOTA0 & 131071] = fTemp8;
      fRec36[0] = fSlow9 + fConst2 * fRec36[1];
      float fTemp9 = fRec35[1] + fConst11 * fRec36[0];
      fRec35[0] = fTemp9 - float(int(fTemp9));
      fRec37[0] = fSlow10 + fConst2 * fRec37[1];
      int iTemp10 =
          int(fConst0 *
              (fRec37[0] *
                   ftbl0FVerbDSPSIG0[std::max<int>(
                       0, std::min<int>(
                              int(65536.0f *
                                  (fRec35[0] +
                                   (0.25f - float(int(fRec35[0] + 0.25f))))),
                              65535))] +
               0.030509727f)) +
          -1;
      float fTemp11 =
          ((fRec30[1] != 0.0f)
               ? (((fRec31[1] > 0.0f) & (fRec31[1] < 1.0f)) ? fRec30[1] : 0.0f)
               : (((fRec31[1] == 0.0f) & (iTemp10 != iRec32[1]))
                      ? fConst12
                      : (((fRec31[1] == 1.0f) & (iTemp10 != iRec33[1]))
                             ? -fConst12
                             : 0.0f)));
      fRec30[0] = fTemp11;
      fRec31[0] =
          std::max<float>(0.0f, std::min<float>(1.0f, fRec31[1] + fTemp11));
      iRec32[0] = (((fRec31[1] >= 1.0f) & (iRec33[1] != iTemp10)) ? iTemp10
                                                                  : iRec32[1]);
      iRec33[0] = (((fRec31[1] <= 0.0f) & (iRec32[1] != iTemp10)) ? iTemp10
                                                                  : iRec33[1]);
      float fTemp12 =
          fVec5[(IOTA0 - std::min<int>(65536, std::max<int>(0, iRec32[0]))) &
                131071];
      fRec10[0] =
          fTemp12 +
          fRec31[0] * (fVec5[(IOTA0 - std::min<int>(
                                          65536, std::max<int>(0, iRec33[0]))) &
                             131071] -
                       fTemp12);
      float fRec11 = -(fRec12[0] * fTemp8);
      float fTemp13 = fRec11 + fRec10[1];
      fVec7[IOTA0 & 32767] = fTemp13;
      fRec9[0] = fVec7[(IOTA0 - iConst13) & 32767] + fRec8[0] * fRec9[1];
      float fTemp14 =
          std::min<float>(0.5f, std::max<float>(0.25f, fRec29[0] + 0.15f));
      float fTemp15 = fTemp14 * fRec6[1] + fRec29[0] * fRec9[0] * fTemp0;
      fVec8[IOTA0 & 32767] = fTemp15;
      fRec6[0] = fVec8[(IOTA0 - iConst14) & 32767];
      float fRec7 = -(fTemp14 * fTemp15);
      fRec0[IOTA0 & 32767] = fRec7 + fRec6[1];
      fRec1[IOTA0 & 16383] = fRec9[0] * fTemp0;
      fRec2[IOTA0 & 32767] = fTemp13;
      fVec9[IOTA0 & 131071] = float(input0[i0]) * fRec27[0];
      fRec52[0] = fVec9[(IOTA0 - iTemp2) & 131071] + fRec25[0] * fRec52[1];
      fRec51[0] = fTemp1 * fRec52[0] + fRec24[0] * fRec51[1];
      float fTemp16 =
          0.5f * fTemp3 * (fRec51[0] - fRec51[1]) - fRec20[0] * fRec49[1];
      fVec10[IOTA0 & 1023] = fTemp16;
      fRec49[0] = fVec10[(IOTA0 - iConst15) & 1023];
      float fRec50 = fRec20[0] * fTemp16;
      float fTemp17 = fRec50 + fRec49[1] - fRec20[0] * fRec47[1];
      fVec11[IOTA0 & 1023] = fTemp17;
      fRec47[0] = fVec11[(IOTA0 - iConst16) & 1023];
      float fRec48 = fRec20[0] * fTemp17;
      float fTemp18 = fRec48 + fRec47[1] - fRec15[0] * fRec45[1];
      fVec12[IOTA0 & 4095] = fTemp18;
      fRec45[0] = fVec12[(IOTA0 - iConst17) & 4095];
      float fRec46 = fRec15[0] * fTemp18;
      float fTemp19 = fRec46 + fRec45[1] - fRec15[0] * fRec43[1];
      fVec13[IOTA0 & 2047] = fTemp19;
      fRec43[0] = fVec13[(IOTA0 - iConst18) & 2047];
      float fRec44 = fRec15[0] * fTemp19;
      float fTemp20 = fRec43[1] +
                      fRec29[0] * fRec0[(IOTA0 - iConst19) & 32767] + fRec44 +
                      fRec12[0] * fRec41[1];
      fVec14[IOTA0 & 131071] = fTemp20;
      int iTemp21 =
          int(fConst0 *
              (fRec37[0] *
                   ftbl0FVerbDSPSIG0[std::max<int>(
                       0, std::min<int>(int(65536.0f * fRec35[0]), 65535))] +
               0.025603978f)) +
          -1;
      float fTemp22 =
          ((fRec53[1] != 0.0f)
               ? (((fRec54[1] > 0.0f) & (fRec54[1] < 1.0f)) ? fRec53[1] : 0.0f)
               : (((fRec54[1] == 0.0f) & (iTemp21 != iRec55[1]))
                      ? fConst12
                      : (((fRec54[1] == 1.0f) & (iTemp21 != iRec56[1]))
                             ? -fConst12
                             : 0.0f)));
      fRec53[0] = fTemp22;
      fRec54[0] =
          std::max<float>(0.0f, std::min<float>(1.0f, fRec54[1] + fTemp22));
      iRec55[0] = (((fRec54[1] >= 1.0f) & (iRec56[1] != iTemp21)) ? iTemp21
                                                                  : iRec55[1]);
      iRec56[0] = (((fRec54[1] <= 0.0f) & (iRec55[1] != iTemp21)) ? iTemp21
                                                                  : iRec56[1]);
      float fTemp23 =
          fVec14[(IOTA0 - std::min<int>(65536, std::max<int>(0, iRec55[0]))) &
                 131071];
      fRec41[0] =
          fTemp23 +
          fRec54[0] *
              (fVec14[(IOTA0 -
                       std::min<int>(65536, std::max<int>(0, iRec56[0]))) &
                      131071] -
               fTemp23);
      float fRec42 = -(fRec12[0] * fTemp20);
      float fTemp24 = fRec42 + fRec41[1];
      fVec15[IOTA0 & 32767] = fTemp24;
      fRec40[0] = fVec15[(IOTA0 - iConst20) & 32767] + fRec8[0] * fRec40[1];
      float fTemp25 = fTemp14 * fRec38[1] + fRec29[0] * fTemp0 * fRec40[0];
      fVec16[IOTA0 & 16383] = fTemp25;
      fRec38[0] = fVec16[(IOTA0 - iConst21) & 16383];
      float fRec39 = -(fTemp14 * fTemp25);
      fRec3[IOTA0 & 32767] = fRec39 + fRec38[1];
      fRec4[IOTA0 & 8191] = fTemp0 * fRec40[0];
      fRec5[IOTA0 & 32767] = fTemp24;
      output0[i0] = FAUSTFLOAT(0.6f * (fRec2[(IOTA0 - iConst28) & 32767] +
                                       fRec2[(IOTA0 - iConst27) & 32767] +
                                       fRec0[(IOTA0 - iConst26) & 32767] -
                                       (fRec1[(IOTA0 - iConst25) & 16383] +
                                        fRec5[(IOTA0 - iConst24) & 32767] +
                                        fRec4[(IOTA0 - iConst23) & 8191] +
                                        fRec3[(IOTA0 - iConst22) & 32767])));
      output1[i0] = FAUSTFLOAT(0.6f * (fRec5[(IOTA0 - iConst35) & 32767] +
                                       fRec5[(IOTA0 - iConst34) & 32767] +
                                       fRec3[(IOTA0 - iConst33) & 32767] -
                                       (fRec4[(IOTA0 - iConst32) & 8191] +
                                        fRec2[(IOTA0 - iConst31) & 32767] +
                                        fRec1[(IOTA0 - iConst30) & 16383] +
                                        fRec0[(IOTA0 - iConst29) & 32767])));
      fRec8[1] = fRec8[0];
      fRec12[1] = fRec12[0];
      fRec15[1] = fRec15[0];
      fRec20[1] = fRec20[0];
      fRec24[1] = fRec24[0];
      fRec25[1] = fRec25[0];
      fRec27[1] = fRec27[0];
      IOTA0 = IOTA0 + 1;
      fRec28[1] = fRec28[0];
      fRec26[1] = fRec26[0];
      fRec23[1] = fRec23[0];
      fRec21[1] = fRec21[0];
      fRec18[1] = fRec18[0];
      fRec16[1] = fRec16[0];
      fRec13[1] = fRec13[0];
      fRec29[1] = fRec29[0];
      fRec36[1] = fRec36[0];
      fRec35[1] = fRec35[0];
      fRec37[1] = fRec37[0];
      fRec30[1] = fRec30[0];
      fRec31[1] = fRec31[0];
      iRec32[1] = iRec32[0];
      iRec33[1] = iRec33[0];
      fRec10[1] = fRec10[0];
      fRec9[1] = fRec9[0];
      fRec6[1] = fRec6[0];
      fRec52[1] = fRec52[0];
      fRec51[1] = fRec51[0];
      fRec49[1] = fRec49[0];
      fRec47[1] = fRec47[0];
      fRec45[1] = fRec45[0];
      fRec43[1] = fRec43[0];
      fRec53[1] = fRec53[0];
      fRec54[1] = fRec54[0];
      iRec55[1] = iRec55[0];
      iRec56[1] = iRec56[0];
      fRec41[1] = fRec41[0];
      fRec40[1] = fRec40[0];
      fRec38[1] = fRec38[0];
    }
  }
};

/***************************END USER SECTION ***************************/

/*******************BEGIN ARCHITECTURE SECTION (part 2/2)***************/

// Factory API
dsp* createFVerbDSP() { return new FVerbDSP(); }

/******************* END minimal-effect.cpp ****************/

#endif

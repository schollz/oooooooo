// src/effects/FVerb.h
#pragma once

#include <cstddef>  // for size_t

#include "../utilities/Utilities.h"  // for FClamp

class FVerbDSP;  // Forward declaration for the Faust class

class FVerb {
 public:
  FVerb() = default;
  ~FVerb();

  void Init(float sample_rate, size_t block_size);
  void Process(float** out, int numSamples);
  void SetDecay(float decay);
  void SetTailDensity(float x);
  void SetInputDiffision1(float x);
  void SetInputDiffision2(float x);

 private:
  FVerbDSP* dsp;
  float** inputs;
  float** outputs;
};
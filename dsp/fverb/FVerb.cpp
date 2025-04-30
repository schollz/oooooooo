// src/effects/FVerb.cpp
#include "FVerb.h"

#include <cmath>

#include "../utilities/Utilities.h"  // for FClamp
#include "FVerbDSP.h"                // Include the Faust-generated header
#include "faust/gui/MapUI.h"         // Include MapUI for parameter control

void FVerb::Init(float sample_rate, size_t block_size) {
  // Initialize the DSP with the sample rate
  dsp = new FVerbDSP();

  // Allocate input/output buffers (this is the correct way)
  inputs = new float*[2];   // 2 channels
  outputs = new float*[2];  // 2 channels

  for (int i = 0; i < 2; i++) {
    inputs[i] = new float[block_size];
    outputs[i] = new float[block_size];
  }

  // Initialize the DSP parameters
  dsp->init(sample_rate);
  dsp->SetParamValue(FVerbDSP::PREDELAY, 150);
  SetDecay(82);
  SetTailDensity(80);
  SetInputDiffision1(70);
  SetInputDiffision2(75);
}

void FVerb::SetDecay(float decay) {
  // Set the decay parameter in the DSP
  dsp->SetParamValue(FVerbDSP::DECAY, FClamp(decay, 0.0f, 100.0f));
}

void FVerb::SetTailDensity(float x) {
  // Set the decay parameter in the DSP
  dsp->SetParamValue(FVerbDSP::TAIL_DENSITY, FClamp(x, 0.0f, 100.0f));
}

void FVerb::SetInputDiffision1(float x) {
  // Set the decay parameter in the DSP
  dsp->SetParamValue(FVerbDSP::INPUT_DIFFUSION_1, FClamp(x, 0.0f, 100.0f));
}

void FVerb::SetInputDiffision2(float x) {
  // Set the decay parameter in the DSP
  dsp->SetParamValue(FVerbDSP::INPUT_DIFFUSION_2, FClamp(x, 0.0f, 100.0f));
}

FVerb::~FVerb() {
  // Properly clean up all memory
  if (dsp) {
    delete dsp;
  }

  if (inputs) {
    for (int i = 0; i < 2; i++) {
      if (inputs[i]) delete[] inputs[i];
    }
    delete[] inputs;
  }

  if (outputs) {
    for (int i = 0; i < 2; i++) {
      if (outputs[i]) delete[] outputs[i];
    }
    delete[] outputs;
  }
}

void FVerb::Process(float** out, int numSamples) {
  // Copy input from out to inputs
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < numSamples; j++) {
      inputs[i][j] = out[i][j];
    }
  }

  // Process through Faust DSP using its compute method
  dsp->compute(numSamples, inputs, outputs);

  // Copy outputs back to out (if needed)
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < numSamples; j++) {
      out[i][j] = outputs[i][j];
    }
  }
}

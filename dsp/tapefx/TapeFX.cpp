#include "TapeFX.h"

#include <math.h>

TapeFX::TapeFX() {}

namespace {
inline float db2amp_(float db) { return powf(10.0f, db / 20.0f); }

inline float amp2db_(float amp) { return 20.0f * log10(amp); }
}  // namespace

void TapeFX::Init(float sample_rate) {
  follower = Follower(sample_rate);
  dc_input_l_ = 0;
  dc_output_l_ = 0;
  dc_input_r_ = 0;
  dc_output_r_ = 0;
  dc_gain_ = 0.99;
  bias_ = 0.0f;
  pregain_ = 1.0f;
  follow_ = 0;
}

void TapeFX::SetBias(float bias) { bias_ = bias; }

void TapeFX::SetPregain(float pregain) { pregain_ = pregain; }

void TapeFX::ProcessMono(sample_t *out, unsigned int numFrames) {
  for (size_t i = 0; i < numFrames; i++) {
    // pregain
    out[i] *= pregain_;
    follow_ = follower.process(out[i]);
    // dc bias with follower
    out[i] = tanhf(out[i] + (follow_ * bias_));
    // DC blocking
    sample_t in = out[i];
    out[i] = in - dc_input_l_ + (dc_gain_ * dc_output_l_);
    dc_output_l_ = out[i];
    dc_input_l_ = in;
    // extra tanh
    out[i] = tanhf(out[i]);
  }
}

void TapeFX::Process(sample_t **out, unsigned int numFrames) {
  for (size_t i = 0; i < numFrames; i++) {
    // pregain
    out[0][i] *= pregain_;
    out[1][i] *= pregain_;

    follow_ = follower.process(out[0][i]);

    // dc bias with follower
    out[0][i] = tanhf(out[0][i] + (follow_ * bias_));
    out[1][i] = tanhf(out[1][i] + (follow_ * bias_));

    // DC blocking
    sample_t in = out[0][i];
    out[0][i] = in - dc_input_l_ + (dc_gain_ * dc_output_l_);
    dc_output_l_ = out[0][i];
    dc_input_l_ = in;
    in = out[1][i];
    out[1][i] = in - dc_input_r_ + (dc_gain_ * dc_output_r_);
    dc_output_r_ = out[1][i];
    dc_input_r_ = in;

    // extra tanh
    out[0][i] = tanhf(out[0][i]);
    out[1][i] = tanhf(out[1][i]);
  }
}

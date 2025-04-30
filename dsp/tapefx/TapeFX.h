#ifndef LIB_TAPEEMU_H
#define LIB_TAPEEMU_H

#include "Follower.h"

class TapeFX {
 public:
  TapeFX();
  void Init(float sample_rate);
  void Process(float **out, unsigned int numFrames);
  void ProcessMono(float *out, unsigned int numFrames);
  void SetBias(float bias);
  void SetPregain(float pregain);
  float getFollowerValue() { return follow_; }
  float GetBias() { return bias_; }
  float GetPregain() { return pregain_; }

 private:
  float dc_input_l_, dc_output_l_, dc_gain_;
  float dc_input_r_, dc_output_r_;
  float bias_, pregain_;
  Follower follower;
  float follow_;
};
#endif

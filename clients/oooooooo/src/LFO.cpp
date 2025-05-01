#include "LFO.h"

#include <cmath>

static inline float Polyblep(float phase_inc, float t);

float LFO::Process() {
  float out, t;
  switch (waveform_) {
    case WAVE_SIN:
      out = sinf(phase_ * 3.1415927410125732421875f * 2.0f);
      break;
    case WAVE_TRI:
      t = -1.0f + (2.0f * phase_);
      out = 2.0f * (fabsf(t) - 0.5f);
      break;
    case WAVE_SAW:
      out = -1.0f * (((phase_ * 2.0f)) - 1.0f);
      break;
    case WAVE_RAMP:
      out = ((phase_ * 2.0f)) - 1.0f;
      break;
    case WAVE_SQUARE:
      out = phase_ < pw_ ? (1.0f) : -1.0f;
      break;
    case WAVE_POLYBLEP_TRI:
      t = phase_;
      out = phase_ < 0.5f ? 1.0f : -1.0f;
      out += Polyblep(phase_inc_, t);
      out -= Polyblep(phase_inc_, (t + 0.5f) - static_cast<int>((t + 0.5f)));
      // Leaky Integrator:
      // y[n] = A + x[n] + (1 - A) * y[n-1]
      out = phase_inc_ * out + (1.0f - phase_inc_) * last_out_;
      last_out_ = out;
      out *= 4.f;  // normalize amplitude after leaky integration
      break;
    case WAVE_POLYBLEP_SAW:
      t = phase_;
      out = (2.0f * t) - 1.0f;
      out -= Polyblep(phase_inc_, t);
      out *= -1.0f;
      break;
    case WAVE_POLYBLEP_SQUARE:
      t = phase_;
      out = phase_ < pw_ ? 1.0f : -1.0f;
      out += Polyblep(phase_inc_, t);
      out -= Polyblep(phase_inc_, (t + (1.0f - pw_)) -
                                      static_cast<int>((t + (1.0f - pw_))));
      out *= 0.707f;  // ?
      break;
    default:
      out = 0.0f;
      break;
  }
  phase_ += phase_inc_;
  if (phase_ > 1.0f) {
    phase_ -= 1.0f;
    eoc_ = true;
  } else {
    eoc_ = false;
  }
  eor_ = (phase_ - phase_inc_ < 0.5f && phase_ >= 0.5f);

  out_ = out * amp_ + add_;
  return out_;
}

float LFO::CalcPhaseInc(float f) { return f * sr_recip_; }

static float Polyblep(float phase_inc, float t) {
  float dt = phase_inc;
  if (t < dt) {
    t /= dt;
    return t + t - t * t - 1.0f;
  } else if (t > 1.0f - dt) {
    t = (t - 1.0f) / dt;
    return t * t + t + t + 1.0f;
  } else {
    return 0.0f;
  }
}
#ifndef LIB_PARAMETER_H
#define LIB_PARAMETER_H
#pragma once

#include <cmath>
#include <functional>
#include <string>

#include "DrawFunctions.h"
#include "LFO.h"
#include "Serializable.h"
#include "Utilities.h"

using namespace softcut_jack_osc;

class Parameter : public Serializable {
 public:
  Parameter();
  ~Parameter();
  JSON toJSON() const override;
  void fromJSON(const JSON& json) override;

  void Init(float sample_rate, float min, float max, float inc,
            float default_value, float lfo_min, float lfo_max, float lfo_inc,
            float lfo_period, std::string name, std::string unit,
            std::function<void(float)> set_callback);

  std::string String();
  std::string Name() const { return name_; }
  void SetStringFunc(std::function<std::string(float)> string_func) {
    string_func_ = string_func;
  }
  void ValueDelta(float delta);
  void ValueSet(float value, bool quiet) { set_(value, quiet); }
  void ValueSetRaw(float value, bool quiet);
  bool RegisterClick(float mouseX, float mouseY, bool dragging);

  void LFODelta(float min_delta, float max_delta);
  void Render(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int width,
              int height, bool selected);
  void Update();
  void Bang();
  void Toggle() {
    if (value_set_raw_ > 0.5f) {
      set_(min_, false);
    } else {
      set_(max_, false);
    }
  }
  bool IsHidden() { return hide_; }
  void Hide(bool hide) { hide_ = hide; }

  float GetRaw() { return value_compute_raw_; }
  float GetRawMin() { return lfo_min_raw_; }
  float GetRawMax() { return lfo_max_raw_; }

  void ToggleLFO() {
    lfo_active_ = !lfo_active_;
    Update();
    Bang();
  }

 private:
  void set_(float value, bool quiet);
  bool hide_ = false;
  std::string name_;
  std::string unit_;

  float x_ = 0.0f;
  float y_ = 0.0f;
  float width_ = 0.0f;
  float height_ = 0.0f;
  float value_set_ = 0.0f;
  float value_set_raw_ = 0.0f;
  float value_compute_ = 0.0f;
  float value_compute_raw_ = 0.0f;
  float min_ = 0.0f;
  float max_ = 1.0f;
  float inc_ = 0.01f;
  float inc_raw_ = 0.01f;
  float lfo_min_delta_ = 0.0f;
  float lfo_max_delta_ = 0.0f;
  float lfo_min_set_ = 0.0f;
  float lfo_max_set_ = 1.0f;
  float lfo_min_raw_ = 0.0f;
  float lfo_max_raw_ = 1.0f;
  float lfo_inc_ = 0.01f;
  float lfo_period_ = 10.0f;
  uint8_t lfo_waveform_ = LFO::WAVE_SIN;
  bool lfo_active_ = false;
  void lfo_min_max_update();
  std::function<void(float)> set_callback_ = nullptr;
  std::function<std::string(float)> string_func_ = nullptr;

  LFO lfo_;
};

#endif
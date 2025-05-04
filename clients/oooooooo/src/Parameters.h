#ifndef LIB_PARAMETERS_H
#define LIB_PARAMETERS_H
#pragma once

#include "DrawFunctions.h"
#include "Parameter.h"
#include "Serializable.h"
#include "SoftcutClient.h"

using namespace softcut_jack_osc;

class Parameters : public Serializable {
 public:
  // Parameters
  enum ParameterName {
    PARAM_LEVEL,
    PARAM_PAN,
    PARAM_LPF,
    PARAM_PREGAIN,
    PARAM_BIAS,
    PARAM_REVERB,
    PARAM_RATE,
    PARAM_DIRECTION,
    PARAM_START,
    PARAM_DURATION,
    PARAM_REC_LEVEL,
    PARAM_PRE_LEVEL,
    PARAM_REC_SLEW,
    PARAM_LEVEL_SLEW,
    PARAM_RATE_SLEW,
    PARAM_PAN_SLEW,
    PARAM_FADE_TIME,
    PARAM_LOOP1_FEEDBACK,
    PARAM_LOOP2_FEEDBACK,
    PARAM_LOOP3_FEEDBACK,
    PARAM_LOOP4_FEEDBACK,
    PARAM_LOOP5_FEEDBACK,
    PARAM_LOOP6_FEEDBACK,
    PARAM_LOOP7_FEEDBACK,
    PARAM_LOOP8_FEEDBACK,
    PARAM_COUNT  // Holds the number of parameters
  };
  Parameter param_[PARAM_COUNT];

  Parameters() = default;
  ~Parameters() = default;
  JSON toJSON() const override {
    JSON json;
    for (int i = 0; i < PARAM_COUNT; i++) {
      json[param_[i].Name()] = param_[i].toJSON();
    }
    return json;
  }
  void fromJSON(const JSON& json) override {
    for (int i = 0; i < PARAM_COUNT; i++) {
      param_[i].fromJSON(json[param_[i].Name()]);
    }
  }

  void Init(SoftcutClient* sc, int voice, float sample_rate);

  void ValueDelta(float delta) { param_[selected_].ValueDelta(delta); }
  void ValueToggle(ParameterName p) { param_[p].Toggle(); }

  void ValueSet(ParameterName p, float value, bool quiet) {
    param_[p].ValueSet(value, quiet);
  }
  void ValueSetRaw(ParameterName p, float value, bool quiet) {
    param_[p].ValueSetRaw(value, quiet);
  }

  void LFODelta(float min_delta, float max_delta) {
    param_[selected_].LFODelta(min_delta, max_delta);
  }

  void ToggleLFO() { param_[selected_].ToggleLFO(); }

  void ToggleView();  // Add this method

  void Render(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int width,
              int height, float brightness = 1.0f);  // Add brightness parameter

  void UpdateFade();  // Add this method to update fade animation

  void Update();

  float GetRaw(ParameterName p) { return param_[p].GetRaw(); }
  float GetRawMin(ParameterName p) { return param_[p].GetRawMin(); }
  float GetRawMax(ParameterName p) { return param_[p].GetRawMax(); }
  void SetMax(ParameterName p, float max) { param_[p].SetMax(max); }

  void SetSelected(int selected) { selected_ = selected; }
  int GetSelected() const { return selected_; }
  void SelectedDelta_(int delta) {
    selected_ += delta;
    while (selected_ < 0) selected_ += PARAM_COUNT;
    while (selected_ >= PARAM_COUNT) selected_ -= PARAM_COUNT;
  }
  void SelectedDelta(int delta) {
    SelectedDelta_(delta);
    while (param_[selected_].IsHidden()) {
      SelectedDelta_(delta);
    }
  }

  bool RegisterClick(float mouseX, float mouseY, bool dragging) {
    if (!view_visible_) {
      return false;
    }
    for (int i = 0; i < PARAM_COUNT; i++) {
      if (param_[i].IsHidden() || (dragging && i != selected_)) {
        continue;
      }
      if (param_[i].RegisterClick(mouseX, mouseY, dragging)) {
        selected_ = i;
        return true;
      }
    }
    return false;
  }

 private:
  bool view_visible_ = true;
  float fade_amount_ = 1.0f;
  float fade_target_ = 1.0f;
  float fade_speed_ = 0.05f;  // Adjust this for faster/slower fade

  SoftcutClient* softCutClient_ = nullptr;
  int selected_ = 0;
  float sample_rate_ = 0.0f;
};

#endif
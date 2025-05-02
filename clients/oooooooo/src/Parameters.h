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
    PARAM_PLAY,
    PARAM_LEVEL,
    PARAM_PAN,
    PARAM_REVERB,
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

  void Render(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int width,
              int height) {
    int j = 0;
    for (int i = 0; i < PARAM_COUNT; i++) {
      if (param_[i].IsHidden()) continue;
      param_[i].Render(renderer, font, x, y + j * (height + 5), width, height,
                       selected_ == i);
      j++;
    }
  }

  void Update() {
    for (int i = 0; i < PARAM_COUNT; i++) {
      param_[i].Update();
    }
  }

  float GetRaw(ParameterName p) { return param_[p].GetRaw(); }
  float GetRawMin(ParameterName p) { return param_[p].GetRawMin(); }
  float GetRawMax(ParameterName p) { return param_[p].GetRawMax(); }

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

 private:
  SoftcutClient* softCutClient_ = nullptr;
  int selected_ = 0;
  float sample_rate_ = 0.0f;
};

#endif
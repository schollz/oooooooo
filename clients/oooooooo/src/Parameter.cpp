#include "Parameter.h"

#include <algorithm>
#include <cmath>

#include "DrawFunctions.h"
#include "Utilities.h"

using namespace softcut_jack_osc;

Parameter::Parameter() = default;
Parameter::~Parameter() = default;

JSON Parameter::toJSON() const {
  JSON json;
  json["name"] = name_;
  json["value"] = value_set_;
  json["lfo_period"] = lfo_period_;
  return json;
}
void Parameter::fromJSON(const JSON& json) {
  name_ = json["name"];
  value_set_ = json["value"];
  lfo_period_ = json["lfo_period"];
  set_(value_set_, true);
}

void Parameter::Init(float sample_rate, float min, float max, float inc,
                     float default_value, float lfo_min, float lfo_max,
                     float lfo_inc, float lfo_period, std::string name,
                     std::string unit,
                     std::function<void(float)> set_callback) {
  name_ = std::move(name);
  unit_ = std::move(unit);
  min_ = min;
  max_ = max;
  inc_ = inc;
  lfo_min_set_ = lfo_min;
  lfo_max_set_ = lfo_max;
  lfo_min_raw_ = linlin(lfo_min_set_, min_, max_, 0.0f, 1.0f);
  lfo_max_raw_ = linlin(lfo_max_set_, min_, max_, 0.0f, 1.0f);
  lfo_inc_ = lfo_inc;

  value_set_ = default_value;
  value_compute_ = value_set_;
  value_set_raw_ = linlin(value_set_, min_, max_, 0.0f, 1.0f);
  value_compute_raw_ = value_set_raw_;
  inc_raw_ = linlin(inc_, min_, max_, 0.0f, 1.0f);

  lfo_.Init(sample_rate);
  lfo_.SetFreq(1.0f / lfo_period);

  set_callback_ = std::move(set_callback);
  Bang();
}

std::string Parameter::String() {
  std::string lfo_str =
      lfo_active_ ? sprintf_str("%2.1f s", lfo_period_) : "   ";
  std::string num_str = sprintf_str("%2.2f", value_compute_);
  if (string_func_) {
    num_str = string_func_(value_compute_);
  }
  return sprintf_str("%s: %s %s %s", name_.c_str(), num_str.c_str(),
                     unit_.c_str(), lfo_str.c_str());
}

void Parameter::Bang() {
  if (set_callback_) {
    set_callback_(value_compute_);
  }
}

void Parameter::Update() {
  if (!lfo_active_) {
    value_compute_ = value_set_;
    value_compute_raw_ = value_set_raw_;
    return;
  }
  lfo_.Process();
  value_compute_ =
      linlin(lfo_.Value(), -1.0f, 1.0f, lfo_min_set_, lfo_max_set_);
  value_compute_raw_ = linlin(value_compute_, min_, max_, 0.0f, 1.0f);
  if (set_callback_) {
    set_callback_(value_compute_);
  }
}

void Parameter::set_(float value, bool quiet) {
  // compute current lfo delts
  float lfo_min_delta = value_set_ - lfo_min_set_;
  float lfo_max_delta = value_set_ - lfo_max_set_;
  value_set_ = fclamp(value, min_, max_);
  // update lfo min max
  lfo_min_set_ = fclamp(value_set_ - lfo_min_delta, min_, max_);
  lfo_max_set_ = fclamp(value_set_ - lfo_max_delta, min_, max_);
  lfo_min_raw_ = linlin(lfo_min_set_, min_, max_, 0, 1);
  lfo_max_raw_ = linlin(lfo_max_set_, min_, max_, 0, 1);
  value_set_raw_ = linlin(value_set_, min_, max_, 0, 1);
  if (!lfo_active_) {
    value_compute_ = value_set_;
    value_compute_raw_ = value_set_raw_;
  }
  if (set_callback_ && !lfo_active_ && !quiet) {
    set_callback_(value_set_);
  }
}

void Parameter::ValueDelta(float delta) {
  set_(value_set_ + delta * inc_, false);
}

void Parameter::LFODelta(float min_delta, float max_delta) {
  lfo_min_set_ = fclamp(lfo_min_set_ + min_delta * lfo_inc_, min_, max_);
  lfo_max_set_ = fclamp(lfo_max_set_ + max_delta * lfo_inc_, min_, max_);
  lfo_min_raw_ = linlin(lfo_min_set_, min_, max_, 0.0f, 1.0f);
  lfo_max_raw_ = linlin(lfo_max_set_, min_, max_, 0.0f, 1.0f);
}

void Parameter::Render(SDL_Renderer* renderer, TTF_Font* font, int x, int y,
                       int width, int height, bool selected) {
  float fill = value_set_raw_;
  float lfo = value_compute_raw_;
  float lfo_min = lfo_min_raw_;
  float lfo_max = lfo_max_raw_;
  std::string label = String();
  // Draw the entire bar as dim gray or brighter if selected
  if (selected) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100,
                           0);  // Brighter gray when selected
  } else {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 0);  // Dim gray
  }
  SDL_Rect barRect = {x, y, width, height};
  SDL_RenderFillRect(renderer, &barRect);

  // Draw the filled area as white or brighter white if selected
  if (selected) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255,
                           0);  // Bright white for selected
  } else {
    SDL_SetRenderDrawColor(renderer, 200, 200, 200,
                           0);  // Slightly dimmer white
  }
  SDL_Rect fillRect = {x, y, static_cast<int>(std::round(width * lfo)), height};
  SDL_RenderFillRect(renderer, &fillRect);

  // Draw a white line for the LFO position
  int lfo_x = static_cast<int>(std::round(x + width * lfo));
  if (lfo_x > fill) {
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 0);  // White for LFO line
  } else {
    SDL_SetRenderDrawColor(renderer, 205, 205, 205,
                           0);  // Dimmer white for LFO line
  }
  SDL_Rect lfoRect = {lfo_x, y, 4, height};
  SDL_RenderFillRect(renderer, &lfoRect);  // Vertical line for LFO

  // Draw a thin horizontal line for the LFO min max
  SDL_SetRenderDrawColor(renderer, 205, 205, 205,
                         0);  // Dimmer white for LFO line
                              // first draw from min to current set point

  // Draw a thin horizontal line from start_x to end_x
  SDL_SetRenderDrawColor(renderer, 205, 205, 205,
                         0);  // Dimmer white for LFO line
  SDL_Rect lfoMinRect = {static_cast<int>(std::round(x + width * lfo_min)),
                         static_cast<int>(std::round(y + height / 2) - 2),
                         static_cast<int>(std::round(width * (lfo - lfo_min))),
                         5};
  SDL_RenderFillRect(renderer, &lfoMinRect);
  SDL_SetRenderDrawColor(renderer, 64, 64, 64, 0);  // Dimmer white for LFO line
  SDL_Rect lfoMaxRect = {static_cast<int>(std::round(x + width * lfo)),
                         static_cast<int>(std::round(y + height / 2) - 2),
                         static_cast<int>(std::round(width * (lfo_max - lfo))),
                         5};
  SDL_RenderFillRect(renderer, &lfoMaxRect);

  // Render the label to the right of the bar
  if (!label.empty()) {
    // highlight text if selected
    if (selected) {
      drawText(renderer, font, label, x + width + 10, y, 175);
    } else {
      drawText(renderer, font, label, x + width + 10, y, 100);
    }
  }
}
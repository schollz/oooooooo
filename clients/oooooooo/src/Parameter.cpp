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
  json["x_"] = x_;
  json["y_"] = y_;
  json["width_"] = width_;
  json["height_"] = height_;
  json["name"] = name_;
  json["unit"] = unit_;
  json["min_"] = min_;
  json["max_"] = max_;
  json["inc_"] = inc_;
  json["value_set_"] = value_set_;
  json["value_set_raw_"] = value_set_raw_;
  json["value_compute_"] = value_compute_;
  json["value_compute_raw_"] = value_compute_raw_;
  json["inc_raw_"] = inc_raw_;
  json["lfo_min_delta_"] = lfo_min_delta_;
  json["lfo_max_delta_"] = lfo_max_delta_;
  json["lfo_min_set_"] = lfo_min_set_;
  json["lfo_max_set_"] = lfo_max_set_;
  json["lfo_min_raw_"] = lfo_min_raw_;
  json["lfo_max_raw_"] = lfo_max_raw_;
  json["lfo_inc_"] = lfo_inc_;
  json["lfo_period_"] = lfo_period_;
  json["lfo_waveform_"] = lfo_waveform_;
  json["lfo_active_"] = lfo_active_;
  return json;
}
void Parameter::fromJSON(const JSON& json) {
  x_ = json["x_"];
  y_ = json["y_"];
  width_ = json["width_"];
  height_ = json["height_"];
  name_ = json["name"];
  unit_ = json["unit"];
  min_ = json["min_"];
  max_ = json["max_"];
  inc_ = json["inc_"];
  value_set_ = json["value_set_"];
  value_set_raw_ = json["value_set_raw_"];
  value_compute_ = json["value_compute_"];
  value_compute_raw_ = json["value_compute_raw_"];
  inc_raw_ = json["inc_raw_"];
  lfo_min_delta_ = json["lfo_min_delta_"];
  lfo_max_delta_ = json["lfo_max_delta_"];
  lfo_min_set_ = json["lfo_min_set_"];
  lfo_max_set_ = json["lfo_max_set_"];
  lfo_min_raw_ = json["lfo_min_raw_"];
  lfo_max_raw_ = json["lfo_max_raw_"];
  lfo_inc_ = json["lfo_inc_"];
  lfo_period_ = json["lfo_period_"];
  lfo_waveform_ = json["lfo_waveform_"];
  lfo_active_ = json["lfo_active_"];
  hide_ = json["hide_"];
  lfo_.SetFreq(1.0f / lfo_period_);
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

  value_set_ = default_value;
  value_compute_ = value_set_;
  value_set_raw_ = linlin(value_set_, min_, max_, 0.0f, 1.0f);
  value_compute_raw_ = value_set_raw_;
  inc_raw_ = linlin(inc_, min_, max_, 0.0f, 1.0f);

  lfo_min_delta_ = std::abs(default_value - lfo_min);
  lfo_max_delta_ = std::abs(default_value - lfo_max);
  lfo_min_max_update();
  lfo_inc_ = lfo_inc;
  lfo_period_ = lfo_period;
  lfo_.Init(sample_rate);
  lfo_.SetFreq(1.0f / lfo_period);

  set_callback_ = std::move(set_callback);
  Bang();
}

void Parameter::DeltaLFOPeriod(float delta) {
  lfo_period_ = fclamp(lfo_period_ + delta * lfo_inc_, 0.01f, 1200.0f);
  lfo_.SetFreq(1.0f / lfo_period_);
}

std::string Parameter::String() {
  if (shared_parameter_) {
    return shared_parameter_->String();
  }

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
  if (shared_parameter_) {
    shared_parameter_->Bang();
    return;
  }

  if (set_callback_) {
    set_callback_(value_compute_);
  }
}

void Parameter::Update() {
  if (shared_parameter_) {
    shared_parameter_->Update();
    return;
  }

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
  if (shared_parameter_) {
    shared_parameter_->set_(value, quiet);
    return;
  }

  value_set_ = fclamp(value, min_, max_);
  value_set_raw_ = linlin(value_set_, min_, max_, 0, 1);
  if (!lfo_active_) {
    value_compute_ = value_set_;
    value_compute_raw_ = value_set_raw_;
  }
  lfo_min_max_update();
  if (set_callback_ && !lfo_active_ && !quiet) {
    set_callback_(value_set_);
  }
}

void Parameter::ValueDelta(float delta) {
  if (shared_parameter_) {
    shared_parameter_->ValueDelta(delta);
    return;
  }

  set_(value_set_ + delta * inc_, false);
}

void Parameter::LFODelta(float min_delta, float max_delta) {
  if (shared_parameter_) {
    shared_parameter_->LFODelta(min_delta, max_delta);
    return;
  }

  lfo_min_delta_ += min_delta * lfo_inc_;
  lfo_max_delta_ += max_delta * lfo_inc_;
  if (lfo_min_delta_ < 0.0f) {
    lfo_min_delta_ = 0.0f;
  }
  if (lfo_max_delta_ < 0.0f) {
    lfo_max_delta_ = 0.0f;
  }
  lfo_min_max_update();
}

void Parameter::lfo_min_max_update() {
  lfo_min_set_ = fclamp(value_set_ - lfo_min_delta_, min_, max_);
  lfo_max_set_ = fclamp(value_set_ + lfo_max_delta_, min_, max_);
  lfo_min_raw_ = linlin(lfo_min_set_, min_, max_, 0, 1);
  lfo_max_raw_ = linlin(lfo_max_set_, min_, max_, 0, 1);
}

void Parameter::Render(SDL_Renderer* renderer, TTF_Font* font, int x, int y,
                       int width, int height, bool selected, float brightness) {
  if (shared_parameter_) {
    shared_parameter_->Render(renderer, font, x, y, width, height, selected,
                              brightness);
    return;
  }

  x_ = x;
  y_ = y;
  width_ = width;
  height_ = height;
  float fill = value_set_raw_;
  float lfo = value_compute_raw_;
  float lfo_min = lfo_min_raw_;
  float lfo_max = lfo_max_raw_;
  std::string label = String();

  // Apply brightness to all colors
  auto applyBrightness = [brightness](int color) {
    return static_cast<int>(color * brightness);
  };

  // Draw the entire bar as dim gray or brighter if selected
  if (selected) {
    SDL_SetRenderDrawColor(renderer, applyBrightness(100), applyBrightness(100),
                           applyBrightness(100),
                           0);  // Brighter gray when selected
  } else {
    SDL_SetRenderDrawColor(renderer, applyBrightness(50), applyBrightness(50),
                           applyBrightness(50),
                           0);  // Dim gray
  }
  SDL_Rect barRect = {x, y, width, height};
  SDL_RenderFillRect(renderer, &barRect);

  // Draw the filled area as white or brighter white if selected
  if (selected) {
    SDL_SetRenderDrawColor(renderer, applyBrightness(255), applyBrightness(255),
                           applyBrightness(255),
                           0);  // Bright white for selected
  } else {
    SDL_SetRenderDrawColor(renderer, applyBrightness(200), applyBrightness(200),
                           applyBrightness(200),
                           0);  // Slightly dimmer white
  }
  SDL_Rect fillRect = {x, y, static_cast<int>(std::round(width * lfo)), height};
  SDL_RenderFillRect(renderer, &fillRect);

  // Draw a white line for the LFO position
  int lfo_x = static_cast<int>(std::round(x + width * lfo));
  if (lfo_x > fill) {
    SDL_SetRenderDrawColor(renderer, applyBrightness(64), applyBrightness(64),
                           applyBrightness(64),
                           0);  // White for LFO line
  } else {
    SDL_SetRenderDrawColor(renderer, applyBrightness(205), applyBrightness(205),
                           applyBrightness(205),
                           0);  // Dimmer white for LFO line
  }
  SDL_Rect lfoRect = {lfo_x, y, 4, height};
  SDL_RenderFillRect(renderer, &lfoRect);  // Vertical line for LFO

  // Draw a thin horizontal line for the LFO min max
  SDL_SetRenderDrawColor(renderer, applyBrightness(205), applyBrightness(205),
                         applyBrightness(205),
                         0);  // Dimmer white for LFO line

  SDL_Rect lfoMinRect = {static_cast<int>(std::round(x + width * lfo_min)),
                         static_cast<int>(std::round(y + height / 2) - 2),
                         static_cast<int>(std::round(width * (lfo - lfo_min))),
                         5};
  SDL_RenderFillRect(renderer, &lfoMinRect);

  SDL_SetRenderDrawColor(renderer, applyBrightness(64), applyBrightness(64),
                         applyBrightness(64),
                         0);  // Dimmer white for LFO line
  SDL_Rect lfoMaxRect = {static_cast<int>(std::round(x + width * lfo)),
                         static_cast<int>(std::round(y + height / 2) - 2),
                         static_cast<int>(std::round(width * (lfo_max - lfo))),
                         5};
  SDL_RenderFillRect(renderer, &lfoMaxRect);

  // Render the label to the right of the bar
  if (!label.empty()) {
    // Apply brightness to text as well
    int textBrightness = selected ? applyBrightness(175) : applyBrightness(100);
    drawText(renderer, font, label, x + width + 10, y, textBrightness);
  }
}

bool Parameter::RegisterClick(float mouseX, float mouseY, bool dragging) {
  if (shared_parameter_) {
    return shared_parameter_->RegisterClick(mouseX, mouseY, dragging);
  }

  // Check if the click is within the bounds of the parameter bar
  if (mouseX >= x_ && mouseX <= x_ + width_ &&
      (dragging || (mouseY >= y_ && mouseY <= y_ + height_))) {
    // Calculate the raw value (0.0-1.0) based on relative position within the
    // bar
    float relativeX = mouseX - x_;
    float rawValue = fclamp(relativeX / width_, 0.0f, 1.0f);

    // Update the parameter's value using ValueSetRaw
    ValueSetRaw(rawValue, false);
    return true;
  }
  return false;
}

void Parameter::ValueSetRaw(float value, bool quiet) {
  if (shared_parameter_) {
    shared_parameter_->ValueSetRaw(value, quiet);
    return;
  }

  value_set_raw_ = fclamp(value, 0.0f, 1.0f);
  value_set_ = linlin(value_set_raw_, 0.0f, 1.0f, min_, max_);
  set_(value_set_, quiet);
}
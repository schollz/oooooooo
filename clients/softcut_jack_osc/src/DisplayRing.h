#pragma once
#include <cmath>

#include "DrawFunctions.h"
#include "SoftcutClient.h"
#include "Utilities.h"

using namespace softcut_jack_osc;

class DisplayRing {
 public:
  DisplayRing() = default;
  ~DisplayRing() = default;
  void Update(SoftcutClient *softCutClient_, int i, float width_,
              float height_) {
    pos_ = softCutClient_->getSavedPosition(i);
    dur_ = softCutClient_->getDuration(i);
    level_ = softCutClient_->getOutLevel(i);
    pan_ = softCutClient_->getPan(i);
    id_ = i;
    x_ = linlin(pan_, -1, 1, 0, width_);
    y_ = linlin(amp2db(level_), -64, 24, height_, 0);
    radius_ = linlin(dur_, 0, 1, 0, 20);
    position_ = pos_ / dur_;
    thickness_ = 2.5f;
  }
  void RegisterClick(float mouseX, float mouseY);
  void Render(SDL_Renderer *renderer, PerlinNoise *perlinGenerator,
              float *noiseTimeValue) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 0);
    drawRing(renderer, perlinGenerator, id_, x_, y_, radius_, position_,
             thickness_, noiseTimeValue, true, true);
  }

 private:
  int id_;
  float pos_;
  float dur_;
  float level_;
  float pan_;
  float x_;
  float y_;
  float radius_;
  float position_;
  float thickness_;
};
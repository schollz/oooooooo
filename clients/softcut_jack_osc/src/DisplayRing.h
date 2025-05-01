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
  void Update(SoftcutClient *softCutClient, int i, float width_,
              float height_) {
    softCutClient_ = softCutClient;
    pos_ = softCutClient_->getSavedPosition(i);
    dur_ = softCutClient_->getDuration(i);
    level_ = softCutClient_->getOutLevel(i);
    pan_ = softCutClient_->getPan(i);
    start_ = softCutClient_->getLoopStart(i);
    end_ = softCutClient_->getLoopEnd(i);
    id_ = i;
    x_ = linlin(pan_, -1, 1, 0, width_);
    y_ = linlin(amp2db(level_), -64, 24, height_, 0);
    radius_ = linlin(dur_, 0, 1, 0, 20);
    position_ = (pos_ - start_) / dur_;
    thickness_ = 2.5f;
  }
  void RegisterClick(float mouseX, float mouseY);
  void HandleDrag(float mouseX, float mouseY, float width, float height);
  void StopDrag();
  void Render(SDL_Renderer *renderer, PerlinNoise *perlinGenerator,
              float *noiseTimeValue) {
    drawRing(renderer, perlinGenerator, id_, x_, y_, radius_, position_,
             thickness_, noiseTimeValue, true, true);
  }
  bool ClickedRing() {
    if (clicked_ring_) {
      clicked_ring_ = false;
      dragging_ = true;
      return true;
    }
    return false;
  }
  bool ClickedRadius() {
    if (clicked_radius_) {
      clicked_radius_ = false;
      dragging_ = false;
      return true;
    }
    return false;
  }
  float GetClickedRadius() { return clicked_radius_angle_normalized_; }
  bool IsDragging() const { return dragging_; }
  int GetId() const { return id_; }

 private:
  SoftcutClient *softCutClient_ = nullptr;
  int id_;
  float pos_;
  float dur_;
  float start_;
  float end_;
  float level_;
  float pan_;
  float x_;
  float y_;
  float radius_;
  float position_;
  float thickness_;
  bool clicked_ring_ = false;
  bool dragging_ = false;
  bool clicked_radius_ = false;
  float clicked_radius_angle_normalized_ = 0;
};
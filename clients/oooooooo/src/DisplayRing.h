#pragma once
#include <cmath>

#include "DrawFunctions.h"
#include "Parameters.h"
#include "SoftcutClient.h"
#include "Utilities.h"

using namespace softcut_jack_osc;

class DisplayRing {
 public:
  DisplayRing() = default;
  ~DisplayRing() = default;
  void Init(SoftcutClient *softCutClient, Parameters *p, int i);
  void Update(float width_, float height_);
  void RegisterClick(float mouseX, float mouseY);
  void HandleDrag(float mouseX, float mouseY, float width, float height);
  void StopDrag();
  void Render(SDL_Renderer *renderer, PerlinNoise *perlinGenerator,
              float *noiseTimeValue, float mainContentFadeAlpha_,
              bool isSelected);
  bool ClickedRing();
  bool ClickedRadius();
  float GetClickedRadius() { return clicked_radius_angle_normalized_; }
  bool IsDragging() const { return dragging_; }
  int GetId() const { return id_; }
  float GetDistanceToCenter() const { return distance_to_center_; }

 private:
  SoftcutClient *softCutClient_ = nullptr;
  Parameters *params_ = nullptr;
  int id_;
  float pos_;
  float dur_;
  float pregain_;
  float start_;
  float end_;
  float pan_;
  float x_;
  float y_;
  float radius_;
  float position_;
  float thickness_;
  bool clicked_ring_ = false;
  float distance_to_center_ = 0;
  bool dragging_ = false;
  bool clicked_radius_ = false;
  float clicked_radius_angle_normalized_ = 0;
};
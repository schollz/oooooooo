#include "DisplayRing.h"

#include <cmath>
#include <iostream>

#include "SoftcutClient.h"

using namespace softcut_jack_osc;

void DisplayRing::Init(SoftcutClient *softCutClient, Parameters *p, int i) {
  softCutClient_ = softCutClient;
  params_ = p;
  id_ = i;
}

void DisplayRing::Update(float width_, float height_) {
  pregain_ = softCutClient_->getPreGain(id_);
  pos_ = softCutClient_->getSavedPosition(id_);
  dur_ = softCutClient_->getDuration(id_);
  start_ = softCutClient_->getLoopStart(id_);
  end_ = softCutClient_->getLoopEnd(id_);
  x_ = params_->GetRaw(Parameters::PARAM_PAN) * width_;
  y_ = (1 - params_->GetRaw(Parameters::PARAM_LEVEL)) * height_;
  radius_ = linlin(dur_, 0, 30, 10, 200);
  // limit radius to 75% width of screen
  if (radius_ > width_ / 3) {
    radius_ = width_ / 3;
  }
  position_ = (pos_ - start_) / dur_;
  thickness_ = 2.5f;
}

void DisplayRing::Render(SDL_Renderer *renderer, PerlinNoise *perlinGenerator,
                         float *noiseTimeValue) {
  drawRing(renderer, perlinGenerator, id_, x_, y_, radius_, position_,
           thickness_, noiseTimeValue, true, true);
}

bool DisplayRing::ClickedRing() {
  if (clicked_ring_) {
    clicked_ring_ = false;
    dragging_ = true;
    return true;
  }
  return false;
}

bool DisplayRing::ClickedRadius() {
  if (clicked_radius_) {
    clicked_radius_ = false;
    dragging_ = false;
    return true;
  }
  return false;
}

void DisplayRing::RegisterClick(float mouseX, float mouseY) {
  // First determine if the mouse is with 0.5 radii of the center of the ring
  float distance = sqrt(pow(mouseX - x_, 2) + pow(mouseY - y_, 2));
  if (distance < radius_ * 0.75f) {
    clicked_ring_ = true;
    distance_to_center_ = distance;
  } else if (distance > radius_ * 0.9f && distance < radius_ * 1.1f) {
    clicked_radius_ = true;
    // determine the angle from the top of the circle
    float angle = atan2(mouseY - y_, mouseX - x_);
    // adjust angle to make 1.0 right before the 12 o'clock position
    angle += M_PI / 2;
    // convert angle to a value between 0 and 1
    clicked_radius_angle_normalized_ =
        fmod((angle + 2 * M_PI), (2 * M_PI)) / (2 * M_PI);

    // do cut
    softCutClient_->handleCommand(new Commands::CommandPacket(
        Commands::Id::SET_CUT_POSITION, id_,
        start_ + dur_ * clicked_radius_angle_normalized_));
  }
}

void DisplayRing::HandleDrag(float mouseX, float mouseY, float width,
                             float height) {
  if (!dragging_ || !softCutClient_) {
    return;
  }
  params_->ValueSetRaw(Parameters::PARAM_LEVEL, 1 - mouseY / height, false);
  params_->ValueSetRaw(Parameters::PARAM_PAN, mouseX / width, false);
  x_ = mouseX;
  y_ = mouseY;
}

void DisplayRing::StopDrag() { dragging_ = false; }
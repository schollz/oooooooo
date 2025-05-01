#include "DisplayRing.h"

#include <cmath>
#include <iostream>

#include "SoftcutClient.h"

using namespace softcut_jack_osc;

void DisplayRing::RegisterClick(float mouseX, float mouseY) {
  // First determine if the mouse is with 0.5 radii of the center of the ring
  float distance = sqrt(pow(mouseX - x_, 2) + pow(mouseY - y_, 2));
  std::cerr << mouseX << " " << mouseY << " " << x_ << " " << y_ << " "
            << distance << " " << radius_ << std::endl;
  if (distance < radius_ * 0.5f) {
    clicked_ring_ = true;
  } else if (distance > radius_ * 0.9f && distance < radius_ * 1.1f) {
    clicked_radius_ = true;
  }
}
#include "DisplayRing.h"

#include <cmath>
#include <iostream>

#include "SoftcutClient.h"

using namespace softcut_jack_osc;

void DisplayRing::RegisterClick(float mouseX, float mouseY) {
  // First determine if the mouse is with 0.5 radii of the center of the ring
  float distance = sqrt(pow(mouseX - x_, 2) + pow(mouseY - y_, 2));
  if (distance < radius_ + 0.5f) {
    // If so, determine if the mouse is within the ring
    if (distance > radius_ - 0.5f) {
      // If so, register the click
      std::cout << "Clicked on ring with ID: " << id_ << std::endl;
    }
  }
}
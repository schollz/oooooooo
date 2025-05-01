#include "DisplayRing.h"

#include <cmath>
#include <iostream>

#include "SoftcutClient.h"

using namespace softcut_jack_osc;

void DisplayRing::RegisterClick(float mouseX, float mouseY) {
  // First determine if the mouse is with 0.5 radii of the center of the ring
  float distance = sqrt(pow(mouseX - x_, 2) + pow(mouseY - y_, 2));
  if (distance < radius_ * 0.75f) {
    clicked_ring_ = true;
  } else if (distance > radius_ * 0.9f && distance < radius_ * 1.1f) {
    clicked_radius_ = true;
    // determine the angle from the top of the circle
    float angle = atan2(mouseY - y_, mouseX - x_);
    // adjust angle to make 1.0 right before the 12 o'clock position
    angle += M_PI / 2;
    // convert angle to a value between 0 and 1
    clicked_radius_angle_normalized_ =
        fmod((angle + 2 * M_PI), (2 * M_PI)) / (2 * M_PI);

    float seconds = start_ + dur_ * clicked_radius_angle_normalized_;
    std::cout << "Clicked radius: " << clicked_radius_angle_normalized_
              << " seconds: " << seconds << " start: " << start_
              << " end: " << end_ << " dur: " << dur_ << std::endl;
    // send the cut position to the softcut client

    // do cut
    softCutClient_->handleCommand(new Commands::CommandPacket(
        Commands::Id::SET_CUT_POSITION, id_, seconds));
  }
}

void DisplayRing::HandleDrag(float mouseX, float mouseY, float width,
                             float height) {
  if (!dragging_ || !softCutClient_) {
    return;
  }

  // Map mouseX to pan (-1 to 1)
  float new_pan = linlin(mouseX, 0, width, -1.0f, 1.0f);
  // Clamp pan value
  new_pan = std::max(-1.0f, std::min(1.0f, new_pan));

  // Map mouseY to level (convert from dB to amplitude)
  float new_level_db = linlin(mouseY, height, 0, -64.0f, 24.0f);
  float new_level = db2amp(new_level_db);

  // Update SoftcutClient with new values
  softCutClient_->handleCommand(
      new Commands::CommandPacket(Commands::Id::SET_LEVEL_CUT, id_, new_level));
  softCutClient_->handleCommand(
      new Commands::CommandPacket(Commands::Id::SET_PAN_CUT, id_, new_pan));

  // Update our local values
  level_ = new_level;
  pan_ = new_pan;

  // Update x and y positions
  x_ = mouseX;
  y_ = mouseY;
}

void DisplayRing::StopDrag() { dragging_ = false; }
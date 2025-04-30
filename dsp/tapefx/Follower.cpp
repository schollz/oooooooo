#include "Follower.h"

float Follower::process(float x) {
  const auto abs_x = std::abs(x);
  if (abs_x > y_) {
    y_ = a_ * y_ + (1 - a_) * abs_x;
  } else {
    y_ = b_ * y_ + (1 - b_) * abs_x;
  }
  return y_;
}
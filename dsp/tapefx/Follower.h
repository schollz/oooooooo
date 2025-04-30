#ifndef LIB_FOLLOWER_H
#define LIB_FOLLOWER_H

#include <cmath>

class Follower {
 public:
  Follower(){};
  Follower(float sampleRate)
      : a_(std::exp(-1.0 / (0.001 * sampleRate))),  // Attack time = 1ms
        b_(std::exp(-1.0 / (0.020 * sampleRate))),  // Decay time = 20ms
        y_(0) {}
  Follower(float sampleRate, float attack, float decay)
      : a_(std::exp(-1.0 / (attack * sampleRate))),  // Attack time = 1ms
        b_(std::exp(-1.0 / (decay * sampleRate))),   // Decay time = 20ms
        y_(0) {}

  float process(float x);

 private:
  float a_, b_, y_;
};

#endif
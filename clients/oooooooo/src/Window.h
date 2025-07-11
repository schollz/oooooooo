//
// Created by emb on 1/25/19.
//

#ifndef CRONE_WINDOW_H
#define CRONE_WINDOW_H

#include <cstddef>

namespace softcut_jack_osc {

class Window {
 public:
  // raised-cosine window
  static constexpr size_t raisedCosShortLen = 48 * 50;
  static const float raisedCosShort[raisedCosShortLen];
};

}  // namespace softcut_jack_osc

#endif  // CRONE_WINDOW_H

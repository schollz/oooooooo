#ifndef HELP_SYSTEM_H
#define HELP_SYSTEM_H
#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

#include <string>
#include <vector>

class HelpSystem {
 public:
  HelpSystem();
  ~HelpSystem();

  void Init(TTF_Font* font);
  void Toggle();
  void Render(SDL_Renderer* renderer, int windowWidth);

  bool isVisible() const { return helpVisible; }

 private:
  TTF_Font* font;
  bool helpVisible;
  float fadeAlpha;   // 0.0 = fully transparent, 1.0 = fully opaque
  float fadeTarget;  // Target alpha value
  float fadeStep;    // Amount to change alpha per frame

  std::vector<std::string> helpMessages;
};
#endif
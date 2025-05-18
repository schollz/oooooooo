#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H
#pragma once

#include <SDL2/SDL.h>

#include <iostream>
#include <memory>
#include <unordered_map>

#include "Parameters.h"
#include "SoftcutClient.h"

class KeyboardHandler {
 public:
  KeyboardHandler() = default;
  ~KeyboardHandler() = default;

  void Init(SoftcutClient* sc, Parameters* params, int numVoices) {
    softcut_ = sc;
    params_ = params;
    numVoices_ = numVoices;
  }

  void handleKeyDown(SDL_Keycode key, bool isRepeat, SDL_Keymod modifiers,
                     int* selectedLoop);
  void handleKeyUp(SDL_Keycode key, int selectedLoop);

 private:
  SoftcutClient* softcut_;
  Parameters* params_;
  int numVoices_;
  int voiceToCopy_ = -1;

  std::unordered_map<SDL_Keycode, bool> keysHeld_;
};

#endif
#include "KeyboardHandler.h"

#include "Parameters.h"
#include "SoftcutClient.h"

using namespace softcut_jack_osc;

void KeyboardHandler::handleKeyDown(SDL_Keycode key, bool isRepeat,
                                    SDL_Keymod modifiers, int selectedLoop) {
  keysHeld_[key] = true;

  std::cerr << "KeyboardHandler::handleKeyDown Key pressed: "
            << SDL_GetKeyName(key) << " " << modifiers << std::endl;

  switch (key) {
    case SDLK_l:
      params_[selectedLoop].ToggleLFO();
      break;
    case SDLK_UP:
      for (int i = 0; i < numVoices_; i++) {
        params_[i].SelectedDelta(isRepeat ? 2 : 1);
      }
      break;
    case SDLK_DOWN:
      for (int i = 0; i < numVoices_; i++) {
        params_[i].SelectedDelta(isRepeat ? -2 : -1);
      }
      break;
    case SDLK_LEFT:
      if (keysHeld_[SDLK_LALT] || keysHeld_[SDLK_RALT]) {
        params_[selectedLoop].LFODelta(0, isRepeat ? -2.0f : -1.0f);
      } else if (keysHeld_[SDLK_LCTRL] || keysHeld_[SDLK_RCTRL]) {
        params_[selectedLoop].LFODelta(isRepeat ? 2.0f : 1.0f, 0);
      } else {
        params_[selectedLoop].ValueDelta(isRepeat ? -2.0f : -1.0f);
      }
      break;
    case SDLK_RIGHT:
      if (keysHeld_[SDLK_LALT] || keysHeld_[SDLK_RALT]) {
        params_[selectedLoop].LFODelta(0, isRepeat ? 2.0f : 1.0f);
      } else if (keysHeld_[SDLK_LCTRL] || keysHeld_[SDLK_RCTRL]) {
        params_[selectedLoop].LFODelta(isRepeat ? -2.0f : -1.0f, 0);
      } else {
        params_[selectedLoop].ValueDelta(isRepeat ? 2.0f : 1.0f);
      }
      break;
    default:
      break;
  }
}

void KeyboardHandler::handleKeyUp(SDL_Keycode key, int selectedLoop) {
  keysHeld_[key] = false;
  std::cerr << "KeyboardHandler::handleKeyUp Key released: "
            << SDL_GetKeyName(key) << " " << selectedLoop << std::endl;
}
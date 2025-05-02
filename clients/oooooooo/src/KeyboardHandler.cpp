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
      params_[selectedLoop].ValueDelta(isRepeat ? -2.0f : -1.0f);
      break;
    case SDLK_RIGHT:
      params_[selectedLoop].ValueDelta(isRepeat ? 2.0f : 1.0f);
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
// KeyboardHandler.cpp
#include "KeyboardHandler.h"

#include <SDL2/SDL.h>

#include <unordered_map>

#include "HelpSystem.h"
#include "src/tapeloops/TapeLoops.h"
#include "src/wav/Wav.h"

// Static variables
static TapeLoops* s_tape_loops_ptr = nullptr;
static LoggerFunc log_info = nullptr;
static LoggerFunc log_debug = nullptr;

// Track whether keys are being held
static std::unordered_map<SDL_Keycode, bool> keys_held;

// Current selected parameter
static int current_parameter = TapeLoop::PARAM_PAN;

void initializeKeyboardHandler(TapeLoops* tape_loops_ptr,
                               LoggerFunc info_logger,
                               LoggerFunc debug_logger) {
  s_tape_loops_ptr = tape_loops_ptr;
  log_info = info_logger;
  log_debug = debug_logger;
  keys_held.clear();
}

void cleanupKeyboardHandler() {
  s_tape_loops_ptr = nullptr;
  log_info = nullptr;
  log_debug = nullptr;
  keys_held.clear();
}

int getCurrentParameter() { return current_parameter; }

void setCurrentParameter(int param) { current_parameter = param; }

// Function to adjust parameters based on currently selected parameter
void adjustParameter(int direction) {
  if (!s_tape_loops_ptr) return;

  int loop_index = s_tape_loops_ptr->GetSelected();

  switch (current_parameter) {
    case TapeLoop::PARAM_LOOP:
      // Change selected loop
      if (direction > 0) {
        s_tape_loops_ptr->SetSelected((loop_index + 1) % CONFIG_VOICE_NUM);
      } else {
        s_tape_loops_ptr->SetSelected((loop_index - 1 + CONFIG_VOICE_NUM) %
                                      CONFIG_VOICE_NUM);
      }
      break;

    default:
      if (direction > 0) {
        s_tape_loops_ptr->loops[loop_index]
            .Params[current_parameter]
            .Increase();
      } else {
        s_tape_loops_ptr->loops[loop_index]
            .Params[current_parameter]
            .Decrease();
      }
      break;
  }
}

// Handle key down events
void handle_key_down(SDL_Keycode key, bool is_repeat, SDL_Keymod modifiers) {
  if (!s_tape_loops_ptr) return;

  // Check if this is the first press or a repeat
  if (is_repeat) {
    // Handle held key logic
    if (log_debug) log_debug("Key held: %d", key);

    // Handle repeated key presses for navigation and adjustment
    switch (key) {
      case SDLK_PLUS:
      case SDLK_EQUALS:
        s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()]
            .Params[current_parameter]
            .IncreaseLFOMax();
        break;
      case SDLK_MINUS:
      case SDLK_UNDERSCORE:
        s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()]
            .Params[current_parameter]
            .IncreaseLFOMin();
        break;
      case SDLK_RIGHTBRACKET:
        s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()]
            .Params[current_parameter]
            .DecreaseLFOMax();
        break;
      case SDLK_LEFTBRACKET:
        s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()]
            .Params[current_parameter]
            .DecreaseLFOMin();
        break;
      case SDLK_QUOTE:
      case SDLK_COMMA:
        s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()]
            .Params[current_parameter]
            .IncreaseLFOPeriod();
        break;
      case SDLK_SEMICOLON:
      case SDLK_COLON:
        s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()]
            .Params[current_parameter]
            .DecreaseLFOPeriod();
        break;

      case SDLK_UP:
        // Navigate parameters up
        current_parameter = (current_parameter - 1 + TapeLoop::PARAM_COUNT) %
                            TapeLoop::PARAM_COUNT;
        break;
      case SDLK_DOWN:
        // Navigate parameters down
        current_parameter = (current_parameter + 1) % TapeLoop::PARAM_COUNT;
        break;
      case SDLK_LEFT:
        // Adjust selected parameter down
        adjustParameter(-1);
        break;
      case SDLK_RIGHT:
        // Adjust selected parameter up
        adjustParameter(1);
        break;
    }

    return;  // Skip the initial press handlers for repeated keys
  }

  // Mark this key as being held
  keys_held[key] = true;

  // Handle initial key press logic
  if (log_debug) log_debug("Initial key press: %d", key);

  switch (key) {
    case SDLK_PLUS:
    case SDLK_EQUALS:
      s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()]
          .Params[current_parameter]
          .IncreaseLFOMax();
      break;
    case SDLK_MINUS:
    case SDLK_UNDERSCORE:
      s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()]
          .Params[current_parameter]
          .IncreaseLFOMin();
      break;
    case SDLK_RIGHTBRACKET:
      s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()]
          .Params[current_parameter]
          .DecreaseLFOMax();
      break;
    case SDLK_LEFTBRACKET:
      s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()]
          .Params[current_parameter]
          .DecreaseLFOMin();
      break;
    case SDLK_QUOTE:
    case SDLK_COMMA:
      s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()]
          .Params[current_parameter]
          .IncreaseLFOPeriod();
      break;
    case SDLK_SEMICOLON:
    case SDLK_COLON:
      s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()]
          .Params[current_parameter]
          .DecreaseLFOPeriod();
      break;

    case SDLK_s:
      // Create a JSON for all loops and save to file
      s_tape_loops_ptr->saveToFile("tape_loops.json");
      // save tapes if they have data
      for (size_t i = 0; i < CONFIG_VOICE_NUM; ++i) {
        if (s_tape_loops_ptr->loops[i].HasData()) {
          std::vector<float> samples = s_tape_loops_ptr->loops[i].DumpSamples();
          log_debug("TapeLoop[%d]::DumpSamples %zu", i, samples.size());
          Wav wav;
          wav.SaveToFile("tape_loop_" + std::to_string(i) + ".wav", samples);
        }
      }
      if (log_info) log_info("Saved tape loops to file");
      break;
    case SDLK_l:
      // toggle lfo
      s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()]
          .Params[current_parameter]
          .ToggleLFO();
      break;

    case SDLK_p:
      s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()].TogglePlay();
      break;
    case SDLK_h:
      toggleHelp();
      break;
    case SDLK_r:
      if (modifiers & KMOD_CTRL) {
        // Call ToggleRecordOnce when Ctrl+R is pressed
        s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()]
            .ToggleRecordOnce();
        if (log_debug) log_debug("ToggleRecordOnce triggered");
      } else {
        // Regular R key (without Ctrl)
        s_tape_loops_ptr->loops[s_tape_loops_ptr->GetSelected()].ToggleRecord();
      }
      break;
    case SDLK_UP:
      // Navigate parameters up
      current_parameter = (current_parameter - 1 + TapeLoop::PARAM_COUNT) %
                          TapeLoop::PARAM_COUNT;
      break;
    case SDLK_DOWN:
      // Navigate parameters down
      current_parameter = (current_parameter + 1) % TapeLoop::PARAM_COUNT;
      break;
    case SDLK_LEFT:
      // Adjust selected parameter down
      adjustParameter(-1);
      break;
    case SDLK_RIGHT:
      // Adjust selected parameter up
      adjustParameter(1);
      break;
    case SDLK_KP_1:
    case SDLK_1:
      s_tape_loops_ptr->SetSelected(0);
      break;
    case SDLK_KP_2:
    case SDLK_2:
      s_tape_loops_ptr->SetSelected(1);
      break;
    case SDLK_KP_3:
    case SDLK_3:
      s_tape_loops_ptr->SetSelected(2);
      break;
    case SDLK_KP_4:
    case SDLK_4:
      s_tape_loops_ptr->SetSelected(3);
      break;
    case SDLK_KP_5:
    case SDLK_5:
      s_tape_loops_ptr->SetSelected(4);
      break;
    case SDLK_KP_6:
    case SDLK_6:
      s_tape_loops_ptr->SetSelected(5);
      break;
    case SDLK_KP_7:
    case SDLK_7:
      s_tape_loops_ptr->SetSelected(6);
      break;
    default:
      break;
  }
}

// Handle key up events
void handle_key_up(SDL_Keycode key) {
  // Mark this key as no longer being held
  keys_held[key] = false;

  if (log_debug) log_debug("Key released: %d", key);

  // Add any key release logic here
  switch (key) {
    // Example: Stop an action when key is released
    case SDLK_s:
      if (log_info) log_info("Stopped action on key release");
      break;
  }
}
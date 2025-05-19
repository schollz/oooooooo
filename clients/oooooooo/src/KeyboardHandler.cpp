
#include "KeyboardHandler.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "Display.h"
#include "Parameters.h"
#include "SoftcutClient.h"

using namespace softcut_jack_osc;

void KeyboardHandler::handleKeyDown(SDL_Keycode key, bool isRepeat,
                                    SDL_Keymod modifiers [[maybe_unused]],
                                    int *selectedLoop) {
  keysHeld_[key] = true;

  // std::cerr << "KeyboardHandler::handleKeyDown Key pressed: "
  //           << SDL_GetKeyName(key) << " " << modifiers << std::endl;

  switch (key) {
    // plus
    case SDLK_PLUS:
    case SDLK_EQUALS:
      params_[*selectedLoop].DeltaLFOPeriod(isRepeat ? 0.5f : 0.1f);
      break;
    case SDLK_MINUS:
    case SDLK_UNDERSCORE:
      params_[*selectedLoop].DeltaLFOPeriod(isRepeat ? -0.5f : -0.1f);
      break;
    case SDLK_a:
      if (!isRepeat) {
        if (keysHeld_[SDLK_LCTRL] || keysHeld_[SDLK_RCTRL]) {
          float value = params_[*selectedLoop].GetValue(
              static_cast<Parameters::ParameterName>(
                  params_[*selectedLoop].GetSelected()));
          for (int i = 0; i < numVoices_; i++) {
            params_[i].ValueSet(static_cast<Parameters::ParameterName>(
                                    params_[i].GetSelected()),
                                value, false);
          }
        }
      }
      break;
    case SDLK_p:
      if (!isRepeat) {
        if (softcut_->WasPrimed(*selectedLoop) &&
            softcut_->IsRecording(*selectedLoop)) {
          softcut_->ToggleRecord(*selectedLoop);
        } else {
          softcut_->TogglePlay(*selectedLoop);
        }
      }
      break;
    case SDLK_c:
      if (!isRepeat) {
        if (keysHeld_[SDLK_LCTRL] || keysHeld_[SDLK_RCTRL]) {
          voiceToCopy_ = *selectedLoop;
        }
      }
      break;
    case SDLK_v:
      if (!isRepeat) {
        if (keysHeld_[SDLK_LCTRL] || keysHeld_[SDLK_RCTRL]) {
          if (voiceToCopy_ != -1) {
            JSON json = params_[voiceToCopy_].toJSON();
            params_[*selectedLoop].fromJSON(json);
            softcut_->copyBufferFromLoopToLoop(voiceToCopy_, *selectedLoop);
            // set the Duration
            params_[*selectedLoop].ValueSet(
                Parameters::PARAM_DURATION,
                params_[voiceToCopy_].GetValue(Parameters::PARAM_DURATION),
                false);
            voiceToCopy_ = -1;
          }
        }
      }
      break;
    case SDLK_r:
      if (!isRepeat) {
        if (keysHeld_[SDLK_LCTRL] || keysHeld_[SDLK_RCTRL]) {
          softcut_->ToggleRecordOnce(*selectedLoop);
        } else if (keysHeld_[SDLK_LSHIFT] || keysHeld_[SDLK_RSHIFT]) {
          softcut_->TogglePrime(*selectedLoop);
          if (softcut_->IsPrimed(*selectedLoop)) {
            std::cerr << "primed" << std::endl;
            // increase the duration
            params_[*selectedLoop].SetMax(Parameters::PARAM_DURATION, 60.0f);
            params_[*selectedLoop].ValueSet(Parameters::PARAM_DURATION, 30.0f,
                                            false);
            // set playing to off
            softcut_->TogglePlay(*selectedLoop, false);
            // set position to 0
            softcut_->handleCommand(new Commands::CommandPacket(
                Commands::Id::SET_CUT_POSITION, *selectedLoop,
                softcut_->getLoopStart(*selectedLoop)));

            // clear the buffer
            softcut_->clearBuffer(*selectedLoop < 4 ? 0 : 1,
                                  softcut_->getLoopStart(*selectedLoop), 60.0f);
          }

        } else {
          softcut_->ToggleRecord(*selectedLoop);
        }
      }
      break;
    case SDLK_s:
      if (!isRepeat) {
        if (keysHeld_[SDLK_LCTRL] || keysHeld_[SDLK_RCTRL]) {
          // save every buffer
          for (int i = 0; i < numVoices_; i++) {
            softcut_->dumpBufferFromLoop(i);
          }
          display_->SetMessage("Audio saved to oooooooo folder", 3);
        } else {
          // save the parameters
          JSON json;
          for (int i = 0; i < numVoices_; i++) {
            json["loop" + std::to_string(i)] = params_[i].toJSON();
          }
          std::cerr << "Saving parameters to file" << std::endl;
          // create folder oooooooo if it doesn't exist
          std::string folder = "oooooooo";
          if (!std::filesystem::exists(folder)) {
            std::filesystem::create_directory(folder);
          }

          // write it to a file
          std::ofstream file("oooooooo/parameters.json");
          if (file.is_open()) {
            file << json.dump(4);
            file.close();
            std::cerr << "Parameters saved to parameters.json" << std::endl;
            display_->SetMessage("Parameters saved to oooooooo folder", 3);
          } else {
            std::cerr << "Error opening file for writing" << std::endl;
          }
        }
      }
      break;
    case SDLK_o:
      if (!isRepeat) {
        if (keysHeld_[SDLK_LCTRL] || keysHeld_[SDLK_RCTRL]) {
          // load every loop audio folder
          for (int i = 0; i < numVoices_; i++) {
            std::string path = "oooooooo/loop_" + std::to_string(i) + ".wav";
            softcut_->loadBufferToLoop(path, i);
          }

          std::filesystem::path filePath("oooooooo/loop_0.wav");
          if (std::filesystem::exists(filePath)) {
            auto ftime = std::filesystem::last_write_time(filePath);
            // Convert file_time_type to system_clock::time_point
            auto sctp = std::chrono::time_point_cast<
                std::chrono::system_clock::duration>(
                ftime - decltype(ftime)::clock::now() +
                std::chrono::system_clock::now());
            // Convert to time_t
            std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
            // Format time
            char timeStr[100];
            std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S",
                          std::localtime(&cftime));

            display_->SetMessage(
                "Audio loaded from "
                "modified: " +
                    std::string(timeStr),
                3);
          }
        } else {
          // load the parameters
          JSON json;
          std::ifstream file("oooooooo/parameters.json");
          // get the last-modified date from the file
          if (file.is_open()) {
            file >> json;
            file.close();
            std::cerr << "Parameters loaded from parameters.json" << std::endl;
            for (int i = 0; i < numVoices_; i++) {
              params_[i].fromJSON(json["loop" + std::to_string(i)]);
            }
            // bang parameters
            for (int i = 0; i < numVoices_; i++) {
              params_[i].Bang();
            }

            std::filesystem::path filePath("oooooooo/parameters.json");
            auto ftime = std::filesystem::last_write_time(filePath);
            // Convert file_time_type to system_clock::time_point
            auto sctp = std::chrono::time_point_cast<
                std::chrono::system_clock::duration>(
                ftime - decltype(ftime)::clock::now() +
                std::chrono::system_clock::now());
            // Convert to time_t
            std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
            // Format time
            char timeStr[100];
            std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S",
                          std::localtime(&cftime));

            display_->SetMessage(
                "Parameters loaded from "
                "modified: " +
                    std::string(timeStr),
                3);
          } else {
            std::cerr << "Error opening file for reading" << std::endl;
          }
        }
      }
      break;
    case SDLK_l:
      if (!isRepeat) {
        if (keysHeld_[SDLK_LCTRL] || keysHeld_[SDLK_RCTRL]) {
          // load every loop audio folder
          for (int i = 0; i < numVoices_; i++) {
            params_[i].ToggleLFO();
          }
        } else {
          // toggle LFO
          params_[*selectedLoop].ToggleLFO();
        }
      }
      break;
    case SDLK_TAB:
      for (int i = 0; i < numVoices_; i++) {
        params_[i].ToggleView();
      }
      break;
    case SDLK_UP:
      params_[0].SelectedDelta(isRepeat ? -2 : -1);
      break;
    case SDLK_DOWN:
      params_[0].SelectedDelta(isRepeat ? 2 : 1);
      break;
    case SDLK_LEFT:
      if (keysHeld_[SDLK_LALT] || keysHeld_[SDLK_RALT]) {
        params_[*selectedLoop].LFODelta(0, isRepeat ? -2.0f : -1.0f);
      } else if (keysHeld_[SDLK_LCTRL] || keysHeld_[SDLK_RCTRL]) {
        params_[*selectedLoop].LFODelta(isRepeat ? 2.0f : 1.0f, 0);
      } else {
        params_[*selectedLoop].ValueDelta(isRepeat ? -2.0f : -1.0f);
      }
      break;
    case SDLK_RIGHT:
      if (keysHeld_[SDLK_LALT] || keysHeld_[SDLK_RALT]) {
        params_[*selectedLoop].LFODelta(0, isRepeat ? 2.0f : 1.0f);
      } else if (keysHeld_[SDLK_LCTRL] || keysHeld_[SDLK_RCTRL]) {
        params_[*selectedLoop].LFODelta(isRepeat ? -2.0f : -1.0f, 0);
      } else {
        params_[*selectedLoop].ValueDelta(isRepeat ? 2.0f : 1.0f);
      }
      break;
    case SDLK_1:
    case SDLK_KP_1:
      *selectedLoop = 0;
      break;
    case SDLK_2:
    case SDLK_KP_2:
      *selectedLoop = 1;
      break;
    case SDLK_3:
    case SDLK_KP_3:
      *selectedLoop = 2;
      break;
    case SDLK_4:
    case SDLK_KP_4:
      *selectedLoop = 3;
      break;
    case SDLK_5:
    case SDLK_KP_5:
      *selectedLoop = 4;
      break;
    case SDLK_6:
    case SDLK_KP_6:
      *selectedLoop = 5;
      break;
    case SDLK_7:
    case SDLK_KP_7:
      *selectedLoop = 6;
      break;
    case SDLK_8:
    case SDLK_KP_8:
      *selectedLoop = 7;
      break;
      // TODO, select all loops?
    // case SDLK_9:
    // case SDLK_KP_9:
    //   *selectedLoop = 8;
    //   break;
    default:
      break;
  }
}

void KeyboardHandler::handleKeyUp(SDL_Keycode key,
                                  int selectedLoop [[maybe_unused]]) {
  keysHeld_[key] = false;
  // std::cerr << "KeyboardHandler::handleKeyUp Key released: "
  //           << SDL_GetKeyName(key) << " " << selectedLoop << std::endl;
}
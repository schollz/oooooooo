#include "Display.h"

#include <SDL2/SDL.h>

#include <iostream>

#include "AudioFile.h"
#include "DisplayFont.h"
#include "DisplayRing.h"
#include "DrawFunctions.h"
#include "KeyboardHandler.h"
#include "Parameters.h"
#include "SoftcutClient.h"

using namespace softcut_jack_osc;
Display::Display(int width, int height)
    : width_(width),
      height_(height),
      window_(nullptr),
      renderer_(nullptr),
      running_(false),
      quitCallback_(nullptr) {
  if (TTF_Init() == -1) {
    std::cerr << "TTF_Init: " << TTF_GetError() << std::endl;
  } else {
    SDL_RWops* rw = SDL_RWFromConstMem(DisplayFont_ttf, DisplayFont_ttf_len);
    if (rw) {
      font =
          TTF_OpenFontRW(rw, 1, 16);  // 1 = auto-free rw after TTF_OpenFontRW
      if (!font) {
        std::cerr << "TTF_OpenFontRW: " << TTF_GetError() << std::endl;
      }
    } else {
      std::cerr << "SDL_RWFromConstMem: " << SDL_GetError() << std::endl;
    }

    if (!font) {
      std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
    } else if (TTF_GetFontStyle(font) != TTF_STYLE_NORMAL) {
      std::cerr << "Font style is not normal: " << TTF_GetError() << std::endl;
    } else if (TTF_GetFontKerning(font) != 0) {
      std::cerr << "Font kerning is not zero: " << TTF_GetError() << std::endl;
    }
  }
}

Display::~Display() {
  // Make sure we're properly stopped

  stop();
}

void Display::start() {
  if (running_) return;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError()
              << std::endl;
    return;
  }

  width_ = 1080;
  height_ = 720;
  window_ = SDL_CreateWindow("oooooooo", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, width_, height_,
                             SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

  if (!window_) {
    std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    SDL_Quit();
    return;
  }

  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer_) {
    std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    SDL_DestroyWindow(window_);
    SDL_Quit();
    return;
  }

  running_ = true;

  std::cout << "SDL Display started successfully" << std::endl;

  // 👇 RUN DIRECTLY IN MAIN THREAD
  SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
  renderLoop();
}

void Display::stop() {
  // First, set the running flag to false to stop the loop
  running_ = false;

  // Then wait for the thread to finish
  if (displayThread_ && displayThread_->joinable()) {
    try {
      displayThread_->join();
    } catch (const std::exception& e) {
      std::cerr << "Error joining display thread: " << e.what() << std::endl;
    }
    displayThread_.reset();
  }

  // Clean up SDL resources (move this after thread join)
  if (renderer_) {
    SDL_DestroyRenderer(renderer_);
    renderer_ = nullptr;
  }

  if (window_) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }

  // Quit SDL only after all resources are cleaned up
  SDL_Quit();

  std::cout << "SDL Display stopped" << std::endl;
}
void Display::renderLoop() {
  std::cout << "Render loop started" << std::endl;

  // Run the loop until running_ becomes false
  while (running_) {
    // Process SDL events in a safer way
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      // Handle window close events
      if (e.type == SDL_QUIT) {
        std::cout << "Window close event received" << std::endl;

        // Call the quit callback before setting running_ to false
        if (quitCallback_) {
          quitCallback_();
        }

        // Set running flag to false
        running_ = false;
        break;
      } else if (e.type == SDL_KEYDOWN) {
        // Handle key down events
        keyboardHandler_.handleKeyDown(
            e.key.keysym.sym, e.key.repeat,
            static_cast<SDL_Keymod>(e.key.keysym.mod), &selected_loop);

        if (e.key.keysym.sym == SDLK_h && !e.key.repeat) {
          helpSystem_.Toggle();
        }
      } else if (e.type == SDL_KEYUP) {
        // Handle key up events
        keyboardHandler_.handleKeyUp(e.key.keysym.sym, selected_loop);
      } else if (e.type == SDL_DROPFILE) {
        // Handle file drop events
        std::cout << "File dropped: " << e.drop.file << std::endl;
        // the number of channels, and sample rate of the file using libsndfile
        AudioFile audioFile(e.drop.file);
        if (!audioFile.isValid()) {
          std::cerr << "Error loading file: " << audioFile.getError()
                    << std::endl;
        } else {
          float totalSeconds = static_cast<float>(audioFile.getFrameCount()) /
                               audioFile.getSampleRate();
          std::cout << "Sample rate: " << audioFile.getSampleRate()
                    << ", Channels: " << audioFile.getChannelCount()
                    << ", Frames: " << audioFile.getFrameCount()
                    << ", Seconds: " << totalSeconds << std::endl;

          // fix the sample rate mismatch
          float baseRate =
              audioFile.getSampleRate() / softCutClient_->getSampleRate();
          softCutClient_->setBaseRate(selected_loop, baseRate);

          // load in the file
          int bufNum = selected_loop < 4 ? 0 : 1;
          float startTimeDest = softCutClient_->getLoopStart(selected_loop);
          softCutClient_->readBufferMono(e.drop.file, 0.f, startTimeDest, -1.f,
                                         0, bufNum);
          // read 1 second of audio extra into the postroll
          softCutClient_->readBufferMono(
              e.drop.file, 0.f, startTimeDest + totalSeconds, 1.f, 0, bufNum);

          // total time in seconds
          // set the loop end to the total time
          params_[selected_loop].ValueSet(Parameters::PARAM_START,
                                          startTimeDest, false);
          params_[selected_loop].SetMax(Parameters::PARAM_DURATION,
                                        totalSeconds);
          params_[selected_loop].ValueSet(Parameters::PARAM_DURATION,
                                          totalSeconds, false);

          // cut to the start
          softCutClient_->handleCommand(new Commands::CommandPacket(
              Commands::Id::SET_CUT_POSITION, selected_loop, startTimeDest));
        }

        SDL_free(e.drop.file);  // Free the dropped file string
      } else if (e.type == SDL_WINDOWEVENT) {
        // Handle window resize events
        if (e.window.event == SDL_WINDOWEVENT_RESIZED ||
            e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          // Update width and height
          width_ = e.window.data1;
          height_ = e.window.data2;
          std::cout << "Window resized to " << width_ << "x" << height_
                    << std::endl;
        }
      } else if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {
          // Reset drag flags
          mouse_dragging = false;
          dragging_bar = false;
          dragged_parameter = -1;

          if (params_[selected_loop].RegisterClick(e.button.x, e.button.y,
                                                   dragging_bar)) {
            // Check if any parameter was clicked
            dragging_bar = true;
          } else {
            // check if any ring was clicked
            std::vector<int> clicked_rings;

            for (int i = 0; i < numVoices_; i++) {
              displayRings_[i].RegisterClick(e.button.x, e.button.y);
              if (displayRings_[i].ClickedRing() && clicked_rings.empty()) {
                std::cout << "Clicked ring " << i << std::endl;
                selected_loop = i;
                mouse_dragging = true;
                break;
              } else if (displayRings_[i].ClickedRadius()) {
                clicked_rings.push_back(i);
              }
            }

            if (clicked_rings.size() > 1) {
              // If multiple rings were clicked, select the one closest to the
              // mouse
              float min_distance = std::numeric_limits<float>::max();
              for (int i : clicked_rings) {
                float distance = displayRings_[i].GetDistanceToCenter();
                if (distance < min_distance) {
                  min_distance = distance;
                  selected_loop = i;
                }
              }
            }
          }
        }
      } else if (e.type == SDL_MOUSEMOTION) {
        // Handle dragging
        if (mouse_dragging && selected_loop >= 0 &&
            selected_loop < numVoices_) {
          displayRings_[selected_loop].HandleDrag(e.button.x, e.button.y,
                                                  width_, height_);
        } else if (dragging_bar) {
          params_[selected_loop].RegisterClick(e.button.x, e.button.y,
                                               dragging_bar);
        }
      } else if (e.type == SDL_MOUSEBUTTONUP) {
        // Stop dragging when mouse is released
        if (e.button.button == SDL_BUTTON_LEFT) {
          mouse_dragging = false;
          dragging_bar = false;
          if (selected_loop >= 0 && selected_loop < numVoices_) {
            displayRings_[selected_loop].StopDrag();
          }
        }
      }
    }

    // If we're no longer running, exit the loop
    if (!running_) {
      break;
    }

    // Make sure our resources are still valid
    if (!renderer_ || !window_) {
      std::cerr << "Renderer or window became invalid" << std::endl;
      running_ = false;
      break;
    }

    // Clear screen
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);  // Black background
    SDL_RenderClear(renderer_);

    // Update all the parameters
    if (softCutClient_) {
      for (int i = 0; i < softCutClient_->getNumVoices(); i++) {
        params_[i].Update();
      }
    }

    // Update all the display rings
    if (softCutClient_) {
      for (int i = 0; i < softCutClient_->getNumVoices(); i++) {
        displayRings_[i].Update(width_, height_);
      }
    }

    // Draw the display rings, highlight the selected one
    if (softCutClient_) {
      // process unselected rings first
      for (int i = 0; i < softCutClient_->getNumVoices(); i++) {
        if (i == selected_loop) {
          continue;  // Skip the selected ring
        }
        if (softCutClient_->IsRecording(i)) {
          SDL_SetRenderDrawColor(renderer_, 176, 97, 97, 0);
        } else {
          SDL_SetRenderDrawColor(renderer_, 120, 120, 120, 0);
        }
        displayRings_[i].Render(renderer_, &perlinGenerator, &noiseTimeValue);
      }
      if (softCutClient_->IsRecording(selected_loop)) {
        SDL_SetRenderDrawColor(renderer_, 176, 50, 2, 0);
      } else {
        SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 0);
      }
      displayRings_[selected_loop].Render(renderer_, &perlinGenerator,
                                          &noiseTimeValue);
    }

    // write ooooo at the top left
    // draw each "o" separately
    for (int i = 0; i < numVoices_; i++) {
      drawText(renderer_, font, "o", 10 + i * 11, 10,
               selected_loop == i ? 255 : 120);
    }

    // render each parameter
    params_[selected_loop].Render(renderer_, font, 10, 35, 85, 20);

    // render the help system
    helpSystem_.Render(renderer_, width_);

    // Update screen
    SDL_RenderPresent(renderer_);

    // Cap at ~60 FPS
    SDL_Delay(16);
    noiseTimeValue += 0.01f;
  }

  std::cout << "Render loop ended" << std::endl;
}

void Display::init(SoftcutClient* sc, int numVoices) {
  softCutClient_ = sc;
  numVoices_ = numVoices;

  // setup parameters
  params_ = new Parameters[numVoices_];
  for (int v = 0; v < numVoices_; v++) {
    params_[v].Init(softCutClient_, v, 1000.0f / 16.0f);
  }

  // setup display rings
  displayRings_ = new DisplayRing[numVoices_];
  for (int i = 0; i < numVoices_; i++) {
    displayRings_[i].Init(softCutClient_, &params_[i], i);
    displayRings_[i].Update(width_, height_);
  }

  // setup keyboard handler
  keyboardHandler_.Init(softCutClient_, params_, numVoices_);

  // setup help system
  helpSystem_.Init(font);
}
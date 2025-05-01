#include "Display.h"

#include <iostream>

#include "AudioFile.h"
#include "Commands.h"
#include "DisplayFont.h"
#include "DisplayRing.h"
#include "DrawFunctions.h"
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
          float endTime = startTimeDest + totalSeconds;
          softCutClient_->handleCommand(new Commands::CommandPacket(
              Commands::Id::SET_CUT_LOOP_END, selected_loop, endTime));
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

          for (int i = 0; i < numVoices_; i++) {
            displayRings_[i].RegisterClick(e.button.x, e.button.y);
            if (displayRings_[i].ClickedRing()) {
              std::cout << "Clicked ring " << i << std::endl;
              selected_loop = i;
              mouse_dragging = true;
              break;
            } else if (displayRings_[i].ClickedRadius()) {
              std::cout << "Clicked radius " << i << std::endl;
              selected_loop = i;
              break;
            }
          }
        }
      } else if (e.type == SDL_MOUSEMOTION) {
        // Handle dragging
        if (mouse_dragging && selected_loop >= 0 &&
            selected_loop < numVoices_) {
          float new_level = 0.0f;
          float new_pan = 0.0f;
          displayRings_[selected_loop].HandleDrag(
              e.button.x, e.button.y, width_, height_, &new_level, &new_pan);
          std::cout << "Dragging loop " << selected_loop
                    << " new level: " << new_level << " new pan: " << new_pan
                    << std::endl;
          params_[selected_loop].param_[Parameters::PARAM_LEVEL].ValueSet(
              new_level, false);
          params_[selected_loop].param_[Parameters::PARAM_PAN].ValueSet(new_pan,
                                                                        false);
        }
      } else if (e.type == SDL_MOUSEBUTTONUP) {
        // Stop dragging when mouse is released
        if (e.button.button == SDL_BUTTON_LEFT) {
          mouse_dragging = false;
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

    // Update all the display rings
    if (softCutClient_) {
      for (int i = 0; i < numVoices_; i++) {
        displayRings_[i].Update(softCutClient_, i, width_, height_);
      }
    }

    // Draw the display rings, highlight the selected one
    if (softCutClient_) {
      // process unselected rings first
      for (int i = 0; i < softCutClient_->getNumVoices(); i++) {
        if (i == selected_loop) {
          continue;  // Skip the selected ring
        }
        SDL_SetRenderDrawColor(renderer_, 120, 120, 120, 0);
        displayRings_[i].Render(renderer_, &perlinGenerator, &noiseTimeValue);
      }
      SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 0);
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
    // for (int i = 0; i < param_count_; i++) {
    //   param_[selected_loop][i].Render(renderer_, font, 10, 50 + i * 30, 50,
    //   20,
    //                                   selected_parameter_ == i);
    // }

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
  displayRings_ = new DisplayRing[numVoices_];

  // setup parameters
  params_ = new Parameters[numVoices_];
  for (int v = 0; v < numVoices_; v++) {
    params_[v].Init(softCutClient_, v);
  }
}
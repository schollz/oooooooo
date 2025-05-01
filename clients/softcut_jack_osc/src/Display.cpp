#include "Display.h"

#include <iostream>

#include "DisplayFont.h"
#include "DrawFunctions.h"
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

    // Print out the position of the first voice
    if (softCutClient_) {
      for (int i = 0; i < softCutClient_->getNumVoices(); i++) {
        float pos = softCutClient_->getSavedPosition(i);
        float dur = softCutClient_->getDuration(i);
        float level = softCutClient_->getOutLevel(i);
        float pan = softCutClient_->getPan(i);
        float id = i;
        float x = linlin(pan, -1, 1, 0, width_);
        float y = linlin(amp2db(level), -64, 24, height_, 0);
        float radius = linlin(dur, 0, 1, 0, 20);
        float position = pos / dur;
        float thickness = 2.5f;
        bool show_notch = true;
        bool sketchy = true;
        // change color to white
        // if (i == 0) {
        //   std::cout << "Voice " << i << ": " << pos << " / " << dur
        //             << " (level: " << level << ", pan: " << pan << ")"
        //             << std::endl;
        // }
        SDL_SetRenderDrawColor(renderer_, 100, 100, 100, 0);
        drawRing(renderer_, &perlinGenerator, id, x, y, radius, position,
                 thickness, &noiseTimeValue, show_notch, sketchy);
      }
    }

    // Update screen
    SDL_RenderPresent(renderer_);

    // Cap at ~60 FPS
    SDL_Delay(16);
    noiseTimeValue += 0.01f;
  }

  std::cout << "Render loop ended" << std::endl;
}

void Display::init(SoftcutClient* sc) { softCutClient_ = sc; }
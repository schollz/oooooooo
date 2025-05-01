#include "Display.h"

#include <iostream>

#include "SoftcutClient.h"

using namespace softcut_jack_osc;
Display::Display(int width, int height)
    : width_(width),
      height_(height),
      window_(nullptr),
      renderer_(nullptr),
      running_(false),
      quitCallback_(nullptr) {}

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
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);

    // Draw a white rectangle
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    SDL_Rect rect = {width_ / 4, height_ / 4, width_ / 2, height_ / 2};
    SDL_RenderFillRect(renderer_, &rect);

    // Update screen
    SDL_RenderPresent(renderer_);

    // Print out the position of the first voice
    if (softCutClient_) {
      std::cout << "Voice 0 position: " << softCutClient_->getSavedPosition(0)
                << " dur " << softCutClient_->getDuration(0) << std::endl;
    }

    // Cap at ~60 FPS
    SDL_Delay(16);
  }

  std::cout << "Render loop ended" << std::endl;
}

void Display::init(SoftcutClient* sc) { softCutClient_ = sc; }
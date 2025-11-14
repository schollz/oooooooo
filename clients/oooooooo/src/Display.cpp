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

// Set hint for high DPI support before creating the window
#ifdef __APPLE__
  // Enable high DPI support on macOS
  SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
#endif

  width_ = 1080;
  height_ = 815;
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

  // Initialize zoom factor based on DPI
  int drawableWidth, drawableHeight;
  SDL_GetRendererOutputSize(renderer_, &drawableWidth, &drawableHeight);

  int windowWidth, windowHeight;
  SDL_GetWindowSize(window_, &windowWidth, &windowHeight);

  // On macOS, automatically set initial zoom based on DPI scaling
  SDL_RenderSetLogicalSize(renderer_, static_cast<int>(width_),
                           static_cast<int>(height_));

  running_ = true;

  std::cout << "SDL Display started successfully" << std::endl;

  // 👇 RUN DIRECTLY IN MAIN THREAD
  SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
  SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
  SDL_EventState(SDL_MOUSEBUTTONUP, SDL_ENABLE);
  SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
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
}

void Display::renderLoop() {
  std::cout << "Render loop started" << std::endl;

  // Calculate DPI scaling factors
  float scaleX = 1.0f;
  float scaleY = 1.0f;

  // Get the drawable size for high DPI scaling
  int drawableWidth, drawableHeight;
  SDL_GetRendererOutputSize(renderer_, &drawableWidth, &drawableHeight);

  // Get the window size (might be different from drawable size on high DPI
  // displays)
  int windowWidth, windowHeight;
  SDL_GetWindowSize(window_, &windowWidth, &windowHeight);

  // Calculate scaling factors if there's a difference
  if (windowWidth > 0 && windowHeight > 0) {
    scaleX = static_cast<float>(drawableWidth) / windowWidth;
    scaleY = static_cast<float>(drawableHeight) / windowHeight;

    std::cout << "DPI Scaling: " << scaleX << "x" << scaleY << std::endl;
  }

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
        if (!introAnimation_.isComplete()) {
          introAnimation_.Stop();
          mainContentFadeAlpha_ = 0.0f;  // Reset fade alpha to start fading in
        }
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
          params_[selected_loop].ValueSet(Parameters::PARAM_BASE_RATE, baseRate,
                                          false);

          // load in the file
          int bufNum = selected_loop < 4 ? 0 : 1;
          float startTimeDest = softCutClient_->getLoopStart(selected_loop);
          softCutClient_->readBufferMono(e.drop.file, 0.f, startTimeDest, -1.f,
                                         0, bufNum);
          // read 1 second of audio extra into the postroll
          softCutClient_->readBufferMono(
              e.drop.file, 0.f, startTimeDest + totalSeconds * baseRate, 1.f, 0,
              bufNum);

          // total time in seconds
          // set the loop end to the total time
          params_[selected_loop].SetMax(Parameters::PARAM_START, totalSeconds);
          params_[selected_loop].SetMax(Parameters::PARAM_DURATION,
                                        totalSeconds * baseRate);
          params_[selected_loop].ValueSet(Parameters::PARAM_START, 0, false);
          params_[selected_loop].ValueSet(Parameters::PARAM_DURATION,
                                          totalSeconds * baseRate, false);

          // cut to the start
          softCutClient_->handleCommand(new Commands::CommandPacket(
              Commands::Id::SET_CUT_POSITION, selected_loop, startTimeDest));

          std::string fileName(e.drop.file);
          size_t lastSlash = fileName.find_last_of("/\\");
          if (lastSlash != std::string::npos) {
            fileName = fileName.substr(lastSlash + 1);
          }
          std::string message = "loaded " + fileName + " into loop " +
                                std::to_string(selected_loop + 1);
          SetMessage(message, 2);
        }

        SDL_free(e.drop.file);  // Free the dropped file string
      } else if (e.type == SDL_WINDOWEVENT) {
        // Handle window resize events
        if (e.window.event == SDL_WINDOWEVENT_RESIZED ||
            e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          // Update drawable size
          SDL_GetRendererOutputSize(renderer_, &drawableWidth, &drawableHeight);

          // Update window size
          SDL_GetWindowSize(window_, &windowWidth, &windowHeight);

          // Recalculate scaling factors
          if (windowWidth > 0 && windowHeight > 0) {
            scaleX = static_cast<float>(drawableWidth) / windowWidth;
            scaleY = static_cast<float>(drawableHeight) / windowHeight;

            std::cout << "Window resized. New DPI Scaling: " << scaleX << "x"
                      << scaleY << std::endl;
          }

          // Update width and height
          width_ = drawableWidth;
          height_ = drawableHeight;
        }
      } else if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (!introAnimation_.isComplete()) {
          introAnimation_.Stop();
          mainContentFadeAlpha_ = 0.0f;  // Reset fade alpha to start fading in
        }
        if (e.button.button == SDL_BUTTON_LEFT) {
          // Scale the mouse coordinates for high DPI displays
          int scaledX = static_cast<int>(e.button.x);
          int scaledY = static_cast<int>(e.button.y);
          /*
          std::cout << "Mouse clicked at: " << scaledX << ", " << scaledY
                    << std::endl;
          std::cout << "Mouse clicked at: " << e.button.x << ", " << e.button.y
                    << std::endl;
          */

          // Reset drag flags
          mouse_dragging = false;
          dragging_bar = false;
          dragged_parameter = -1;

          bool clickedOnO = false;
          for (int i = 0; i < numVoices_; i++) {
            // Each "o" is at position (10 + i * 11, 10)
            // Estimate the character is about 8x16 pixels
            int oX = 10 + i * 11;
            int oY = 10;
            int oWidth = 8;
            int oHeight = 16;

            if (scaledX >= oX && scaledX <= oX + oWidth && scaledY >= oY &&
                scaledY <= oY + oHeight) {
              selected_loop = i;
              clickedOnO = true;
              break;
            }
          }
          if (!clickedOnO) {
            // If not clicked on "o", check if any parameter was clicked

            if (params_[selected_loop].RegisterClick(scaledX, scaledY,
                                                     dragging_bar)) {
              // Check if any parameter was clicked
              dragging_bar = true;
            } else {
              // check if any ring was clicked
              std::vector<int> clicked_rings;

              for (int i = 0; i < numVoices_; i++) {
                displayRings_[i].RegisterClick(scaledX, scaledY);
                if (displayRings_[i].ClickedRing()) {
                  // std::cout << "Clicked ring " << i << std::endl;
                  clicked_rings.push_back(i);
                }
              }

              if (clicked_rings.size() > 0) {
                // If multiple rings were clicked, select the one closest to the
                // mouse
                float min_distance = std::numeric_limits<float>::max();
                for (int i : clicked_rings) {
                  float distance = displayRings_[i].GetDistanceToCenter();
                  if (distance < min_distance) {
                    min_distance = distance;
                    selected_loop = i;
                    mouse_dragging = true;
                  }
                }
              } else {
                // check to see if radii were clicked
                for (int i = 0; i < numVoices_; i++) {
                  if (displayRings_[i].ClickedRadius()) {
                    selected_loop = i;
                  }
                }
              }
            }
          }
        }
      } else if (e.type == SDL_MOUSEMOTION) {
        // Scale the mouse coordinates for high DPI displays
        int scaledX = static_cast<int>(e.motion.x);
        int scaledY = static_cast<int>(e.motion.y);

        // Handle dragging
        if (mouse_dragging && selected_loop >= 0 &&
            selected_loop < numVoices_) {
          displayRings_[selected_loop].HandleDrag(scaledX, scaledY, width_,
                                                  height_);
        } else if (dragging_bar) {
          params_[selected_loop].RegisterClick(scaledX, scaledY, dragging_bar);
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
    SDL_RenderSetLogicalSize(renderer_, width_, height_);
    SDL_RenderClear(renderer_);
    introAnimation_.Update();
    if (!introAnimation_.isComplete()) {
      // Render only intro animation
      introAnimation_.Render(renderer_, width_, height_);
    } else {
      // Fade in main content after intro completes
      if (mainContentFadeAlpha_ < 1.0f) {
        mainContentFadeAlpha_ += 0.02f;  // Adjust fade speed as needed
        if (mainContentFadeAlpha_ >= 1.0f) {
          mainContentFadeAlpha_ = 1.0f;
        }
      }

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

      // Draw the display rings with fade effect
      if (softCutClient_) {
        // Set up SDL blending mode for the fade effect
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

        // Process unselected rings first
        for (int i = 0; i < softCutClient_->getNumVoices(); i++) {
          if (i == selected_loop) {
            continue;  // Skip the selected ring
          }

          displayRings_[i].Render(renderer_, &perlinGenerator, &noiseTimeValue,
                                  mainContentFadeAlpha_, false);
        }

        displayRings_[selected_loop].Render(renderer_, &perlinGenerator,
                                            &noiseTimeValue,
                                            mainContentFadeAlpha_, true);
      }

      // Draw "oooooooo" text with fade
      for (int i = 0; i < numVoices_; i++) {
        Uint8 textAlpha = static_cast<Uint8>((selected_loop == i ? 255 : 120) *
                                             mainContentFadeAlpha_);
        drawText(renderer_, font, "o", 10 + i * 11, 10, textAlpha);
      }

      // Render parameters with fade
      SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
      params_[selected_loop].Render(renderer_, font, 10, 35, 85, 20,
                                    mainContentFadeAlpha_);

      // Render help system
      helpSystem_.Render(renderer_, width_);

      // Render VU Meter
      float vuLevel = -100.0f;  // Default to minimum level
      if (softCutClient_) {
        vuLevel = softCutClient_->getVULevel(selected_loop);
      }

      // Define meter dimensions
      float vuMeter_height = 20;
      float vuMeter_width = 100;
      float vuMeter_x = width_ - vuMeter_width - 10;
      float vuMeter_y = height_ - vuMeter_height - 10;

      // Draw background for the VU meter
      SDL_SetRenderDrawColor(renderer_, 50, 50, 50, 255);
      SDL_Rect vuMeterRect = {
          static_cast<int>(vuMeter_x), static_cast<int>(vuMeter_y),
          static_cast<int>(vuMeter_width), static_cast<int>(vuMeter_height)};
      SDL_RenderFillRect(renderer_, &vuMeterRect);

      // Convert dB level to visual meter value (normalize from -60dB to 0dB)
      float normalizedLevel =
          (vuLevel + 60.0f) / 60.0f;  // Map -60dB -> 0dB to 0 -> 1
      normalizedLevel = std::max(
          0.0f, std::min(normalizedLevel, 1.0f));  // Clamp between 0 and 1

      // Draw the filled area for the VU meter
      SDL_SetRenderDrawColor(renderer_, 200, 200, 200, 255);  // Off-white color
      SDL_Rect vuFillRect = {static_cast<int>(vuMeter_x),
                             static_cast<int>(vuMeter_y),
                             static_cast<int>(vuMeter_width * normalizedLevel),
                             static_cast<int>(vuMeter_height)};
      SDL_RenderFillRect(renderer_, &vuFillRect);

      // Draw the outline for the VU meter (white)
      SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
      SDL_RenderDrawRect(renderer_, &vuMeterRect);

      // Draw the text for the VU meter
      char vuText[32];
      snprintf(vuText, sizeof(vuText), "%.0f dB", roundf(vuLevel));

      int textWidth, textHeight;
      TTF_SizeText(font, vuText, &textWidth, &textHeight);
      SDL_Surface* textSurface = TTF_RenderText_Solid(
          font, vuText, {255, 255, 255, 255});  // White text
      SDL_Texture* textTexture =
          SDL_CreateTextureFromSurface(renderer_, textSurface);
      SDL_Rect textRect = {
          static_cast<int>(vuMeter_x + vuMeter_width / 2 - textWidth / 2),
          static_cast<int>(vuMeter_y + vuMeter_height / 2 - textHeight / 2),
          textWidth, textHeight};
      SDL_RenderCopy(renderer_, textTexture, nullptr, &textRect);
      SDL_DestroyTexture(textTexture);
      SDL_FreeSurface(textSurface);

      displayMessage_.Update();
      displayMessage_.Render(renderer_, width_, height_);
    }

    // show the CPU usage in the top right corner
    float cpuUsage = 0.0f;
    if (softCutClient_) {
      cpuUsage = softCutClient_->getCPUUsage();
    }
    std::string cpuText = sprintf_str("CPU: %.0f%%", roundf(cpuUsage));
    int cpuTextWidth, cpuTextHeight;
    TTF_SizeText(font, cpuText.c_str(), &cpuTextWidth, &cpuTextHeight);
    SDL_Surface* cpuTextSurface = TTF_RenderText_Solid(
        font, cpuText.c_str(), {255, 255, 255, 255});  // White text
    SDL_Texture* cpuTextTexture =
        SDL_CreateTextureFromSurface(renderer_, cpuTextSurface);
    SDL_Rect cpuTextRect = {width_ - cpuTextWidth - 10, 10, cpuTextWidth,
                            cpuTextHeight};
    SDL_RenderCopy(renderer_, cpuTextTexture, nullptr, &cpuTextRect);
    SDL_DestroyTexture(cpuTextTexture);
    SDL_FreeSurface(cpuTextSurface);

    // show the help text in the bottom left corner
    std::string helpText = "Tab: toggle menu, h: toggle help";
    int helpTextWidth, helpTextHeight;
    TTF_SizeText(font, helpText.c_str(), &helpTextWidth, &helpTextHeight);
    SDL_Surface* helpTextSurface = TTF_RenderText_Solid(
        font, helpText.c_str(), {40, 40, 40, 255});  // Dark gray text
    SDL_Texture* helpTextTexture =
        SDL_CreateTextureFromSurface(renderer_, helpTextSurface);
    SDL_Rect helpTextRect = {10, height_ - helpTextHeight - 10, helpTextWidth,
                             helpTextHeight};
    SDL_RenderCopy(renderer_, helpTextTexture, nullptr, &helpTextRect);
    SDL_DestroyTexture(helpTextTexture);
    SDL_FreeSurface(helpTextSurface);

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

  // After initializing all parameters, share global parameters
  // For each voice after the first one, share the reverb decay parameter
  for (int v = 1; v < numVoices_; v++) {
    // Share reverb decay parameter from voice 0 to all other voices
    params_[v].param_[Parameters::PARAM_REVERB_DECAY].ShareFrom(
        &params_[0].param_[Parameters::PARAM_REVERB_DECAY]);

    // Share reverb density parameter from voice 0 to all other voices
    params_[v].param_[Parameters::PARAM_REVERB_DENSITY].ShareFrom(
        &params_[0].param_[Parameters::PARAM_REVERB_DENSITY]);

    // share prime sensitivity parameter from voice 0 to all other voices
    params_[v].param_[Parameters::PARAM_PRIME_SENSITIVITY].ShareFrom(
        &params_[0].param_[Parameters::PARAM_PRIME_SENSITIVITY]);

    // share prime quantize parameter from voice 0 to all other voices
    params_[v].param_[Parameters::PARAM_PRIME_QUANTIZE].ShareFrom(
        &params_[0].param_[Parameters::PARAM_PRIME_QUANTIZE]);
  }

  // setup display rings
  displayRings_ = new DisplayRing[numVoices_];
  for (int i = 0; i < numVoices_; i++) {
    displayRings_[i].Init(softCutClient_, &params_[i], i);
    displayRings_[i].Update(width_, height_);
  }

  // setup keyboard handler
  keyboardHandler_.Init(softCutClient_, params_, numVoices_, this);

  // setup help system
  helpSystem_.Init(font);

  // setup intro animation
  introAnimation_.Init(font);
  introAnimation_.Start();

  // setup display message
  displayMessage_.Init(font);

  // set input level to 1.0 for all voices for channel 0
  for (int i = 0; i < numVoices_; i++) {
    softCutClient_->handleCommand(new Commands::CommandPacket(
        Commands::Id::SET_LEVEL_IN_CUT, 0, i, 1.0f));
  }
}

void Display::SetMessage(const std::string& message, int secondsToDisplay) {
  displayMessage_.SetMessage(message, secondsToDisplay);
}

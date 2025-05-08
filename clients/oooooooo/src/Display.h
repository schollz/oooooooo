#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL2/SDL.h>
#include <lo/lo.h>

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "DisplayMessage.h"
#include "DisplayRing.h"
#include "DrawFunctions.h"
#include "HelpSystem.h"
#include "IntroAnimation.h"
#include "KeyboardHandler.h"
#include "Parameters.h"
#include "Perlin.h"
#include "SoftcutClient.h"

using namespace softcut_jack_osc;

class Display {
 public:
  void requestShutdown() { running_ = false; }
  // Define a callback type for quit notification
  using QuitCallback = std::function<void()>;

  Display(int width = 1200, int height = 900);
  ~Display();

  void init(SoftcutClient* sc, int numVoices);

  void start();
  void stop();
  bool isRunning() const { return running_; }
  void SetMessage(const std::string& message, int secondsToDisplay);

  // Set the callback to be called when window is closed
  void setQuitCallback(QuitCallback callback) { quitCallback_ = callback; }

 private:
  void renderLoop();

  // Window properties
  int width_;
  int height_;

  // SDL components
  SDL_Window* window_;
  SDL_Renderer* renderer_;

  // State
  std::atomic<bool> running_;

  // Thread
  std::unique_ptr<std::thread> displayThread_;

  // Quit callback
  QuitCallback quitCallback_;

  // SofcutClient
  int numVoices_;
  SoftcutClient* softCutClient_ = nullptr;
  DisplayRing* displayRings_;

  // Keyboard
  KeyboardHandler keyboardHandler_;

  // Display
  TTF_Font* font = nullptr;
  PerlinNoise perlinGenerator;
  float noiseTimeValue = 0.0f;
  int selected_loop = 0;
  bool mouse_dragging = false;
  bool dragging_bar = false;
  int dragged_parameter = -1;

  // Parameters
  Parameters* params_ = nullptr;
  int param_count_ = Parameters::PARAM_COUNT;

  // Help System
  HelpSystem helpSystem_;

  // Intro Animation
  IntroAnimation introAnimation_;
  float mainContentFadeAlpha_;

  // Display Message
  DisplayMessage displayMessage_;

  // Zoom settings
  float zoomFactor_ = 1.0f;
  const float minZoomFactor_ = 0.5f;
  const float maxZoomFactor_ = 2.0f;
  const float zoomStep_ = 0.1f;
  float mouseScaleFactor_ = 1.0f;
};

#endif  // DISPLAY_H
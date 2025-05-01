#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL2/SDL.h>
#include <lo/lo.h>

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "SoftcutClient.h"
using namespace softcut_jack_osc;

class Display {
 public:
  void requestShutdown() { running_ = false; }
  // Define a callback type for quit notification
  using QuitCallback = std::function<void()>;

  Display(int width = 800, int height = 600);
  ~Display();

  void init(SoftcutClient* sc);

  void start();
  void stop();
  bool isRunning() const { return running_; }

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
  SoftcutClient* softCutClient_ = nullptr;
};

#endif  // DISPLAY_H
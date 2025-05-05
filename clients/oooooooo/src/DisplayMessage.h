#ifndef MESSAGE_H
#define MESSAGE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <chrono>
#include <string>

class DisplayMessage {
 public:
  DisplayMessage();

  void Init(TTF_Font* font);
  void SetMessage(const std::string& message, int displaySeconds);
  void Update();
  void Render(SDL_Renderer* renderer, int windowWidth, int windowHeight);
  bool IsActive() const;

 private:
  enum class State { IDLE, FADING_IN, DISPLAYING, FADING_OUT };

  TTF_Font* font_;
  std::string currentMessage_;
  State state_;
  float alpha_;

  std::chrono::steady_clock::time_point startTime_;
  std::chrono::steady_clock::time_point displayEndTime_;

  int displayDurationMs_;
  const int FADE_DURATION_MS = 500;  // 500ms fade in/out

  SDL_Color textColor_;
};

#endif  // MESSAGE_H
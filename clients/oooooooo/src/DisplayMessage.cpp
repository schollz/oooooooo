#include "DisplayMessage.h"

#include <algorithm>

DisplayMessage::DisplayMessage()
    : font_(nullptr),
      state_(State::IDLE),
      alpha_(0.0f),
      displayDurationMs_(0),
      textColor_({255, 255, 255, 255}) {}

void DisplayMessage::Init(TTF_Font* font) { font_ = font; }

void DisplayMessage::SetMessage(const std::string& message,
                                int displaySeconds) {
  currentMessage_ = message;
  displayDurationMs_ = displaySeconds * 1000;
  state_ = State::FADING_IN;
  alpha_ = 0.0f;
  startTime_ = std::chrono::steady_clock::now();
  displayEndTime_ = startTime_ + std::chrono::milliseconds(FADE_DURATION_MS +
                                                           displayDurationMs_);
}

void DisplayMessage::Update() {
  if (state_ == State::IDLE) {
    return;
  }

  auto now = std::chrono::steady_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_)
          .count();

  switch (state_) {
    case State::FADING_IN:
      if (elapsed < FADE_DURATION_MS) {
        alpha_ = static_cast<float>(elapsed) / FADE_DURATION_MS;
      } else {
        alpha_ = 1.0f;
        state_ = State::DISPLAYING;
      }
      break;

    case State::DISPLAYING:
      if (elapsed > FADE_DURATION_MS + displayDurationMs_) {
        state_ = State::FADING_OUT;
      }
      break;

    case State::FADING_OUT:
      if (elapsed < FADE_DURATION_MS + displayDurationMs_ + FADE_DURATION_MS) {
        float fadeOutProgress = static_cast<float>(elapsed - FADE_DURATION_MS -
                                                   displayDurationMs_) /
                                FADE_DURATION_MS;
        alpha_ = 1.0f - fadeOutProgress;
      } else {
        alpha_ = 0.0f;
        state_ = State::IDLE;
        currentMessage_.clear();
      }
      break;

    case State::IDLE:
      // Already handled above
      break;
  }
}

void DisplayMessage::Render(SDL_Renderer* renderer,
                            int windowWidth [[maybe_unused]],
                            int windowHeight [[maybe_unused]]) {
  if (state_ == State::IDLE || alpha_ <= 0.0f || currentMessage_.empty() ||
      !font_) {
    return;
  }

  SDL_Color color = textColor_;
  color.a = static_cast<Uint8>(255 * alpha_);

  // Create surface and texture
  SDL_Surface* surface =
      TTF_RenderText_Blended(font_, currentMessage_.c_str(), color);
  if (!surface) {
    return;
  }

  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!texture) {
    SDL_FreeSurface(surface);
    return;
  }
  // Position the message in the bottom-right corner with some padding
  int padding = 10;
  int x = padding;
  int y = windowHeight - surface->h - padding;

  SDL_Rect destRect = {x, y, surface->w, surface->h};
  SDL_RenderCopy(renderer, texture, nullptr, &destRect);

  // Clean up
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
}

bool DisplayMessage::IsActive() const { return state_ != State::IDLE; }
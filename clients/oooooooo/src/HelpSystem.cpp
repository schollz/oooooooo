#include "HelpSystem.h"

#include <algorithm>

HelpSystem::HelpSystem()
    : font(nullptr),
      helpVisible(false),
      fadeAlpha(0.0f),
      fadeTarget(0.0f),
      fadeStep(0.05f) {
  // Initialize help messages
  helpMessages = {
      "basics",
      "h toggles help",
      "m toggles menu",
      "1-8 selects loop",
      "up/down selects parameter",
      "left/right adjusts parameter",
      "",
      "playing/recording",
      "p toggles play",
      "r toggles record",
      "ctl + r toggles record once",
      "",
      "lfos",
      "l toggles lfo",
      "ctrl+left/right adjusts min lfo",
      "alt+left/right adjusts max lfo",
      "",
      "saving/loading",
      "drag-and-drop audio file to load",
      "s will save tape loops",
  };
}

HelpSystem::~HelpSystem() { font = nullptr; }

void HelpSystem::Init(TTF_Font* font) { this->font = font; }

void HelpSystem::Toggle() {
  helpVisible = !helpVisible;
  fadeTarget = helpVisible ? 1.0f : 0.0f;
}

void HelpSystem::Render(SDL_Renderer* renderer, int windowWidth) {
  // Update fade alpha based on target
  if (fadeAlpha < fadeTarget) {
    fadeAlpha += fadeStep;
    if (fadeAlpha > fadeTarget) fadeAlpha = fadeTarget;
  } else if (fadeAlpha > fadeTarget) {
    fadeAlpha -= fadeStep;
    if (fadeAlpha < fadeTarget) fadeAlpha = fadeTarget;
  }
  // Only proceed if we have a font and some alpha
  if (!font || fadeAlpha <= 0.01f) return;

  // Calculate the alpha value for SDL (0-255)
  uint8_t alpha = static_cast<uint8_t>(fadeAlpha * 255);

  // Set up position for text on right side of screen
  int textX = windowWidth - 450;  // 450 pixels from right edge
  int textY = 10;                 // Start 10 pixels from top
  int lineHeight = 20;            // Spacing between lines

  // Ensure text position is always visible
  textX = std::max(10, textX);

  // Draw each help message
  for (const auto& message : helpMessages) {
    SDL_Color textColor = {128, 128, 128, alpha};  // Gray with current alpha
    SDL_Surface* textSurface =
        TTF_RenderText_Blended(font, message.c_str(), textColor);

    if (textSurface) {
      SDL_Texture* textTexture =
          SDL_CreateTextureFromSurface(renderer, textSurface);
      if (textTexture) {
        SDL_Rect textRect = {textX, textY, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
        SDL_DestroyTexture(textTexture);
      }
      SDL_FreeSurface(textSurface);
    }

    // Move down for next line
    textY += lineHeight;
  }
}
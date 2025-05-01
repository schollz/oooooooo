// HelpSystem.cpp
#include "HelpSystem.h"

#include <string>
#include <vector>

// Static variables
static TTF_Font* s_font = nullptr;
static LoggerFunc log_info = nullptr;
static LoggerFunc log_debug = nullptr;

// Help system state
static bool help_visible = false;
static float fade_alpha = 0.0f;   // 0.0 = fully transparent, 1.0 = fully opaque
static float fade_target = 0.0f;  // Target alpha value
static float fade_step = 0.05f;   // Amount to change alpha per frame

// Help messages to display
static std::vector<std::string> help_messages = {
    "basics",
    "pressing h will toggle help",
    "pressing 1-7 will select loop 1-7",
    "pressing up/down will navigate parameters",
    "pressing left/right will adjust parameter",
    "",
    "playing/recording",
    "pressing r will toggle recording",
    "pressing ctl + r will toggle record once",
    "pressing p will toggle play",
    "",
    "lfos",
    "pressing l will toggle lfo",
    "pressing : or , will adjust lfo period",
    "pressing - or + will increase lfo depth",
    "pressing [ or ] will decrease lfo depth",
    "",
    "saving/loading",
    "drag-and-drop an audio file to load it",
    "pressing s will save tape loops",

};

void initializeHelpSystem(TTF_Font** font, LoggerFunc info_logger,
                          LoggerFunc debug_logger) {
  s_font = *font;
  log_info = info_logger;
  log_debug = debug_logger;

  if (log_debug) log_debug("Help system initialized");
}

void cleanupHelpSystem() {
  s_font = nullptr;
  log_info = nullptr;
  log_debug = nullptr;
}

void toggleHelp() {
  help_visible = !help_visible;
  fade_target = help_visible ? 1.0f : 0.0f;

  if (log_debug) log_debug("Help visibility toggled to: %d", help_visible);
}

void updateHelpText() {
  // Update fade alpha based on target
  if (fade_alpha < fade_target) {
    fade_alpha += fade_step;
    if (fade_alpha > fade_target) fade_alpha = fade_target;
  } else if (fade_alpha > fade_target) {
    fade_alpha -= fade_step;
    if (fade_alpha < fade_target) fade_alpha = fade_target;
  }
}

void drawHelpText(SDL_Renderer* renderer, int window_width, int window_height) {
  // Only proceed if we have a font and some alpha
  if (!s_font || fade_alpha <= 0.01f) return;

  // Calculate the alpha value for SDL (0-255)
  uint8_t alpha = static_cast<uint8_t>(fade_alpha * 255);

  // Set up position for text on right side of screen
  int text_x = window_width - 450;  // 300 pixels from right edge
  int text_y = 10;                  // Start 50 pixels from top
  int line_height = 20;             // Spacing between lines

  // Ensure text position is always visible
  text_x = std::max(10, text_x);

  // Draw each help message
  for (const auto& message : help_messages) {
    // Draw the message with the current alpha
    SDL_Color textColor = {128, 128, 128, alpha};  // White with current alpha
    SDL_Surface* textSurface =
        TTF_RenderText_Blended(s_font, message.c_str(), textColor);

    if (textSurface) {
      SDL_Texture* textTexture =
          SDL_CreateTextureFromSurface(renderer, textSurface);
      if (textTexture) {
        SDL_Rect textRect = {text_x, text_y, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
        SDL_DestroyTexture(textTexture);
      }
      SDL_FreeSurface(textSurface);
    }

    // Move down for next line
    text_y += line_height;
  }
}
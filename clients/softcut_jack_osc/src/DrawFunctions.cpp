// DrawFunctions.cpp

#include "DrawFunctions.h"

#include <cmath>
#include <iostream>

#include "DisplayUtils.h"
#include "Perlin.h"

// Global static TTF_Font pointer
static TTF_Font* s_font = nullptr;
static PerlinNoise* s_perlin_generator = nullptr;

void initializeDrawingResources(TTF_Font** font, PerlinNoise* perlin) {
  s_font = *font;
  s_perlin_generator = perlin;
}

void cleanupDrawingResources() {
  s_font = nullptr;
  s_perlin_generator = nullptr;
}

void drawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
              int x, int y, uint8_t color) {
  if (!font) return;

  SDL_Surface* textSurface =
      TTF_RenderText_Solid(font, text.c_str(), {color, color, color, 0});
  if (textSurface) {
    SDL_Texture* textTexture =
        SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture) {
      SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
      SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
      SDL_DestroyTexture(textTexture);
    }
    SDL_FreeSurface(textSurface);
  }
}

void drawBar(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int width,
             int height, float fill, float lfo, float lfo_min, float lfo_max,
             const std::string& label, bool selected) {
  // Draw the entire bar as dim gray or brighter if selected
  if (selected) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100,
                           0);  // Brighter gray when selected
  } else {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 0);  // Dim gray
  }
  SDL_Rect barRect = {x, y, width, height};
  SDL_RenderFillRect(renderer, &barRect);

  // Draw the filled area as white or brighter white if selected
  if (selected) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255,
                           0);  // Bright white for selected
  } else {
    SDL_SetRenderDrawColor(renderer, 200, 200, 200,
                           0);  // Slightly dimmer white
  }
  SDL_Rect fillRect = {x, y, static_cast<int>(std::round(width * lfo)), height};
  SDL_RenderFillRect(renderer, &fillRect);

  // Draw a white line for the LFO position
  int lfo_x = static_cast<int>(std::round(x + width * lfo));
  if (lfo_x > fill) {
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 0);  // White for LFO line
  } else {
    SDL_SetRenderDrawColor(renderer, 205, 205, 205,
                           0);  // Dimmer white for LFO line
  }
  SDL_Rect lfoRect = {lfo_x, y, 4, height};
  SDL_RenderFillRect(renderer, &lfoRect);  // Vertical line for LFO

  // Draw a thin horizontal line for the LFO min max
  SDL_SetRenderDrawColor(renderer, 205, 205, 205,
                         0);  // Dimmer white for LFO line
                              // first draw from min to current set point

  // Draw a thin horizontal line from start_x to end_x
  SDL_SetRenderDrawColor(renderer, 205, 205, 205,
                         0);  // Dimmer white for LFO line
  SDL_Rect lfoMinRect = {static_cast<int>(std::round(x + width * lfo_min)),
                         static_cast<int>(std::round(y + height / 2) - 2),
                         static_cast<int>(std::round(width * (lfo - lfo_min))),
                         5};
  SDL_RenderFillRect(renderer, &lfoMinRect);
  SDL_SetRenderDrawColor(renderer, 64, 64, 64, 0);  // Dimmer white for LFO line
  SDL_Rect lfoMaxRect = {static_cast<int>(std::round(x + width * lfo)),
                         static_cast<int>(std::round(y + height / 2) - 2),
                         static_cast<int>(std::round(width * (lfo_max - lfo))),
                         5};
  SDL_RenderFillRect(renderer, &lfoMaxRect);

  // Render the label to the right of the bar
  if (!label.empty()) {
    // highlight text if selected
    if (selected) {
      drawText(renderer, font, label, x + width + 10, y, 175);
    } else {
      drawText(renderer, font, label, x + width + 10, y, 100);
    }
  }
}

void drawRing(SDL_Renderer* renderer, PerlinNoise* perlin, float id, int x,
              int y, int radius, float position, float thickness,
              float* noiseTimeValue, bool show_notch, bool sketchy) {
  if (!perlin) return;

  // Draw outer circle with optional sketchy effect
  for (int w = -radius - thickness; w <= radius + thickness; w++) {
    for (int h = -radius - thickness; h <= radius + thickness; h++) {
      float dist = sqrtf(w * w + h * h);

      if (sketchy) {
        // Use perlin noise to create sketchy effect
        float noiseValue =
            perlin->noise((id + x + w) * 0.05, (id + y + h) * 0.05,
                          id + *noiseTimeValue) *
            3.0f;

        // Adjust the distance check with noise
        float innerRadius = radius - thickness + noiseValue;
        float outerRadius = radius + thickness + noiseValue;

        if (dist <= outerRadius && dist >= innerRadius) {
          SDL_RenderDrawPoint(renderer, x + w, y + h);
        }
      } else {
        // Regular non-sketchy circle
        if (dist <= radius + thickness && dist >= radius - thickness) {
          SDL_RenderDrawPoint(renderer, x + w, y + h);
        }
      }
    }
  }

  // Draw position indicator (notch)
  float angle = position * 2.0f * M_PI;  // Convert position (0.0-1.0) to angle
  int notch_x = x + sinf(angle) * radius;
  int notch_y =
      y - cosf(angle) * radius;  // Subtract because SDL Y-axis is inverted

  // Draw a small circle for the notch
  if (show_notch) {
    const int notchRadius = 5;
    for (int w = -notchRadius; w <= notchRadius; w++) {
      for (int h = -notchRadius; h <= notchRadius; h++) {
        if (w * w + h * h <= notchRadius * notchRadius) {
          SDL_RenderDrawPoint(renderer, notch_x + w, notch_y + h);
        }
      }
    }
  }

  // Increment noise time value for animation
  *noiseTimeValue += 0.001f;
}

bool isPointInBar(int x, int y, int bar_x, int bar_y, int bar_width,
                  int bar_height) {
  return (x >= bar_x && x <= bar_x + bar_width && y >= bar_y &&
          y <= bar_y + bar_height);
}

bool isPointInLoop(int x, int y, int loop_x, int loop_y, int radius) {
  // Calculate distance from point to circle center
  int dx = x - loop_x;
  int dy = y - loop_y;
  float distance = sqrt(dx * dx + dy * dy);

  // Check if distance is less than or equal to radius
  return (distance <= radius);
}

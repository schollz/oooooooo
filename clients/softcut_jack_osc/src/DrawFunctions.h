// DrawFunctions.h
#ifndef DRAW_FUNCTIONS_H
#define DRAW_FUNCTIONS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>

#include "Perlin.h"

// Initialize font and perlin noise
void initializeDrawingResources(TTF_Font** font, PerlinNoise* perlin);
void cleanupDrawingResources();

// Text drawing function
void drawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
              int x, int y, uint8_t color);

// Draw a bar for the volume level with a text label
void drawBar(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int width,
             int height, float fill, float lfo, float lfo_min, float lfo_max,
             const std::string& label, bool selected);

// Draw a ring with a position indicator
void drawRing(SDL_Renderer* renderer, PerlinNoise* perlin, float id, int x,
              int y, int radius, float position, float thickness,
              float* noiseTimeValue, bool show_notch = true,
              bool sketchy = false);

// Utility functions for hit testing
bool isPointInBar(int x, int y, int bar_x, int bar_y, int bar_width,
                  int bar_height);
bool isPointInLoop(int x, int y, int loop_x, int loop_y, int radius);

#endif  // DRAW_FUNCTIONS_H
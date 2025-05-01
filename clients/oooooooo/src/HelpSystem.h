// HelpSystem.h
#ifndef HELP_SYSTEM_H
#define HELP_SYSTEM_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>
#include <vector>

// Forward declarations
typedef void (*LoggerFunc)(const char* format, ...);

// Initialize the help system
void initializeHelpSystem(TTF_Font** font, LoggerFunc info_logger,
                          LoggerFunc debug_logger);

// Cleanup help system resources
void cleanupHelpSystem();

// Toggle help visibility
void toggleHelp();

// Draw help text
void drawHelpText(SDL_Renderer* renderer, int window_width, int window_height);

// Update help text (for fading effects)
void updateHelpText();

#endif  // HELP_SYSTEM_H
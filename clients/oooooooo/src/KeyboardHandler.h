// KeyboardHandler.h
#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H

#include <SDL2/SDL.h>

#include <unordered_map>

// Forward declarations
class TapeLoops;
typedef void (*LoggerFunc)(const char* format, ...);

// Initialize keyboard handler with needed pointers
void initializeKeyboardHandler(TapeLoops* tape_loops_ptr,
                               LoggerFunc info_logger, LoggerFunc debug_logger);

// Cleanup keyboard handler resources
void cleanupKeyboardHandler();

// Key handling functions
void handle_key_down(SDL_Keycode key, bool is_repeat, SDL_Keymod modifiers);
void handle_key_up(SDL_Keycode key);

// Get/set the current parameter
int getCurrentParameter();
void setCurrentParameter(int param);

// Adjust parameter based on currently selected parameter
void adjustParameter(int direction);

#endif  // KEYBOARD_HANDLER_H
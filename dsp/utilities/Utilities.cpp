#include "Utilities.h"

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <string>
#include <vector>

float MidiToFreq(float midi) {
  return powf(2.0f, (midi - 69.0f) / 12.0f) * 440.0f;
}

float LinLin(float in, float inMin, float inMax, float outMin, float outMax) {
  return (in - inMin) / (inMax - inMin) * (outMax - outMin) + outMin;
}

float FClamp(float x, float min, float max) {
  return std::max(min, std::min(x, max));
}

// Format a string similar to sprintf but returning a std::string
std::string FormatString(const char* format, ...) {
  // First, determine the required buffer size
  va_list args;
  va_start(args, format);
  // Use vsnprintf with a null buffer and 0 size to calculate required size
  int size = vsnprintf(nullptr, 0, format, args) + 1;  // +1 for null terminator
  va_end(args);

  // Allocate the buffer
  char* buffer = new char[size];

  // Format the string into the buffer
  va_start(args, format);
  vsnprintf(buffer, size, format, args);
  va_end(args);

  // Create a string and clean up
  std::string result(buffer);
  delete[] buffer;

  return result;
}

float DBAmp(float db) { return powf(10.0f, db / 20.0f); }

float AmpDB(float amp) {
  if (amp < 0.001) {
    return -64.0f;  // Avoid log(0) which is undefined
  }
  return 20.0f * log10f(amp);
}
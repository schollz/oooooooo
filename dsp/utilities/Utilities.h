#ifndef UTILITIES_H
#define UTILITIES_H
#pragma once

#include <cstdarg>
#include <string>

// Declaration of the FormatString function
std::string FormatString(const char* format, ...);

float MidiToFreq(float midi);

float LinLin(float in, float inMin, float inMax, float outMin, float outMax);

float FClamp(float x, float min, float max);

float DBAmp(float db);

float AmpDB(float amp);

#endif
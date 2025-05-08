//
// Created on 05/07/2025.
//

#include "VUMeter.h"

#include <algorithm>
#include <cmath>
#include <limits>

VUMeter::VUMeter()
    : currentLevelDB(-100.0f)  // Start at a very low level (effectively silent)
      ,
      attackCoeff(0.0f),
      decayCoeff(0.0f),
      currentPeak(0.0f),
      sampleRate(48000.0f)  // Default to common sample rate
      ,
      attackTime(0.01f)  // 10ms attack (fast)
      ,
      decayTime(0.3f)  // 300ms decay (slow)
{
  updateCoefficients();
}

void VUMeter::setSampleRate(float sr) {
  sampleRate = sr;
  updateCoefficients();
}

void VUMeter::setAttackTime(float timeInSeconds) {
  attackTime = timeInSeconds;
  updateCoefficients();
}

void VUMeter::setDecayTime(float timeInSeconds) {
  decayTime = timeInSeconds;
  updateCoefficients();
}

void VUMeter::updateCoefficients() {
  // Calculate time constants as coefficients
  // Formula: coeff = exp(-1.0 / (time * sampleRate))
  attackCoeff = std::exp(-1.0f / (attackTime * sampleRate));
  decayCoeff = std::exp(-1.0f / (decayTime * sampleRate));
}

float VUMeter::ampToDB(float amp) {
  // Avoid log of zero or negative values
  const float minAmp = 1.0e-6f;  // -120dB
  if (amp < minAmp) {
    return -100.0f;  // Return a very low dB value for near-silence
  }

  // Convert amplitude to dB, reference is 1.0
  return 20.0f * std::log10(amp);
}

float VUMeter::dBToAmp(float db) { return std::pow(10.0f, db / 20.0f); }

void VUMeter::process(const sample_t* buffer, size_t numFrames) {
  if (numFrames == 0) return;

  // Find peak in this buffer
  float bufferPeak = 0.0f;
  for (size_t i = 0; i < numFrames; ++i) {
    float absValue = std::fabs(static_cast<float>(buffer[i]));
    bufferPeak = std::max(bufferPeak, absValue);
  }

  // Apply envelope follower (fast attack, slow decay)
  if (bufferPeak > currentPeak) {
    // Attack phase - quick response to peaks
    currentPeak = attackCoeff * currentPeak + (1.0f - attackCoeff) * bufferPeak;
  } else {
    // Decay/release phase - slow fall-off
    currentPeak = decayCoeff * currentPeak + (1.0f - decayCoeff) * bufferPeak;
  }

  // Convert to dB and update the atomic value for thread-safe access
  currentLevelDB.store(ampToDB(currentPeak));
}

float VUMeter::getLevel() const { return currentLevelDB.load(); }

void VUMeter::reset() {
  currentPeak = 0.0f;
  currentLevelDB.store(-100.0f);
}

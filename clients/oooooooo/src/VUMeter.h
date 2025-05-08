//
// Created on 05/07/2025.
//

#ifndef CRONE_VUMETER_H
#define CRONE_VUMETER_H

#include <atomic>
#include <cmath>

namespace softcut_jack_osc {

class VUMeter {
 public:
  VUMeter();

  // Initialize with sample rate
  void setSampleRate(float sr);

  // Set attack time in seconds
  void setAttackTime(float timeInSeconds);

  // Set release/decay time in seconds
  void setDecayTime(float timeInSeconds);

  // Process a mono buffer and update the VU level
  void process(const double* buffer, size_t numFrames);

  // Get the current VU level in dB (thread-safe)
  float getLevel() const;

  // Convert linear amplitude to dB
  static float ampToDB(float amp);

  // Convert dB to linear amplitude
  static float dBToAmp(float db);

  // Reset the meter
  void reset();

 private:
  std::atomic<float>
      currentLevelDB;  // Current level in dB (atomic for thread safety)
  float attackCoeff;   // Attack coefficient
  float decayCoeff;    // Decay/release coefficient
  float currentPeak;   // Current peak value in linear amplitude
  float sampleRate;    // Current sample rate
  float attackTime;    // Attack time in seconds
  float decayTime;     // Decay time in seconds

  // Recalculate coefficients when times change
  void updateCoefficients();
};

}  // namespace softcut_jack_osc

#endif  // CRONE_VUMETER_H
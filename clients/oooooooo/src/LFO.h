/*
Copyright (c) 2020 Electrosmith, Corp

Use of this source code is governed by an MIT-style
license that can be found in the LICENSE file or at
https://opensource.org/licenses/MIT.
*/

#pragma once
#ifndef DSY_LFO_H
#define DSY_LFO_H
#include <stdint.h>

#ifdef __cplusplus

/** Synthesis of several waveforms, including polyBLEP bandlimited waveforms.
 */
class LFO {
 public:
  LFO() {}
  ~LFO() {}
  /** Choices for output waveforms, POLYBLEP are appropriately labeled. Others
   * are naive forms.
   */
  enum Waveform {
    WAVE_SIN,
    WAVE_TRI,
    WAVE_SAW,
    WAVE_RAMP,
    WAVE_SQUARE,
    WAVE_POLYBLEP_TRI,
    WAVE_POLYBLEP_SAW,
    WAVE_POLYBLEP_SQUARE,
    WAVE_LAST,
  };

  /** Initializes the LFO

      \param sample_rate - sample rate of the audio engine being run, and the
     frequency that the Process function will be called.

      Defaults:
      - freq_ = 100 Hz
      - amp_ = 0.5
      - waveform_ = sine wave.
  */
  void Init(float sample_rate) {
    sr_ = sample_rate;
    out_ = 0.0f;
    add_ = 0.0f;
    sr_recip_ = 1.0f / sample_rate;
    freq_ = 100.0f;
    amp_ = 1.0f;
    pw_ = 0.5f;
    phase_ = 0.0f;
    phase_inc_ = CalcPhaseInc(freq_);
    waveform_ = WAVE_SIN;
    eoc_ = true;
    eor_ = true;
  }

  /** Changes the frequency of the LFO, and recalculates phase
   * increment.
   */
  inline void SetFreq(const float f) {
    freq_ = f;
    phase_inc_ = CalcPhaseInc(f);
  }

  /** Sets the amplitude of the waveform.
   */
  inline void SetAmp(const float a) { amp_ = a; }

  float GetAmp() { return amp_; }

  float GetPeriod() { return 1.0f / freq_; }

  inline void SetAdd(float a) { add_ = a; }
  /** Sets the waveform to be synthesized by the Process() function.
   */
  inline void SetWaveform(const Waveform wf) {
    waveform_ = wf < WAVE_LAST ? wf : WAVE_SIN;
  }
  /** Sets the pulse width for WAVE_SQUARE and WAVE_POLYBLEP_SQUARE (range 0 -
   * 1)
   */
  inline void SetPw(const float pw) {
    pw_ = pw < 0.0f ? 0.0f : pw > 1.0f ? 1.0f : pw;
  }

  /** Returns true if cycle is at end of rise. Set during call to Process.
   */
  inline bool IsEOR() { return eor_; }

  /** Returns true if cycle is at end of cycle. Set during call to Process.
   */
  inline bool IsEOC() { return eoc_; }

  /** Returns true if cycle rising.
   */
  inline bool IsRising() { return phase_ < 0.5f; }

  /** Returns true if cycle falling.
   */
  inline bool IsFalling() { return phase_ >= 0.5f; }

  /** Processes the waveform to be generated, returning one sample. This should
   * be called once per sample period.
   */
  float Process();

  float Value() { return out_; }

  /** Adds a value 0.0-1.0 (equivalent to 0.0-TWO_PI) to the current phase.
   * Useful for PM and "FM" synthesis.
   */
  void PhaseAdd(float _phase) { phase_ += _phase; }
  /** Resets the phase to the input argument. If no argumeNt is present, it will
   * reset phase to 0.0;
   */
  void Reset(float _phase = 0.0f) { phase_ = _phase; }

 private:
  float CalcPhaseInc(float f);
  Waveform waveform_;
  float add_, amp_, freq_, pw_;
  float sr_, sr_recip_, phase_, phase_inc_;
  float last_out_, last_freq_;
  float out_;
  bool eor_, eoc_;
};

#endif
#endif
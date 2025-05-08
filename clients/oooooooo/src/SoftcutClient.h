//
// Created by emb on 11/28/18.
//

#ifndef CRONE_CUTCLIENT_H
#define CRONE_CUTCLIENT_H

#include <iostream>

#include "BufDiskWorker.h"
#include "Bus.h"
#include "JackClient.h"
#include "Utilities.h"
#include "VUMeter.h"
#include "dsp/fverb/FVerb.h"
#include "softcut/Softcut.h"
#include "softcut/Types.h"

namespace softcut_jack_osc {
class SoftcutClient : public JackClient<2, 2> {
 public:
  enum { MaxBlockFrames = 2048 };
  // >2^24 as long as sample_t is double,
  // otherwise 2^24
  enum {
    BufFrames = 134217728
  };  // 2^27, ~6 minutes of mono audio at 48kHz per loop
  enum { NumVoices = 8 };
  typedef enum { SourceAdc = 0 } SourceId;
  typedef Bus<2, MaxBlockFrames> StereoBus;
  typedef Bus<1, MaxBlockFrames> MonoBus;

 public:
  SoftcutClient();
  void init();
  float getSavedPosition(int i) { return cut.getSavedPosition(i); }
  float getPan(int i) { return (outPan[i].getValue() - 0.5) * 2; }
  float getInLevel(int i) { return inLevel[0][i].getValue(); }
  float getOutLevel(int i) { return outLevel[i].getValue(); }
  float getDuration(int i) { return cut.getDuration(i); }
  float getLoopStart(int i) { return cut.getLoopStart(i); }
  float getLoopMin(int i) { return loopMin[i]; }
  float getLoopEnd(int i) { return cut.getLoopEnd(i); }
  float getPreGain(int i) { return cut.getPreGain(i); }
  float getRate(int i) { return rateSet[i]; }
  void setBaseRate(int i, float rate) {
    rateBase[i] = rate;
    updateRate(i);
  }
  void setRateDirection(int i, bool forward) {
    rateForward[i] = forward;
    updateRate(i);
  }
  void updateRate(int i) {
    cut.setRate(i, rateSet[i] * rateBase[i] * (rateForward[i] ? 1.f : -1.f));
  }
  bool IsRecording(int i) { return cut.getRecFlag(i); }
  bool IsPlaying(int i) { return cut.getPlayFlag(i); }
  void ToggleRecord(int i) { cut.setRecFlag(i, !IsRecording(i)); }
  void TogglePlay(int i) { cut.setPlayFlag(i, !IsPlaying(i)); }
  float getVULevel(int voice) const {
    if (voice >= 0 && voice < NumVoices) {
      return vuMeters[voice].getLevel();
    }
    return -100.0f;  // Return silence for invalid voice
  }
  float getCPUUsage() { return static_cast<float>(jack_cpu_load(client)); }

 private:
  // processors
  softcut::Softcut<NumVoices> cut;
  // main buffer
  sample_t buf[2][BufFrames];
  // buffer index for use with BufDiskWorker
  int bufIdx[2];
  // busses
  StereoBus mix;
  MonoBus input[NumVoices];
  MonoBus output[NumVoices];
  // levels
  LogRamp inLevel[2][NumVoices];
  LogRamp outLevel[NumVoices];
  LogRamp outPan[NumVoices];
  LogRamp fbLevel[NumVoices][NumVoices];
  // enabled flags
  bool enabled[NumVoices];
  softcut::phase_t quantPhase[NumVoices];
  float sampleRate;

  VUMeter vuMeters[NumVoices];

 private:
  void process(jack_nframes_t numFrames) override;
  void setSampleRate(jack_nframes_t) override;
  inline size_t secToFrame(float sec) {
    return static_cast<size_t>(sec * jack_get_sample_rate(JackClient::client));
  }

 public:
  /// FIXME: the "commands" structure shouldn't really be necessary.
  /// should be able to refactor most/all parameters for atomic access.
  // called from audio thread
  void handleCommand(Commands::CommandPacket *p) override;

  // these accessors can be called from other threads, so don't need to go
  // through the commands queue
  //-- buffer manipulation
  //-- time parameters are in seconds
  //-- negative 'dur' parameter reads/clears/writes as much as possible.
  void readBufferMono(const std::string &path, float startTimeSrc = 0.f,
                      float startTimeDst = 0.f, float dur = -1.f,
                      int chanSrc = 0, int chanDst = 0) {
    BufDiskWorker::requestReadMono(bufIdx[chanDst], path, startTimeSrc,
                                   startTimeDst, dur, chanSrc);
  }

  void readBufferStereo(const std::string &path, float startTimeSrc = 0.f,
                        float startTimeDst = 0.f, float dur = -1.f) {
    BufDiskWorker::requestReadStereo(bufIdx[0], bufIdx[1], path, startTimeSrc,
                                     startTimeDst, dur);
  }

  void writeBufferMono(const std::string &path, float start, float dur,
                       int chan) {
    BufDiskWorker::requestWriteMono(bufIdx[chan], path, start, dur);
  }

  void writeBufferStereo(const std::string &path, float start, float dur) {
    BufDiskWorker::requestWriteStereo(bufIdx[0], bufIdx[1], path, start, dur);
  }

  void clearBuffer(int chan, float start = 0.f, float dur = -1) {
    if (chan < 0 || chan > 1) {
      return;
    }
    BufDiskWorker::requestClear(bufIdx[chan], start, dur);
  }

  // check if quantized phase has changed for a given voice
  // returns true
  bool checkVoiceQuantPhase(int i) {
    if (quantPhase[i] != cut.getQuantPhase(i)) {
      quantPhase[i] = cut.getQuantPhase(i);
      return true;
    } else {
      return false;
    }
  }
  softcut::phase_t getQuantPhase(int i) { return cut.getQuantPhase(i); }
  void setPhaseQuant(int i, softcut::phase_t q) { cut.setPhaseQuant(i, q); }
  void setPhaseOffset(int i, float sec) { cut.setPhaseOffset(i, sec); }

  int getNumVoices() const { return NumVoices; }

  float getSampleRate() const { return sampleRate; }

  void reset();

  // Reverb methods
  void setReverbEnabled(bool enabled) { reverbEnabled = enabled; }

  void setReverbMix(float mix) { reverbMix.setTarget(mix); }

  void setReverbSend(int voice, float level) {
    if (voice >= 0 && voice < NumVoices) {
      reverbSend[voice].setTarget(level);
    }
  }

  void setReverbDecay(float decay) { reverb.SetDecay(decay); }

  void setReverbTailDensity(float density) { reverb.SetTailDensity(density); }

  void setReverbInputDiffusion1(float diffusion) {
    reverb.SetInputDiffision1(diffusion);
  }

  void setReverbInputDiffusion2(float diffusion) {
    reverb.SetInputDiffision2(diffusion);
  }

 private:
  FVerb reverb;
  bool reverbEnabled = false;
  LogRamp reverbMix;
  LogRamp reverbSend[NumVoices];
  float rateBase[NumVoices];
  float rateSet[NumVoices];
  float loopMin[NumVoices];
  bool rateForward[NumVoices];
  StereoBus reverbBus;
  void clearBusses(size_t numFrames);
  void mixInput(size_t numFrames);
  void mixOutput(size_t numFrames);
};
}  // namespace softcut_jack_osc

#endif  // CRONE_CUTCLIENT_H

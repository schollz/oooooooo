//
// Created by emb on 11/28/18.
//

#ifndef CRONE_CUTCLIENT_H
#define CRONE_CUTCLIENT_H

#include <filesystem>
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
  void ToggleRecord(int i) { ToggleRecord(i, !IsRecording(i)); }
  void ToggleRecord(int i, bool rec) {
    if (rec) {
      cut.setPlayFlag(i, true);
      cut.setRecFlag(i, true);
      if (isPrimed[i]) {
        wasPrimed[i] = true;
        isPrimed[i] = false;
      }
    } else {
      cut.setRecFlag(i, false);
      doneRecordingPrimed[i] = wasPrimed[i];
      if (wasPrimed[i]) {
        doneRecordingPrimed[i] = true;
      }
    }
  }
  void TogglePrime(int i) {
    wasPrimed[i] = false;
    doneRecordingPrimed[i] = false;
    isPrimed[i] = !isPrimed[i];
  }
  void TogglePrimeToRecordOnce(int i) {
    wasPrimed[i] = false;
    doneRecordingPrimed[i] = false;
    isPrimed[i] = !isPrimed[i];
    isPrimedToRecordOnce[i] = isPrimed[i];
  }
  bool IsPrimed(int i) { return isPrimed[i]; }
  bool IsPrimedToRecordOnce(int i) {
    return isPrimed[i] && isPrimedToRecordOnce[i];
  }
  bool WasPrimed(int i) { return wasPrimed[i]; }
  void SetWasPrimed(int i, bool was) { wasPrimed[i] = was; }
  bool IsDoneRecordingPrimed(int i) {
    if (doneRecordingPrimed[i]) {
      doneRecordingPrimed[i] = false;
      return true;
    }
    return false;
  }
  void ToggleRecordOnce(int i) {
    if (!IsRecording(i)) {
      cut.setPlayFlag(i, true);
      float pos = getLoopStart(i);
      Commands::softcutCommands.post(Commands::Id::SET_CUT_POSITION, i, pos);
      cut.setRecOnceFlag(i, true);
    } else {
      cut.setRecFlag(i, false);
    }
  }
  void TogglePlay(int i) {
    cut.setPlayFlag(i, !IsPlaying(i));
    isPrimed[i] = false;
    isPrimedToRecordOnce[i] = false;
    wasPrimed[i] = false;
    doneRecordingPrimed[i] = false;
  }
  void TogglePlay(int i, bool play) { cut.setPlayFlag(i, play); }
  float getVULevel(int voice) const {
    if (voice >= 0 && voice < NumVoices) {
      return vuMeters[voice].getLevel();
    }
    return -100.0f;  // Return silence for invalid voice
  }
  float getCPUUsage() { return static_cast<float>(jack_cpu_load(client)); }
  void SetPrimeSensitivity(int i, float sensitivity) {
    for (int j = 0; j < NumVoices; ++j) {
      primeSensitivity[j] = sensitivity;
    }
    std::cerr << "SetPrimeSensitivity: " << i << " to " << primeSensitivity[i]
              << std::endl;
  }

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
  sample_t blockRMS[NumVoices];
  bool isPrimed[NumVoices];
  bool isPrimedToRecordOnce[NumVoices];
  bool wasPrimed[NumVoices];
  bool doneRecordingPrimed[NumVoices];
  float primeSensitivity[NumVoices];

 private:
  void process(jack_nframes_t numFrames) override;
  void setSampleRate(jack_nframes_t) override;
  inline size_t secToFrame(float sec) {
    return static_cast<size_t>(sec * jack_get_sample_rate(JackClient::client));
  }
  float getLoopDuration();

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

  void copyBufferFromLoopToLoop(int loopSrc, int loopDst) {
    // total seconds
    float cutDuration = getLoopDuration();
    int bufSrc = loopSrc < 4 ? 0 : 1;
    int bufDst = loopDst < 4 ? 0 : 1;
    float startSrc = loopMin[loopSrc];
    float startDst = loopMin[loopDst];
    // generate random file name
    std::string path = "/tmp/softcut_" + std::to_string(rand()) + ".wav";
    // write buffer to file
    writeBufferMono(path, startSrc, cutDuration, bufSrc);
    // read buffer from file
    readBufferMono(path, 0.f, startDst, cutDuration, bufSrc, bufDst);
    std::cerr << "copyBufferFromLoopToLoop: " << path << std::endl;
    // delete file
    remove(path.c_str());
  }

  void dumpBufferFromLoop(int loop) {
    // total seconds
    float cutDuration = getLoopDuration();
    int bufSrc = loop < 4 ? 0 : 1;
    float startSrc = loopMin[loop];
    // create folder oooooooo if it doesn't exist
    std::string folder = "oooooooo";
    if (!std::filesystem::exists(folder)) {
      std::filesystem::create_directory(folder);
    }
    // generate random file name
    std::string path = "oooooooo/loop_" + std::to_string(loop) + ".wav";
    // write buffer to file
    writeBufferMono(path, startSrc, cutDuration, bufSrc);
    std::cerr << "dumpBufferFromLoop: " << path << std::endl;
  }

  void loadBufferToLoop(const std::string &path, int loop) {
    // total seconds
    float cutDuration = getLoopDuration();
    int bufSrc = loop < 4 ? 0 : 1;
    float startSrc = loopMin[loop];
    // read buffer from file
    readBufferMono(path, 0.f, startSrc, cutDuration, bufSrc, bufSrc);
    std::cerr << "loadBufferToLoop: " << path << std::endl;
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

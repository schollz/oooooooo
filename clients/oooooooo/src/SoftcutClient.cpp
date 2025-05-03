//
// Created by emb on 11/28/18.
//

#include "SoftcutClient.h"

#include <sndfile.hh>

#include "BufDiskWorker.h"
#include "Commands.h"

using namespace softcut_jack_osc;

// clamp unsigned int to upper bound, inclusive
static inline void clamp(size_t &x, const size_t a) {
  if (x > a) {
    x = a;
  }
}

void SoftcutClient::init() {
  // set each loop to be 2 seconds long, equally spaced across the buffer
  float totalSeconds = static_cast<float>(BufFrames) / sampleRate;
  float cutDuration = totalSeconds / static_cast<float>(NumVoices / 2);
  // initialize seed
  srand(static_cast<unsigned int>(time(nullptr)));
  for (int i = 0; i < NumVoices; ++i) {
    rateBase[i] = 1.0f;
    rateSet[i] = 1.0f;
    rateForward[i] = true;
    loopMin[i] = cutDuration * static_cast<float>(i);
    if (i >= 4) {
      loopMin[i] = cutDuration * static_cast<float>(i - 4);
    }

    // enable
    SoftcutClient::handleCommand(
        new Commands::CommandPacket(Commands::Id::SET_ENABLED_CUT, i, 1.0f));
    // set buffer to 0/1
    SoftcutClient::handleCommand(new Commands::CommandPacket(
        Commands::Id::SET_CUT_BUFFER, i, i < 4 ? 0 : 1));

    // set post filter dry to 0
    SoftcutClient::handleCommand(new Commands::CommandPacket(
        Commands::Id::SET_CUT_POST_FILTER_DRY, i, 0.0f));

    // set lp filter to 1
    SoftcutClient::handleCommand(new Commands::CommandPacket(
        Commands::Id::SET_CUT_POST_FILTER_LP, i, 1.0f));

    // set lp fc to 19kHz
    SoftcutClient::handleCommand(new Commands::CommandPacket(
        Commands::Id::SET_CUT_POST_FILTER_FC, i, 19000.0f));

    // set pan to  random number between -0.25 and 0.25
    float pan =
        (static_cast<float>(rand()) / RAND_MAX) * 1.25f - (1.25f / 2.0f);
    SoftcutClient::handleCommand(
        new Commands::CommandPacket(Commands::Id::SET_PAN_CUT, i, pan));
    // set loop to on
    SoftcutClient::handleCommand(
        new Commands::CommandPacket(Commands::Id::SET_CUT_LOOP_FLAG, i, 1.0f));

    SoftcutClient::handleCommand(new Commands::CommandPacket(
        Commands::Id::SET_CUT_LOOP_START, i, loopMin[i]));
    SoftcutClient::handleCommand(new Commands::CommandPacket(
        Commands::Id::SET_CUT_LOOP_END, i, loopMin[i] + 2.0f));

    // set play flag on
    SoftcutClient::handleCommand(
        new Commands::CommandPacket(Commands::Id::SET_CUT_PLAY_FLAG, i, 0.0f));

    // cut to the beginning
    SoftcutClient::handleCommand(new Commands::CommandPacket(
        Commands::Id::SET_CUT_POSITION, i, loopMin[i]));
  }
}

SoftcutClient::SoftcutClient() : JackClient<2, 2>("softcut") {
  for (unsigned int i = 0; i < NumVoices; ++i) {
    cut.setVoiceBuffer(i, buf[i & 1], BufFrames);

    // Initialize reverb send levels
    reverbSend[i].setTarget(0.0f);
    reverbSend[i].setTime(0.001f);
  }

  // Initialize main reverb parameters
  reverbMix.setTarget(0.5f);
  reverbMix.setTime(0.001f);
  reverbEnabled = false;

  for (unsigned int i = 0; i < NumVoices; ++i) {
    cut.setVoiceBuffer(i, buf[i & 1], BufFrames);
  }
  bufIdx[0] = BufDiskWorker::registerBuffer(buf[0], BufFrames);
  bufIdx[1] = BufDiskWorker::registerBuffer(buf[1], BufFrames);
}

void SoftcutClient::process(jack_nframes_t numFrames) {
  Commands::softcutCommands.handlePending(this);
  clearBusses(numFrames);
  mixInput(numFrames);
  // process softcuts (overwrites output bus)
  for (int v = 0; v < NumVoices; ++v) {
    if (enabled[v]) {
      cut.processBlock(v, input[v].buf[0], output[v].buf[0],
                       static_cast<int>(numFrames));
    }
  }
  mixOutput(numFrames);
  mix.copyTo(sink[0], numFrames);
}

void SoftcutClient::setSampleRate(jack_nframes_t sr) {
  sampleRate = sr;
  std::cerr << "SoftcutClient::setSampleRate: " << sr << std::endl;
  reverb.Init(static_cast<float>(sr));
  cut.setSampleRate(sr);

  for (int i = 0; i < NumVoices; ++i) {
    reverbSend[i].setSampleRate(sr);
  }
  reverbMix.setSampleRate(sr);
}

void SoftcutClient::clearBusses(size_t numFrames) {
  mix.clear(numFrames);
  for (auto &b : input) {
    b.clear(numFrames);
  }
}

void SoftcutClient::mixInput(size_t numFrames) {
  for (int dst = 0; dst < NumVoices; ++dst) {
    if (cut.getRecFlag(dst)) {
      for (int ch = 0; ch < 2; ++ch) {
        input[dst].mixFrom(&source[SourceAdc][ch], numFrames, inLevel[ch][dst]);
      }
      for (int src = 0; src < NumVoices; ++src) {
        if (cut.getPlayFlag(src)) {
          input[dst].mixFrom(output[src], numFrames, fbLevel[src][dst]);
        }
      }
    }
  }
}

void SoftcutClient::mixOutput(size_t numFrames) {
  reverbBus.clear(numFrames);

  for (int v = 0; v < NumVoices; ++v) {
    if (cut.getPlayFlag(v)) {
      mix.panMixEpFrom(output[v], numFrames, outLevel[v], outPan[v]);

      // Send to reverb bus with the same panning as the main output
      // This preserves the stereo field in the reverb
      reverbBus.panMixEpFrom(output[v], numFrames, reverbSend[v], outPan[v]);
    }
  }

  if (reverbEnabled) {
    // Process reverb (FVerb handles stereo in-place processing)
    float reverbFloat[2][MaxBlockFrames];
    for (size_t ch = 0; ch < 2; ch++) {
      for (size_t i = 0; i < numFrames; i++) {
        reverbFloat[ch][i] = static_cast<float>(reverbBus.buf[ch][i]);
      }
    }
    float *reverbInOut[2] = {reverbFloat[0], reverbFloat[1]};
    reverb.Process(reverbInOut, numFrames);
    // Convert back to double
    for (size_t ch = 0; ch < 2; ch++) {
      for (size_t i = 0; i < numFrames; i++) {
        reverbBus.buf[ch][i] = static_cast<sample_t>(reverbFloat[ch][i]);
      }
    }

    // Mix the processed reverb into the main output
    mix.addFrom(reverbBus, numFrames);
  }
}

void SoftcutClient::handleCommand(Commands::CommandPacket *p) {
  switch (p->id) {
    //-- softcut routing
    case Commands::Id::SET_ENABLED_CUT:
      enabled[p->idx_0] = p->value > 0.f;
      break;
    case Commands::Id::SET_LEVEL_CUT:
      outLevel[p->idx_0].setTarget(p->value);
      break;
    case Commands::Id::SET_PAN_CUT:
      outPan[p->idx_0].setTarget((p->value / 2) + 0.5);  // map -1,1 to 0,1
      break;
    case Commands::Id::SET_LEVEL_IN_CUT:
      inLevel[p->idx_0][p->idx_1].setTarget(p->value);
      break;
    case Commands::Id::SET_LEVEL_CUT_CUT:
      fbLevel[p->idx_0][p->idx_1].setTarget(p->value);
      break;
      //-- softcut commands
    case Commands::Id::SET_CUT_RATE:
      rateSet[p->idx_0] = p->value;
      updateRate(p->idx_0);
      break;
    case Commands::Id::SET_CUT_LOOP_START:
      cut.setLoopStart(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_LOOP_END:
      cut.setLoopEnd(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_LOOP_FLAG:
      cut.setLoopFlag(p->idx_0, p->value > 0.f);
      break;
    case Commands::Id::SET_CUT_FADE_TIME:
      cut.setFadeTime(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_REC_LEVEL:
      cut.setRecLevel(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_PRE_LEVEL:
      cut.setPreLevel(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_REC_FLAG:
      cut.setRecFlag(p->idx_0, p->value > 0.f);
      break;
    case Commands::Id::SET_CUT_PLAY_FLAG:
      cut.setPlayFlag(p->idx_0, p->value > 0.f);
      break;
    case Commands::Id::SET_CUT_REC_OFFSET:
      cut.setRecOffset(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_POSITION:
      cut.cutToPos(p->idx_0, p->value);
      break;
      // input filter
    case Commands::Id::SET_CUT_PRE_FILTER_FC:
      cut.setPreFilterFc(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_PRE_FILTER_FC_MOD:
      cut.setPreFilterFcMod(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_PRE_FILTER_RQ:
      cut.setPreFilterRq(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_PRE_FILTER_LP:
      cut.setPreFilterLp(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_PRE_FILTER_HP:
      cut.setPreFilterHp(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_PRE_FILTER_BP:
      cut.setPreFilterBp(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_PRE_FILTER_BR:
      cut.setPreFilterBr(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_PRE_FILTER_DRY:
      cut.setPreFilterDry(p->idx_0, p->value);
      break;
      // output filter
    case Commands::Id::SET_CUT_POST_FILTER_FC:
      cut.setPostFilterFc(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_POST_FILTER_RQ:
      cut.setPostFilterRq(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_POST_FILTER_LP:
      cut.setPostFilterLp(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_POST_FILTER_HP:
      cut.setPostFilterHp(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_POST_FILTER_BP:
      cut.setPostFilterBp(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_POST_FILTER_BR:
      cut.setPostFilterBr(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_POST_FILTER_DRY:
      cut.setPostFilterDry(p->idx_0, p->value);
      break;

    case Commands::Id::SET_CUT_LEVEL_SLEW_TIME:
      outLevel[p->idx_0].setTime(p->value);
      break;
    case Commands::Id::SET_CUT_PAN_SLEW_TIME:
      outPan[p->idx_0].setTime(p->value);
      break;
    case Commands::Id::SET_CUT_RECPRE_SLEW_TIME:
      cut.setRecPreSlewTime(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_RATE_SLEW_TIME:
      cut.setRateSlewTime(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_VOICE_SYNC:
      cut.syncVoice(p->idx_0, p->idx_1, p->value);
      break;
    case Commands::Id::SET_CUT_BUFFER:
      cut.setVoiceBuffer(p->idx_0, buf[p->idx_1], BufFrames);
      break;
    case Commands::Id::SET_CUT_TAPE_BIAS:
      cut.setTapeBias(p->idx_0, p->value);
      break;
    case Commands::Id::SET_CUT_TAPE_PREGAIN:
      cut.setTapePregain(p->idx_0, p->value);
      break;
    default:;
      ;
  }
}

void SoftcutClient::reset() {
  for (int v = 0; v < NumVoices; ++v) {
    cut.setVoiceBuffer(v, buf[v % 2], BufFrames);
    outLevel[v].setTarget(0.f);
    outLevel->setTime(0.001);
    outPan[v].setTarget(0.5f);
    outPan->setTime(0.001);

    // Reset reverb send levels
    reverbSend[v].setTarget(0.0f);
    reverbSend[v].setTime(0.001f);

    enabled[v] = false;

    setPhaseQuant(v, 1.f);
    setPhaseOffset(v, 0.f);

    for (int i = 0; i < 2; ++i) {
      inLevel[i][v].setTime(0.001);
      inLevel[i][v].setTarget(0.0);
    }

    for (int w = 0; w < NumVoices; ++w) {
      fbLevel[v][w].setTime(0.001);
      fbLevel[v][w].setTarget(0.0);
    }

    cut.setLoopStart(v, v * 2);
    cut.setLoopEnd(v, v * 2 + 1);

    output[v].clear();
    input[v].clear();
  }

  // Reset reverb state
  reverbEnabled = false;
  reverbMix.setTarget(0.5f);
  reverbMix.setTime(0.001f);
  reverbBus.clear();
  cut.reset();
}

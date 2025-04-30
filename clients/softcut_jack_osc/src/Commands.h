//
// Created by ezra on 11/3/18.
//

#ifndef CRONE_COMMANDS_H
#define CRONE_COMMANDS_H

// #include <boost/lockfree/spsc_queue.hpp>
#include "readerwriterqueue/readerwriterqueue.h"

namespace softcut_jack_osc {

class SoftcutClient;

class Commands {
 public:
  typedef enum {
    //-- softcut commands

    // mix
    SET_ENABLED_CUT,
    SET_LEVEL_CUT,
    SET_PAN_CUT,
    // level of individual input channel -> cut voice
    // (separate commands just to avoid a 3rd parameter)
    SET_LEVEL_IN_CUT,
    SET_LEVEL_CUT_CUT,

    // params
    SET_CUT_REC_FLAG,
    SET_CUT_PLAY_FLAG,

    SET_CUT_RATE,
    SET_CUT_LOOP_START,
    SET_CUT_LOOP_END,
    SET_CUT_LOOP_FLAG,
    SET_CUT_POSITION,

    SET_CUT_FADE_TIME,
    SET_CUT_REC_LEVEL,
    SET_CUT_PRE_LEVEL,
    SET_CUT_REC_OFFSET,

    SET_CUT_PRE_FILTER_FC,
    SET_CUT_PRE_FILTER_FC_MOD,
    SET_CUT_PRE_FILTER_RQ,
    SET_CUT_PRE_FILTER_LP,
    SET_CUT_PRE_FILTER_HP,
    SET_CUT_PRE_FILTER_BP,
    SET_CUT_PRE_FILTER_BR,
    SET_CUT_PRE_FILTER_DRY,

    SET_CUT_POST_FILTER_FC,
    SET_CUT_POST_FILTER_RQ,
    SET_CUT_POST_FILTER_LP,
    SET_CUT_POST_FILTER_HP,
    SET_CUT_POST_FILTER_BP,
    SET_CUT_POST_FILTER_BR,
    SET_CUT_POST_FILTER_DRY,

    SET_CUT_LEVEL_SLEW_TIME,
    SET_CUT_PAN_SLEW_TIME,
    SET_CUT_RECPRE_SLEW_TIME,
    SET_CUT_RATE_SLEW_TIME,
    SET_CUT_VOICE_SYNC,
    SET_CUT_BUFFER,

    SET_CUT_REC_ONCE,
    SET_CUT_TAPE_BIAS,
    SET_CUT_TAPE_PREGAIN,
    NUM_COMMANDS,
  } Id;

 public:
  Commands();
  void post(Commands::Id id, float f);
  void post(Commands::Id id, int i, float f);
  void post(Commands::Id id, int i, int j);
  void post(Commands::Id id, int i, int j, float f);

  void handlePending(SoftcutClient *client);

  struct CommandPacket {
    CommandPacket() = default;
    CommandPacket(Commands::Id i, int i0, float f)
        : id(i), idx_0(i0), idx_1(-1), value(f) {}
    CommandPacket(Commands::Id i, int i0, int i1)
        : id(i), idx_0(i0), idx_1(i1) {}
    CommandPacket(Commands::Id i, int i0, int i1, float f)
        : id(i), idx_0(i0), idx_1(i1), value(f) {}
    Id id;
    int idx_0{};
    int idx_1{};
    float value{};
  };

  static Commands softcutCommands;

 private:
  //        boost::lockfree::spsc_queue <CommandPacket,
  //                boost::lockfree::capacity<200> > q;
  moodycamel::ReaderWriterQueue<CommandPacket> q;
};

}  // namespace softcut_jack_osc

#endif  // CRONE_COMMANDS_H

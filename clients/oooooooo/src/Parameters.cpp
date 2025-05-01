#include "Parameters.h"

void Parameters::Init(SoftcutClient* sc, int voice) {
  softCutClient_ = sc;
  for (int i = 0; i < PARAM_COUNT; i++) {
    float default_value = 0.0f;
    switch (i) {
      case PARAM_LEVEL:
        default_value = (static_cast<float>(rand()) / RAND_MAX) * 38.0f -
                        32.0f;  // -32 to +6
        param_[i].Init(
            softCutClient_->getSampleRate(), -32.0, 12.0f, 0.5f, default_value,
            -12.0f, -1.0f, 0.5f, 10.0f, "Level", "dB",
            [this, voice](float value) {
              std::cout << value << " Level set to: " << db2amp(value)
                        << std::endl;
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_LEVEL_CUT, voice, db2amp(value)));
            });
        break;
      case PARAM_PAN:
        default_value =
            (static_cast<float>(rand()) / RAND_MAX) * 1.25f - (1.25f / 2.0f);
        param_[i].Init(
            softCutClient_->getSampleRate(), -1.0, 1.0f, 0.05f, default_value,
            -1.0f, 1.0f, 0.1f, 12.0f, "Pan", "", [this, voice](float value) {
              std::cout << "Pan set to: " << value << std::endl;
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_PAN_CUT, voice, value));
            });
        break;
    }
  }
}
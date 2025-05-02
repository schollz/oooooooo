#include "Parameters.h"

void Parameters::Init(SoftcutClient* sc, int voice, float sample_rate) {
  softCutClient_ = sc;
  sample_rate_ = sample_rate;
  for (int i = 0; i < PARAM_COUNT; i++) {
    float default_value = 0.0f;
    switch (i) {
      case PARAM_LEVEL:
        default_value = (static_cast<float>(rand()) / RAND_MAX) * 38.0f -
                        32.0f;  // -32 to +6
        param_[i].Init(
            sample_rate_, -32.0, 12.0f, 0.5f, default_value,
            default_value - 6.0f, default_value + 6.0f, 0.5f, 10.0f, "Level",
            "dB", [this, voice](float value) {
              std::cout << "Parameters::Init " << value
                        << " Level set to: " << db2amp(value) << std::endl;
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_LEVEL_CUT, voice, db2amp(value)));
            });
        break;
      case PARAM_PAN:
        default_value =
            (static_cast<float>(rand()) / RAND_MAX) * 1.25f - (1.25f / 2.0f);
        param_[i].Init(
            sample_rate_, -1.0, 1.0f, 0.05f, default_value, default_value - 0.1,
            default_value + 0.1, 0.1f, 12.0f, "Pan", "",
            [this, voice](float value) {
              std::cout << "Parameters::Init " << "Pan set to: " << value
                        << std::endl;
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_PAN_CUT, voice, value));
            });
        break;
      case PARAM_REVERB:
        default_value = 0;
        param_[i].Init(sample_rate_, 0.0, 1.0f, 0.01f, default_value, 0.0f,
                       0.2f, 0.1f, 10.0f, "Reverb", "",
                       [this, voice](float value) {
                         std::cout << "Parameters::Init "
                                   << "Reverb set to: " << value << std::endl;
                         softCutClient_->setReverbEnabled(true);
                         softCutClient_->setReverbSend(voice, value);
                       });
        break;
    }
  }
}
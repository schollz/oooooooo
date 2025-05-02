#include "Parameters.h"

#include "Utilities.h"

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
            sample_rate_, -32.0, 12.0f, 0.1f, default_value,
            default_value - 6.0f, default_value + 6.0f, 0.5f, 10.0f, "Level",
            "dB", [this, voice](float value) {
              std::cout << "Parameters::Init " << value
                        << " Level set to: " << db2amp(value) << std::endl;
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_LEVEL_CUT, voice, db2amp(value)));
            });
        param_[i].SetStringFunc([](float value) {
          if (value > 0)
            return sprintf_str("+%2.1f", value);
          else
            return sprintf_str("%2.1f", value);
        });
        break;
      case PARAM_PAN:
        default_value =
            (static_cast<float>(rand()) / RAND_MAX) * 1.25f - (1.25f / 2.0f);
        param_[i].Init(
            sample_rate_, -1.0, 1.0f, 0.01f, default_value,
            fclamp(default_value - 1.0f, -1.0f, 1.0f),
            fclamp(default_value + 1.0f, -1.0f, 1.0f), 0.1f, 12.0f, "Pan", "",
            [this, voice](float value) {
              std::cout << "Parameters::Init " << "Pan set to: " << value
                        << std::endl;
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_PAN_CUT, voice, value));
            });
        break;
      case PARAM_LPF:
        default_value = 135.0f;
        param_[i].Init(
            sample_rate_, 20.0f, 135.0f, 0.1f, default_value, 125.0f, 140.0f,
            0.5f, 10.0f, "LPF", "", [this, voice](float value) {
              float freq = midi2freq(value);
              std::cout << "Parameters::Init " << "LPF set to: " << freq
                        << std::endl;
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_POST_FILTER_FC, voice, freq));
            });
        param_[i].SetStringFunc([](float value) {
          float freq = midi2freq(value);
          if (freq > 1000) {
            return sprintf_str("%2.1f kHz", freq / 1000.0f);
          } else {
            return sprintf_str("%2.0f Hz", freq);
          }
        });
        break;
      case PARAM_PREGAIN:
        default_value = 0.0f;
        param_[i].Init(
            sample_rate_, -32.0, 36.0f, 0.1f, default_value,
            default_value - 6.0f, default_value + 6.0f, 0.5f, 10.0f, "PreGain",
            "dB", [this, voice](float value) {
              std::cout << "Parameters::Init " << value
                        << " PreGain set to: " << db2amp(value) << std::endl;
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_TAPE_PREGAIN, voice, db2amp(value)));
            });
        param_[i].SetStringFunc([](float value) {
          if (value > 0)
            return sprintf_str("+%2.1f", value);
          else
            return sprintf_str("%2.1f", value);
        });
        break;
      case PARAM_BIAS:
        default_value = -24.0f;
        param_[i].Init(
            sample_rate_, -32.0, 12.0f, 0.1f, default_value,
            default_value - 6.0f, default_value + 6.0f, 0.5f, 10.0f, "Bias",
            "dB", [this, voice](float value) {
              std::cout << "Parameters::Init " << value
                        << " Bias set to: " << db2amp(value) << std::endl;
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_TAPE_BIAS, voice, db2amp(value)));
            });
        param_[i].SetStringFunc([](float value) {
          if (value > 0)
            return sprintf_str("+%2.1f", value);
          else
            return sprintf_str("%2.1f", value);
        });
        break;
      case PARAM_REVERB:
        default_value = 0;
        param_[i].Init(sample_rate_, 0.0, 1.0f, 0.01f, default_value, 0.0f,
                       0.2f, 0.1f, 10.0f, "Reverb", "%",
                       [this, voice](float value) {
                         std::cout << "Parameters::Init "
                                   << "Reverb set to: " << value << std::endl;
                         softCutClient_->setReverbEnabled(true);
                         softCutClient_->setReverbSend(voice, value);
                       });
        param_[i].SetStringFunc([](float value) {
          return sprintf_str("%d", static_cast<int>(roundf(value * 100.0f)));
        });
        break;
      case PARAM_RATE:
        default_value = 1.0f;
        param_[i].Init(
            sample_rate_, 0.0, 2.0f, 0.01f, default_value, 0.99f, 1.01f, 0.1f,
            10.0f, "Rate", "", [this, voice](float value) {
              std::cout << "Parameters::Init " << "Rate set to: " << value
                        << std::endl;
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_RATE, voice, value));
            });
        break;
    }
  }
}

void Parameters::ToggleView() {
  view_visible_ = !view_visible_;
  fade_target_ = view_visible_ ? 1.0f : 0.0f;
}

void Parameters::Update() {
  UpdateFade();  // Add this line
  for (int i = 0; i < PARAM_COUNT; i++) {
    param_[i].Update();
  }
}

void Parameters::UpdateFade() {
  if (fade_amount_ != fade_target_) {
    if (fade_amount_ < fade_target_) {
      fade_amount_ = std::min(fade_amount_ + fade_speed_, fade_target_);
    } else {
      fade_amount_ = std::max(fade_amount_ - fade_speed_, fade_target_);
    }
  }
}

void Parameters::Render(SDL_Renderer* renderer, TTF_Font* font, int x, int y,
                        int width, int height, float brightness) {
  // Update fade animation
  UpdateFade();

  // Apply overall brightness (modified from original function)
  float effective_brightness = brightness * fade_amount_;

  // Don't render if completely faded out
  if (effective_brightness <= 0.0f) {
    return;
  }

  int j = 0;
  for (int i = 0; i < PARAM_COUNT; i++) {
    if (param_[i].IsHidden()) continue;
    param_[i].Render(renderer, font, x, y + j * (height + 5), width, height,
                     selected_ == i, effective_brightness);  // Pass brightness
    j++;
  }
}
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
            default_value - 6.0f, default_value + 6.0f, 0.5f, 10.0f, "level",
            "dB", [this, voice](float value) {
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
            fclamp(default_value + 1.0f, -1.0f, 1.0f), 0.1f, 12.0f, "pan", "",
            [this, voice](float value) {
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_PAN_CUT, voice, value));
            });
        break;
      case PARAM_LPF:
        default_value = 135.0f;
        param_[i].Init(
            sample_rate_, 20.0f, 135.0f, 0.1f, default_value, 125.0f, 140.0f,
            0.5f, 10.0f, "lpf", "", [this, voice](float value) {
              float freq = midi2freq(value);
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
            default_value - 6.0f, default_value + 6.0f, 0.5f, 10.0f, "pregain",
            "dB", [this, voice](float value) {
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
            default_value - 6.0f, default_value + 6.0f, 0.5f, 10.0f, "bias",
            "dB", [this, voice](float value) {
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
                       0.2f, 0.1f, 10.0f, "reverb", "%",
                       [this, voice](float value) {
                         softCutClient_->setReverbEnabled(true);
                         softCutClient_->setReverbSend(voice, value);
                       });
        param_[i].SetStringFunc([](float value) {
          return sprintf_str("%d", static_cast<int>(roundf(value * 100.0f)));
        });
        break;
      case PARAM_REVERB_DECAY:
        default_value = 82.0f;
        param_[i].Init(sample_rate_, 0.0, 100.0f, 0.1f, default_value, 80.0f,
                       90.0f, 0.5f, 10.0f, "decay", "%", [this](float value) {
                         std::cout << "decay: " << value << std::endl;
                         softCutClient_->setReverbDecay(value);
                       });
        param_[i].SetStringFunc(
            [](float value) { return sprintf_str("%2.0f", value); });
        break;
      case PARAM_REVERB_DENSITY:
        default_value = 80.0f;
        param_[i].Init(sample_rate_, 0.0, 100.0f, 0.1f, default_value, 70.0f,
                       90.0f, 0.5f, 10.0f, "density", "%", [this](float value) {
                         softCutClient_->setReverbTailDensity(value);
                       });
        param_[i].SetStringFunc(
            [](float value) { return sprintf_str("%2.0f", value); });
        break;
      case PARAM_RATE:
        default_value = 1.0f;
        param_[i].Init(
            sample_rate_, 0.0, 2.0f, 0.01f, default_value, 0.99f, 1.01f, 0.1f,
            10.0f, "rate", "", [this, voice](float value) {
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_RATE, voice, value));
            });
        break;
      case PARAM_DIRECTION:
        default_value = 0.0f;
        param_[i].Init(sample_rate_, -1.0, 1.0f, 0.1f, default_value, -0.5f,
                       0.5f, 0.1f, 10.0f, "direction", "",
                       [this, voice](float value) {
                         softCutClient_->setRateDirection(voice, value >= 0);
                       });
        param_[i].SetStringFunc([](float value) {
          if (value >= 0)
            return "fwd";
          else
            return "rev";
        });
        break;
      case PARAM_START:
        default_value = 0.0f;
        param_[i].Init(
            sample_rate_, 0.0, softCutClient_->getLoopEnd(voice), 0.01f,
            default_value, 0.0f, 0.2f, 0.1f, 10.0f, "start", "s",
            [this, voice](float value) {
              float loop_duration = softCutClient_->getDuration(voice);
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_LOOP_START, voice,
                  value + softCutClient_->getLoopMin(voice)));
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_LOOP_END, voice,
                  value + softCutClient_->getLoopMin(voice) + loop_duration));
            });
        break;
      case PARAM_DURATION:
        default_value = 2.0f;
        param_[i].Init(
            sample_rate_, 0.0, 30.0f, 0.01f, default_value,
            default_value - 1.0f, default_value + 1.0f, 0.1f, 10.0f, "duration",
            "s", [this, voice](float value) {
              float start = softCutClient_->getLoopStart(voice);
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_LOOP_END, voice, value + start));
              param_[PARAM_START].SetMax(value);
            });
        break;
      case PARAM_REC_LEVEL:
        default_value = 0.0f;
        param_[i].Init(
            sample_rate_, -48.0, 12.0f, 0.1f, default_value,
            default_value - 6.0f, default_value + 6.0f, 0.5f, 10.0f,
            "rec level", "dB", [this, voice](float value) {
              float amp = db2amp(value);
              if (value <= -42.0f) {
                amp = 0.0f;
              }
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_REC_LEVEL, voice, amp));
            });
        param_[i].SetStringFunc([](float value) {
          if (value > 0)
            return sprintf_str("+%2.1f", value);
          else
            return sprintf_str("%2.1f", value);
        });
        break;
      case PARAM_PRE_LEVEL:
        default_value = -48.0f;
        param_[i].Init(
            sample_rate_, -48.0, 12.0f, 0.1f, default_value,
            default_value - 6.0f, default_value + 6.0f, 0.5f, 10.0f,
            "rec pre level", "dB", [this, voice](float value) {
              float amp = db2amp(value);
              if (value <= -42.0f) {
                amp = 0.0f;
              }
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_PRE_LEVEL, voice, amp));
            });
        param_[i].SetStringFunc([](float value) {
          if (value > 0)
            return sprintf_str("+%2.1f", value);
          else
            return sprintf_str("%2.1f", value);
        });
        break;
      case PARAM_REC_SLEW:
        default_value = 0.2f;  // seconds
        param_[i].Init(
            sample_rate_, 0.0, 4.0f, 0.01f, default_value, 0.0f, 1.0f, 0.1f,
            10.0f, "rec slew", "", [this, voice](float value) {
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_RECPRE_SLEW_TIME, voice, value));
            });
        param_[i].SetStringFunc([](float value) {
          if (value > 1.0f)
            return sprintf_str("%2.1f s", value);
          else
            return sprintf_str("%2.0f ms", value * 1000.0f);
        });
        break;
      case PARAM_LEVEL_SLEW:
        default_value = 0.2f;  // seconds
        param_[i].Init(
            sample_rate_, 0.0, 4.0f, 0.01f, default_value, 0.0f, 1.0f, 0.1f,
            10.0f, "level slew", "", [this, voice](float value) {
              std::cout << "Parameters::Init " << "Level Slew set to: " << value
                        << std::endl;
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_LEVEL_SLEW_TIME, voice, value));
            });
        param_[i].SetStringFunc([](float value) {
          if (value > 1.0f)
            return sprintf_str("%2.1f s", value);
          else
            return sprintf_str("%2.0f ms", value * 1000.0f);
        });
        break;
      case PARAM_RATE_SLEW:
        default_value = 0.2f;  // seconds
        param_[i].Init(
            sample_rate_, 0.0, 4.0f, 0.01f, default_value, 0.0f, 1.0f, 0.1f,
            10.0f, "rate slew", "", [this, voice](float value) {
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_RATE_SLEW_TIME, voice, value));
            });
        param_[i].SetStringFunc([](float value) {
          if (value > 1.0f)
            return sprintf_str("%2.1f s", value);
          else
            return sprintf_str("%2.0f ms", value * 1000.0f);
        });
        break;
      case PARAM_PAN_SLEW:
        default_value = 0.2f;  // seconds
        param_[i].Init(
            sample_rate_, 0.0, 4.0f, 0.01f, default_value, 0.0f, 1.0f, 0.1f,
            10.0f, "pan slew", "", [this, voice](float value) {
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_PAN_SLEW_TIME, voice, value));
            });
        param_[i].SetStringFunc([](float value) {
          if (value > 1.0f)
            return sprintf_str("%2.1f s", value);
          else
            return sprintf_str("%2.0f ms", value * 1000.0f);
        });
        break;
      case PARAM_FADE_TIME:
        default_value = 0.2f;  // seconds
        param_[i].Init(
            sample_rate_, 0.0, 4.0f, 0.01f, default_value, 0.0f, 1.0f, 0.1f,
            10.0f, "fade time", "", [this, voice](float value) {
              softCutClient_->handleCommand(new Commands::CommandPacket(
                  Commands::Id::SET_CUT_FADE_TIME, voice, value));
            });
        param_[i].SetStringFunc([](float value) {
          if (value > 1.0f)
            return sprintf_str("%2.1f s", value);
          else
            return sprintf_str("%2.0f ms", value * 1000.0f);
        });
        break;
      case PARAM_LOOP1_FEEDBACK:
      case PARAM_LOOP2_FEEDBACK:
      case PARAM_LOOP3_FEEDBACK:
      case PARAM_LOOP4_FEEDBACK:
      case PARAM_LOOP5_FEEDBACK:
      case PARAM_LOOP6_FEEDBACK:
      case PARAM_LOOP7_FEEDBACK:
      case PARAM_LOOP8_FEEDBACK:
        default_value = 0.0f;
        int srcVoice = i - PARAM_LOOP1_FEEDBACK;
        int destVoice = voice;
        param_[i].Init(
            sample_rate_, 0.0, 1.0f, 0.01f, default_value, 0.0f, 1.0f, 0.1f,
            10.0f, sprintf_str("loop %d fb", srcVoice + 1), "%",
            [this, srcVoice, destVoice](float value) {
              std::cout << "Parameters::Init " << "Feedback set to: " << value
                        << " from voice: " << srcVoice
                        << " to voice: " << destVoice << std::endl;
              // softCutClient_->handleCommand(new
              // Commands::CommandPacket(
              //     Commands::Id::SET_CUT_LEVEL_CUT_CUT,
              //     voice + i - PARAM_LOOP1_FEEDBACK, value));
            });
        param_[i].SetStringFunc([](float value) {
          return sprintf_str("%d", static_cast<int>(roundf(value * 100.0f)));
        });
        break;
    }
  }
  view_visible_ = false;
  fade_amount_ = 0.0f;
  fade_target_ = 0.0f;
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
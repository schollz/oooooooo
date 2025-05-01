#ifndef LIB_PARAMETERS_H
#define LIB_PARAMETERS_H
#pragma once

#include "DrawFunctions.h"
#include "Parameter.h"
#include "Serializable.h"
#include "SoftcutClient.h"

using namespace softcut_jack_osc;

class Parameters : public Serializable {
 public:
  // Parameters
  enum ParameterName {
    PARAM_LEVEL,
    PARAM_PAN,
    PARAM_COUNT  // Holds the number of parameters
  };
  Parameter param_[PARAM_COUNT];

  Parameters() = default;
  ~Parameters() = default;
  JSON toJSON() const override {
    JSON json;
    for (int i = 0; i < PARAM_COUNT; i++) {
      json[param_[i].Name()] = param_[i].toJSON();
    }
    return json;
  }
  void fromJSON(const JSON& json) override {
    for (int i = 0; i < PARAM_COUNT; i++) {
      param_[i].fromJSON(json[param_[i].Name()]);
    }
  }

  void Init(SoftcutClient* sc, int voice);

  void ValueDelta(ParameterName p, float delta) { param_[p].ValueDelta(delta); }

 private:
  SoftcutClient* softCutClient_ = nullptr;
};

#endif
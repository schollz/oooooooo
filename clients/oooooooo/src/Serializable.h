// Add to a new file like "Serializable.h"
#pragma once
#include "JSON.hpp"
using JSON = nlohmann::json;

class Serializable {
 public:
  virtual ~Serializable() = default;
  virtual JSON toJSON() const = 0;
  virtual void fromJSON(const JSON& json) = 0;
};
// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#pragma once

#include "MultiType.hpp"

namespace Variants {

// A key value pair for JsonObject.
struct JsonPair {
  const char* key;
  MultiType value;
};
}

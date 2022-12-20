// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#pragma once

#include "VariantFloat.hpp"
#include "VariantInteger.hpp"

namespace Variants {

namespace Internals {
// A union that defines the actual content of a MultiType.
// The enum JsonVariantType determines which member is in use.
union VariantContent {
  VariantFloat asFloat;     // used for double and float
  VariantUInt asUnsInteger;    // used for bool, char, short, int and longs
  VariantInteger asInteger;    // used for bool, char, short, int and longs
  bool asBool;
  char* asString;  // asString can be null
};
}
}

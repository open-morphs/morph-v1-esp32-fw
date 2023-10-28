// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#pragma once

namespace Variants {


namespace Internals {

// Enumerated type to know the current type of a MultiType.
// The value determines which member of VariantContent is used.
enum JsonVariantType {
  JSON_UNDEFINED,         // MultiType has not been initialized
  JSON_UNPARSED,          // MultiType contains an unparsed string
  JSON_STRING,            // MultiType stores a const char*
  JSON_BOOLEAN,           // MultiType stores a bool
  JSON_INTEGER,  // MultiType stores an VariantUInt
  JSON_POSITIVE_INTEGER,  // MultiType stores an VariantUInt
  JSON_FLOAT              // MultiType stores a VariantFloat
};
}
}

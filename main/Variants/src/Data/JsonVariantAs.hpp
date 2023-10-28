// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#pragma once

namespace Variants {
namespace Internals {

// A metafunction that returns the type of the value returned by
// MultiType::as<T>()
template <typename T>
struct JsonVariantAs {
  typedef T type;
};

template <>
struct JsonVariantAs<char*> {
  typedef const char* type;
};

template <>
struct JsonVariantAs<JsonArray> {
  typedef JsonArray& type;
};

template <>
struct JsonVariantAs<const JsonArray> {
  typedef const JsonArray& type;
};

}
}

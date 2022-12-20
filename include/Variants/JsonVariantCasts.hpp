// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#pragma once

#include "../ArduinoJson-v6.18.4.h"
#include "Data/JsonVariantAs.hpp"
#include "Polyfills/attributes.hpp"

namespace Variants {
namespace Internals {

template <typename TImpl>
class JsonVariantCasts {
 public:

  // Gets the variant as an array.
  // Returns a reference to the JsonArray or JsonArray::invalid() if the
  // variant
  // is not an array.
  FORCE_INLINE operator JsonArray &() const {
    return impl()->template as<JsonArray &>();
  }

 

  template <typename T>
  FORCE_INLINE operator T() const {
    return impl()->template as<T>();
  }

 private:
  const TImpl *impl() const {
    return static_cast<const TImpl *>(this);
  }
};
}
}

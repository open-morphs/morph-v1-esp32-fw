// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#pragma once

#include "Data/JsonVariantAs.hpp"
#include "Polyfills/attributes.hpp"
#include "StringTraits/StringTraits.hpp"
#include "TypeTraits/EnableIf.hpp"

namespace Variants {
namespace Internals {

// Forward declarations.
class JsonArraySubscript;
template <typename TKey>
class JsonObjectSubscript;

template <typename TImpl>
class JsonVariantSubscripts {
 public:
  // Mimics an array or an object.
  // Returns the size of the array or object if the variant has that type.
  // Returns 0 if the variant is neither an array nor an object
  size_t size() const {
    return 0;
  }

  // Mimics an array.
  // Returns the element at specified index if the variant is an array.
  // Returns MultiType::invalid() if the variant is not an array.
  FORCE_INLINE const JsonArraySubscript operator[](size_t index) const;
  FORCE_INLINE JsonArraySubscript operator[](size_t index);

  // Mimics an object.
  // Returns the value associated with the specified key if the variant is
  // an object.
  // Return MultiType::invalid() if the variant is not an object.


 private:
  const TImpl *impl() const {
    return static_cast<const TImpl *>(this);
  }
};
}
}

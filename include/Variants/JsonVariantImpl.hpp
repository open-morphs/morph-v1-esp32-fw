// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#pragma once

#include "Configuration.hpp"
#include "MultiType.hpp"
#include "Polyfills/isFloat.hpp"
#include "Polyfills/isInteger.hpp"
#include "Polyfills/parseFloat.hpp"
#include "Polyfills/parseInteger.hpp"

#include <string.h>  // for strcmp

namespace Variants {

template <typename T>
inline T MultiType::variantAsInteger() const {
  using namespace Internals;
  switch (_type) {
    case JSON_UNDEFINED:
      return 0;
    case JSON_POSITIVE_INTEGER:
      return T(_content.asUnsInteger);
    break;
    case JSON_BOOLEAN:
      return T(_content.asBool);
    case JSON_INTEGER:
      return T(_content.asInteger);
    case JSON_STRING:
    case JSON_UNPARSED:
      return parseInteger<T>(_content.asString);
    default:
      return T(_content.asFloat);
  }
}

inline const char *MultiType::variantAsString() const {
  using namespace Internals;
  if (_type == JSON_UNPARSED && _content.asString &&
      !strcmp("null", _content.asString))
    return NULL;
  if (_type == JSON_STRING || _type == JSON_UNPARSED) return _content.asString;
  return NULL;
}

template <typename T>
inline T MultiType::variantAsFloat() const {
  using namespace Internals;
  switch (_type) {
    case JSON_UNDEFINED:
      return 0;
    case JSON_POSITIVE_INTEGER:
    case JSON_BOOLEAN:
      return static_cast<T>(_content.asUnsInteger);
    case JSON_INTEGER:
      return static_cast<T>(_content.asInteger);
    case JSON_STRING:
    case JSON_UNPARSED:
      return parseFloat<T>(_content.asString);
    default:
      return static_cast<T>(_content.asFloat);
  }
}

inline bool MultiType::variantIsBoolean() const {
  using namespace Internals;
  if (_type == JSON_BOOLEAN) return true;

  if (_type != JSON_UNPARSED || _content.asString == NULL) return false;

  return !strcmp(_content.asString, "true") ||
         !strcmp(_content.asString, "false");
}

inline bool MultiType::variantIsInteger() const {
  using namespace Internals;

  return _type == JSON_POSITIVE_INTEGER || _type == JSON_INTEGER ||
         (_type == JSON_UNPARSED && isInteger(_content.asString));
}

inline bool MultiType::variantIsFloat() const {
  using namespace Internals;

  return _type == JSON_FLOAT || _type == JSON_POSITIVE_INTEGER ||
         _type == JSON_INTEGER ||
         (_type == JSON_UNPARSED && isFloat(_content.asString));
}

#if ARDUINOJSON_ENABLE_STD_STREAM
inline std::ostream &operator<<(std::ostream &os, const MultiType &source) {
  return source.printTo(os);
}
#endif

}  // namespace Variants

// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#pragma once

#include "../Configuration.hpp"

namespace Variants {
namespace Internals {

#if ARDUINOJSON_USE_DOUBLE
typedef double VariantFloat;
#else
typedef float VariantFloat;
#endif
}
}

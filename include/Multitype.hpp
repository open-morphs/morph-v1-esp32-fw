#pragma once

#include "TypeTraits.hpp"
#include <string>


enum class eMultiType {
  TYPE_UNDEFINED,         // MultiType has not been initialized
  TYPE_STRING,            // MultiType stores a const char*
  TYPE_BOOLEAN,           // MultiType stores a bool
  TYPE_INTEGER,           // MultiType stores an int32_t
  TYPE_POSITIVE_INTEGER,  // MultiType stores an uint32_t
  JSON_FLOAT              // MultiType stores a float
};

union MultitypeContent {
  float asFloat;            
  uint32_t asUnsInteger;
  int32_t asInteger;    
  bool asBool;
  char* asString;
};

struct Multitype() {

    template <typename T>
    const typename EnableIf<Internals::IsIntegral<T>::value, T>::type
    as() const {
        return variantAsInteger<T>();
    }

  template <typename T>
  T variantAsInteger() {
    switch (_type) {
      case static_cast<int>(eMultiType::TYPE_UNDEFINED):
        return 0;
      case static_cast<int>(eMultiType::TYPE_POSITIVE_INTEGER):
        return T(_content.asUnsInteger);
      break;
      case static_cast<int>(eMultiType::TYPE_BOOLEAN):
        return T(_content.asBool);
      case static_cast<int>(eMultiType::TYPE_INTEGER):
        return T(_content.asInteger);
      case static_cast<int>(eMultiType::TYPE_STRING):
        try
        {
          auto integer = std::stoi(_content.asString);
          return integer;
        }
        catch(const std::exception& e)
        {
          std::cout << e.what() << '\n';
        }
      default:
        return T(_content.asFloat);
    }
  }

    private:
        eMultiType _type;
        MultitypeContent _data;
        size_t _stringLength;
};
// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "Data/VariantContent.hpp"
#include "Data/JsonVariantDefault.hpp"
#include "Data/JsonVariantType.hpp"
#include "JsonVariantBase.hpp"
#include "Serialization/JsonPrintable.hpp"
#include "TypeTraits/EnableIf.hpp"
#include "TypeTraits/IsChar.hpp"
#include "TypeTraits/IsFloatingPoint.hpp"
#include "TypeTraits/IsIntegral.hpp"
#include "TypeTraits/IsSame.hpp"
#include "TypeTraits/IsSignedIntegral.hpp"
#include "TypeTraits/IsUnsignedIntegral.hpp"
#include "TypeTraits/RemoveConst.hpp"
#include "TypeTraits/RemoveReference.hpp"

namespace Variants {

class MultiType : public Internals::JsonVariantBase<MultiType>{
  template <typename Print>
  friend class Internals::JsonSerializer;

  protected:
    explicit MultiType(const char *key)
      : _type (Internals::JsonVariantType::JSON_UNDEFINED) {
        stringSize = 0;
        keyName = key;
        _content= {};
    }
  public:
    explicit MultiType(Internals::JsonVariantType const type, uint8_t strSize, const char * key) : MultiType(key) {
        _type = type;
        _content.asString = new char[strSize];
        stringSize = strSize;
    }
    ~MultiType() {
      if (NULL != _content.asString and stringSize > 0) {
        delete[] _content.asString;
      }
    }

    explicit MultiType(Internals::JsonVariantType type, const char *key) : MultiType(key) {
        _type = type;
    }

    Internals::JsonVariantType getType()const{
        return _type;
    }

    template <typename T>
    const typename Internals::EnableIf<Internals::IsIntegral<T>::value, T>::type
    as() const {
        return variantAsInteger<T>();
    }
    // bool as<bool>() const
    template <typename T>
    const typename Internals::EnableIf<Internals::IsSame<T, bool>::value, T>::type
    as() const {
        return variantAsInteger<int>() != 0;
    }
    //
    // double as<double>() const;
    // float as<float>() const;
    template <typename T>
    const typename Internals::EnableIf<Internals::IsFloatingPoint<T>::value,
                                      T>::type
    as() const {
        return variantAsFloat<T>();
    }
    //
    // const char* as<const char*>() const;
    // const char* as<char*>() const;
    template <typename T>
    typename Internals::EnableIf<Internals::IsSame<T, const char *>::value ||
                                    Internals::IsSame<T, char *>::value,
                                const char *>::type
    as() const {
        return variantAsString();
    }
    
    //
    // std::string as<std::string>() const;
    // String as<String>() const;
    template <typename T>
    typename Internals::EnableIf<Internals::StringTraits<T>::has_append, T>::type
    as() const {
        const char *cstr = variantAsString();
        if (cstr) return T(cstr);
        T s;
        printTo(s);
        return s;
    }

    //
    // MultiType as<MultiType> const;
    template <typename T>
    typename Internals::EnableIf<Internals::IsSame<T, MultiType>::value,
                                T>::type
    as() const {
        return *this;
    }

    // Tells weither the variant has the specified type.
    // Returns true if the variant has type type T, false otherwise.
    //
    // bool is<char>() const;
    // bool is<signed char>() const;
    // bool is<signed short>() const;
    // bool is<signed int>() const;
    // bool is<signed long>() const;
    // bool is<unsigned char>() const;
    // bool is<unsigned short>() const;
    // bool is<unsigned int>() const;
    // bool is<unsigned long>() const;
    template <typename T>
    typename Internals::EnableIf<Internals::IsIntegral<T>::value, bool>::type is()const {
        return variantIsInteger();
    }
    //
    // bool is<double>() const;
    // bool is<float>() const;
    template <typename T>
    typename Internals::EnableIf<Internals::IsFloatingPoint<T>::value, bool>::type
    is() const {
        return variantIsFloat();
    }
    //
    // bool is<bool>() const
    template <typename T>
    typename Internals::EnableIf<Internals::IsSame<T, bool>::value, bool>::type
    is() const {
        return variantIsBoolean();
    }
    //
    // bool is<const char*>() const;
    // bool is<char*>() const;
    // bool is<std::string>() const;
    template <typename T>
    typename Internals::EnableIf<Internals::IsSame<T, const char *>::value ||
                                    Internals::IsSame<T, char *>::value ||
                                    Internals::StringTraits<T>::has_append,
                                bool>::type
    is() const {
        return variantIsString();
    }

    size_t size()
    {
        return static_cast<size_t>(stringSize);
    }


    template<typename T>
    MultiType& operator=(T *value)
    {
        set<T>(value);
        return *this;
    }

    template<typename T>
    MultiType& operator=(const T &value)
    {
      set<T>(value);
      return *this;
    }

    template<typename T>
    void operator++(T)
    {
      switch (_type)
      {
        case Internals::JSON_INTEGER:
          _content.asInteger++;
          setChangedFlag();
        break;
        
        case Internals::JSON_POSITIVE_INTEGER:
          _content.asUnsInteger++;
          setChangedFlag();
        break;
      
        default:
          break;
      }
    }

    template<typename TChar>
    void set(TChar *value, typename Internals::EnableIf<Internals::IsChar<TChar>::value>::type * = 0)
    {
      if (Internals::JSON_STRING != _type){
        return;
      }
      if(strcmp(_content.asString, value) != 0) 
      { 
        strlcpy(_content.asString, value, stringSize);
        setChangedFlag();
      }
    }

    template<typename T>
    void set(T value, typename Internals::EnableIf<Internals::IsSignedIntegral<T>::value ||
                                    Internals::IsUnsignedIntegral<T>::value || Internals::IsSame<T, bool>::value>::type * = 0)
    {
      using namespace Internals;
      switch (_type)
      {
        case Internals::JSON_POSITIVE_INTEGER:
          if(_content.asUnsInteger != static_cast<VariantUInt>(value))
          {
            _content.asUnsInteger = static_cast<VariantUInt>(value);
            setChangedFlag();
          }
          break;

        case Internals::JSON_INTEGER:
          if(_content.asInteger != static_cast<VariantInteger>(value))
          {
            _content.asInteger = static_cast<VariantInteger>(value);
            setChangedFlag();
          }
          break;  

        case Internals::JSON_BOOLEAN:
          if(_content.asBool != static_cast<bool>(value))
          {
            _content.asBool = static_cast<bool>(value);
            setChangedFlag();
          }
          break;
      
        default:
          break;
      }
    }


    // Returns true if the variant has a value
    bool success() const {
      return _type != Internals::JSON_UNDEFINED;
    }

  private:
    const char *variantAsString() const;
    template <typename T>
    T variantAsFloat() const;
    template <typename T>
    T variantAsInteger() const;
    bool variantIsBoolean() const;
    bool variantIsFloat() const;
    bool variantIsInteger() const;

    bool variantIsString() const {
      return _type == Internals::JSON_STRING ||
            (_type == Internals::JSON_UNPARSED && _content.asString &&
              !strcmp("null", _content.asString));
    }

    // The current type of the variant
    Internals::JsonVariantType _type;

    // The various alternatives for the value of the variant.
    Internals::VariantContent _content;

    uint8_t stringSize;
  };

}  // namespace Variants

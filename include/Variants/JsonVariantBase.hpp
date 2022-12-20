// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#pragma once

#include "JsonVariantCasts.hpp"
#include "JsonVariantComparisons.hpp"
#include "JsonVariantOr.hpp"
#include "JsonVariantSubscripts.hpp"
#include "Serialization/JsonPrintable.hpp"

namespace Variants {
namespace Internals {

template <typename TImpl>
class JsonVariantBase : public JsonPrintable<TImpl>,
                        public JsonVariantCasts<TImpl>,
                        public JsonVariantComparisons<TImpl>,
                        public JsonVariantOr<TImpl>,
                        public JsonVariantSubscripts<TImpl>,
                        public JsonVariantTag 
{
    private:

    enum flags
    {
        VALUE_CHANGED = 0,
        STORAGE_LOCATION = 1
    };

    enum VariantType
    {
        UNDEFINED = 0,
        P_INTEGER,
        N_INTEGER,
        STRING,
    };

    public:
    JsonVariantBase()
                    : flags(0) {

    }
    enum storageLoc
    {
        FILE_SYSTEM = false,
        RAM = true
    };

    protected:
        const char *keyName;
        uint8_t flags;
    
    public:
        const char *getKeyName(void) const {return keyName;}
        bool wasChanged(void) const{return flags & (1 << VALUE_CHANGED);}
        void setChangedFlag(void) {flags |= (1 << VALUE_CHANGED);}
        void clearChangedFlag(void) {flags &= ~(1 << VALUE_CHANGED);}
        bool getStorageLoc(void) {return flags & (1 << STORAGE_LOCATION);}
        void setStorageLoc(storageLoc location) {
        switch (location)
        {
            case FILE_SYSTEM:
            flags &= ~(1 << STORAGE_LOCATION);          
            break;

            case RAM:
            flags |= (1 << STORAGE_LOCATION);                    
            break;  

            default:
            break;
        }
        }
};
}
}

#pragma once

#include "stdint.h"
#include <algorithm>
#include "attributes.h"
#include "GAIACommands.hpp"
#include "GAIAFeatures.hpp"
#include "GAIANotifications.hpp"
#include "MorphCommands.hpp"
#include "MorphFeatures.hpp"
#include "MorphNotifications.hpp"

#define PDU_PACKET_LENGTH 4UL

struct GaiaVendor_t {
    uint8_t firstByte;
    uint8_t secondByte;

    bool operator==(const GaiaVendor_t &other) const {
        return other.firstByte == this->firstByte and other.secondByte == this->secondByte;
    }
};

enum ePduTypes : uint8_t {
    PDU_TYPE_COMMAND,
    PDU_TYPE_NOTIFICATION,
    PDU_TYPE_RESPONSE,
    PDU_TYPE_ERROR,
    PDU_TYPE_MAX
};

class PduPacket {

public:
    PduPacket()
            : _vendor{}, _featureID{}, _pduType{}, _specID{} {

    }

    explicit PduPacket(const GaiaVendor_t &vendor, const uint8_t feature,
                       const ePduTypes pduType, const uint8_t cmd)
            : _vendor{vendor}, _featureID{feature}, _pduType{pduType}, _specID{cmd} {

    }

    void toBytesArray(uint8_t *arr) const {

        arr[0] = _vendor.firstByte;
        arr[1] = _vendor.secondByte;

        const uint8_t pduTypeFirstBit = (static_cast<uint8_t>(_pduType) / 2) & 0x01;
        const uint8_t pduTypeSecondBit = static_cast<uint8_t>(_pduType) & 0x01;

        arr[2] = _featureID * 2 | pduTypeFirstBit;
        arr[3] = (_specID & 0x7f) | (pduTypeSecondBit ? (1UL << 8UL) : 0x00);
    }

    void fillFromArray(const uint8_t *arr) {
        _vendor.firstByte = arr[0];
        _vendor.secondByte = arr[1];

        _featureID = arr[2] >> 1;
        _specID = arr[3] & 0x7f;

        const uint8_t pduType = (arr[2] & 0x01) << 1 | ((arr[3] >> 7) & 0x01);
        _pduType = static_cast<ePduTypes>(pduType);
    }

ATTRIBUTE(GaiaVendor_t, vendor);
ATTRIBUTE(uint8_t, featureID);
ATTRIBUTE(ePduTypes, pduType);
ATTRIBUTE(uint8_t, specID);
};

#pragma once

#include <string.h>
#include <string>
#include "stdint.h"
#include "attributes.h"
#include "SingletonTemplate.hpp"
#include <WiFiUdp.h>
#include <arpa/inet.h>
#include "AncModes.hpp"
#include "PairedDevice.hpp"
#include <map>
#include <set>
#include "Utils/ThreadSafeDbg.hpp"

enum class ePrimaryEarbud {
    EARBUD_LEFT,
    EARBUD_RIGHT,
    EARBUD_UNKNOWN
};

class MorphDevice : public Singleton<MorphDevice> {
    friend class Singleton;
    ATTRIBUTE(bool, anc);
    ATTRIBUTE(uint8_t, volume);
    ATTRIBUTE(bool, micMuteStatus);
    ATTRIBUTE(uint8_t, chargeLeft);
    ATTRIBUTE(uint8_t, chargeRight);
    ATTRIBUTE(std::string, handsetName);
    ATTRIBUTE(eAncMode, ancMode);
    ATTRIBUTE(bool, handsetConnected);
    ATTRIBUTE(uint64_t, handsetMac);
    ATTRIBUTE(uint32_t, btProfileMask);
    ATTRIBUTE(ePrimaryEarbud, primaryEarbud);

    public:
        std::set<PairedDevice> pairedDeviceList() const {
            return _pairedDeviceList;
        }

        void loadPairedDeviceListFromFlash();
        void savePairedDeviceListToFlash();
        bool addPairedDevice(PairedDevice device);
        bool isDeviceHandset(const PairedDevice& device) const;
        void clearPairedDevicesList() {
            _pairedDeviceList.clear();
        }

        void deletePairedDevice(const uint64_t mac);
        bool deleteDevicesOutOfSet(const std::set<uint64_t>& srcSet);

    private:
        MorphDevice() : _anc{}, _volume{}, _micMuteStatus{}, _chargeLeft{},
            _chargeRight{}, _handsetName{}, _ancMode{}, _handsetConnected{}, _handsetMac{},
            _primaryEarbud{ePrimaryEarbud::EARBUD_UNKNOWN} {

        }
    private:
        std::set<PairedDevice> _pairedDeviceList;
};

#define MORPH_DEVICE MorphDevice::instance()

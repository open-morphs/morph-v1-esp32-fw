#include <MorphDevice.hpp>
#include <Utils/ThreadSafeDbg.hpp>
#include <ArduinoJson-v6.18.4.h>
#include <NVS.hpp>

static const char* const pairedDeviceStorage{"pairedDevices"};
static const char* const devicesNameJsonKey{"name"};
static const char* const devicesMacJsonKey{"mac"};
static const char* const devicesBtProfileMaskJsonKey{"btMask"};

void MorphDevice::loadPairedDeviceListFromFlash() {

    std::string content{};

    NVS nvs;

    bool readRes{nvs.read(pairedDeviceStorage, "devices", content)};

    if (!readRes) {
        DBG_PRINTLN("failed to load file with paired devices!");
        return;
    }

    DynamicJsonDocument doc(MAX_JSONDOC_SIZE);

    const DeserializationError err {deserializeJson(doc, content)};

    if (DeserializationError::Ok != err) {
        DBG_PRINTF("failed to deserialize json! Error: %s\r\n", err.c_str());
        return;
    }   

    JsonArray arr = doc.as<JsonArray>();

    for (auto obj : arr) {
        if (!obj.is<JsonObject>()) {
            DBG_PRINTLN("is not JsonObject!");
            return;
        }
        if (!obj.containsKey(devicesNameJsonKey) or !obj.containsKey(devicesMacJsonKey) or
            !obj.containsKey(devicesBtProfileMaskJsonKey)) {
            DBG_PRINTLN("keys not found!");
            return;
        }
        if (!obj[devicesNameJsonKey].is<char*>() or !obj[devicesMacJsonKey].is<uint64_t>() or
            !obj[devicesBtProfileMaskJsonKey].is<uint32_t>()) {
            DBG_PRINTLN("keys invalid types!");
            return;
        }
    }

    _pairedDeviceList.clear();
    for (auto obj : arr) {
        PairedDevice device {obj[devicesMacJsonKey], obj[devicesBtProfileMaskJsonKey], {}};
        std::string name{obj[devicesNameJsonKey].as<char*>()};
        memcpy(device.name, name.c_str(), name.length() > MAX_DEVICE_NAME_LENGTH ? MAX_DEVICE_NAME_LENGTH : name.length());
        _pairedDeviceList.insert(device);
    }
}

void MorphDevice::savePairedDeviceListToFlash() {

    DynamicJsonDocument doc(MAX_JSONDOC_SIZE);
    NVS nvs;

    JsonArray arr {doc.to<JsonArray>()};

    for (auto device : _pairedDeviceList) {
        JsonObject obj = arr.createNestedObject();
        obj[devicesMacJsonKey] = device.macAddress;
        obj[devicesNameJsonKey] = device.name;
        obj[devicesBtProfileMaskJsonKey] = device.btProfileMask;
    }

    std::string serialized {};

    serializeJson(doc, serialized);

    const bool saveRes {nvs.write(pairedDeviceStorage, "devices", serialized.c_str())};

    if (!saveRes) {
        DBG_PRINTLN("failed to save file to NVS!");
        return;
    }
}

void MorphDevice::deletePairedDevice(const uint64_t mac) {

    PairedDevice tmp {mac};
    std::set<PairedDevice>::iterator it = _pairedDeviceList.find(tmp);
    if (it != _pairedDeviceList.end()) {
        _pairedDeviceList.erase(it);
        DBG_PRINTF("paired device deleted!\r\n");
        savePairedDeviceListToFlash();
    }
    else {
        DBG_PRINTF("deleting device was not found\r\n");
    }
}

bool MorphDevice::deleteDevicesOutOfSet(const std::set<uint64_t>& srcSet) {

    bool wasDeviceDeleted {};
    const auto internalSet {_pairedDeviceList};
    for (auto deviceFromInnerSet : internalSet) {
        bool deviceFound {false};

        for (auto& macFromSrcSet : srcSet) {
            if (deviceFromInnerSet.macAddress == macFromSrcSet) {
                deviceFound = true;
                break;
            }
        }

        if (!deviceFound) {
            deletePairedDevice(deviceFromInnerSet.macAddress);
            wasDeviceDeleted = true;
        }
    }

    return wasDeviceDeleted;
}

bool MorphDevice::addPairedDevice(PairedDevice device) {

    if (false == isDeviceHandset(device)) {
        return false;
    }

    if (!strlen(device.name)) {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(&device.macAddress);
        sprintf(device.name, "%02X:%02X:%02X:%02X:%02X:%02X", p[5], p[4], p[3], p[2], p[1], p[0]);
    }

    const bool addRes {_pairedDeviceList.insert(device).second};
    if (addRes) {
        savePairedDeviceListToFlash();
    }
    else {
        std::set<PairedDevice>::iterator it = _pairedDeviceList.find(device);
        if (it != _pairedDeviceList.end()) {
            if (strcmp(it->name, device.name) != 0) {
                DBG_PRINTF("changing device name: %s to: %s\r\n", it->name, device.name);
                strcpy(const_cast<char*>(it->name), device.name);
                savePairedDeviceListToFlash();
            }
            if (it->btProfileMask != device.btProfileMask) {
                DBG_PRINTF("changing device btProfileMask: %u to: %u\r\n", it->btProfileMask, device.btProfileMask);
                const_cast<uint32_t&>(it->btProfileMask) = device.btProfileMask;
                savePairedDeviceListToFlash();
            }
        }
    }
    return true;
}

bool MorphDevice::isDeviceHandset(const PairedDevice& device) const {
    return device.btProfileMask != 0;
}

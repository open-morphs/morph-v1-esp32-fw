#include "Settings.hpp"
#include "MessageDispatcher.hpp"
#include "Utils/threadSafeDbg.hpp"
#include "NVS.hpp"
#include "BLEAddress.h"

static const char* const storageNamespace = "settings";
static const char* const TAG = "SETTINGS";
static const char* const defaultMac {"00:00:00:00:00:00"};

std::vector<MultiType*> _container;

Settings::Settings()
                : _fwVersion(Internals::JSON_STRING, 255, "fw"), 
                  _hw(Internals::JSON_STRING, 255, "hw"),
                  _displayOffPeriod(Internals::JSON_POSITIVE_INTEGER, "dsp_off_per"),
                  _unlockEnable(Internals::JSON_BOOLEAN, "lck_en"),
                  _bondedMac(Internals::JSON_STRING, 255, "mac"),
                  _bondedMacType(Internals::JSON_POSITIVE_INTEGER, "mac_t"),
                  _wifiSsid(Internals::JSON_STRING, 255, "ssid"),
                  _wifiPass(Internals::JSON_STRING, 255, "pass"),
                  _isBleMacDefault(Internals::JSON_BOOLEAN, "mac_default"),
                  _shutDownPeriod(Internals::JSON_POSITIVE_INTEGER, "shutdown_per"),
                  _language(Internals::JSON_POSITIVE_INTEGER, "lang"),
                  _isBeepFeatureEnabled(Internals::JSON_BOOLEAN, "beep_feature"),
                  _screenlockImage(Internals::JSON_POSITIVE_INTEGER, "lockscr_img") {
    
    restoreDefaults();

    _container.push_back(&_fwVersion);
    _container.push_back(&_hw);
    _container.push_back(&_displayOffPeriod);
    _container.push_back(&_unlockEnable);
    _container.push_back(&_bondedMac);
    _container.push_back(&_bondedMacType);
    _container.push_back(&_wifiSsid);
    _container.push_back(&_wifiPass);
    _container.push_back(&_isBleMacDefault);
    _container.push_back(&_shutDownPeriod);
    _container.push_back(&_language);
    _container.push_back(&_isBeepFeatureEnabled);
    _container.push_back(&_screenlockImage);
}

void Settings::init() {

    if (false == load()) {
        DBG_PRINT_TAG(TAG, "failed to load some settings");
        restoreDefaults();
        save(true);
    }

    DBG_PRINTLN("SETTINGS:");

    DBG_PRINTF("hw: %s\r\n", hw().c_str());
    DBG_PRINTF("fw: %s\r\n", fwVersion().c_str());
    DBG_PRINTF("display off period: %u\r\n", displayOffPeriod());
    DBG_PRINTF("shutdown period: %u\r\n", shutdownPeriod());
    DBG_PRINTF("swipe to unlock: %s\r\n", isSwipeToUnlockActive() ? "true" : "false");
    DBG_PRINTF("bonded mac %s\r\n", bondedMac().c_str());
    DBG_PRINTF("bonded mac type %u\r\n", bondedMacType());
    DBG_PRINTF("ssid: %s\r\n", ssid().c_str());
    DBG_PRINTF("pass: %s\r\n", pass().c_str());
    DBG_PRINTF("is MAC default: %s\r\n", isBleMacDefault() ? "true" : "false");
    DBG_PRINTF("language: %u\r\n", language());
    DBG_PRINTF("beep feature: %s\r\n", isBeepFeatureEnabled() ? "true" : "false");
    DBG_PRINTF("lockscreen image preset: %u\r\n", lockScreenImage());
}

bool Settings::load() {
    NVS nvs;
    nvs.init();
    bool ret = true;
    for (auto setting : _container) {
        switch (setting->getType()) {
            case Internals::JSON_INTEGER:{
                int64_t readRes = 0;
                if (true == nvs.read(storageNamespace, setting->getKeyName(), readRes)) {
                    setting->set(static_cast<int>(readRes));
                }
                else {
                    ret = false;
                }
            }
            break;
            case Internals::JSON_POSITIVE_INTEGER:{
                int64_t readRes = 0;
                if (true == nvs.read(storageNamespace, setting->getKeyName(), readRes)){
                    setting->set((uint32_t)readRes);
                }
                else {
                    ret = false;
                }
            }
            break;
            case Internals::JSON_BOOLEAN:{
                int64_t readRes = 0;
                if (true == nvs.read(storageNamespace, setting->getKeyName(), readRes)){
                    setting->set(readRes? 1:0);
                }
                else {
                    ret = false;
                }
            }
            break;

            case Internals::JSON_STRING:{
                string readRes = "";
                if (true == nvs.read(storageNamespace, setting->getKeyName(), readRes)){
                    setting->set(readRes.c_str());
                }
                else {
                    ret = false;
                }
            }
            break;

            default:
                break;        
        }
        setting->clearChangedFlag();
    }
    return ret;
}

void Settings::save(const bool forced) {

    NVS nvs;

    for (auto setting : _container) {
        if (setting->wasChanged() or forced) {

            switch (setting->getType()) {
                case Internals::JSON_INTEGER:
                    nvs.write(storageNamespace, setting->getKeyName(), setting->as<int>());
                break;

                case Internals::JSON_POSITIVE_INTEGER:
                    nvs.write(storageNamespace, setting->getKeyName(), setting->as<uint32_t>());
                break;

                case Internals::JSON_BOOLEAN:
                    nvs.write(storageNamespace, setting->getKeyName(), setting->as<bool>()? 1:0);
                break;

                case Internals::JSON_STRING:
                    nvs.write(storageNamespace, setting->getKeyName(), setting->as<const char*>());
                break;
                
                default:
                    assert(false); //shouldn't be here
                break;
            }
            DBG_PRINT_TAG(TAG, "setting %s saved", setting->getKeyName());
            setting->clearChangedFlag();
        }
    }
}

void Settings::restoreDefaults() {
   
    _fwVersion = FW_VERSION;
    _hw = HW_VERSION;
    _displayOffPeriod = 30;
    _unlockEnable = false;
    _bondedMac = defaultMac;
    _bondedMacType = static_cast<uint8_t>(esp_ble_addr_type_t::BLE_ADDR_TYPE_PUBLIC);
    _wifiSsid = "";
    _wifiPass = "";
    _isBleMacDefault = true;
    _shutDownPeriod = SHUTDOWN_PERIOD_MS;
    _language = static_cast<uint8_t>(eLanguage::LANGUAGE_ENG);
    _isBeepFeatureEnabled = false;
    _screenlockImage = 0;
}

std::string Settings::hw() const {
    return _hw.as<char*>();
}

std::string Settings::ssid() const {
    return _wifiSsid.as<char*>();
}

std::string Settings::pass() const {
    return _wifiPass.as<char*>();
}

std::string Settings::fwVersion() const {
    return _fwVersion.as<char*>();
}

size_t Settings::displayOffPeriod() const {
    return _displayOffPeriod.as<uint32_t>();
}

std::string Settings::bondedMac() const {
    return _bondedMac.as<char*>();
}

uint8_t Settings::bondedMacType() const {
    return _bondedMacType.as<uint8_t>();
}

bool Settings::isSwipeToUnlockActive() const {
    return _unlockEnable.as<bool>();
}

bool Settings::isBleMacDefault() const {
    return _isBleMacDefault.as<bool>();
}

size_t Settings::shutdownPeriod() const {
    return _shutDownPeriod.as<size_t>();
}

uint8_t Settings::language() const {
    return _language;
}

bool Settings::isBeepFeatureEnabled() const {
    return _isBeepFeatureEnabled;
}

uint8_t Settings::lockScreenImage() const {
    return _screenlockImage;
}

void Settings::setFwVersion(const std::string& value) { 
    _fwVersion = value.c_str();
}

void Settings::setHw(const std::string& value) {
    _hw = value.c_str();
}

void Settings::setSsid(const std::string& value) {
    _wifiSsid = value.c_str();
}

void Settings::setPass(const std::string& value) {
    _wifiPass = value.c_str();
}

void Settings::setDisplayOffPeriod(const uint32_t value) {
    _displayOffPeriod = value;
}

void Settings::setSwipeToUnlock(const bool value) {
    _unlockEnable = value;
}

void Settings::setBondedMac(const std::string& value) {
    _bondedMac = value.c_str();
}

void Settings::setBondedMacType(const uint8_t type) {
    _bondedMacType = type;
}

void Settings::setBleMacDefault(const bool value) {
    _isBleMacDefault = value;
}

void Settings::setShutdownPeriod(const size_t value) {
    _shutDownPeriod = value;
}

void Settings::setLanguage(const uint8_t lang) {
    _language = lang;
}

void Settings::setBeepFeatureEnabled(const bool value) {
    _isBeepFeatureEnabled = value;
}

void Settings::setLockScreenImage(const uint8_t value) {
    _screenlockImage = value;
}
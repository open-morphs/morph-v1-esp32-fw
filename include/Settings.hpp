#pragma once

#include "Variants/Variants.h"
#include "SingletonTemplate.hpp"
#include "SettingsSaver.hpp"
#include <map>
#include "Languages.hpp"

using namespace Languages;
using namespace std;

class Settings : public Singleton<Settings>{
    
    friend class Singleton;
    public:

        void init();
        void restoreDefaults();
        void save(const bool forced = false);

        // read settings
        std::string fwVersion() const;
        std::string hw() const;
        size_t displayOffPeriod() const;
        bool isSwipeToUnlockActive() const;
        std::string bondedMac() const;
        uint8_t bondedMacType() const;
        std::string ssid() const;
        std::string pass() const;
        bool isBleMacDefault() const;
        size_t shutdownPeriod() const;
        uint8_t language() const;
        bool isBeepFeatureEnabled() const;
        uint8_t lockScreenImage() const;

        // write setings
        void setFwVersion(const std::string& value);
        void setHw(const std::string&);
        void setDisplayOffPeriod(const uint32_t value);
        void setSwipeToUnlock(const bool value);
        void setBondedMac(const std::string&);
        void setBondedMacType(const uint8_t type);
        void setSsid(const std::string&);
        void setPass(const std::string&);
        void setBleMacDefault(const bool value);
        void setShutdownPeriod(const size_t value);
        void setLanguage(const uint8_t lang);
        void setBeepFeatureEnabled(const bool value);
        void setLockScreenImage(const uint8_t value);
        
    private:
        Settings();
        bool load();
    private:
        MultiType _fwVersion;
        MultiType _hw;
        MultiType _displayOffPeriod;
        MultiType _unlockEnable;
        MultiType _bondedMac;
        MultiType _bondedMacType;
        MultiType _wifiSsid;
        MultiType _wifiPass;
        MultiType _isBleMacDefault;
        MultiType _shutDownPeriod;
        MultiType _language;
        MultiType _isBeepFeatureEnabled;
        MultiType _screenlockImage;
};

#define SETTINGS Settings::instance()

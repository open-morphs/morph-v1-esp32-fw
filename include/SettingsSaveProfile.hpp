#pragma once

#include <string>
#include "Variants/Variants.h"

using namespace std;

typedef enum{
    SAVE_ALLWAYS,
    SAVE_WHEN_NO_POWER
}eOnChangedPolicy;

class SettingsSaveProfile{

    private:
        MultiType* _pSetting;
        eOnChangedPolicy   _onChangedPolicy;
        string _directoryName;

    public:
        explicit SettingsSaveProfile(MultiType* pSetting, const eOnChangedPolicy onChangedPolicy, const string& directoryName)
                        : _pSetting(pSetting),
                        _onChangedPolicy(onChangedPolicy),
                        _directoryName(directoryName) {

        }

        MultiType* getSettingPointer()const{
            return _pSetting;
        }

        eOnChangedPolicy getOnChangedPolicy()const{
            return _onChangedPolicy;
        }

        string getNamespace() const {
            return _directoryName;
        }

        bool operator==(const SettingsSaveProfile& value) const{
            return (value.getSettingPointer() == _pSetting and value.getNamespace() == _directoryName and
                value.getOnChangedPolicy() == _onChangedPolicy);
        }

        bool operator!=(const SettingsSaveProfile& value) const{
            return (value.getSettingPointer() != _pSetting or value.getNamespace() != _directoryName or
                value.getOnChangedPolicy() != _onChangedPolicy);
        }
};

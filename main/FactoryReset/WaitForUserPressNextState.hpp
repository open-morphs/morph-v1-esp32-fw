#pragma once

#include "FactoryResetState.hpp"
#include "Utils/threadSafeDbg.hpp"
#include "FactoryResetEvents.hpp"
#include "FactoryResetSignals.hpp"
#include "GUI.hpp"
#include "LanguagePackCollection.hpp"
#include "Settings.hpp"
#include "MorphDevice.hpp"

using namespace Languages;

struct WaitForUserPressNextState : public FactoryResetState{

    WaitForUserPressNextState(EventGroupHandle_t eventGroup)
        :   FactoryResetState(eventGroup) {

    }

    const char* name() override {
        return "WaitForUserPressNextState";
    }

    void enter() override {
        GRAPHICS.setResetScreen("1/8", LANGUAGE_PACK[Languages::eTitleType::TITLE_RESET_PRESS_NEXT], true, false, true);
    }

    uint8_t process() override {
        static const size_t fullBitMask{NEXT_BIT};
        const uint32_t eventBits {xEventGroupWaitBits(_eventGroup, fullBitMask, pdTRUE, pdFALSE, portMAX_DELAY)};
    
        if (eventBits) {
            SETTINGS.restoreDefaults();
            SETTINGS.save();
            MORPH_DEVICE.clearPairedDevicesList();
            MORPH_DEVICE.savePairedDeviceListToFlash();
            GRAPHICS.updateDevicesList();
            GRAPHICS.updateSettings();
            return FSM_SIGNAL_USER_CONFIRMATION;
        }

        return FSM_SIGNAL_ERROR;
    }

    void exit() override {}

    private:
};
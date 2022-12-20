#pragma once

#include "WaitForUserPressNextState.hpp"
#include "BLE.hpp"

struct EarbudsFactoryResetState : public FactoryResetState{

    EarbudsFactoryResetState(EventGroupHandle_t eventGroup/*, const std::string& descr*/)
        :   FactoryResetState(eventGroup/*, descr*/) {

    }
    const char* name() override {
        return "EarbudsFactoryResetState";
    }
    void enter() override {
        GRAPHICS.setResetScreen("2/8", LANGUAGE_PACK[Languages::eTitleType::TITLE_RESET_EARBUDS_RESET], true, false, true);
    }

    uint8_t process() override {

        // just to initialize ble module, it's needed for bonding deletion
        BLE_DEVICE.start();
        delay(1000);
        assert (BLE_DEVICE.deleteBondingInformation());
        BLE_DEVICE.stop();
        // reset information about primary device
        MORPH_DEVICE.set_primaryEarbud(ePrimaryEarbud::EARBUD_UNKNOWN);

        static const size_t fullBitMask{NEXT_BIT};
        const uint32_t eventBits {xEventGroupWaitBits(_eventGroup, fullBitMask, pdTRUE, pdFALSE, portMAX_DELAY)};
    
        if (eventBits) {
            return FSM_SIGNAL_EARBUDS_RESET_DONE;
        }

        return FSM_SIGNAL_ERROR;
    }

    void exit() override {}
};
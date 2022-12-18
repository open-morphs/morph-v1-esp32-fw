#pragma once
#include "FactoryResetState.hpp"
#include "MorphDevice.hpp"
#include "BLE.hpp"
#include "GUI.hpp"

struct TakeEarbudsFromCaseState : FactoryResetState {
    
    explicit TakeEarbudsFromCaseState(EventGroupHandle_t eventGroup)
        :   FactoryResetState(eventGroup) {}

    const char* name() override {
        return "TakeEarbudsFromCaseState";
    }

    void enter() override {
        BLE_DEVICE.changeCurrentUsedMac();
    }

    void exit() override {}

    uint8_t process() override {

        auto primaryDevice {MORPH_DEVICE.get_primaryEarbud()};
        if (ePrimaryEarbud::EARBUD_UNKNOWN == primaryDevice) {
            DBG_PRINT_TAG(name(), "failed to request primary device role");
            return FSM_SIGNAL_WRONG_PRIMARY_DEVICE_ROLE;
        }

        const std::string descrMessage = stringUtils::format(LANGUAGE_PACK[Languages::eTitleType::TITLE_RESET_TAKE_EARBUDS], 
            ePrimaryEarbud::EARBUD_LEFT == primaryDevice ? LANGUAGE_PACK[Languages::eTitleType::TITLE_RIGHT] : LANGUAGE_PACK[Languages::eTitleType::TITLE_LEFT],
            ePrimaryEarbud::EARBUD_LEFT == primaryDevice ? LANGUAGE_PACK[Languages::eTitleType::TITLE_LEFT] : LANGUAGE_PACK[Languages::eTitleType::TITLE_RIGHT] );
        
        GRAPHICS.setResetScreen("5/8", descrMessage, true, false, true);
        
        DBG_PRINT_TAG(name(), "waiting for user confirmation...");

        static const size_t fullBitMask{NEXT_BIT};
        const uint32_t eventBits {xEventGroupWaitBits(_eventGroup, fullBitMask, pdTRUE, pdFALSE, portMAX_DELAY)};

        if (eventBits) {
            DBG_PRINT_TAG(name(), "user pressed next..");
            return FSM_SIGNAL_EARBUDS_ARE_OUT_OF_CASE;
        }

        return FSM_SIGNAL_ERROR;        
    }
};
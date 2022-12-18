#pragma once
#include "FactoryResetState.hpp"
#include "MorphDevice.hpp"
#include "BLE.hpp"
#include "GUI.hpp"

struct PutEarbudsInCaseState : FactoryResetState {
    
    explicit PutEarbudsInCaseState(EventGroupHandle_t eventGroup)
        :   FactoryResetState(eventGroup) {}

    const char* name() override {
        return "PutEarbudsInCaseState";
    }

    void enter() override {}
    void exit() override {}

    uint8_t process() override {

        auto primaryDevice {MORPH_DEVICE.get_primaryEarbud()};
        if (ePrimaryEarbud::EARBUD_UNKNOWN == primaryDevice) {
            DBG_PRINT_TAG(name(), "failed to request primary device role");
            return FSM_SIGNAL_WRONG_PRIMARY_DEVICE_ROLE;
        }
        
        const std::string descrMessage = stringUtils::format(LANGUAGE_PACK[Languages::eTitleType::TITLE_RESET_PUT_EARBUDS], 
            ePrimaryEarbud::EARBUD_LEFT == primaryDevice ? LANGUAGE_PACK[Languages::eTitleType::TITLE_RIGHT] : LANGUAGE_PACK[Languages::eTitleType::TITLE_LEFT],
            ePrimaryEarbud::EARBUD_LEFT == primaryDevice ? LANGUAGE_PACK[Languages::eTitleType::TITLE_LEFT] : LANGUAGE_PACK[Languages::eTitleType::TITLE_RIGHT] );
        
        GRAPHICS.setResetScreen(_incomingSignal == FSM_SIGNAL_FIRST_PAIRING_DONE ? "4/8" : "7/8", descrMessage, true, false, true);
        
        DBG_PRINT_TAG(name(), "waiting for user confirmation...");

        static const size_t fullBitMask{NEXT_BIT};
        const uint32_t eventBits {xEventGroupWaitBits(_eventGroup, fullBitMask, pdTRUE, pdFALSE, portMAX_DELAY)};

        if (eventBits) {
            DBG_PRINT_TAG(name(), "user pressed next..");
            if (FSM_SIGNAL_FIRST_PAIRING_DONE == _incomingSignal) {
                return  FSM_SIGNAL_EARBUDS_ARE_IN_CASE_FIRST;
            }
            else if (FSM_SIGNAL_SECOND_PAIRING_DONE == _incomingSignal) {
                return FSM_SIGNAL_EARBUDS_ARE_IN_CASE_SECOND;
            }
        }
        return FSM_SIGNAL_ERROR;        
    }
};
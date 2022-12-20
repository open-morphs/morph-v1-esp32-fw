#pragma once

#include "FactoryResetState.hpp"
#include "MorphDevice.hpp"
#include "BLE.hpp"
#include "GUI.hpp"

struct FinalState : FactoryResetState {

    explicit FinalState(EventGroupHandle_t eventGroup)
            : FactoryResetState(eventGroup) {}

    const char *name() override {
        return "FinalState";
    }

    void enter() override {}

    void exit() override {}

    uint8_t process() override {

        std::string descrMsg{};

        switch (_incomingSignal) {
            case FSM_SIGNAL_WRONG_PRIMARY_DEVICE_ROLE:
                if (MORPH_DEVICE.get_primaryEarbud() == ePrimaryEarbud::EARBUD_UNKNOWN) {
                    descrMsg = LANGUAGE_PACK[Languages::eTitleType::TITLE_RESET_FAILED_GET_PRIMARY];
                    break;
                }
                descrMsg = stringUtils::format(LANGUAGE_PACK[Languages::eTitleType::TTITLE_RESET_NOT_EXPECTED_PRIMARY],
                                               MORPH_DEVICE.get_primaryEarbud() == ePrimaryEarbud::EARBUD_LEFT
                                               ? LANGUAGE_PACK[Languages::eTitleType::TITLE_LEFT]
                                               : LANGUAGE_PACK[Languages::eTitleType::TITLE_RIGHT]);
                break;

            case FSM_SIGNAL_PRIMARY_DEVICE_REQ_FAILED:
                descrMsg = LANGUAGE_PACK[Languages::eTitleType::TTITLE_RESET_FAILED_REQUEST_PRIMARY];
                break;

            case FSM_SIGNAL_PAIRING_TIMEOUT:
                descrMsg = LANGUAGE_PACK[Languages::eTitleType::TITLE_RESET_PAIRING_TIMEOUT];
                break;

            case FSM_SIGNAL_EARBUDS_ARE_IN_CASE_SECOND:
                descrMsg = LANGUAGE_PACK[Languages::eTitleType::TITLE_RESET_COMPLETED];
                break;

            default:
                DBG_PRINT_TAG(name(), "signal: %u", _incomingSignal);
                descrMsg = LANGUAGE_PACK[Languages::eTitleType::TITLE_RESET_SMTH_WRONG];
                break;
        }

        GRAPHICS.setResetScreen("", descrMsg, false, true, false);

        while (1) {
            delay(1000);
        }
    }
};
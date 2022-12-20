#pragma once

#include "WaitForUserPressNextState.hpp"
#include "GUI.hpp"
#include "BLE.hpp"
#include "Utils/TimeUtils.hpp"

struct WaitForConnectionState : FactoryResetState {

    WaitForConnectionState(EventGroupHandle_t eventGroup)
            : FactoryResetState(eventGroup) {}

    void enter() {}

    const char *name() override {
        return "WaitForConnectionState";
    }

    uint8_t process() override {

        GRAPHICS.setResetScreen(_incomingSignal == FSM_SIGNAL_EARBUDS_RESET_DONE ? "3/8" : "6/8",
                                LANGUAGE_PACK[Languages::eTitleType::TITLE_RESET_WAIT_FOR_CONNECTION], true, false,
                                false);
        DBG_PRINT_TAG(name(), "waiting for BLE CONNECTION");

        const ePrimaryEarbud previousPrimaryDevice{MORPH_DEVICE.get_primaryEarbud()};

        const size_t start{TimeUtils::nowMs()};
        BLE_DEVICE.start();

        while (false == BLE_DEVICE.isConnected() and false == TimeUtils::isPeriodPassed(start, 35000)) {
            delay(1000);
        }

        if (false == BLE_DEVICE.isConnected()) {
            DBG_PRINT_TAG(name(), "connection timeout");
            return FSM_SIGNAL_PAIRING_TIMEOUT;
        } else {
            DBG_PRINT_TAG(name(), "connection OK");

            const bool primaryEarbudRequested{BLE_DEVICE.requestPrimaryDevice()};

            if (!primaryEarbudRequested) {
                DBG_PRINT_TAG(name(), "failed to request primary device role");
                return FSM_SIGNAL_PRIMARY_DEVICE_REQ_FAILED;
            }
            delay(1000);

            if (ePrimaryEarbud::EARBUD_UNKNOWN != previousPrimaryDevice and
                previousPrimaryDevice == MORPH_DEVICE.get_primaryEarbud()) {

                DBG_PRINT_TAG(name(), "Wrong primary device connected!");
                return FSM_SIGNAL_WRONG_PRIMARY_DEVICE_ROLE;
            }

            return ePrimaryEarbud::EARBUD_UNKNOWN == previousPrimaryDevice ?
                   FSM_SIGNAL_FIRST_PAIRING_DONE : FSM_SIGNAL_SECOND_PAIRING_DONE;
        }
    }

    void exit() override {
        if (BLE_DEVICE.isRunning()) {
            BLE_DEVICE.stop();
        }
    }
};
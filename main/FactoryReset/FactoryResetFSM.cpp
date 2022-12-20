#include <FactoryReset/FactoryResetFSM.hpp>
#include <FactoryReset/WaitForUserPressNextState.hpp>
#include <Utils/ThreadSafeDbg.hpp>
#include <GUI.hpp>
#include <SmartCase.hpp>

static const char* const TAG {"FactoryResetFSM"};

FactoryResetFSM::FactoryResetFSM()
    :   ITask("FactoryResetFSM", configMINIMAL_STACK_SIZE*3, 1) {
    
}

void FactoryResetFSM::run (void* args) {

    BLE_DEVICE.setMacSwapEnabled(false);
    setState(_initState);
    SMARTCASE.disableDisplayManager();

    while (1) {
        // DBG_PRINT_TAG(TAG, "processing %s state", currentState()->name());
        const uint8_t signal {currentState()->process()};

        AbstractState* newState {handleSignal(signal)};
        assert(newState);
        // DBG_PRINT_TAG(TAG, "setting new state: %s", newState->name());
        setState(newState, signal);
    }
}

void FactoryResetFSM::onStop() {
    SMARTCASE.enableDisplayManager();
    BLE_DEVICE.stop();
    BLE_DEVICE.setMacSwapEnabled(true);
}

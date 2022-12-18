#include "DisplayManager.hpp"
#include "Utils/threadSafeDbg.hpp"
#include "lvgl.h"
#include "Settings.hpp"
#include "Dimmer.hpp"
#include "HttpWebserver.hpp"
#include "SmartCase.hpp"
#include "LanguagePackCollection.hpp"

using namespace Languages;

static const char* const TAG {"DISP"};

DisplayManager::DisplayManager()
    :   ITask("Display", configMINIMAL_STACK_SIZE*3, 1),
        _sleepTime(0),
        _shutdownTime(SHUTDOWN_PERIOD_MS),
        _onSleepCb(nullptr),
        _onWakeUpCb(nullptr) {
        
}

// void DisplayManager::triggerAcitivity() {
//     lv_disp_trig_activity(NULL);
// }

void DisplayManager::run(void* args) {

    DBG_PRINT_TAG(TAG, "started!");

    while(1) {
        static bool isScreenActive {true};
        setSleepTime(SETTINGS.displayOffPeriod() * 1000);

        if (lv_disp_get_inactive_time(NULL) > _shutdownTime) {
            SMARTCASE.safeSwitchOff(LANGUAGE_PACK[eTitleType::TITLE_SWITCHING_OFF]);
            while(1) {
                // actually shouldn't be here
                delay(100);
            }
        }
        if(lv_disp_get_inactive_time(NULL) > _sleepTime and false == WEBSERVER.isOtaRunning()) {
            if (false == BLE_DEVICE.isConnected() and false == WEBSERVER.isActive()) {
                SMARTCASE.safeSwitchOff(LANGUAGE_PACK[eTitleType::TITLE_GOING_TO_POWER_SAVING]);
                while(1) {
                // actually shouldn't be here
                    delay(100);
                }
            }
            if (true == isScreenActive) {
                if (_onSleepCb) {
                    _onSleepCb();
                }
                // esp_light_sleep_start();
                // DBG_PRINT_TAG(TAG,"Woken up and ready to work!");
                // triggerAcitivity();
            }
            isScreenActive = false;
        }
        else {
            if (false == isScreenActive) {
                if (_onWakeUpCb) {
                    _onWakeUpCb();
                }
            }
            isScreenActive = true;
        }
        delay(100);
    }
}

void DisplayManager::setSleepTime(const size_t time) {
    _sleepTime = time;
}

void DisplayManager::setSleepCb(std::function<void()> cb) {
    _onSleepCb = cb;
}

void DisplayManager::setWakeUpCb(std::function<void()> cb) {
    _onWakeUpCb = cb;
}
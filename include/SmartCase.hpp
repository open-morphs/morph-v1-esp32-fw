#pragma once
#include "SingletonTemplate.hpp"
#include "attributes.h"
#include "Utils/GpioUtils.hpp"
#include "GUI.hpp"
#include "BLE.hpp"
#include "HttpWebserver.hpp"
#include "ADC.hpp"
#include "Settings.hpp"
#include "DisplayManager.hpp"
#include "Dimmer.hpp"
#include "LanguagePackCollection.hpp"

class SmartCase : public Singleton<SmartCase> {

        ATTRIBUTE(uint8_t, batteryPercent);
    public:
        void setSelfPower(const bool value){
            ESP_ERROR_CHECK(value ? GpioUtils::gpioSetHigh(SELF_POWER_GPIO) : GpioUtils::gpioSetLow(SELF_POWER_GPIO));
        }

        void setBackLight(const uint8_t percent) {
            _dimmer->dimm(percent, Dimmer::eDimmerTransitionType::DIMM_SLOW);
        }

        void init() {
            _dimmer->init();
            auto wakeCb = [this] {
                GRAPHICS.hideLoadingPage();
                if (!_dimmer->level()) {
                    DBG_PRINTLN("dimming on..");
                    GRAPHICS.setCaseChargeLevel(get_batteryPercent());
                    setBackLight(100);
                }
            };
            auto sleepCb = [this] {
                if (_dimmer->level()) {
                    DBG_PRINTLN("dimming off..");
                    setBackLight(0);
                    if (SETTINGS.isSwipeToUnlockActive()) {
                        DBG_PRINTLN("locking screen..");
                        GRAPHICS.showLockPage();
                    }
                    else {
                        GRAPHICS.showLoadingPage("", false);
                    }
                }
            };
            _dispManager->setSleepCb(sleepCb);
            _dispManager->setWakeUpCb(wakeCb);
        }

        void safeSwitchOff(const std::string& mesasge) {
            DBG_PRINTLN("switching off..");
            GRAPHICS.showLoadingPage(mesasge);
            vTaskDelay(pdMS_TO_TICKS(2000));
            BLE_DEVICE.stop();
            WEBSERVER.stop();
            ADC_MODULE.stop();
            SETTINGS.save(true);
            setSelfPower(false);
            GRAPHICS.showLoadingPage(LANGUAGE_PACK[Languages::eTitleType::TITLE_RELEASE_BUTTON]);
        }

        void enableDisplayManager() {
            if (!_dispManager->isRunning()) {
                _dispManager->start();
            }
        }

        void disableDisplayManager() {
            _dispManager->stop();
        }

    friend class Singleton;
    private:
        DisplayManager* _dispManager = nullptr;
        Dimmer* _dimmer = nullptr;

        SmartCase() : _batteryPercent{}, _dispManager{new DisplayManager},
            _dimmer{new Dimmer (static_cast<gpio_num_t>(LED_BL_GPIO), ledc_channel_t::LEDC_CHANNEL_0, 1000, false)} {
            ESP_ERROR_CHECK(GpioUtils::setGpioMode(SELF_POWER_GPIO, gpio_mode_t::GPIO_MODE_OUTPUT, false, false));
                    
        }
};

#define SMARTCASE SmartCase::instance()
#include <GpioButton.hpp>
#include "esp_log.h"
#include <Utils/GpioUtils.hpp>

static const char* const TAG {"GpioButton"};

GpioButton::GpioButton(const gpio_num_t gpioNum, const eGpioButtonPressedLogicLevel pressedLogicLevel,
    const bool isPullup, const bool isPullDown, const uint32_t deBouncePeriodMs)
    :   AbstractButton(deBouncePeriodMs),
        _gpioNum(gpioNum),
        _pressedLogicLevel(pressedLogicLevel),
        _isPullup(isPullup),
        _isPulldown(isPullDown) {
    
    if(GPIO_NUM_MAX <= _gpioNum) {
        ESP_LOGE(TAG, "invalid pinNum: %u", _gpioNum);
    }
}

bool GpioButton::initProcedure() {
    if (ESP_OK != GpioUtils::setGpioMode(_gpioNum, gpio_mode_t::GPIO_MODE_INPUT, _isPullup, _isPulldown)) {
        ESP_LOGE(TAG, "Failed to set INPUT mode to gpioNum %u!", _gpioNum);
        return false;
    }
    return true;
}

eButtonState GpioButton::readState() const {
    const GpioUtils::eGpioOutputValue gpioReadValue {GpioUtils::gpioRead(_gpioNum)};
    if (eGpioButtonPressedLogicLevel::BUTTON_PRESSED_LEVEL_HIGH == _pressedLogicLevel) {
        return GpioUtils::eGpioOutputValue::GPIO_HIGH == gpioReadValue ? eButtonState::BUTTON_PRESSED : eButtonState::BUTTON_RELEASED;
    }
    else {
        return GpioUtils::eGpioOutputValue::GPIO_HIGH == gpioReadValue ? eButtonState::BUTTON_RELEASED : eButtonState::BUTTON_PRESSED;
    }
}

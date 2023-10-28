#pragma once

#include "AbstractButton.hpp"
#include "driver/gpio.h"

enum class eGpioButtonPressedLogicLevel {
    BUTTON_PRESSED_LEVEL_LOW = 0,
    BUTTON_PRESSED_LEVEL_HIGH = 1
};

class GpioButton : public AbstractButton {
    public:
        explicit GpioButton (const gpio_num_t gpioNum, const eGpioButtonPressedLogicLevel pressedLogicLevel,
            const bool isPullUp, const bool isPullDown, const uint32_t deBouncePeriodMs = 200);
    private:
        bool initProcedure() override final;
        eButtonState readState() const override final;
    private:
        gpio_num_t _gpioNum;
        eGpioButtonPressedLogicLevel _pressedLogicLevel;
        bool _isPullup;
        bool _isPulldown;
};
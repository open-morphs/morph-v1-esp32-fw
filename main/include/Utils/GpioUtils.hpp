#pragma once

#include "driver/gpio.h"

namespace GpioUtils {
    enum class eGpioOutputValue : uint32_t {
        GPIO_LOW = 0,
        GPIO_HIGH = 1
    };
    esp_err_t setGpioMode(const gpio_num_t gpioNum, const gpio_mode_t mode, const bool pullUp, const bool pullDown);
    esp_err_t setGpioModeForGroup(const uint64_t gpioBitMask, const gpio_mode_t mode, const bool pullUp, const bool pullDown);
    esp_err_t gpioWrite(const gpio_num_t gpioNum, const eGpioOutputValue value);
    esp_err_t gpioSetHigh(const gpio_num_t gpioNum);
    esp_err_t gpioSetLow(const gpio_num_t gpioNum);
    esp_err_t resetGpio(const gpio_num_t gpioNum);
    esp_err_t disablePullDown(const gpio_num_t gpioNum);
    esp_err_t enablePullDown(const gpio_num_t gpioNum);
    eGpioOutputValue gpioRead(const gpio_num_t gpioNum);
};

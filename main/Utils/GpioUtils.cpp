#include <Utils/GpioUtils.hpp>

esp_err_t
GpioUtils::setGpioMode(const gpio_num_t gpioNum, const gpio_mode_t mode, const bool pullUp, const bool pullDown) {
    const uint64_t bitMask{(1UL << gpioNum)};
    return setGpioModeForGroup(bitMask, mode, pullUp, pullDown);
}

esp_err_t GpioUtils::setGpioModeForGroup(const uint64_t gpioBitMask, const gpio_mode_t mode, const bool pullUp,
                                         const bool pullDown) {
    gpio_config_t conf{
            .pin_bit_mask = gpioBitMask,
            .mode = mode,
            .pull_up_en = pullUp ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
            .pull_down_en = pullDown ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
            .intr_type = gpio_int_type_t::GPIO_INTR_DISABLE
    };
    return gpio_config(&conf);
}

esp_err_t GpioUtils::disablePullDown(const gpio_num_t gpioNum) {
    return gpio_pulldown_dis(gpioNum);
}

esp_err_t GpioUtils::enablePullDown(const gpio_num_t gpioNum) {
    return gpio_pulldown_en(gpioNum);
}

esp_err_t GpioUtils::gpioWrite(const gpio_num_t gpioNum, const eGpioOutputValue value) {
    return gpio_set_level(gpioNum, static_cast<uint32_t>(value));
}

GpioUtils::eGpioOutputValue GpioUtils::gpioRead(const gpio_num_t gpioNum) {
    return gpio_get_level(gpioNum) == 1 ? eGpioOutputValue::GPIO_HIGH : eGpioOutputValue::GPIO_LOW;
}

esp_err_t GpioUtils::gpioSetHigh(const gpio_num_t gpioNum) {
    return gpioWrite(gpioNum, eGpioOutputValue::GPIO_HIGH);
}

esp_err_t GpioUtils::gpioSetLow(const gpio_num_t gpioNum) {
    return gpioWrite(gpioNum, eGpioOutputValue::GPIO_LOW);
}

esp_err_t GpioUtils::resetGpio(const gpio_num_t gpioNum) {
    return gpio_reset_pin(gpioNum);
}

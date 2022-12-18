#pragma once

#include "driver/gpio.h"
#include "driver/ledc.h"

class Dimmer {

    public:
        enum class eDimmerTransitionType {
            DIMM_INSTANTLY,
            DIMM_FAST,
            DIMM_MEDIUM,
            DIMM_SLOW
        };
        explicit Dimmer(const gpio_num_t gpio, const ledc_channel_t channel, const size_t freq, const bool isInversed = false);
        void init();
        void dimm(const unsigned char percent, const eDimmerTransitionType transitionType = eDimmerTransitionType::DIMM_INSTANTLY);
        void on();
        void off();
        unsigned short level() const;
    private:
        void setLevel(const unsigned short value);
    private:
        gpio_num_t _gpioNum;
        ledc_channel_t _channel;
        size_t _freq;
        unsigned short _currentLevel;
        bool _isInversed;
};
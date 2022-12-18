#include "Dimmer.hpp"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Utils/threadSafeDbg.hpp"
#include <cmath>

#define TRANSITION_STEP_FAST            40UL
#define TRANSITION_STEP_MEDIUM          20UL
#define TRANSITION_STEP_SLOW            15UL

static const char* const TAG {"DIMMER"};

Dimmer::Dimmer(const gpio_num_t gpio, const ledc_channel_t channel, const size_t freq, const bool isInversed)
    :   _gpioNum(gpio),
        _channel(channel),
        _freq(freq),
        _isInversed(isInversed) {

}

void Dimmer::init() {

    ledc_channel_config_t chCfg = {
        .gpio_num = _gpioNum,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = _channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
    };
    
    ESP_ERROR_CHECK(ledc_channel_config(&chCfg));

    ledc_timer_config_t timerConf = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        ledc_timer_bit_t::LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = _freq,
        .clk_cfg = ledc_clk_cfg_t::LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timerConf));

    off();
}

void Dimmer::dimm(const unsigned char percent, const eDimmerTransitionType transitionType) {

    const unsigned short newValue = (1024 / 100) * percent;
    // DBG_PRINT_TAG(TAG, "dimming to percent: %u, newValue: %u", percent, newValue);

    if (eDimmerTransitionType::DIMM_INSTANTLY == transitionType) {
        setLevel(newValue);
        return;
    }

    int step {0};

    switch (transitionType) {
        case eDimmerTransitionType::DIMM_FAST:
            step = TRANSITION_STEP_FAST;
        break;
        case eDimmerTransitionType::DIMM_MEDIUM:
            step = TRANSITION_STEP_MEDIUM;
        break;
        case eDimmerTransitionType::DIMM_SLOW:
            step = TRANSITION_STEP_SLOW;
        break;
    
    default:
        break;
    }

    const int incValue {newValue > _currentLevel ? step : -step};


    while (std::abs(newValue - _currentLevel) >= step) {
        setLevel(_currentLevel + incValue);
        vTaskDelay(10/portTICK_RATE_MS);
    }
    setLevel(newValue);
}


void Dimmer::on() {
    dimm(100);
}

void Dimmer::off() {
    dimm(0);
}

unsigned short Dimmer::level() const {
    return _currentLevel;
}

void Dimmer::setLevel(const unsigned short value) {
    // DBG_PRINT_TAG(TAG, "setting level: %u", value);
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_HIGH_SPEED_MODE, _channel, value));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_HIGH_SPEED_MODE, _channel));
    _currentLevel = value;
}
#include "TouchButton.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "Utils/threadSafeDbg.hpp"
#include "math.h"
#include "Utils/TimeUtils.hpp"

#define TOUCHPAD_FILTER_TOUCH_PERIOD_MS         10UL
#define TOUCH_BUTTON_UPDATE_BASELINE_PERIOD     800UL
#define INIT_VALUE_READINGS                     5
#define RELEASE_HYSTERESIS                      0.4     // 40 % from sensitivity
#define AUTO_CALIBRATION_PERIOD_S               60

static bool isGlobalTouchPadDriverInitialized {false};
static xSemaphoreHandle touchPadDriverMux {NULL};
static const char* const TAG {"TouchButton"};

struct TouchButtonData_t {
    uint16_t baseLine;
    uint32_t baseLineUpdatedTs;
    float touchThreshold;
    float releaseThresholdHysteresis;
    eButtonState previousState;
};

TouchButton::TouchButton(const touch_pad_t touchPad, const double sensitivity, const uint32_t deBouncePeriodMs)
    :   AbstractButton(deBouncePeriodMs),
        _touchPad(touchPad),
        _sensitivity(sensitivity),
        _touchData(new TouchButtonData_t()) {

}

TouchButton::~TouchButton() {
    delete _touchData;
}

bool TouchButton::initProcedure() {

    if (false == isGlobalTouchPadDriverInitialized) {
        if (ESP_OK != globalTouchPadDriverInit()) {
            ESP_LOGE(TAG, "Failed to initialize touchPad driver");
            return false;
        }
    }
    uint32_t initValue {0};

    ESP_ERROR_CHECK(touch_pad_config(_touchPad, 0));
    delay(1000); 
    for (int i = 0; i < INIT_VALUE_READINGS; i++) {
        uint16_t rawValue {0};
        ESP_ERROR_CHECK(touch_pad_read_raw_data(_touchPad, &rawValue));
        initValue += rawValue;
        delay(100);
    }

    initValue /= INIT_VALUE_READINGS;
    
    _touchData->baseLine = initValue;
    _touchData->baseLineUpdatedTs = TimeUtils::nowMs();
    _touchData->touchThreshold = -_sensitivity;
    _touchData->previousState = eButtonState::BUTTON_RELEASED;
    _touchData->releaseThresholdHysteresis = _touchData->touchThreshold * RELEASE_HYSTERESIS;
    ESP_LOGD(TAG, "initialized:\r\nbaseLine: %hu\r\ntouchThreshold: %f\r\nreleaseThresholdHysteresis: %f\r\n",
    _touchData->baseLine, _touchData->touchThreshold, _touchData->releaseThresholdHysteresis);
    return true;
}

esp_err_t TouchButton::globalTouchPadDriverInit() {

    touchPadDriverMux = xSemaphoreCreateMutex();
    assert (NULL != touchPadDriverMux);
    ESP_ERROR_CHECK(touch_pad_init());
    ESP_ERROR_CHECK(touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V));
    ESP_ERROR_CHECK(touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD_MS));
    isGlobalTouchPadDriverInitialized = true;
    return ESP_OK;
}

eButtonState TouchButton::readState() const {

    eButtonState ret {_touchData->previousState};
    uint16_t filteredValue {0};
    if (ESP_OK != touch_pad_read_filtered(_touchPad, &filteredValue)) {
        ESP_LOGE(TAG, "Failet to read filtered value");
        return ret;
    };    

    const int32_t delta {filteredValue - _touchData->baseLine};
    const double deltaPercent {static_cast<double>(delta)/_touchData->baseLine};
    if (deltaPercent < (_touchData->touchThreshold)) {
        ret = eButtonState::BUTTON_PRESSED;
    }
    else if (deltaPercent > _touchData->releaseThresholdHysteresis){
        ret = eButtonState::BUTTON_RELEASED;
        if (ret != _touchData->previousState) {
            ESP_LOGD(TAG, "release updating baseline..");            
            updateBaseLine(filteredValue);
        }
    }

    if (true == TimeUtils::isPeriodPassed(_touchData->baseLineUpdatedTs, AUTO_CALIBRATION_PERIOD_S * 1000UL)
        and true == isCurrentStateConstFor(AUTO_CALIBRATION_PERIOD_S * 1000UL)) {
        ESP_LOGD(TAG, "auto updating baseline..");
        updateBaseLine(filteredValue);
    }

    _touchData->previousState = ret;
    return ret;
}

void TouchButton::updateBaseLine(const uint16_t value) const {
    if (false == TimeUtils::isPeriodPassed(_touchData->baseLineUpdatedTs, TOUCH_BUTTON_UPDATE_BASELINE_PERIOD)) {
        return;
    }
    _touchData->baseLineUpdatedTs = TimeUtils::nowMs();
    _touchData->baseLine = value;
    ESP_LOGD(TAG, "baseLine updated");
}
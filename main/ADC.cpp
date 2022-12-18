#include "ADC.hpp"
#include "driver/adc.h"
#include "Utils/threadSafeDbg.hpp"
#include "SmartCase.hpp"
#include "BatteryMeter.hpp"
#include "GUI.hpp"
#include "AdcCalibrator.hpp"
#include "ESP32.hpp"

#define ADC_TASK_PERIOD_MS          5000UL
#define ADC_RESOLUTION              adc_bits_width_t::ADC_WIDTH_BIT_12
#define ADC_DEFAULT_VREFF           1100UL
#define ADC_ATTENUATION             adc_atten_t::ADC_ATTEN_DB_6
#define ADC_CHANNEL_BATTERY         adc1_channel_t::ADC1_CHANNEL_6
#define BATTERY_MEASUREMENT_COUNT   10
#define BATTERY_CIRCUIT_DIVIDER_L   10UL
#define BATTERY_CIRCUIT_DIVIDER_H   20UL
#define BATTERY_LOW_VOLTAGE         3250UL
#define BATTERY_HIGH_VOLTAGE        4150UL
#define ADC_GET_RAW_ERROR_CODE      -1

static const char* const TAG {"ADC"};

ADC::ADC()
    :   _adcChars{nullptr} {

    setName(TAG);
    setPriority(1);
    setStackSize(configMINIMAL_STACK_SIZE * 4);
}

void ADC::run(void* args) {

    int16_t delta {_deltas.delta_db2_5};
    static BatteryMeter battery {BATTERY_LOW_VOLTAGE, BATTERY_HIGH_VOLTAGE};
    while(1) {

        int32_t rawAdcSum {};
        for (size_t i{}; i < BATTERY_MEASUREMENT_COUNT; i++) {
            int rawAdc {adc1_get_raw(ADC_CHANNEL_BATTERY)};
            assert(ADC_GET_RAW_ERROR_CODE != rawAdc);
    
            rawAdc += delta;
            if (rawAdc < 0) {
                rawAdc = 0;
            }
            rawAdcSum += rawAdc;
            delay(10);
        }
        rawAdcSum /= BATTERY_MEASUREMENT_COUNT;

        uint32_t mVolts {esp_adc_cal_raw_to_voltage(rawAdcSum, _adcChars)};

        if(mVolts == _zeroOffsset) {
            mVolts = 0;
        }

        // mVolts = rand() % 400 + 1000;
        // DBG_PRINT_TAG(TAG, "mVolts: %u", mVolts);

        const uint32_t batteryVotlage {mVolts * (BATTERY_CIRCUIT_DIVIDER_L + BATTERY_CIRCUIT_DIVIDER_H) / BATTERY_CIRCUIT_DIVIDER_L};
        std::string toast {stringUtils::format("battery voltage: %u", batteryVotlage)};
        //GRAPHICS.addToast(toast);
        DBG_PRINT_TAG(TAG, "batteryVotlage: %u", batteryVotlage);
        SMARTCASE.set_batteryPercent(battery.chargePercent(batteryVotlage));   

        // DBG_PRINT_TAG(TAG, "batteryPercent: %u", SMARTCASE.get_batteryPercent());


        delay(ADC_TASK_PERIOD_MS);
    }
}

void ADC::init() {

    AdcCalibrator calibrator;

    if (false == calibrator.loadCalibrationData()) {
        DBG_PRINT_TAG(TAG, "failed to load calibration data..");
        calibrator.makeCalibration();
        calibrator.saveCalibrationDate();
        DBG_PRINT_TAG(TAG, "rebooting now...");
        delay(100);
        ESP32Utils::reboot();
    }
    else {
        _deltas = calibrator.calibrationData();
    }

    adc1_config_width(ADC_RESOLUTION);

    esp_adc_cal_value_t calRes {esp_adc_cal_value_t::ESP_ADC_CAL_VAL_DEFAULT_VREF};
    // // TODO: Try to read Vreff Value in SPIFFS settings file
    uint32_t defaultVreff = ADC_DEFAULT_VREFF;

    _adcChars = static_cast<esp_adc_cal_characteristics_t*>(calloc(1, sizeof(esp_adc_cal_characteristics_t)));
    calRes =  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTENUATION, ADC_RESOLUTION, defaultVreff, _adcChars);
    
    if (calRes == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        DBG_PRINT_TAG(TAG, "ADC calibrated using efuse_Vreff = %u", _adcChars->vref);
    }
    else if (calRes == ESP_ADC_CAL_VAL_EFUSE_TP) {
        DBG_PRINT_TAG(TAG, "ADC calibrated using efuse_two_point");
    }
    else {
        DBG_PRINT_TAG(TAG, "ADC calibratied with default vReff = %u", defaultVreff);
    }

    _zeroOffsset = esp_adc_cal_raw_to_voltage(0, _adcChars);

    adc1_config_channel_atten(ADC_CHANNEL_BATTERY, ADC_ATTENUATION);
}
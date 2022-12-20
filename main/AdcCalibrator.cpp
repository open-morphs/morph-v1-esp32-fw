#include <AdcCalibrator.hpp>
#include <tuple>
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/task.h"
#include <Utils/ThreadSafeDbg.hpp>
#include <NVS.hpp>

static const char* const fileName {"/adc/deltas"};
static const char* const TAG {"CALIBRATOR"};
#define CALIBRATION_GPIO    GPIO_NUM_14
#define CALIBRATION_CHANNEL ADC2_CHANNEL_6

namespace AdcCalibratorJsonKeys {
    const char* const deltaKey_0db {"db_0"};
    const char* const deltaKey_2_5db {"db_2_5"};
    const char* const deltaKey_6db {"db_6"};
    const char* const deltaKey_11db {"db_11"};
};

AdcCalibrator::AdcCalibrator()
    :  _deltaData{} {

}

AdcDelta_t AdcCalibrator::calibrationData() const {
    return _deltaData;
}

void AdcCalibrator::makeCalibration() {

    DBG_PRINTF("making adc calibration...");
    adc1_config_width(ADC_WIDTH_BIT_12);

    
    // calibration curves initialization
    esp_adc_cal_characteristics_t* adc_2_char_2_5_db = static_cast<esp_adc_cal_characteristics_t*>(calloc(1, sizeof(esp_adc_cal_characteristics_t)));
    esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_2_5, ADC_WIDTH_BIT_12, 1100, adc_2_char_2_5_db);
    esp_adc_cal_characteristics_t* adc_2_char_6_db = static_cast<esp_adc_cal_characteristics_t*>(calloc(1, sizeof(esp_adc_cal_characteristics_t)));
    esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_6, ADC_WIDTH_BIT_12, 1100, adc_2_char_6_db);
    esp_adc_cal_characteristics_t* adc_2_char_11_db = static_cast<esp_adc_cal_characteristics_t*>(calloc(1, sizeof(esp_adc_cal_characteristics_t)));
    esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, adc_2_char_11_db);

    // reading efuse Vreff mV value
    const uint32_t eFuseVreff {adc_2_char_2_5_db->vref};
    DBG_PRINTF("eFuseVreff: %u\r\n", eFuseVreff);

    // putting Vreff on GPIO26 (DAC2)
    ESP_ERROR_CHECK(adc2_vref_to_gpio(CALIBRATION_GPIO));
    vTaskDelay(100/portTICK_RATE_MS);

    // calculating Vreff adc value    
    uint32_t totalVreff [3]{};
    const uint8_t vreffReadings {20};
    for (int i{} ; i < vreffReadings; i++) {
        int rawAdc {0};
        adc2_config_channel_atten(CALIBRATION_CHANNEL, ADC_ATTEN_DB_2_5);
        vTaskDelay(50/portTICK_RATE_MS);
        ESP_ERROR_CHECK(adc2_get_raw(CALIBRATION_CHANNEL, adc_bits_width_t::ADC_WIDTH_BIT_12, &rawAdc));
        totalVreff[0] += rawAdc;

        adc2_config_channel_atten(CALIBRATION_CHANNEL, ADC_ATTEN_DB_6);
        vTaskDelay(50/portTICK_RATE_MS);
        ESP_ERROR_CHECK(adc2_get_raw(CALIBRATION_CHANNEL, adc_bits_width_t::ADC_WIDTH_BIT_12, &rawAdc));
        totalVreff[1] += rawAdc;

        adc2_config_channel_atten(CALIBRATION_CHANNEL, ADC_ATTEN_DB_11);
        vTaskDelay(50/portTICK_RATE_MS);
        ESP_ERROR_CHECK(adc2_get_raw(CALIBRATION_CHANNEL, adc_bits_width_t::ADC_WIDTH_BIT_12, &rawAdc));
        totalVreff[2] += rawAdc;
        vTaskDelay(50/portTICK_RATE_MS);
    }
    const uint16_t vReffAdc[3] {
        static_cast<uint16_t>(totalVreff[0] / vreffReadings),
        static_cast<uint16_t>(totalVreff[1] / vreffReadings),
        static_cast<uint16_t>(totalVreff[2] / vreffReadings),
    };
    DBG_PRINTF("vReffAdc: %u\r\n", vReffAdc[0]);
    DBG_PRINTF("vReffAdc: %u\r\n", vReffAdc[1]);
    DBG_PRINTF("vReffAdc: %u\r\n", vReffAdc[2]);

    uint16_t calcAdcVreff_25db{};
    uint16_t calcAdcVreff_6db{};
    uint16_t calcAdcVreff_11db{};

    for (uint32_t i = 0; i < 4096 ; i++) {
        uint32_t calcEfuseVreff_25_db = esp_adc_cal_raw_to_voltage(i, adc_2_char_2_5_db);
        uint32_t calcEfuseVreff_6_db = esp_adc_cal_raw_to_voltage(i, adc_2_char_6_db);
        uint32_t calcEfuseVreff_11_db = esp_adc_cal_raw_to_voltage(i, adc_2_char_11_db);

        if (eFuseVreff < calcEfuseVreff_25_db && !calcAdcVreff_25db) {
            calcAdcVreff_25db = i;
        }
        if (eFuseVreff < calcEfuseVreff_6_db && !calcAdcVreff_6db) {
            calcAdcVreff_6db = i;
        }
        if (eFuseVreff < calcEfuseVreff_11_db && !calcAdcVreff_11db) {
            calcAdcVreff_11db = i;
        }

        if (calcAdcVreff_11db and calcAdcVreff_6db and calcAdcVreff_25db) {
            break;
        }
    }

    const int32_t delta {calcAdcVreff_25db - vReffAdc[0]};

    _deltaData = {
        (int16_t)(delta/1.33),
        (int16_t)delta,
        (int16_t)(delta * 1.4),
        (int16_t)(delta * 1.4)
    };

    DBG_PRINTF("delta_db0: %d\r\n", _deltaData.delta_db0);
    DBG_PRINTF("delta_db2_5: %d\r\n", _deltaData.delta_db2_5);
    DBG_PRINTF("delta_db_6: %d\r\n", _deltaData.delta_db_6);
    DBG_PRINTF("delta_db11: %d\r\n", _deltaData.delta_db11);
}

bool AdcCalibrator::loadCalibrationData() {
    DBG_PRINT_TAG(TAG, "loading calibration data...");

    std::string content{};
    NVS nvs;
    bool readRes{nvs.read("adc", "calibration", content)};

    if (!readRes) {
        DBG_PRINTLN("failed to load file with adc calibration");
        return false;
    }

    DBG_PRINT_TAG(TAG, "content: %s", content.c_str());

    StaticJsonDocument<1024> jsonDoc;

    const DeserializationError err {deserializeJson(jsonDoc, content)};

    if (DeserializationError::Ok != err) {
        DBG_PRINT_TAG(TAG, "failed to deserialize");
        return false;
    }

    if (false == jsonDoc.is<JsonObject>()) {
        DBG_PRINT_TAG(TAG, "not JsonObject");
        return false;
    }

    JsonObject rootObj = jsonDoc.as<JsonObject>();
    if (false == isDocValid(rootObj)) {
        DBG_PRINT_TAG(TAG, "not valid");
        return false;
    }

    _deltaData = {
        rootObj[AdcCalibratorJsonKeys::deltaKey_0db],
        rootObj[AdcCalibratorJsonKeys::deltaKey_2_5db],
        rootObj[AdcCalibratorJsonKeys::deltaKey_6db],
        rootObj[AdcCalibratorJsonKeys::deltaKey_11db]
    };

    return true;
}

void AdcCalibrator::saveCalibrationDate() {

    DBG_PRINT_TAG(TAG, "saving calibration data...");

    NVS nvs;
    StaticJsonDocument<1024> jsonDoc;
    JsonObject rootObject = jsonDoc.to<JsonObject>();

    rootObject[AdcCalibratorJsonKeys::deltaKey_0db] = _deltaData.delta_db0;
    rootObject[AdcCalibratorJsonKeys::deltaKey_2_5db] = _deltaData.delta_db2_5;
    rootObject[AdcCalibratorJsonKeys::deltaKey_6db] = _deltaData.delta_db_6;
    rootObject[AdcCalibratorJsonKeys::deltaKey_11db] = _deltaData.delta_db11;

    std::string content {""};
    serializeJson(jsonDoc, content);

    const bool saveRes {nvs.write("adc", "calibration", content.c_str())};

    if (!saveRes) {
        DBG_PRINTLN("failed to save file to NVS!");
        return;
    }
}

bool AdcCalibrator::isDocValid(const JsonObject obj) const {

    if (!obj.containsKey(AdcCalibratorJsonKeys::deltaKey_0db) || 
        !obj.containsKey(AdcCalibratorJsonKeys::deltaKey_2_5db) || 
        !obj.containsKey(AdcCalibratorJsonKeys::deltaKey_6db) || 
        !obj.containsKey(AdcCalibratorJsonKeys::deltaKey_11db)) {
        return false;
    }

    if (!obj[AdcCalibratorJsonKeys::deltaKey_0db].is<int16_t>() || 
        !obj[AdcCalibratorJsonKeys::deltaKey_2_5db].is<int16_t>() || 
        !obj[AdcCalibratorJsonKeys::deltaKey_6db].is<int16_t>() || 
        !obj[AdcCalibratorJsonKeys::deltaKey_11db].is<int16_t>()) {
        return false;
    }

    return true;    
}

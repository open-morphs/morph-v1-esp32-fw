#pragma once

#include "ITask.hpp"
#include "SingletonTemplate.hpp"
#include <esp_adc_cal.h>
#include "AdcCalibrator.hpp"


class ADC : public Singleton<ADC>, public ITask {
    friend class Singleton;

public:
    void init();

private:
    ADC();

    void run(void *args) override;

private:
    esp_adc_cal_characteristics_t *_adcChars;
    uint32_t _zeroOffsset;
    AdcDelta_t _deltas;
};

#define ADC_MODULE ADC::instance()
#pragma once
#include "stdint.h"
#include "ArduinoJson-v6.18.4.h"

typedef struct {
    int16_t delta_db0;
    int16_t delta_db2_5;
    int16_t delta_db_6;
    int16_t delta_db11;
} AdcDelta_t;

class AdcCalibrator {

    public:
        AdcCalibrator();
        bool loadCalibrationData();
        void makeCalibration();
        void saveCalibrationDate();
        AdcDelta_t calibrationData() const;
    private:
        bool isDocValid(const JsonObject obj) const;
    private:
        AdcDelta_t _deltaData;
};
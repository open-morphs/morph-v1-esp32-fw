#pragma once

#include <stdint.h>

namespace DoubleInt16ToInt32 {
    int32_t combineTwoInt16ToInt32(const int16_t left, const int16_t right);

    int16_t leftInt16FromInt32(const int32_t num);

    int16_t rightInt16FromInt32(const int32_t num);
};
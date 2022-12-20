#include <Utils/DoubleInt16ToInt32.hpp>

int32_t DoubleInt16ToInt32::combineTwoInt16ToInt32(const int16_t left, const int16_t right) {
    const uint16_t regLeftPart{static_cast<uint16_t>(left)};
    const uint16_t regRightPart{static_cast<uint16_t>(right)};
    const uint32_t reg{static_cast<uint32_t>(regLeftPart << 16) + regRightPart};
    return static_cast<int32_t> (reg);
}

int16_t DoubleInt16ToInt32::leftInt16FromInt32(const int32_t num) {
    uint32_t bits{static_cast<uint32_t>(num) >> 16};
    return static_cast<int16_t>(bits);
}

int16_t DoubleInt16ToInt32::rightInt16FromInt32(const int32_t num) {
    uint32_t bits{static_cast<uint32_t>(num) & 0xFFFF};
    return static_cast<int16_t>(bits);
}

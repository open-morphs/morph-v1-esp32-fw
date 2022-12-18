#pragma once

#include "stdint.h"

#define MAX_DEVICE_NAME_LENGTH 29UL

// just simple DTO wiht public members
struct PairedDevice {
    uint64_t macAddress;
    uint32_t btProfileMask;
    char name[MAX_DEVICE_NAME_LENGTH + 1];
    bool operator<(const PairedDevice other) const{
        return macAddress < other.macAddress;
    }
};
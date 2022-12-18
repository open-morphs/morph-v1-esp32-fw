#pragma once
#include "stdint.h"
#include "cstring"

#define MAC_LENGTH                  6UL
#define BT_PROFILE_MASK_LENTH       4UL

namespace MacUtils {
    inline uint64_t macFromRawBytes(const uint8_t* const bytes) {
        uint64_t tmp {};
        memcpy((uint8_t*)&tmp, bytes, MAC_LENGTH);
        tmp = __builtin_bswap64(tmp);
        tmp = (tmp >> 16);
        return tmp;
    }

    inline uint32_t profileMaskFromRawBytes(const uint8_t* const bytes) {
        uint32_t tmp {};
        memcpy((uint8_t*)&tmp, bytes, BT_PROFILE_MASK_LENTH);
        tmp = __builtin_bswap32(tmp);
        return tmp;
    }
    
    inline void macToByteArray(const uint64_t mac, uint8_t* dest) {
        uint64_t tmpMac {mac};
        tmpMac = __builtin_bswap64(tmpMac);
        tmpMac = (tmpMac >> 16);
        memcpy(dest, (uint8_t*)&tmpMac, MAC_LENGTH);
    }

    inline void profileMaskToByteArray(const uint32_t profileMask, uint8_t* dest) {
        uint32_t tmp {profileMask};
        tmp = __builtin_bswap32(tmp);
        memcpy(dest, (uint8_t*)&tmp, BT_PROFILE_MASK_LENTH);
    }
};
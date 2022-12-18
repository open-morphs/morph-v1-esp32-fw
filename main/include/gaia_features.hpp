#pragma once
#include "stdint.h"


enum class eQccV3Features : uint8_t {
    QCC_V3_FEATURE_FRAMEWORK    = 0x00,     
    QCC_V3_FEATURE_APPLICATION  = 0x01,     
    QCC_V3_FEATURE_FW_UPGRADE   = 0x06,     
    QCC_V3_FEATURE_AUDIOCURATION= 0x08,     
    QCC_V3_FEATURE_FIT_STATUS   = 0x09,     
    QCC_V3_FEATURE_VOICE        = 0x03,     
    QCC_V3_FEATURE_DBG_CMDS     = 0x04,     
    QCC_V3_FEATURE_MUSIC_PRCS   = 0x05,     
    QCC_V3_FEATURE_MAX          = 0x7F,
};
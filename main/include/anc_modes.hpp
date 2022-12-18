#pragma once

#ifdef ANC_TRANSP_NEW_VERSION
enum eAncMode {
    ANC_ADAPTIVE = 0x01,
    ANC_TRANSP = 0x02,
};
#else
enum eAncMode {
    ANC_ADAPTIVE = 0x01,
    ANC_STATIC_BALANCED = 0x02,
    ANC_STATIC_DEEP_FLAT = 0x03,
    ANC_STATIC_WIDE_TUNNING = 0x04,
    ANC_TRANSP = 0x05,
};
#endif
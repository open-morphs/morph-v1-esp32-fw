#pragma once
#include <stdint.h>

enum class eMorphNotifications : uint8_t {
    MORPH_NOTIFICATION_VOLUME_CHANGED = 1,                  // that's it, volume has changed
    MORPH_NOTIFICATION_HANDSET_CONNECTED_RMT,               // successfull connection to handset (multiple times for HFP, AVRCP, A2DP )
    MORPH_NOTIFICATION_HANDSET_DISCONNECTED_RMT,            // successfull disconnection from handset
    MORPH_NOTIFICATION_CONNECTION_ATTEMPT_COMPLETED,        // connection attempt finished (with fail/success)
    MORPH_NOTIFICATION_DISCONNECTION_ATTEMPT_COMPLETED,     // disconnection attempt finished (with fail/success)
    MORPH_NOTIFICATION_MIC_STATE_CHANGED,                   // that's it, mic status has changed
};
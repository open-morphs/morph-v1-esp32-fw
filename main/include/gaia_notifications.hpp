#pragma once



// GAIA Framework Feature Notifications
enum class eQccV3Notifications_Framework {
    QCC_V3_NOTIFICATION_CHARGER_STATUS = 0,             // charger plugged/unplugged
};

// Earbud Application Feature Notifications 
enum class eQccV3Notifications_EarbudApplication {
    QCC_V3_NOTIFICATION_PRIMARY_ABOUT_TO_CHANGE = 0,    // primary earbud gona be changed
    QCC_V3_NOTIFICATION_PRIMARY_CHANGED,                // primary earbud gona be changed

};

// Firmware upgrade Feature Notifications 
enum class eQccV3Notifications_FirmwareUpgrade {
    QCC_V3_NOTIFICATION_UPGRADE_DATA = 0,               // upgrade messages from device (in process???)
    QCC_V3_NOTIFICATION_UPGRADE_TO_STOP,                // device would like upgrade to stop
    QCC_V3_NOTIFICATION_UPGRADE_START,                  // device would like upgrade to (re)start
};

// Audio Curation Feature Notifications 
enum class eQccV3Notifications_AudioCuration {
    QCC_V3_NOTIFICATION_STATE_CHANGED = 0,
    QCC_V3_NOTIFICATION_MODE_CHANGED,
    QCC_V3_NOTIFICATION_GAIN_CHANGED,
    QCC_V3_NOTIFICATION_TOGGLE_CFG,
    QCC_V3_NOTIFICATION_SENARIO_CFG,
    QCC_V3_NOTIFICATION_DEMO_STATE,
    QCC_V3_NOTIFICATION_ADAPT_STATUS_CHANGED,
};

// Fit status Feature Notifications 
enum class eQccV3Notifications_FitStatus {
    QCC_V3_NOTIFICATION_FIT_STATUS = 0,                 // earbuds fit status
};

// Voice Feature Notifications 
enum class eQccV3Notifications_Voice {
    QCC_V3_NOTIFICATION_ASSISTANT_CHANGED = 0,          // when the active Voice Assistant is changed
};

// Music Processing Notifications 
enum class eQccV3Notifications_MusicProcessing{
    QCC_V3_NOTIFICATION_EQ_SET_CHANGED = 0,             // will be told if the User EQ is not present
    QCC_V3_NOTIFICATION_EQ_BAND_CHANGED,                // there are User EQ band changes

};
#pragma once
#include <stdint.h>



// Feature framework feature 
enum class eQccV3Commands_Framework : uint8_t {
    QCC_V3_COMMAND_API_VERSION = 0,             // get api version
    QCC_V3_COMMAND_SUPPORTED_FEATURES,          // get supported features
    QCC_V3_COMMAND_FEATURES_NEXT,               // list continuation of features the device supports
    QCC_V3_COMMAND_SERIAL_NUMBER,               // get serial number
    QCC_V3_COMMAND_VARIANT,                     // gets the customer-provided variant name
    QCC_V3_COMMAND_APPLICATION_VERSION,         // gets the customer-provided application version number
    QCC_V3_COMMAND_DEVICE_RESET,                // does a warm reset
    QCC_V3_COMMAND_REGISTER_NOTIFICATIONS,      // to receive all notifications from a Feature
    QCC_V3_COMMAND_UNREGISTER_NOTIFICATIONS,    // to stop receiving Feature notifications
    QCC_V3_COMMAND_DATA_TRANSFER_SETUP,         // to set up a data transfer channel
    QCC_V3_COMMAND_DATA_TRANSFER_GET,           // to request the device to senddata bytes as the response
    QCC_V3_COMMAND_DATA_TRANSFER_SET,           // to send data bytes to the device
    QCC_V3_COMMAND_TRANSPORT_INFO,              // to get information about the GAIA transport
    QCC_V3_COMMAND_SET_TRANSPORT_PARAM,         // to request changing a GAIA transport parameter
    QCC_V3_COMMAND_GET_USER_FEATURE,            // to read the user feature data from the device
    QCC_V3_COMMAND_GET_USER_FEATURE_NEXT,       // n to read the continuous part of the user feature data
    MAX
};

// Feature framework feature 
enum class eQccV3Commands_Application : uint8_t {
    QCC_V3_COMMAND_WHICH_PRIMARY = 0,            // get primary device
    MAX
};

// Audio curaation feature 
enum class eQccV3Commands_AudioCuration : uint8_t {
    QCC_V3_COMMAND_GET_STATE = 0,               // get anc state (on/off)
    QCC_V3_COMMAND_SET_STATE,                   // set anc state (on/off)
    QCC_V3_COMMAND_GET_MODE_COUNTS,             // get anc modes count
    QCC_V3_COMMAND_GET_CURR_MODE,               // get current anc mode
    QCC_V3_COMMAND_SET_MODE,                    // set anc current mode
    QCC_V3_COMMAND_GET_GAIN,                    // get gain
    QCC_V3_COMMAND_SET_GAIN,                    // set gain
    QCC_V3_COMMAND_GET_CONF_COUNT,              // get configurations count
    QCC_V3_COMMAND_GET_TOGGLE_CONF,             // number of toggle configurations
    QCC_V3_COMMAND_SET_TOGGLE_CONF,             // sets the toggle option the user can toggle using controls
    QCC_V3_COMMAND_GET_SCENARIO_CFG,            // gets the behavior configured for a specific scenario      
    QCC_V3_COMMAND_SET_SENARIO_CFG,             // sets the behavior to be used when in different scenarios
    QCC_V3_COMMAND_GET_DEMO_SUP,                // if the device supports demonstration state
    QCC_V3_COMMAND_GET_DEMO_STATE,              // if the device is currently in a demonstration state
    QCC_V3_COMMAND_SET_DEMO_STATE,              // AC is to be moved into or out of demo
    QCC_V3_COMMAND_GET_ADAPT_CTRL_STATUS,       // gets the state of gain adaptation
    QCC_V3_COMMAND_SET_ADAPT_CTRL_STATUS,       // sets the state of gain adaptation
    MAX,
};

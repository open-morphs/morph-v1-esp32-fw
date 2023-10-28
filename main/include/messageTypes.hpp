#pragma once

#include <stdint.h>

typedef enum : uint8_t {
    MESSAGE_BLE_CONNECTION_STATE_CHANGED,
    MESSAGE_EARBUDS_CHARGE_LEVEL_CHANGED,
    MESSAGE_ANC_STATE_CHANGED,
    MESSAGE_ANC_MODE_CHANGED,
    MESSAGE_MIC_STATE_CHANGED,
    MESSAGE_VOLUME_CHANGED,
    MESSAGE_HANDSET_INFO_CHANGED,
    MESSAGE_HANDSET_LIST_CHANGED,
    MESSAGE_HANDSET_CONNECTION_STARTED,
    MESSAGE_HANDSET_CONNECTION_FINISHED,
    MESSAGE_HIDE_LOADING_PAGE,
    MESSAGE_WEBSERVER_STATE_CHANGED,
    MESSAGE_PRIMARY_DEVICE_CHANGED,
    MESSAGE_OTA_STARTED,
    MESSAGE_OTA_FAILED,
    MESSAGE_TOAST_ADDED,

    MESSAGE_HANDSET_STATE_CHANGED,
    MESSAGE_HANDSET_NUMBER_RECEIVED,
}eMessageCode;


#pragma pack(push, 1)
typedef union messageData_t{
  
    uint8_t rawBytes[32];
    
    uint32_t ui32Param;         // 4

    uint8_t ui8Param;           // 1

    void* pAnyData;             // 4

    const char* pStringData;    // 4

    uint64_t ui64Param;         // 8

    bool booleanParam;          // 1

    char stringParam[32];       // 32

}messageData_t; // 32
#pragma pack(pop)


typedef struct message_t{
    eMessageCode messageCode;   // 1
    messageData_t data;         // 35
}message_t; // 36

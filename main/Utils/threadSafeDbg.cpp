#include "Utils/threadSafeDbg.hpp"
#include "Utils/stringUtils.hpp"
#include <cstdarg>

static const char* const TAG = "DBG";
#define MAX_QUEUE_LENGTH    50
#define MAX_MSG_LENGTH      2000UL

threadSafeDbg::threadSafeDbg()
    :   allowPrint(true) {
    init();
}

void threadSafeDbg::init() {
    _dbgQueue = xQueueCreate(MAX_QUEUE_LENGTH, sizeof(debugMessage_t));
    setPriority(1);
    setStackSize(configMINIMAL_STACK_SIZE*3);
}

void threadSafeDbg::run(void* data) {

    DBG_PRINT_TAG(TAG, "started!");
    while(1) {

        debugMessage_t message;
        if (pdTRUE == xQueueReceive(_dbgQueue, &message, 1000/portTICK_RATE_MS)) {
            printf(message.pointer);
            delete[] message.pointer; 
        }
    }   
}

void threadSafeDbg::printHex(const uint8_t byte) {
    if (false == allowPrint) {
        return;
    }
    std::string str {stringUtils::format("%02X", byte)};
    doPrint(str.c_str());
}

void threadSafeDbg::printLine() {
    if (false == allowPrint) {
        return;
    }
    const char* const newLn = "\r\n";
    doPrint(newLn);
}

void threadSafeDbg::printLine(const string& message) {
    if (false == allowPrint) {
        return;
    }
    std::string str {message};
    str.append("\r\n");
    doPrint(str.c_str());
}

void threadSafeDbg::printLine(const char * const message) {
    if (false == allowPrint) {
        return;
    }
    std::string str {message};
    str.append("\r\n");
    doPrint(str.c_str());
}

void threadSafeDbg::doPrint(const char * const message) {
    
        if (false == allowPrint) {
            return;
        }
        size_t length {strlen(message)};
        size_t offset {0};

        while (length > MAX_MSG_LENGTH) {
            char* buff = new char[MAX_MSG_LENGTH+1];
            memcpy(buff, message + offset, MAX_MSG_LENGTH);
            buff[MAX_MSG_LENGTH] = '\0';
            debugMessage_t messageRx = {.pointer = buff};
            xQueueSendToBack(_dbgQueue, &messageRx, portMAX_DELAY);

            offset += MAX_MSG_LENGTH;
            length -= MAX_MSG_LENGTH;
        }

        char* buff = new char[length+1];
        memcpy(buff, message + offset, length);
        buff[length] = '\0';
        debugMessage_t messageRx = {.pointer = buff};
        xQueueSendToBack(_dbgQueue, &messageRx, portMAX_DELAY);
}

void threadSafeDbg::setAllowPrint(const bool val) {

    allowPrint = val;
}

void threadSafeDbg::print(const char* const message) {
    doPrint(message);
}
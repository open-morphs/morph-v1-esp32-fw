#pragma once

#include "messageTypes.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static const uint32_t queueLength {5};
static const UBaseType_t defaultConsumerPriority {5};

static const uint32_t infinityRxTimeout = 0xFFFFFFFF;

class MessageConsumer {

    public:
        explicit MessageConsumer(const uint32_t queueLength = queueLength)
            : _queueRx (xQueueCreate(queueLength, sizeof(message_t))) {
        }

        ~MessageConsumer() {
            vQueueDelete(_queueRx);
        }

        xQueueHandle getRxQueue() {
            return _queueRx;
        }

        bool consumeMessage(message_t& out, const uint32_t timeoutMs = infinityRxTimeout) {
            assert(NULL != _queueRx);
            bool ret = false;
            if (pdTRUE == xQueueReceive(_queueRx, &out, (infinityRxTimeout == timeoutMs)? portMAX_DELAY : (timeoutMs/portTICK_RATE_MS))) {
                ret = true;
            }
            return ret;
        }
    private:
        xQueueHandle _queueRx;
};
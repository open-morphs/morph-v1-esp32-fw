#pragma once

#include "MessageTypes.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "Utils/ThreadSafeDbg.hpp"

class MessageProducer {

    public:
    static const uint32_t infinityTxTimeout {0xFFFFFFFF};
    static const uint32_t defaultTxTimeoutMs {100};
        explicit MessageProducer(xQueueHandle queue = NULL)
                        : _txQueue(queue) {
        }
        void setOutcomingQueue(xQueueHandle queue) {
            _txQueue = queue;
        }
        xQueueHandle getOutcomingQueue()const {
            return _txQueue;
        }

        bool produceMessage(const message_t& message, const uint32_t timeoutMs = defaultTxTimeoutMs) {
            assert(NULL != _txQueue);
            bool ret = false;
            if (pdTRUE == xQueueSendToBack(_txQueue, &message, timeoutMs == infinityTxTimeout?
                             portMAX_DELAY : timeoutMs/portTICK_RATE_MS)) {
                ret = true;
            }
            assert(true == ret);
            return ret;
        }
    protected:
        xQueueHandle _txQueue;
};
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

struct ResourceAccessor {
    explicit ResourceAccessor(xSemaphoreHandle smphr)
        : _smphr(smphr) {
        xSemaphoreTakeRecursive(_smphr, portMAX_DELAY);
    }

    ~ResourceAccessor() {
        xSemaphoreGiveRecursive(_smphr);
    }

    private:
       xSemaphoreHandle _smphr;
};
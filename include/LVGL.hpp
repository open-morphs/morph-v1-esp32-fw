#pragma once

#include "ITask.hpp"
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class LVGL : public ITask {

public:
    explicit LVGL(const size_t periodMs = 10);

    xSemaphoreHandle acceccMutex();

    void startAccess();

    void finishAccess();

    bool init();

private:
    void run(void *args) override;
};
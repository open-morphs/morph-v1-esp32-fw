#include "LVGL.hpp"
#include "lvgl_helpers.h"

static lv_disp_buf_t _dispBuf;
static lv_color_t* _colorBuf{nullptr};

static void lv_tick_task(void *arg) {
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

LVGL::LVGL(const size_t periodMs) {

    setStackSize(configMINIMAL_STACK_SIZE*4);
    setName("LVGL");
    setPeriod(periodMs);
    setPriority(5);
}

xSemaphoreHandle LVGL::acceccMutex() {
    static xSemaphoreHandle mutex = xSemaphoreCreateRecursiveMutex();
    return mutex;
}

void LVGL::startAccess() {
    xSemaphoreTakeRecursive(acceccMutex(), portMAX_DELAY);
}

void LVGL::finishAccess() {
    xSemaphoreGiveRecursive(acceccMutex());
}

void LVGL::run(void* args) {

    while (1) {
        startAccess();
        lv_task_handler();
        finishAccess();
        delay(_periodMs);        
    }
}

bool LVGL::init() {

    lv_init();

    lvgl_driver_init();

    _colorBuf = new lv_color_t[DISP_BUF_SIZE];
    assert(_colorBuf != NULL);
    lv_disp_buf_init(&_dispBuf, _colorBuf, NULL, DISP_BUF_SIZE);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.buffer = &_dispBuf;
    
    assert (nullptr != lv_disp_drv_register(&disp_drv));

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    lv_indev_drv_t indev_drv;

    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touch_driver_read;  
    if (nullptr == lv_indev_drv_register(&indev_drv)) {
        return false;
    }

    return true;
}
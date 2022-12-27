#pragma once

#include "LVGLLabel.hpp"
#include "FreeRTOS.h"
#include "freertos/semphr.h"

class LVGLToaster : public LVGLLabel {

    public:
        eLvglType type() const override {return eLvglType::LVGL_CLASS_TOASTER;}
        explicit LVGLToaster(SemaphoreHandle_t smphr, LVGLBase* const parent = NULL);
        explicit LVGLToaster(SemaphoreHandle_t smphr, lv_obj_t* const parent);
        void setToast(const std::string& txt);
        void setTime(const size_t timeMs);
    private:
        static void toastAppearedCallback(lv_anim_t* old_anim);
        static void toastReadyCallback(lv_anim_t* old_anim);
        size_t _toastTimeMs;
        lv_anim_t _anim;
        SemaphoreHandle_t _tostReadymphr = NULL;
};
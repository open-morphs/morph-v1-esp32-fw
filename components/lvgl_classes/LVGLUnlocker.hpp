#pragma once

#include "LVGLSlider.hpp"

class LVGLUnlocker : public LVGLSlider {
    typedef std::function<void(LVGLUnlocker*)> unlockCallBack_t;
    public:
        eLvglType type() const override {return eLvglType::LVGL_CLASS_UNLOCKER;}
        explicit LVGLUnlocker(const uint8_t unlockThreshold, LVGLBase* const parent = NULL);
        explicit LVGLUnlocker(const uint8_t unlockThreshold, lv_obj_t* const parent);
        void onUnlock(unlockCallBack_t cb);
    private:
        void init() override;
        unlockCallBack_t _onUnlockCallback = nullptr;
        uint8_t _unlockPercentThreshold;
};

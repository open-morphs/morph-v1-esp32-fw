#pragma once
#include "LVGLLabel.hpp"
#include "AbstractValueChangable.hpp"

enum class eBatteryState {
    BATTERY_EMPTY,
    BATTERY_25,
    BATTERY_50,
    BATTERY_75,
    BATTERY_FULL
};

enum class eBatteryIndicatorSize {
    BATTERY_SIZE_SMALL,
    BATTERY_SIZE_MEDIUM,
    BATTERY_SIZE_LARGE,
    BATTERY_SIZE_HUGE,
};

class LVGLBatteryIndicator : public AbstractValueChangable<uint8_t>, public LVGLLabel {
    public:
        eLvglType type() const override {return eLvglType::LVGL_CLASS_BATTERY_IND;}
        explicit LVGLBatteryIndicator(const eBatteryIndicatorSize size, const lv_color_t color, LVGLBase* const parent = NULL);
        void setValue(const uint8_t value) override;
    private:
        eBatteryState stateByValue(const uint8_t value) const;
        void setChargeLevel(const uint8_t value);
        void changed() override;
    private:
        lv_obj_t* _canvas = nullptr;
        lv_color_t* _colorBuff = nullptr;
        size_t _canvasWidth;
        size_t _canvasHeight;
        lv_color_t _color;
};
#include "LVGLUnlocker.hpp"

LVGLUnlocker::LVGLUnlocker(const uint8_t unlockThreshold, LVGLBase* const parent) 
    :   LVGLSlider(0, 100, 1, parent),
        _unlockPercentThreshold{unlockThreshold} {

    init();
}

LVGLUnlocker::LVGLUnlocker(const uint8_t unlockThreshold, lv_obj_t* const parent)
    :   LVGLSlider(0, 100, 1, parent), 
        _unlockPercentThreshold{unlockThreshold} {

    init();
    applyTheme();
}


void LVGLUnlocker::init() {

    lv_obj_set_adv_hittest(_obj, true);

    static lv_style_t defaultKnobStyle;

    lv_style_init(&defaultKnobStyle);
    lv_style_set_radius(&defaultKnobStyle, LV_STATE_DEFAULT, 5);

    const size_t horPad { static_cast<size_t>(width() * 0.04)};
    const size_t vertPad { static_cast<size_t>(height() * 1.0)};

    lv_style_set_pad_hor(&defaultKnobStyle, LV_STATE_DEFAULT, horPad);
    lv_style_set_pad_ver(&defaultKnobStyle, LV_STATE_DEFAULT, vertPad);

    lv_slider_set_range(_obj, minValue(), maxValue());

    setStyle(LV_SLIDER_PART_KNOB, &defaultKnobStyle);

    auto eventCb = [this](const lv_event_t event) {
        switch (event) {
            case LV_EVENT_VALUE_CHANGED:
            break;

            case LV_EVENT_RELEASED:
                if (lv_slider_get_value(this->_obj) >= (_unlockPercentThreshold/100.0 * maxValue())) {
                    if (_onUnlockCallback) {
                        _onUnlockCallback(this);
                    }
                }
                lv_slider_set_value(_obj, minValue(), 100);
            break;

            default:
                break;
        }
    };

    setEventCallBack(eventCb);
}

void LVGLUnlocker::onUnlock(unlockCallBack_t cb) {
    _onUnlockCallback = cb;
}
#include "LVGLSlider.hpp"

LVGLSlider::LVGLSlider(const int minValue, const int maxValue, const size_t step, LVGLBase* const parent) 
    :   AbstractRangeValueContainer(minValue, maxValue, step),
        LVGLBase(lv_slider_create(parent ? parent->innerData() : lv_disp_get_scr_act(NULL), NULL), parent) {

    init();
    applyTheme();
}

LVGLSlider::LVGLSlider(const int minValue, const int maxValue, const size_t step, lv_obj_t* const parent)
    :   AbstractRangeValueContainer(minValue, maxValue, step),
        LVGLBase(lv_slider_create(parent, NULL), parent) {

    init();
    applyTheme();
}

void LVGLSlider::init() {

    lv_slider_set_range(_obj, minValue(), maxValue());

    setValue(minValue());

    auto eventCb = [this](const lv_event_t event) {
        switch (event) {
            case LV_EVENT_RELEASED:
                moved(lv_slider_get_value(this->_obj));
            break;
            default:
                break;
        }
    };
    setEventCallBack(eventCb);
}

void LVGLSlider::moved(const int value) {
    setValue(value);
    if (_onMovedCb) {
        _onMovedCb(value);
    }
}

void LVGLSlider::changed() {
    if (lv_slider_get_value(this->_obj) != this->value()) {
        lv_slider_set_value(_obj, this->value(), 100);
    }
    if (_onChangedCb) {
        _onChangedCb(this);
    }
}

void LVGLSlider::onMoved(onMovedCallbackType cb) {
    _onMovedCb = cb;
}

void LVGLSlider::setValue(const int value) {
    AbstractRangeValueContainer<int>::setValue(value);
    changed();
}
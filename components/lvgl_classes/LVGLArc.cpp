#include "LVGLArc.hpp"

LVGLArc::LVGLArc(const bool adjustable, const size_t startAngle,
            const size_t endAngle, const std::string& label, LVGLBase* const parent)
    :   LVGLBase(lv_arc_create(parent? parent->innerData() : lv_disp_get_scr_act(NULL), NULL), parent),
        _valueString{label},
        _valueLbl {new LVGLLabel("value", this)},
        _isAdjustable {adjustable},
        _startAngle(startAngle),
        _endAngle(endAngle) {

    init();
    applyTheme();
}

LVGLArc::LVGLArc(const bool adjustable, const size_t startAngle,
            const size_t endAngle, const std::string& label, lv_obj_t* const parent) 
    :   LVGLBase(lv_arc_create(parent, NULL), parent),
        _valueString{label},
        _valueLbl {new LVGLLabel("value", this)},
        _isAdjustable {adjustable},
        _startAngle(startAngle),
        _endAngle(endAngle) {

    init();
    applyTheme();
}

static void arcAnimCb(void * ptr, lv_anim_value_t val)
{
    lv_obj_t* arc     = static_cast<lv_obj_t*>(ptr);
    lv_arc_set_value(arc, val);
}


void LVGLArc::init() {

    lv_obj_set_auto_realign(_valueLbl->innerData(), true);
    _valueLbl->align(this, LV_ALIGN_CENTER, 0, 0);
    setClickable(_isAdjustable);
    lv_arc_set_adjustable(_obj, _isAdjustable);
    lv_arc_set_bg_angles(_obj, _startAngle, _endAngle);
    lv_arc_set_angles(_obj, _startAngle, _startAngle);

    auto eventCb = [this](const lv_event_t event) {
        switch (event) {
            case LV_EVENT_RELEASED:
                this->setValue(lv_arc_get_value(this->_obj));
            break;
            default:
                break;
        }
    };
    setEventCallBack(eventCb);
}

void LVGLArc::redrawText() {
    _valueLbl->setText("%d %s", value(), _valueString.c_str());
}

void LVGLArc::setValue(const int value) {
    AbstractValueChangable<int>::setValue(value);
    changed();
}

void LVGLArc::changed() {
    redrawText();
    if (lv_arc_get_value(this->_obj) != this->value()) {
        
        static lv_anim_path_t path;
        lv_anim_path_init(&path);
        lv_anim_path_set_cb(&path, lv_anim_path_linear);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, _obj);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)arcAnimCb);
        lv_anim_set_time(&a, 500);
        lv_anim_set_path(&a, &path);
        lv_anim_set_repeat_count(&a, 1);
        lv_anim_set_values(&a, lv_arc_get_value(this->_obj), this->value());
        lv_anim_start(&a);
    }
    if (_onChangedCb) {
        _onChangedCb(this);
    }
}
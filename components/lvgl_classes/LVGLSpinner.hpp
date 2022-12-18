#pragma once
#include "LVGLBase.hpp"

class LVGLSpinner : public LVGLBase{

    public: 
        eLvglType type() const override {return eLvglType::LVGL_CLASS_SPINNER;}
        explicit LVGLSpinner(const uint16_t spinTime, LVGLBase* const parent)
            : LVGLBase(lv_spinner_create(parent? parent->innerData() : lv_disp_get_scr_act(NULL), NULL), parent) {
            setSpinTime(spinTime);
            applyTheme();
        }
        explicit LVGLSpinner(const uint16_t spinTime, lv_obj_t* const parent) 
            :    LVGLBase(lv_spinner_create(parent, NULL), parent) {
            setSpinTime(spinTime);
            applyTheme();
        }
    
        void setSpinTime(const uint16_t spinTime) {
            lv_spinner_set_spin_time(_obj, spinTime);
        }
};
#pragma once

#include "LVGLBase.hpp"
#include "LVGLContainer.hpp"

class LVGLColumn : public LVGLContainer {

    public:
        eLvglType type() const override {return eLvglType::LVGL_CLASS_COLUMN;}
        explicit LVGLColumn(LVGLBase* const parent = NULL);
        explicit LVGLColumn(lv_obj_t* const parent);
        explicit LVGLColumn(lv_obj_t* const src, LVGLBase* const parent);      
};
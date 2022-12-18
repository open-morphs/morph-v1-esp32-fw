#pragma once
#include "LVGLBase.hpp"

class LVGLImage : public LVGLBase {

    public:
        eLvglType type() const override {return eLvglType::LVGL_CLASS_IMAGE;}
        explicit LVGLImage(const lv_img_dsc_t* img, LVGLBase* const parent = NULL);
        void updateImage(const lv_img_dsc_t* img);
        // void zoomOut(const size_t percent);
};
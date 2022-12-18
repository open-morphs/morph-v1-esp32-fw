#pragma once
#include "LVGLBase.hpp"

class LVGLPage : public LVGLBase {

    public:       
        eLvglType type() const override {return eLvglType::LVGL_CLASS_PAGE;}
        explicit LVGLPage(LVGLBase* const parent = NULL);
        explicit LVGLPage(lv_obj_t* const parent);
        explicit LVGLPage(lv_obj_t* const src, LVGLBase* const parent);

        void setScrollBarMode(const lv_scrollbar_mode_t mode);
        void setScrollPropogation(const bool val);
        void setScrollableFit(const lv_fit_t fit);
        void setScrollableFit(const lv_fit_t, const lv_fit_t);
        void setScrollableFit(const lv_fit_t, const lv_fit_t, const lv_fit_t, const lv_fit_t);
        void scrollVertically(const lv_coord_t value) {
            lv_page_scroll_ver(_obj, value);
        }
        void scrollHoriazontally(const lv_coord_t value) {
            lv_page_scroll_hor(_obj, value);
        }
};
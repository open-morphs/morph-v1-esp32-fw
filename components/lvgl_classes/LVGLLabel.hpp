#pragma once

#include <string>

#include "LVGLBase.hpp"

class LVGLLabel : public LVGLBase{

    public:
        eLvglType type() const override {return eLvglType::LVGL_CLASS_LABEL;}
        explicit LVGLLabel(const std::string& txt, LVGLBase* const parent = NULL);
        explicit LVGLLabel(const std::string& txt, lv_obj_t* const parent);
        explicit LVGLLabel(lv_obj_t* const src, LVGLBase* const parent = NULL);
        
        void setRecolor(const bool value) {
            lv_label_set_recolor(_obj, value);
        }
        void setLabelLongMode(const lv_label_long_mode_t mode) {
            lv_label_set_long_mode(_obj, mode);
        }

        void setText(const std::string& txt) {
            lv_label_set_text(_obj, txt.c_str());
        }

        template<typename ... Args>
        void setText(const char * const frmt, Args ... args) {
            lv_label_set_text_fmt(_obj, frmt, args ...);

        }

        void setText(const int value) {
            lv_label_set_text_fmt(_obj, "%d", value);
        }

        std::string text() const {
            return lv_label_get_text(_obj);
        }

        void setAlignMode(const lv_label_align_t mode) {
            lv_label_set_align(_obj, mode);
        }
};
#include "LVGLLabel.hpp"


LVGLLabel::LVGLLabel(const std::string& txt, LVGLBase* const parent)
    :   LVGLBase(lv_label_create(parent? parent->innerData() : lv_disp_get_scr_act(NULL), NULL), parent) {
    
    lv_label_set_text(_obj, txt.c_str());
    applyTheme();
}

LVGLLabel::LVGLLabel(const std::string& txt, lv_obj_t* const parent)
    :   LVGLBase(lv_label_create(parent, NULL), parent) {

    lv_label_set_text(_obj, txt.c_str());
    applyTheme();
}

LVGLLabel::LVGLLabel(lv_obj_t* const src, LVGLBase* const parent)
    : LVGLBase(src, parent) {
}
#include "LVGLRow.hpp"

LVGLRow::LVGLRow(LVGLBase*  parent)
    :   LVGLContainer(parent) {

    setClickable(false);
    setFit(LV_FIT_TIGHT);
    setLayout(LV_LAYOUT_ROW_MID);
    applyTheme();
}

LVGLRow::LVGLRow(lv_obj_t* const parent)
    :   LVGLContainer(parent) {

    setClickable(false);
    setFit(LV_FIT_TIGHT);
    setLayout(LV_LAYOUT_ROW_MID);  
    applyTheme();
}

LVGLRow::LVGLRow(lv_obj_t* const src, LVGLBase* const parent)
    :   LVGLContainer(src, parent) {

    applyTheme();
}
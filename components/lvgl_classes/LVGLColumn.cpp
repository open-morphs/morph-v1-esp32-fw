#include "LVGLColumn.hpp"

LVGLColumn::LVGLColumn(LVGLBase*  parent)
    :   LVGLContainer(parent) {

    setClickable(false);
    setFit(LV_FIT_TIGHT);
    setLayout(LV_LAYOUT_COLUMN_MID);
    applyTheme();
}

LVGLColumn::LVGLColumn(lv_obj_t* const parent)
    :   LVGLContainer(parent){

    setClickable(false);
    setFit(LV_FIT_TIGHT);
    setLayout(LV_LAYOUT_COLUMN_MID);  
    applyTheme();
    lv_obj_set_auto_realign(_obj, false);
}

LVGLColumn::LVGLColumn(lv_obj_t* const src, LVGLBase* const parent)
    :   LVGLContainer(src, parent) {

}


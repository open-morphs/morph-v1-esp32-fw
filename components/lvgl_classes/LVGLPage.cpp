#include "LVGLPage.hpp"

LVGLPage::LVGLPage(LVGLBase* const parent)
    :   LVGLBase(lv_page_create(parent ? parent->innerData() : lv_disp_get_scr_act(NULL), NULL),parent) {

    applyTheme();
}

LVGLPage::LVGLPage(lv_obj_t* const parent)
    :   LVGLBase(lv_page_create(parent, NULL)) {

    lv_page_set_scrollbar_mode(_obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_auto_realign(_obj, true);
    lv_obj_align_origo(_obj, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_cont_set_fit(_obj, LV_FIT_TIGHT);
    applyTheme();
}

LVGLPage::LVGLPage(lv_obj_t* const src, LVGLBase* const parent)
    :   LVGLBase(src, parent) {
    applyTheme();
    // lv_obj_set_auto_realign(_obj, true);

    // lv_page_set_scrollbar_mode(_obj, LV_SCROLLBAR_MODE_OFF);
    // lv_obj_set_auto_realign(_obj, true);
    // lv_obj_align_origo(_obj, NULL, LV_ALIGN_CENTER, 0, 0);
    // lv_cont_set_fit(_obj, LV_FIT_TIGHT);
}

void LVGLPage::setScrollBarMode(const lv_scrollbar_mode_t mode) {
    lv_page_set_scrlbar_mode(_obj, mode);    
}

void LVGLPage::setScrollPropogation(const bool val) { 
    lv_page_set_scroll_propagation(_obj, val);
}

void LVGLPage::setScrollableFit(const lv_fit_t fit) {
    lv_page_set_scrollable_fit(_obj, fit);
}

void LVGLPage::setScrollableFit(const lv_fit_t fitHor, const lv_fit_t fitVer) {
    lv_page_set_scrollable_fit2(_obj, fitHor, fitVer);
}

void LVGLPage::setScrollableFit(const lv_fit_t fitLeft, const lv_fit_t fitRight, const lv_fit_t fitTop, const lv_fit_t fitBottom) {
    lv_page_set_scrollable_fit4(_obj, fitLeft, fitRight, fitTop, fitBottom);
}
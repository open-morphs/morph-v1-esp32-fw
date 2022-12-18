#include "LVGLContainer.hpp"

LVGLContainer::LVGLContainer(LVGLBase* const parent)
    :   LVGLBase(lv_cont_create(parent? parent->innerData() : lv_disp_get_scr_act(NULL), NULL)) {

}

LVGLContainer::LVGLContainer(lv_obj_t* const parent)
    :   LVGLBase(lv_cont_create(parent, NULL)) {
}

LVGLContainer::LVGLContainer(lv_obj_t* const src, LVGLBase* const parent)
    :   LVGLBase(src, parent) {
}

void LVGLContainer::addObject(LVGLBase* const obj) {
    obj->setParent(this);
}

void LVGLContainer::setLayout(const lv_layout_t layout) {
    lv_cont_set_layout(_obj, layout);
}

void LVGLContainer::setFit(const lv_fit_t fit) {
    lv_cont_set_fit(_obj, fit);
}

void LVGLContainer::setFit(const lv_fit_t hor, const lv_fit_t ver) {
    lv_cont_set_fit2(_obj, hor, ver);
}

void LVGLContainer::setFit(const lv_fit_t left, const lv_fit_t right, const lv_fit_t top, const lv_fit_t bottom) {
    lv_cont_set_fit4(_obj, left, right, top, bottom);
}
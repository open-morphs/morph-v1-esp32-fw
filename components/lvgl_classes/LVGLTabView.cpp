#include "LVGLTabView.hpp"

LVGLTabView::LVGLTabView(LVGLBase* const parent)
    :   LVGLBase(lv_tabview_create(parent? parent->innerData() : lv_disp_get_scr_act(NULL), NULL),
        parent) {

    init();
    applyTheme();
}

void LVGLTabView::init() {
    auto onTabChangedCb = [this](const lv_event_t event){
        if(LV_EVENT_VALUE_CHANGED != event)
            return;
        
        if (!_onTabChangedCb) {
            return;
        }

        lv_tabview_ext_t* ext = (lv_tabview_ext_t*) lv_obj_get_ext_attr(this->_obj);
        const uint16_t tabIndex {ext->tab_cur};

        _onTabChangedCb(tabIndex);
    };

    setEventCallBack(onTabChangedCb);
}

LVGLTabView::LVGLTabView(lv_obj_t* const parent)
    :   LVGLBase(lv_tabview_create(parent, NULL)) {
    init();
    applyTheme();
}

LVGLPage* LVGLTabView::addTab(const std::string& title) {
    return new LVGLPage(lv_tabview_add_tab(_obj, title.c_str()), this);
}

void LVGLTabView::setNavigationBarsPosition(const uint8_t position) {
    lv_tabview_set_btns_pos(_obj, position);
}

void LVGLTabView::setAnimationTime(const size_t ms) {
    lv_tabview_set_anim_time(_obj, ms);
}

void LVGLTabView::setIndicatorWidth(const size_t w) {
    lv_obj_set_style_local_size(innerData(), LV_TABVIEW_PART_INDIC, LV_STATE_DEFAULT, w);
}
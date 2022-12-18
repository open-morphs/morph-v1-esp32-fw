// #include "LVGLFactory.hpp"
// #include "lvgl_classes.h"

// AbstractTheme* LVGLFactory::_theme {nullptr};

// LVGLButton* LVGLFactory::createButton(const std::string& label, LVGLBase* parent) {

//     LVGLButton* btn = new LVGLButton(label, parent);
//     btn->resetStyle(LV_BTN_PART_MAIN);
//     btn->_lbl->resetStyle(LV_LABEL_PART_MAIN);

//     if (_theme) {
//         btn->setStyle(LV_BTN_PART_MAIN, _theme->commonBgStyle());
//         btn->setStyle(LV_BTN_PART_MAIN, _theme->commonBorderStyle());
//         btn->setStyle(LV_BTN_PART_MAIN, _theme->commonOutlineStyle());
//         btn->setStyle(LV_BTN_PART_MAIN, _theme->commonShapeStyle());
//     }

//     return btn;
// }

// LVGLSwitch* LVGLFactory::createSwitch(const size_t animTime, LVGLBase* parent) {

//     LVGLSwitch* sw = new LVGLSwitch(animTime, parent);
//     sw->resetStyle(LV_SWITCH_PART_BG);
//     sw->resetStyle(LV_SWITCH_PART_INDIC);
//     sw->resetStyle(LV_SWITCH_PART_KNOB);
    
//     if (_theme) {
//         sw->setStyle(LV_SWITCH_PART_BG, _theme->commonBgStyle());
//         sw->setStyle(LV_SWITCH_PART_BG, _theme->commonBorderStyle());
//         sw->setStyle(LV_SWITCH_PART_BG, _theme->commonOutlineStyle());
//         sw->setStyle(LV_SWITCH_PART_BG, _theme->commonShapeStyle());   
//         sw->setStyle(LV_SWITCH_PART_INDIC, _theme->indicatorStyle());     
//         sw->setStyle(LV_SWITCH_PART_KNOB, _theme->knobStyle());     
//     }

//     return sw;
// }

// LVGLTabView* LVGLFactory::createTabView(LVGLBase* parent) {

//     LVGLTabView* tv = new LVGLTabView(parent);

//     if (_theme) {
//         // tv->setStyle(LV_TABVIEW_PART_BG, _theme->commonBgStyle());
//         // tv->setStyle(LV_TABVIEW_PART_BG, _theme->commonBorderStyle());
//         tv->setStyle(LV_TABVIEW_PART_TAB_BG, _theme->commonBgStyle());
//         tv->setStyle(LV_TABVIEW_PART_TAB_BTN, _theme->tabButtonStyle());

//         // tv->setStyle(LV_TABVIEW_PART_TAB_BG, _theme->commonBgStyle());

//         // tv->setStyle(LV_SWITCH_PART_BG, _theme->commonBorderStyle());
//         // tv->setStyle(LV_SWITCH_PART_BG, _theme->commonOutlineStyle());
//         // tv->setStyle(LV_SWITCH_PART_BG, _theme->commonShapeStyle());   
//         // tv->setStyle(LV_SWITCH_PART_INDIC, _theme->indicatorStyle());     
//         // tv->setStyle(LV_SWITCH_PART_KNOB, _theme->knobStyle());     
//     }
//     return  tv;    
// }

// void LVGLFactory::setTheme(AbstractTheme* const theme) {
//     _theme = theme;
// }
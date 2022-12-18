#pragma once

#include "lvgl.h"
#include "LVGLStyle.hpp"
#include <vector>

class AbstractTheme {
    
    public: 
        virtual ~AbstractTheme() = default;

        LVGLStyle& commonBgStyle() {
            return _bgStyle;
        }
        LVGLStyle& commonBorderStyle() {
            return _borderStyle;
        }
        LVGLStyle& commonOutlineStyle() {
            return _outlineStyle;
        }
        LVGLStyle& commonShapeStyle() {
            return _rectShapeStyle;
        }
        LVGLStyle& indicatorStyle() {
            return _indicatorStyle;
        }
        LVGLStyle& knobStyle() {
            return _knobStyle;
        }
        LVGLStyle& tabButtonStyle() {
            return _tabBtnsStyle;
        }
        LVGLStyle& commonTextStyle() {
            return _textStyle;
        }
        LVGLStyle& containerPadStyle() {
            return _containerPaddingStyle;
        }
        LVGLStyle& unlockerKnobStyle() {
            return _unlockerKnobStyle;
        }
        void init();
    private:
        virtual void fillBgStyle(LVGLStyle&) {};
        virtual void fillBorderStyles(LVGLStyle&){};
        virtual void fillOutlineStyles(LVGLStyle&){};
        virtual void fillShapeStyles(LVGLStyle&){};
        virtual void fillIndicatorBgStyles(LVGLStyle&){};
        virtual void fillKnobBgStyles(LVGLStyle&){};
        virtual void fillTabButtonStyle(LVGLStyle&){};
        virtual void fillTextStyle(LVGLStyle&){};
        virtual void fillContainerPadStyle(LVGLStyle&){};
        virtual void fillUnlockerKnobStyle(LVGLStyle&){};


        LVGLStyle _bgStyle;
        LVGLStyle _borderStyle;
        LVGLStyle _outlineStyle;
        LVGLStyle _rectShapeStyle;        
        LVGLStyle _textStyle;
        LVGLStyle _containerPaddingStyle;

        LVGLStyle _indicatorStyle;
        LVGLStyle _knobStyle;
        LVGLStyle _tabBtnsStyle;
        LVGLStyle _unlockerKnobStyle;

};

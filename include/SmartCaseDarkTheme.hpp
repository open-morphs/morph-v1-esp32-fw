#pragma once

#include "../components/lvgl_classes/AbstractTheme.hpp"

class SmartCaseDarkTheme : public AbstractTheme {
    public:
        SmartCaseDarkTheme() = default;
    private:

        void fillBgStyle(LVGLStyle&) override;
        void fillBorderStyles(LVGLStyle&) override;
        void fillOutlineStyles(LVGLStyle&) override;
        void fillShapeStyles(LVGLStyle&) override;
        void fillIndicatorBgStyles(LVGLStyle&) override;
        void fillKnobBgStyles(LVGLStyle&) override;
        void fillTabButtonStyle(LVGLStyle&) override;
        void fillTextStyle(LVGLStyle&)override;
        void fillContainerPadStyle(LVGLStyle&) override;
        void fillUnlockerKnobStyle(LVGLStyle&) override;
};

#include <SmartCaseDarkTheme.hpp>
#include <GUIConfig.hpp>


void SmartCaseDarkTheme::fillBgStyle(LVGLStyle& style) {

    BackgroundStyle_t bgStyles[]{
        {
            LV_STATE_DEFAULT,
            GUI_BG_COLOR,
            GUI_BG_COLOR,
            LV_OPA_COVER
        }
    };

    LineStyle_t indLineStyles [] {
        {
            LV_STATE_DEFAULT,
            20,
            GUI_SECONDARY_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_OPA_COVER,
            true
        }
    };
 
    for (auto bgStyle : bgStyles) {
        style.addBackgroundStyle(bgStyle);
    }
    for (auto indLineStyle : indLineStyles) {
        style.addLineStyle(indLineStyle);
    }
}

void SmartCaseDarkTheme::fillContainerPadStyle(LVGLStyle& style) {

    PaddingStyle_t commonPaddingStyles[] {
        {
            LV_STATE_DEFAULT,
            5,
            5,
            5,
            5,
            15
        },
    };
    for (auto commonPaddingStyle : commonPaddingStyles) {
        style.addPaddingStyle(commonPaddingStyle);
    }
}


void SmartCaseDarkTheme::fillBorderStyles(LVGLStyle& style) {

    BorderStyle_t borderStyles[] {
        {
            LV_STATE_DEFAULT,
            2,
            GUI_MAIN_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_OPA_COVER
        },
        {
            LV_STATE_PRESSED,
            2,
            GUI_HIGHLIGTH_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_OPA_COVER
        },
        {
            LV_STATE_DISABLED,
            2,
            GUI_DISABLED_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_OPA_COVER
        }
    };
    for (auto borderStyle : borderStyles) {
        style.addBorderStyle(borderStyle);
    }
}

void SmartCaseDarkTheme::fillOutlineStyles(LVGLStyle& style) {

    OutlineStyle_t outlineStyles[] {
        {
            LV_STATE_DEFAULT,
            10,
            GUI_HIGHLIGTH_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_OPA_0,
            0
        },
        {
            LV_STATE_PRESSED,
            2,
            GUI_HIGHLIGTH_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_OPA_50,
            0
        },
        {
            LV_STATE_FOCUSED,
            2,
            GUI_HIGHLIGTH_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_OPA_0,
            0
        }
    };
    for (auto outlineStyle : outlineStyles) {
        style.addOutlineStyle(outlineStyle);
    }
}

void SmartCaseDarkTheme::fillShapeStyles(LVGLStyle& style) {
    RectShapeStyle_t shapeStyles[] {
        {
            LV_STATE_DEFAULT,
            LV_RADIUS_CIRCLE
        }
    };

    for (auto shapeStyle : shapeStyles) {
        style.addShapeStyle(shapeStyle);
    }
}

void SmartCaseDarkTheme::fillIndicatorBgStyles(LVGLStyle& style) {
    BackgroundStyle_t indBgStyles[]{
        {
            LV_STATE_DEFAULT,
            GUI_HIGHLIGTH_COLOR,
            GUI_BG_COLOR,
            LV_OPA_COVER
        },
        {
            LV_STATE_DISABLED,
            GUI_DISABLED_COLOR,
            GUI_BG_COLOR,
            LV_OPA_COVER
        }
    };

    RectShapeStyle_t indShapeStyles[] {
        {
            LV_STATE_DEFAULT,
            LV_RADIUS_CIRCLE
        },
    };

    LineStyle_t indLineStyles [] {
        {
            LV_STATE_DEFAULT,
            20,
            GUI_HIGHLIGTH_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_OPA_COVER,
            true
        },
        {
            LV_STATE_DISABLED,
            20,
            GUI_DISABLED_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_OPA_COVER,
            true
        }
    };

    for (auto indBgStyle : indBgStyles) {
        style.addBackgroundStyle(indBgStyle);
    }
    for (auto indShapeStyle : indShapeStyles) {
        style.addShapeStyle(indShapeStyle);
    }
    for (auto indLineStyle : indLineStyles) {
        style.addLineStyle(indLineStyle);
    }
}

void SmartCaseDarkTheme::fillKnobBgStyles(LVGLStyle& style) {

    BackgroundStyle_t knobBgStyles[] {
        {
            LV_STATE_DEFAULT,
            GUI_MAIN_COLOR,
            GUI_BG_COLOR,
            LV_OPA_COVER
        },
        {
            LV_STATE_DISABLED,
            GUI_SECONDARY_COLOR,
            GUI_BG_COLOR,
            LV_OPA_COVER
        }
    };

    PaddingStyle_t knobPaddingStyles[] {
        {
            LV_STATE_DEFAULT,
            -5,
            -5,
            -5,
            -5
        },
    };

    RectShapeStyle_t knobShapeStyles[] {
        {
            LV_STATE_DEFAULT,
            LV_RADIUS_CIRCLE
        },
    };

    for (auto knobBgStyle : knobBgStyles) {
        style.addBackgroundStyle(knobBgStyle);
    }
    for (auto knobPaddingStyle : knobPaddingStyles) {
        style.addPaddingStyle(knobPaddingStyle);
    }
    for (auto knobShapeStyle : knobShapeStyles) {
        style.addShapeStyle(knobShapeStyle);
    }
}

void SmartCaseDarkTheme::fillTabButtonStyle(LVGLStyle& style) {

    PaddingStyle_t tabBtnPaddingStyles[] {
        {
            LV_STATE_DEFAULT,
            25,
            25,
            25,
            25
        },
    };

    TextStyle_t tabBtnTextStyles[] {
        {
            LV_STATE_DEFAULT,
            GUI_SECONDARY_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_TEXT_DECOR_NONE,
            &lv_font_montserrat_14,
            0,
            0,
            LV_OPA_COVER      
        },
        {
            LV_STATE_CHECKED,
            GUI_MAIN_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_TEXT_DECOR_NONE,
            &lv_font_montserrat_22,
            0,
            0,
            LV_OPA_COVER      
        },
    };

    BackgroundStyle_t tabBtnBgStyles[] {
        {
            LV_STATE_DEFAULT,
            GUI_BG_COLOR,
            GUI_BG_COLOR,
            LV_OPA_COVER
        },
        {
            LV_STATE_PRESSED,
            GUI_SECONDARY_COLOR,
            GUI_BG_COLOR,
            LV_OPA_50
        },
    };

    for (auto tabBtnPaddingStyle : tabBtnPaddingStyles) {
        style.addPaddingStyle(tabBtnPaddingStyle);
    }
    for (auto tabBtnBgStyle : tabBtnBgStyles) {
        style.addBackgroundStyle(tabBtnBgStyle);
    }
    for (auto tabBtnTextStyle : tabBtnTextStyles) {
        style.addTextStyle(tabBtnTextStyle);
    }
}

void SmartCaseDarkTheme::fillTextStyle(LVGLStyle& style) {
    TextStyle_t textStyles[] {
        {
            LV_STATE_DEFAULT,
            GUI_MAIN_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_TEXT_DECOR_NONE,
            &lv_font_montserrat_14,
            0,
            0,
            LV_OPA_COVER      
        },
        {
            LV_STATE_DISABLED,
            GUI_DISABLED_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_TEXT_DECOR_NONE,
            &lv_font_montserrat_14,
            0,
            0,
            LV_OPA_COVER      
        }
    };
    for (auto textStyle : textStyles) {
        style.addTextStyle(textStyle);
    }
}

void SmartCaseDarkTheme::fillUnlockerKnobStyle(LVGLStyle& style) {

    RectShapeStyle_t shapeStyles[] {
        {
            LV_STATE_DEFAULT,
            5
        }
    };

    BackgroundStyle_t knobBgStyles[] {
        {
            LV_STATE_DEFAULT,
            GUI_MAIN_COLOR,
            GUI_BG_COLOR,
            LV_OPA_COVER
        },
        {
            LV_STATE_DISABLED,
            GUI_SECONDARY_COLOR,
            GUI_BG_COLOR,
            LV_OPA_COVER
        }
    };

     BorderStyle_t borderStyles[] {
        {
            LV_STATE_DEFAULT,
            2,
            GUI_SECONDARY_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_OPA_COVER
        },
        {
            LV_STATE_PRESSED,
            2,
            GUI_HIGHLIGTH_COLOR,
            LV_BLEND_MODE_NORMAL,
            LV_OPA_COVER
        },
    };

    for (auto knobBgStyle : knobBgStyles) {
        style.addBackgroundStyle(knobBgStyle);
    }

    for (auto shapeStyle : shapeStyles) {
        style.addShapeStyle(shapeStyle);
    }

    for (auto borderStyle : borderStyles) {
        style.addBorderStyle(borderStyle);
    }
}

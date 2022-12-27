#pragma once

#include "lvgl.h"

typedef struct {
    lv_state_t     state;
    lv_style_int_t  radius;
}RectShapeStyle_t;

typedef struct {
    lv_state_t      state;
    lv_style_int_t  lineWidth;
    lv_color_t      lineColor;
    lv_blend_mode_t lineBlendMode;
    lv_opa_t        lineOpacityMode;
    bool            lineRounded;
}LineStyle_t;
typedef struct {
    lv_state_t     state;
    lv_style_int_t topPadding;
    lv_style_int_t bottomPadding;
    lv_style_int_t leftPadding;
    lv_style_int_t rightPadding;
    lv_style_int_t innerPadding;
} PaddingStyle_t;

typedef struct {
    lv_state_t     state;
    lv_style_int_t topMargin;
    lv_style_int_t bottomMargin;
    lv_style_int_t leftMargin;
    lv_style_int_t rightMargin;
} MarginStyle_t;

typedef struct {
    lv_state_t state;
    lv_color_t bgColor;
    lv_color_t bgGradientColor;
    lv_opa_t   bgOpacityMode;
} BackgroundStyle_t;

typedef struct {
    lv_state_t      state;
    lv_style_int_t  borderWidth;
    lv_color_t      borderColor;
    lv_blend_mode_t borderBlendMode;
    lv_opa_t        borderOpacityMode;
} BorderStyle_t;

typedef struct {
    lv_state_t      state;
    lv_style_int_t  outlineWidth;
    lv_color_t      outlineColor;
    lv_blend_mode_t outlineBlendMode;
    lv_opa_t        outlineOpacityMode;
    lv_style_int_t  outlinePadding;
} OutlineStyle_t;

typedef struct {
    lv_state_t      state;
    lv_color_t      textColor;
    lv_blend_mode_t textBlendMode;
    lv_text_decor_t textDecorMode;
    lv_font_t*      textFont;
    lv_style_int_t  textLetterSpacing;
    lv_style_int_t  textLineSpacing;
    lv_opa_t        textOpacityMode;
} TextStyle_t;


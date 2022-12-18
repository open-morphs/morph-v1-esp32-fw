#include "LVGLStyle.hpp"

LVGLStyle::LVGLStyle() {

    lv_style_init(&_style);
}

void LVGLStyle::addShapeStyle(const RectShapeStyle_t& style) {
    lv_style_set_radius(&_style, style.state, style.radius);
}

void LVGLStyle::addBackgroundStyle(const BackgroundStyle_t& style){
    lv_style_set_bg_color(&_style, style.state, style.bgColor);
    lv_style_set_bg_grad_color(&_style, style.state, style.bgGradientColor);
    lv_style_set_bg_opa(&_style, style.state, style.bgOpacityMode);
}

void LVGLStyle::addMarginStyle(const MarginStyle_t& style){
    lv_style_set_margin_bottom(&_style, style.state, style.bottomMargin);
    lv_style_set_margin_left(&_style, style.state, style.leftMargin);
    lv_style_set_margin_right(&_style, style.state, style.rightMargin);
    lv_style_set_margin_top(&_style, style.state, style.topMargin);
}

void LVGLStyle::addPaddingStyle(const PaddingStyle_t& style) {
    lv_style_set_pad_bottom(&_style, style.state, style.bottomPadding);
    lv_style_set_pad_left(&_style, style.state, style.leftPadding);
    lv_style_set_pad_right(&_style, style.state, style.rightPadding);
    lv_style_set_pad_top(&_style, style.state, style.topPadding);
    lv_style_set_pad_inner(&_style, style.state, style.innerPadding);
}

void LVGLStyle::addLineStyle(const LineStyle_t& style) {

    lv_style_set_line_blend_mode(&_style, style.state, style.lineBlendMode);
    lv_style_set_line_color(&_style, style.state, style.lineColor);
    lv_style_set_line_opa(&_style, style.state, style.lineOpacityMode);
    lv_style_set_line_width(&_style, style.state, style.lineWidth);
    lv_style_set_line_rounded(&_style, style.state, style.lineRounded);
}

void LVGLStyle::addBorderStyle(const BorderStyle_t& style) {
    lv_style_set_border_blend_mode(&_style, style.state, style.borderBlendMode);
    lv_style_set_border_color(&_style, style.state, style.borderColor);
    lv_style_set_border_opa(&_style, style.state, style.borderOpacityMode);
    lv_style_set_border_width(&_style, style.state, style.borderWidth);
}

void LVGLStyle::addOutlineStyle(const OutlineStyle_t& style) {
    lv_style_set_outline_blend_mode(&_style, style.state, style.outlineBlendMode);
    lv_style_set_outline_color(&_style, style.state, style.outlineColor);
    lv_style_set_outline_opa(&_style, style.state, style.outlineOpacityMode);
    lv_style_set_outline_pad(&_style, style.state, style.outlinePadding);
    lv_style_set_outline_width(&_style, style.state, style.outlineWidth);
} 

void LVGLStyle::addTextStyle(const TextStyle_t& style) {

    lv_style_set_text_blend_mode(&_style, style.state, style.textBlendMode);
    lv_style_set_text_color(&_style, style.state, style.textColor);
    lv_style_set_text_decor(&_style, style.state, style.textDecorMode);
    lv_style_set_text_font(&_style, style.state, style.textFont);
    lv_style_set_text_letter_space(&_style, style.state, style.textLetterSpacing);
    lv_style_set_text_line_space(&_style, style.state, style.textLineSpacing);
    lv_style_set_text_opa(&_style, style.state, style.textOpacityMode);
}

lv_style_t* LVGLStyle::style() {
    return &_style;
}